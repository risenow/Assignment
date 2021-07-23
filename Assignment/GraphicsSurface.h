#pragma once
#include <d3d11.h>
#include "Texture2D.h"
#include "GraphicsDevice.h"
#include <glm\glm.hpp>

enum GraphicsSurfaceType
{
    GraphicsSurfaceType_RENDER_TARGET,
    GraphicsSurfaceType_DEPTH_STENCIL
};

template<class T>
void CreateView(GraphicsDevice& device, Texture2D* texture, T** view);

template<>
void CreateView(GraphicsDevice& device, Texture2D* texture, ID3D11RenderTargetView** view);

template<>
void CreateView(GraphicsDevice& device, Texture2D* texture, ID3D11DepthStencilView** view);

template<class T>
class GraphicsSurface
{
public:
    GraphicsSurface();
    //GraphicsSurface(ID3D11RenderTargetView* renderTargetView);
    //GraphicsSurface(ID3D11DepthStencilView* depthStencilView);
    GraphicsSurface(GraphicsDevice& device, Texture2D texture);
    GraphicsSurface(GraphicsDevice& device, size_t width, size_t height,
        unsigned int mipLevels, unsigned int arraySize, DXGI_FORMAT dxgiFormat,
        DXGI_SAMPLE_DESC sampleDesc, D3D11_USAGE usage, UINT bindFlags,
        UINT cpuAccessFlags, UINT miscFlags);
    ~GraphicsSurface();

    void SetForeignTexture(GraphicsDevice& device, const Texture2D& texture);

    void Resize(GraphicsDevice& device, size_t width, size_t height, MultisampleType msType);

    void Bind(GraphicsDevice& device) //maybe add DepthSurface or smth as func parameter
    {
        
    }
    //void Validate(const GraphicsSurface& surface);

    T* GetView() const
    {
        return (T*)m_View;
    }

    size_t GetWidth() const;
    size_t GetHeight() const;
    Texture2D* GetTexture() { return &m_Texture; }

    void ReleaseGPUData();
private:
    T* m_View;
    Texture2D m_Texture;

    bool m_TextureIsOwned;
};

typedef GraphicsSurface<ID3D11RenderTargetView> ColorSurface;
typedef GraphicsSurface<ID3D11DepthStencilView> DepthSurface;


void BindRenderTargetsDepthTarget(GraphicsDevice& device, const std::vector<ColorSurface>& renderTargets, DepthSurface& depthTarget);
void ClearRenderTarget(GraphicsDevice& device, ColorSurface& colorTarget, glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
void ClearDepthTarget(GraphicsDevice& device, DepthSurface& depthSurface);