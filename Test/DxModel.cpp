#include "DxModel.h"
#include <DirectXMath.h>
#include "GeometryGenerator.h"

DX::Model::Model(DX::Renderer* renderer, physx::PxPhysics* physics, physx::PxScene* scene) : m_DxRenderer(renderer), m_Physics(physics), m_Scene(scene)
{
}

void DX::Model::Create(float x, float y, float z)
{
	GeometryGenerator::CreateBox(1.0f, 1.0f, 1.0f, &m_MeshData);

	// Create input buffers
	CreateVertexBuffer();
	CreateIndexBuffer();

	// Create dynamic objects
	auto material = m_Physics->createMaterial(0.4f, 0.4f, 0.4f);
	auto shape = m_Physics->createShape(physx::PxBoxGeometry(1.0f, 1.0f, 1.0f), *material);

	// Set position
	physx::PxVec3 position = physx::PxVec3(physx::PxReal(x), physx::PxReal(y), physx::PxReal(z));
	physx::PxTransform transform(position);

	m_Body = m_Physics->createRigidDynamic(transform);
	m_Body->attachShape(*shape);
	physx::PxRigidBodyExt::updateMassAndInertia(*m_Body, 100.0f);
	m_Scene->addActor(*m_Body);

	World = DirectX::XMMatrixIdentity();
	World = DirectX::XMMatrixTranslation(x, y, z);
}

void DX::Model::CreateVertexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * m_MeshData.vertices.size());
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = m_MeshData.vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, &vertex_subdata, m_d3dVertexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Model::CreateIndexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC index_buffer_desc = {};
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * m_MeshData.indices.size());
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA index_subdata = {};
	index_subdata.pSysMem = m_MeshData.indices.data();

	DX::Check(d3dDevice->CreateBuffer(&index_buffer_desc, &index_subdata, m_d3dIndexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Model::Render()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// We need the stride and offset for the vertex
	UINT vertex_stride = sizeof(Vertex);
	auto vertex_offset = 0u;

	// Bind the vertex buffer to the Input Assembler
	d3dDeviceContext->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &vertex_stride, &vertex_offset);

	// Bind the index buffer to the Input Assembler
	d3dDeviceContext->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the Input Assembler
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render geometry
	d3dDeviceContext->DrawIndexed(static_cast<UINT>(m_MeshData.indices.size()), 0, 0);
}

void DX::Model::Update()
{
	physx::PxTransform global_pose = m_Body->getGlobalPose();

	float x = global_pose.p.x;
	float y = global_pose.p.y;
	float z = global_pose.p.z;

	World = DirectX::XMMatrixTranslation(x, y, z);
}
 