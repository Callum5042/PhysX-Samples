#include "DynamicModel.h"
#include <DirectXMath.h>
#include "GeometryGenerator.h"

DX::DynamicModel::DynamicModel(DX::Renderer* renderer, PX::Physics* physics) : m_DxRenderer(renderer), m_Physics(physics)
{
	Colour = DirectX::XMFLOAT4(1.0f, 0.0, 0.0f, 1.0f);
}

void DX::DynamicModel::Create(float x, float y, float z)
{
	Create(x, y, z, 1.0f, 1.0f, 1.0f);
}

void DX::DynamicModel::Create(float x, float y, float z, float width, float height, float depth)
{
	m_Position = DirectX::XMFLOAT3(x, y, z);
	m_Dimensions = DirectX::XMFLOAT3(width, height, depth);

	GeometryGenerator::CreatePyramid(width, height, depth, &m_MeshData);

	// Create input buffers
	CreateVertexBuffer();
	CreateIndexBuffer();

	// Create dynamic object
	CreatePhysicsActor();

	World *= DirectX::XMMatrixTranslation(x, y, z);
}

void DX::DynamicModel::CreateVertexBuffer()
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

void DX::DynamicModel::CreateIndexBuffer()
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

void DX::DynamicModel::CreatePhysicsActor()
{
	physx::PxMaterial* material = m_Physics->GetPhysics()->createMaterial(0.4f, 0.4f, 0.4f);
	physx::PxShape* shape = m_Physics->GetPhysics()->createShape(physx::PxBoxGeometry(m_Dimensions.x, m_Dimensions.y, m_Dimensions.z), *material);

	// Set position
	physx::PxVec3 position = physx::PxVec3(physx::PxReal(m_Position.x), physx::PxReal(m_Position.y), physx::PxReal(m_Position.z));
	physx::PxTransform transform(position);

	m_Body = m_Physics->GetPhysics()->createRigidDynamic(transform);
	m_Body->attachShape(*shape);
	physx::PxRigidBodyExt::updateMassAndInertia(*m_Body, 100.0f);
	m_Physics->GetScene()->addActor(*m_Body);
}

void DX::DynamicModel::Render()
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

void DX::DynamicModel::Update()
{
	physx::PxTransform global_pose = m_Body->getGlobalPose();
	m_Position.x = global_pose.p.x;
	m_Position.y = global_pose.p.y;
	m_Position.z = global_pose.p.z;

	World = DirectX::XMMatrixIdentity();
	World *= DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(global_pose.q.x, global_pose.q.y, global_pose.q.z, global_pose.q.w));
	World *= DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
}
 