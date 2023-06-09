#include "CharacterKinematicModel.h"
#include <DirectXMath.h>
#include "GeometryGenerator.h"

DX::CharacterKinematicModel::CharacterKinematicModel(DX::Renderer* renderer, PX::Physics* physics) : m_DxRenderer(renderer), m_Physics(physics)
{
	Colour = DirectX::XMFLOAT4(0.0f, 0.0, 1.0f, 1.0f);
}

void DX::CharacterKinematicModel::Create(float x, float y, float z)
{
	Create(x, y, z, 0.25f, 1.0f, 0.25f);
}

void DX::CharacterKinematicModel::Create(float x, float y, float z, float width, float height, float depth)
{
	m_Position = DirectX::XMFLOAT3(x, y, z);
	m_Dimensions = DirectX::XMFLOAT3(width, height, depth);

	GeometryGenerator::CreateBox(width, height, depth, &m_MeshData);

	// Create input buffers
	CreateVertexBuffer();
	CreateIndexBuffer();

	// Create dynamic object
	CreatePhysicsActor();

	World *= DirectX::XMMatrixTranslation(x, y, z);
}

void DX::CharacterKinematicModel::CreateVertexBuffer()
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

void DX::CharacterKinematicModel::CreateIndexBuffer()
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

void DX::CharacterKinematicModel::CreatePhysicsActor()
{
	physx::PxMaterial* material = m_Physics->GetPhysics()->createMaterial(1.0f, 1.0f, 1.0f);

	physx::PxBoxControllerDesc desc;
	// physx::PxCapsuleControllerDesc desc;
	/*desc.height = 2.0f;
	desc.radius = 0.5f;*/

	desc.halfForwardExtent = m_Dimensions.z + 0.01f;
	desc.halfHeight = m_Dimensions.y + 0.01f;
	desc.halfSideExtent = m_Dimensions.x + 0.01f;
	desc.position = physx::PxExtendedVec3(physx::PxReal(m_Position.x), physx::PxReal(m_Position.y), physx::PxReal(m_Position.z));
	desc.density = 1000.0f;
	desc.material = material;
	desc.contactOffset = 0.01f;
	desc.scaleCoeff = 0.99f;
	desc.behaviorCallback = this;
	// desc.stepOffset = 0.0f;

	m_Controller = m_Physics->GetControllerManager()->createController(desc);
}

physx::PxControllerBehaviorFlags DX::CharacterKinematicModel::getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT | physx::PxControllerBehaviorFlag::eCCT_SLIDE;
}

physx::PxControllerBehaviorFlags DX::CharacterKinematicModel::getBehaviorFlags(const physx::PxController& controller)
{
	return physx::PxControllerBehaviorFlags();
}

physx::PxControllerBehaviorFlags DX::CharacterKinematicModel::getBehaviorFlags(const physx::PxObstacle& obstacle)
{
	return physx::PxControllerBehaviorFlags();
}

void DX::CharacterKinematicModel::Render()
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

void DX::CharacterKinematicModel::Update(float delta)
{
	physx::PxVec3 gravity = m_Physics->GetScene()->getGravity();
	physx::PxVec3 velocity = gravity;

	physx::PxControllerFilters filters;
	m_Controller->move(velocity * delta, 0.001f, delta, filters);

	// Update model
	physx::PxExtendedVec3 position = m_Controller->getPosition();
	m_Position.x = static_cast<float>(position.x);
	m_Position.y = static_cast<float>(position.y);
	m_Position.z = static_cast<float>(position.z);

	World = DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
}
