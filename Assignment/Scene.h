#pragma once
#include "GraphicsDevice.h"
#include "Camera.h"
#include "SuperMeshInstance.h"
#include <fstream>

class Scene
{
public:
    Scene();

    void Render(GraphicsDevice& device, Camera& camera);

    void Update(Camera& cam);

    void Consume(const std::vector<SuperMeshInstance*>& meshInsts) 
    {
        m_MeshInsts = meshInsts;
    }

    void ReleaseGPUData();
private:

    std::vector<SuperMeshInstance*> m_MeshInsts;
};