#pragma once

#include "ShaderStorage.h"

class DeferredLightingShaderStorage : public ShaderStorage
{
public:
    static DeferredLightingShaderStorage& GetInstance()
    {
        static DeferredLightingShaderStorage inst;
        return inst;
    }
    DeferredLightingShaderStorage() {
        m_ShaderFile = L"Data/shaders/lightingcs.hlsl";
        DeferredLightingShaderStorage::GAMMA_CORRECTION = 1 << 0;

        m_MacroSet = { GraphicsShaderMacro("GAMMA_CORRECTION", "1") };
    }
    void Load(GraphicsDevice& device)
    {
        ShaderStorage::Load(device, GraphicsShaderType_Compute);
    }

    static size_t GAMMA_CORRECTION;

};