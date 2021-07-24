#pragma once
#include "Camera.h"
#include "GraphicsDevice.h"
#include "GraphicsViewport.h"
#include "GraphicsTextureCollection.h"
#include "GraphicsSurface.h"
#include "SuperMeshInstance.h"
#include "AABB.h"
//#include <d3d11.h>

class DirectionalShadowMap
{
public:
    DirectionalShadowMap(GraphicsDevice& device)
    {
        float scale = 2.0f;
        m_Camera.SetProjectionAsOrtho(-1000 * scale, 1000 * scale, -1000 * scale, 1000 * scale, 10.0, 5500.0);

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

    void OnFrameBegin()
    {
        float scale = 2.0f;
        m_Camera.SetProjectionAsOrtho(-1000 * scale, 1000 * scale, -1000 * scale, 1000 * scale, 10.0, 5500.0);
    }

    Texture2D* GetShadowMapTexture()
    {
        return m_DepthTarget.GetTexture();
    }

    void Render(GraphicsDevice& device, Camera& mainCamera, const AABB& sceneAABB, const glm::vec3& lightPosition, const glm::vec3& lightDirection)
    {
        glm::mat4x4 cropMatrix = CalcCropMatrix(mainCamera, sceneAABB);

        m_Camera.SetPosition(lightPosition);
        m_Camera.SetOrientation(lightDirection);

        m_Camera.AlterProjectionMatrix(cropMatrix);

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
    glm::mat4x4 CalcCropMatrix(Camera& mainCamera, const AABB& sceneAABB)
    {
        AABB targetVolume = mainCamera.CalcFrustumAABB();
        targetVolume.ShrinkBy(sceneAABB);

        AABB targetVolumeProjSpace;
        for (size_t i = 0; i < 8; i++)
        {
            glm::vec4 p = m_Camera.GetViewProjectionMatrix() * glm::vec4(targetVolume.GetPoint(i), 1.0f);
            p = p / p.w;

            targetVolumeProjSpace.Extend(glm::vec3(p.x, p.y, p.z));
        }

        float sX = 2.0f / (targetVolumeProjSpace.m_Max.x - targetVolumeProjSpace.m_Min.x);
        float sY = 2.0f / (targetVolumeProjSpace.m_Max.y - targetVolumeProjSpace.m_Min.y);

        float oX = -0.5 * (targetVolumeProjSpace.m_Max.x + targetVolumeProjSpace.m_Min.x) * sX;
        float oY = -0.5 * (targetVolumeProjSpace.m_Max.y + targetVolumeProjSpace.m_Min.y) * sY;

        glm::mat4x4 cropMatrix;
        cropMatrix[0] = glm::vec4(sX, 0.0, 0.0, 0.0);
        cropMatrix[1] = glm::vec4(0.0f, sY, 0.0, 0.0);
        cropMatrix[2] = glm::vec4(0.0f, 0.0, 1.0, 0.0);
        cropMatrix[3] = glm::vec4(oX, oY, 0.0, 1.0);

        return cropMatrix;
    }

    ID3D11RasterizerState* m_RastState;
    ID3D11DepthStencilState* m_DepthState;
    ID3D11BlendState* m_BlendState;
    
    DepthSurface m_DepthTarget;

    Camera m_Camera;

    std::vector<SuperMeshInstance*> m_Meshes;
};