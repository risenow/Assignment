#pragma once

#include "GraphicsDevice.h"
#include "Camera.h"
#include "GraphicsBuffer.h"
#include "VertexData.h"
#include "GraphicsInputLayout.h"
#include "GraphicsConstantsBuffer.h"
#include "Mesh.h"
#include "BasicVertexShaderStorage.h"
#include "basicvsconstants.h"
#include "SkyBoxConsts.h"

class SkyBox
{
public:
    SkyBox(GraphicsDevice& device) : m_Center(0.0f, 0.0f, 0.0f), m_Radius(2000.0f)
    {
        BasicVSConsts temp;
        m_ConstantsBuffer = GraphicsConstantsBuffer<BasicVSConsts>(device, temp);
        assert(m_ConstantsBuffer.GetBuffer());
        SkyBoxConsts pstemp;
        m_PSConstantsBuffer = GraphicsConstantsBuffer<SkyBoxConsts>(device, pstemp);
        assert(m_PSConstantsBuffer.GetBuffer());

        m_VertexShader = BasicVertexShaderStorage::GetInstance().GetShader(BasicVertexShaderStorage::NORMALS_ENABLED | BasicVertexShaderStorage::TEXCOORDS_ENABLED);
        m_PixelShader = GraphicsShader::FromFile(device, GraphicsShaderType_Pixel, L"Data/shaders/skyboxps.hlsl");

        InitMesh(device);
    }

    void Render(GraphicsDevice& device, Camera& camera, const glm::vec3& sunPos)
    {
        const float SunSize = 30.0f;

        m_VertexShader.Bind(device);
        m_PixelShader.Bind(device);
        m_InputLayout.Bind(device);
        m_VertexBuffer.Bind(device);

        BasicVSConsts consts;
        consts.view = camera.GetViewMatrix();
        consts.projection = camera.GetProjectionMatrix();
        consts.model = glm::translate(glm::identity<glm::mat4x4>(), camera.GetPosition());

        glm::mat4x4 modelView = consts.view * consts.model;

        SkyBoxConsts psConsts;
        psConsts.SunPos = modelView * glm::vec4(sunPos, 1.0f);
        psConsts.SunPos.w = SunSize;
        psConsts.SkySphere = modelView * glm::vec4(m_Center, 1.0f);
        psConsts.SkySphere.w = m_Radius;

        GraphicsConstantsBuffer<BasicVSConsts>& constsBuffer = m_ConstantsBuffer;
        constsBuffer.Update(device, consts);
        constsBuffer.Bind(device, GraphicsShaderMask_Vertex);
        GraphicsConstantsBuffer<SkyBoxConsts>& psConstsBuffer = m_PSConstantsBuffer;
        psConstsBuffer.Update(device, psConsts);
        psConstsBuffer.Bind(device, GraphicsShaderMask_Pixel);

        device.GetD3D11DeviceContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device.GetD3D11DeviceContext()->Draw(m_VertexCount, 0);
    }
private:
    void InitMesh(GraphicsDevice& device)
    {
        const float r = m_Radius;
        const float side = 2*r;
        const float side2 = side * side;
        float d = sqrt(3 * side2); 

        const glm::vec3 center = m_Center;
        const glm::vec3 dVec(d * 0.5f, d * 0.5f, d * 0.5f);

        AABB skyBox = AABB(center - dVec, center + dVec);

        std::vector<VertexPropertyPrototype> vertexPropertyProtos = { VertexFormatHelper::PurePosPropProto(0, 0) };
        vertexPropertyProtos.push_back(VertexFormatHelper::PureNormalPropProto(0, 0));
        vertexPropertyProtos.push_back(VertexFormatHelper::PureTexCoordPropProto(0, 0));

        VertexFormat vertexFormat(vertexPropertyProtos);
        VertexData vertexData(vertexFormat, 6 * 6);
        VertexProperty posDesc = vertexData.GetVertexPropertyByName(VertexDataSemantics_PurePos);
        VertexProperty tcDesc = vertexData.GetVertexPropertyByName(VertexDataSemantics_PureTexCoord);
        VertexProperty normsDesc = vertexData.GetVertexPropertyByName(VertexDataSemantics_PureNormal);

        glm::vec3 dummyNormal = glm::vec3();
        glm::vec2 dummyTc = glm::vec2();

        //bottom
        int vx = 0;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftBottomBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightBottomBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightBottomFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightBottomFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftBottomFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftBottomBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        //top
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftTopBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightTopBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightTopFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightTopFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftTopFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftTopBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        //left
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftBottomBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftBottomFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftTopFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftTopFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftTopBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftBottomBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        //right
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightBottomBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightBottomFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightTopFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightTopFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightTopBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightBottomBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        //back
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftBottomBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightBottomBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightTopBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightTopBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftTopBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftBottomBack), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        //front
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftBottomFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightBottomFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightTopFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::RightTopFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftTopFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;
        PullVecToVertexData<glm::vec4>(vx, glm::vec4(skyBox.GetPoint(AABB::LeftBottomFront), 1.0f), vertexData, posDesc);
        PullVecToVertexData<glm::vec3>(vx, dummyNormal, vertexData, normsDesc);
        PullVecToVertexData<glm::vec2>(vx, dummyTc, vertexData, tcDesc); vx++;

        m_VertexCount = vx;

        m_VertexBuffer = VertexBuffer(device, vertexData);
        m_InputLayout = GraphicsInputLayout(device, vertexFormat, m_VertexShader);
    }


    float m_Radius;
    glm::vec3 m_Center;

    size_t m_VertexCount;

    VertexBuffer m_VertexBuffer;
    GraphicsInputLayout m_InputLayout;
    GraphicsConstantsBuffer<BasicVSConsts> m_ConstantsBuffer;
    GraphicsConstantsBuffer<SkyBoxConsts> m_PSConstantsBuffer;

    GraphicsShader m_VertexShader;
    GraphicsShader m_PixelShader;
};