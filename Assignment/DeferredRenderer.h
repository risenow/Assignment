#pragma once
#include "GraphicsDevice.h"
#include "GraphicsSurface.h"
#include "SuperMeshInstance.h"
#include "BasicPixelShaderStorage.h"
#include "GraphicsConstantsBuffer.h"
#include "DeferredLightingConsts.h"
#include "DirectionalShadowMap.h"
#include <vector>

//uses backbuffer as color buffer for memory economy
//generally not a good idea
//if we could use pixel shader for lighting then we could create sep color buffer and then light it directly to swapchain
//but since we need to use compute shaders and directx prohibits UAV for backbuffer it is not a possibility
class DeferredRenderer
{
public:
    DeferredRenderer(GraphicsDevice& device, ColorSurface backBuffer) 
    {
        //m_LightingShader = GraphicsShader::FromFile(device, GraphicsShaderType_Compute, L"Data/shaders/lightingcs.hlsl");

        DeferredLightingConsts consts;
        m_ConstantsBuffer = GraphicsConstantsBuffer<DeferredLightingConsts>(device, consts);

        InitGBuffer(device, backBuffer);

        D3D11_SAMPLER_DESC shadowSamplerDesc;
        shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        shadowSamplerDesc.MipLODBias = 0.0f;
        shadowSamplerDesc.MaxAnisotropy = 1;
        shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
        shadowSamplerDesc.BorderColor[0] = 0;
        shadowSamplerDesc.BorderColor[1] = 0;
        shadowSamplerDesc.BorderColor[2] = 0;
        shadowSamplerDesc.BorderColor[3] = 0;
        shadowSamplerDesc.MinLOD = 0;
        shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        device.GetD3D11Device()->CreateSamplerState(&shadowSamplerDesc, &m_ShadowSampler);
    }

    void Consume(const std::vector<SuperMeshInstance*> meshInsts)
    {
        m_Meshes = meshInsts;
    }
    void Validate(GraphicsDevice& device, ColorSurface backBuffer)
    {
        m_ColorSurface = backBuffer;

        size_t newWidth = m_ColorSurface.GetWidth();
        size_t newHeight = m_ColorSurface.GetHeight();

        if (newWidth != m_DepthSurface.GetWidth() || newHeight != m_DepthSurface.GetHeight())
        {
            m_DepthSurface.Resize(device, newWidth, newHeight, MULTISAMPLE_TYPE_NONE);
            m_NormalsSurface.Resize(device, newWidth, newHeight, MULTISAMPLE_TYPE_NONE);
            m_LightBuffer.Resize(device, newWidth, newHeight, MULTISAMPLE_TYPE_NONE);
        }
    }

    void Render(GraphicsDevice& device, Camera& camera, DirectionalShadowMap& shadowMap, const glm::vec3& lightDir)
    {
        Camera& shadowMapCamera = shadowMap.GetCamera();

        ClearRenderTarget(device, m_ColorSurface);
        ClearRenderTarget(device, m_NormalsSurface);
        ClearDepthTarget(device, m_DepthSurface);

        BindRenderTargetsDepthTarget(device, { m_ColorSurface, m_NormalsSurface }, m_DepthSurface);

        for (SuperMeshInstance* mesh : m_Meshes)
            mesh->Render(device, camera, false, BasicPixelShaderStorage::GBUFFER);

        UnbindRenderTargetsDepthTarget(device, 2);

        m_LightingShader = DeferredLightingShaderStorage::GetInstance().GetShader(DeferredLightingShaderStorage::GAMMA_CORRECTION);

        m_LightingShader.Bind(device);
        
        DeferredLightingConsts consts;
        consts.lightDir = glm::vec4(glm::mat3x3(camera.GetViewMatrix()) * glm::vec3(0.0, -1.0f, 0.0f), 1.0f);
        consts.proj = camera.GetProjectionMatrix();
        consts.unproj = glm::inverse(consts.proj);
        consts.unview = glm::inverse(camera.GetViewMatrix());
        consts.shadowview = shadowMapCamera.GetViewMatrix();
        consts.shadowproj = shadowMapCamera.GetProjectionMatrix();

        m_ConstantsBuffer.Update(device, consts);

        m_ConstantsBuffer.Bind(device, GraphicsShaderMask_Compute);

        ID3D11ShaderResourceView* srvs[] = { m_DepthSurface.GetTexture()->GetSRV(), m_NormalsSurface.GetTexture()->GetSRV(),  m_ColorSurface.GetTexture()->GetSRV(), shadowMap.GetShadowMapTexture()->GetSRV() };
        ID3D11UnorderedAccessView* uavs[] = { m_LightBuffer.GetUAV() };
        ID3D11SamplerState* samplers[] = { m_ShadowSampler};

        device.GetD3D11DeviceContext()->CSSetShaderResources(0, 4, srvs);
        device.GetD3D11DeviceContext()->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);
        device.GetD3D11DeviceContext()->CSSetSamplers(0, 1, samplers);

        device.GetD3D11DeviceContext()->Dispatch(ceilf((float)m_ColorSurface.GetWidth() / 8.0f), ceilf((float)m_ColorSurface.GetHeight() / 8.0f), 1);

        ZeroMemory(srvs, sizeof(srvs));
        ZeroMemory(uavs, sizeof(uavs));
        device.GetD3D11DeviceContext()->CSSetShaderResources(0, 4, srvs);
        device.GetD3D11DeviceContext()->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);
    }

    void FlushTo(GraphicsDevice& device, Texture2D target)
    {
        device.GetD3D11DeviceContext()->CopyResource(target.GetD3D11Texture2D(), m_LightBuffer.GetD3D11Texture2D());
    }

    DepthSurface GetDepthTarget()
    {
        return m_DepthSurface;
    }
private:
    void InitGBuffer(GraphicsDevice& device, ColorSurface backBuffer)
    {
        size_t width = backBuffer.GetWidth();
        size_t height = backBuffer.GetHeight();
        m_ColorSurface = backBuffer;
        m_DepthSurface = DepthSurface(device, width, height, 1, 1, DXGI_FORMAT_R24G8_TYPELESS, GetSampleDesc(device, DXGI_FORMAT_D24_UNORM_S8_UINT, MultisampleType::MULTISAMPLE_TYPE_NONE), D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, 0, 0);
        m_NormalsSurface = ColorSurface(device, width, height, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, GetSampleDesc(device, DXGI_FORMAT_R8G8B8A8_UNORM, MultisampleType::MULTISAMPLE_TYPE_NONE), D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0, 0);
        m_LightBuffer = Texture2D(device, width, height, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, GetSampleDesc(device, DXGI_FORMAT_R8G8B8A8_UNORM, MultisampleType::MULTISAMPLE_TYPE_NONE), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, 0, 0);
    }

    GraphicsShader m_LightingShader;

    GraphicsConstantsBuffer<DeferredLightingConsts> m_ConstantsBuffer;

    ColorSurface m_ColorSurface;
    ColorSurface m_NormalsSurface;
    DepthSurface m_DepthSurface;

    Texture2D m_LightBuffer;

    ID3D11SamplerState* m_ShadowSampler;
    ID3D11SamplerState* m_PointSampler;

    std::vector<SuperMeshInstance*> m_Meshes;
};