#include <iostream>
#include <random>
#include "SuperMeshInstance.h"
#include "GraphicsDevice.h"
#include "Window.h"
#include "GraphicsSwapchain.h"
#include "GraphicsShader.h"
#include "dxlogicsafety.h"
#include "GraphicsBuffer.h"
#include "GraphicsConstantsBuffer.h"
#include "GraphicsInputLayout.h"
#include "GraphicsViewport.h"
#include "GraphicsTextureCollection.h"
#include "MouseKeyboardCameraController.h"
#include "Camera.h"
#include "SuperViewport.h"
#include "DisplayAdaptersList.h"
#include "MicrosecondsTimer.h"
#include "GraphicsMarker.h"
#include "DemoScene1Generate.h"
#include "randomutils.h"
#include "basicvsconstants.h"

Camera CreateInitialCamera(float aspect)
{
    const glm::vec3 position = glm::vec3(0.0f, .0f, 0.0f);
    const glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    Camera camera = Camera(position, rotation);
    camera.SetProjection(75.5f, aspect, 0.1f, 100000.0f);//(90.5f, aspect, 0.1f, 100000.0f);

    return camera;
}

static const float Camera2FixedY = 4000.0f;

Camera CreateInitialCamera2(float aspect)
{
    const glm::vec3 position = glm::vec3(0.0f, Camera2FixedY, -550.0f);
    const glm::vec3 rotation = glm::vec3(-3.14f / 2.0f, 0.0f, 0.0f);
    Camera camera = Camera(position, rotation);
    camera.SetProjection(45.0f, aspect, 0.1f, 100000.0f);

    return camera;
}

glm::mat4x4 CreatePawnTranform(const Camera& camera)
{
    glm::vec3 cameraRot = camera.GetRotation();
    glm::mat4x4 pawnTransform = glm::rotate(glm::scale(glm::identity<glm::mat4x4>(), glm::vec3(100.0f, 100.0f, 100.0f)), cameraRot.y, glm::vec3(0.0, 1.0, 0.0));
    pawnTransform[3] = glm::vec4(camera.GetPosition(), 1.0f);

    return pawnTransform;
}

#define MULTISAMPLE_TYPE MultisampleType::MULTISAMPLE_TYPE_NONE

int main()
{
    srand(time(NULL));

    DisplayAdaptersList displayAdapters;

    GraphicsDevice device(D3D11DeviceCreationFlags(true, true), FEATURE_LEVEL_ONLY_D3D11, nullptr);

    static const std::string WindowTitle = "Zibra Assignment";
    Window window(WindowTitle, 1, 1, 1024, 768);
    GraphicsSwapChain swapchain(device, window, MULTISAMPLE_TYPE);
    GraphicsTextureCollection textureCollection(device);

    D3D11_RASTERIZER_DESC rastState;
    rastState.AntialiasedLineEnable = FALSE;
    rastState.CullMode = D3D11_CULL_NONE;
    rastState.FillMode = D3D11_FILL_SOLID;
    rastState.FrontCounterClockwise = TRUE;
    rastState.DepthBias = 0;
    rastState.SlopeScaledDepthBias = 0.0f;
    rastState.DepthBiasClamp = 0.0f;
    rastState.DepthClipEnable = FALSE;
    rastState.ScissorEnable = FALSE;
    rastState.MultisampleEnable = FALSE;

    ID3D11RasterizerState* d3dRastState;

    D3D_HR_OP(device.GetD3D11Device()->CreateRasterizerState(&rastState, &d3dRastState));
    device.GetD3D11DeviceContext()->RSSetState(d3dRastState);

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

    ID3D11DepthStencilState* noDepthStencilState;
    D3D_HR_OP(device.GetD3D11Device()->CreateDepthStencilState(&depthStencilDesc, &noDepthStencilState));
    device.GetD3D11DeviceContext()->OMSetDepthStencilState(noDepthStencilState, 0);

    ColorSurface colorTarget = swapchain.GetBackBufferSurface();
    DepthSurface depthTarget(device, colorTarget.GetWidth(), colorTarget.GetHeight(), 1, 1, DXGI_FORMAT_R24G8_TYPELESS, GetSampleDesc(device, DXGI_FORMAT_D24_UNORM_S8_UINT, MULTISAMPLE_TYPE), D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, 0, 0);

    Camera camera = CreateInitialCamera((float)window.GetWidth() / (float)window.GetHeight());
    camera.m_UseAngles = true;

    Scene scene;

    GraphicsViewport viewport(BoundRect(Point2D(0, 0), window.GetWidth(), window.GetHeight()));
    SuperViewport superViewport(viewport, camera, &scene);

    MouseKeyboardCameraController cameraController(camera);

    while (!window.IsClosed())
    {
        viewport.Bind(device);

        swapchain.Present();
        device.OnPresent();
        cameraController.Update(window);
        window.ExplicitUpdate();
    }
}
