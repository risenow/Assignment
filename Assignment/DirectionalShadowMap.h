#pragma once
#include "Camera.h"
#include "GraphicsDevice.h"
#include "GraphicsViewport.h"
#include "GraphicsTextureCollection.h"
#include "GraphicsSurface.h"
#include "SuperMeshInstance.h"
//#include <d3d11.h>

class DirectionalShadowMap
{
public:
    DirectionalShadowMap(GraphicsDevice& device)
    {
        m_Camera.SetProjectionAsOrtho(-500, 500, -500, 500, 10.0, 5500.0);

        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
        depthStencilDesc.DepthEnable = TRUE;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
        depthStencilDesc.StencilEnable = FALSE;
        depthStencilDesc.StencilReadMask = 0xFF;
        depthStencilDesc.StencilWriteMask = 0xFF;

        depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS; 

        D3D11_BLEND_DESC blendStateDesc;
        blendStateDesc.AlphaToCoverageEnable = false;
        blendStateDesc.IndependentBlendEnable = false;
        blendStateDesc.RenderTarget[0].BlendEnable = false;
        blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_COLOR;
        blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_COLOR;
        blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0;
        blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
        blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_COLOR;

        D3D11_RASTERIZER_DESC rastStateDesc;
        rastStateDesc.AntialiasedLineEnable = true;
        rastStateDesc.CullMode = D3D11_CULL_NONE;
        rastStateDesc.FillMode = D3D11_FILL_SOLID;
        rastStateDesc.FrontCounterClockwise = TRUE;
        rastStateDesc.DepthBias = 0;
        rastStateDesc.SlopeScaledDepthBias = 0.0f;
        rastStateDesc.DepthBiasClamp = 0.0f;
        rastStateDesc.DepthClipEnable = FALSE;
        rastStateDesc.ScissorEnable = FALSE;
        rastStateDesc.MultisampleEnable = FALSE;

        device.GetD3D11Device()->CreateRasterizerState(&rastStateDesc, &m_RastState);
        device.GetD3D11Device()->CreateBlendState(&blendStateDesc, &m_BlendState);
        device.GetD3D11Device()->CreateDepthStencilState(&depthStencilDesc, &m_DepthState);

        const size_t WIDTH = 2048;
        const size_t HEIGHT = 2048;

        m_DepthTarget = DepthSurface(device, WIDTH, HEIGHT, 1, 1, DXGI_FORMAT_R24G8_TYPELESS, GetSampleDesc(device, DXGI_FORMAT_D24_UNORM_S8_UINT, MultisampleType::MULTISAMPLE_TYPE_NONE), D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, 0, 0);
    }

    Texture2D* GetShadowMapTexture()
    {
        return nullptr;//return &m_ShadowMapRenderSet.GetDepthTexture();
    }

    void Render(GraphicsDevice& device,  const glm::vec3& lightPosition, const glm::vec3& lightDirection)
    {
        m_Camera.SetPosition(lightPosition);
        m_Camera.SetOrientation(lightDirection);

        ClearDepthTarget(device, m_DepthTarget);

        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        device.GetD3D11DeviceContext()->RSSetState(m_RastState);
        device.GetD3D11DeviceContext()->OMSetBlendState(m_BlendState, clearColor, 0xffffffff);
        device.GetD3D11DeviceContext()->OMSetDepthStencilState(m_DepthState, 0);

        BindRenderTargetsDepthTarget(device, {}, m_DepthTarget);

        GraphicsViewport viewport = GraphicsViewport(m_DepthTarget);
        viewport.Bind(device);

        for (SuperMeshInstance* mesh : m_Meshes)
            mesh->Render(device, m_Camera, true);

        device.GetD3D11DeviceContext()->ClearState();
        //UnbindRenderTargetsDepthTarget(device, 0);
    }

    Camera& GetCamera()
    {
        return m_Camera;
    }

    void Consume(const std::vector<SuperMeshInstance*>& insts)
    {
        m_Meshes = insts;
    }
private:
    ID3D11RasterizerState* m_RastState;
    ID3D11DepthStencilState* m_DepthState;
    ID3D11BlendState* m_BlendState;
    
    DepthSurface m_DepthTarget;

    Camera m_Camera;

    std::vector<SuperMeshInstance*> m_Meshes;
};