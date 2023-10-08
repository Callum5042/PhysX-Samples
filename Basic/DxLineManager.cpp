#include "DxLineManager.h"
#include "DxShader.h"

DX::LineManager::LineManager(Renderer* renderer) : m_Renderer(renderer)
{
}

void DX::LineManager::Create()
{
    D3D11_BUFFER_DESC vertexbuffer_desc = {};
    vertexbuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    vertexbuffer_desc.ByteWidth = static_cast<UINT>(sizeof(LineVertex) * 1000000);
    vertexbuffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexbuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    m_Renderer->GetDevice()->CreateBuffer(&vertexbuffer_desc, nullptr, m_LineVertexBuffer.ReleaseAndGetAddressOf());
}

void DX::LineManager::ClearLines()
{
    m_LineList.clear();
}

void DX::LineManager::AddLine(LineVertex a, LineVertex b)
{
    m_LineList.push_back(a);
    m_LineList.push_back(b);
}

void DX::LineManager::AddSceneLine(PX::Physics* physics)
{
    std::vector<LineVertex> vertices;
    const physx::PxRenderBuffer& rb = physics->GetScene()->getRenderBuffer();
    for (physx::PxU32 i = 0; i < rb.getNbLines(); i++)
    {
        const physx::PxDebugLine& line = rb.getLines()[i];

        LineVertex v1;
        v1.x = line.pos0.x;
        v1.y = line.pos0.y;
        v1.z = line.pos0.z;

        v1.r = static_cast<float>(line.color0 & 0x000000ff);
        v1.g = static_cast<float>((line.color0 & 0x0000ff00) >> 8);
        v1.b = static_cast<float>((line.color0 & 0x00ff0000) >> 16);
        v1.a = static_cast<float>((line.color0 & 0xff000000) >> 24);

        LineVertex v2;
        v2.x = line.pos1.x;
        v2.y = line.pos1.y;
        v2.z = line.pos1.z;

        v2.r = static_cast<float>(line.color1 & 0x000000ff);
        v2.g = static_cast<float>((line.color1 & 0x0000ff00) >> 8);
        v2.b = static_cast<float>((line.color1 & 0x00ff0000) >> 16);
        v2.a = static_cast<float>((line.color1 & 0xff000000) >> 24);

        vertices.push_back(v1);
        vertices.push_back(v2);
    }

    m_LineList.insert(m_LineList.end(), vertices.begin(), vertices.end());
}

void DX::LineManager::Render()
{
    // Map new resource
    D3D11_MAPPED_SUBRESOURCE resource = {};
    int memorysize = sizeof(LineVertex) * static_cast<int>(m_LineList.size());

    DX::Check(m_Renderer->GetDeviceContext()->Map(m_LineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
    std::memcpy(resource.pData, m_LineList.data(), memorysize);
    m_Renderer->GetDeviceContext()->Unmap(m_LineVertexBuffer.Get(), 0);

    // Bind the vertex buffer
    UINT stride = sizeof(LineVertex);
    UINT offset = 0;

    // Bind the vertex buffer to the pipeline's Input Assembler stage
    m_Renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, m_LineVertexBuffer.GetAddressOf(), &stride, &offset);

    // Bind the geometry topology to the pipeline's Input Assembler stage
    m_Renderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // Render
    m_Renderer->GetDeviceContext()->Draw(static_cast<UINT>(m_LineList.size()), 0);
}
