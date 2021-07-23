#include "Scene.h"

Scene::Scene() {}

void Scene::Render(GraphicsDevice& device, Camera& camera )
{
    for (SuperMeshInstance* meshInst : m_MeshInsts)
        meshInst->Render(device, camera);
}

void Scene::Update(Camera& cam)
{
}

void Scene::ReleaseGPUData()
{
}