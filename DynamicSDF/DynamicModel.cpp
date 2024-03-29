#include "DynamicModel.h"
#include <DirectXMath.h>
#include "GeometryGenerator.h"
#include <PxPhysicsAPI.h>

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
	std::vector<physx::PxVec3> vecs;
	for (auto& v : m_MeshData.vertices)
	{
		vecs.push_back(physx::PxVec3(v.x, v.y, v.z));
	}

	// Cook
	physx::PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = m_MeshData.vertices.size();
	meshDesc.points.stride = sizeof(physx::PxVec3);
	meshDesc.points.data = vecs.data();

	meshDesc.triangles.count = m_MeshData.indices.size();;
	meshDesc.triangles.stride = 3 * sizeof(UINT);
	meshDesc.triangles.data = m_MeshData.indices.data();

	physx::PxSDFDesc sdfDesc;
	sdfDesc.spacing = 0.5f;
	sdfDesc.subgridSize = 6;
	sdfDesc.bitsPerSubgridPixel = physx::PxSdfBitsPerSubgridPixel::e16_BIT_PER_PIXEL;
	sdfDesc.numThreadsForSdfConstruction = 16;

	meshDesc.sdfDesc = &sdfDesc;

	physx::PxTolerancesScale scale;
	physx::PxCookingParams params(scale);
	params.meshWeldTolerance = 0.001f;
	params.meshPreprocessParams = physx::PxMeshPreprocessingFlags(physx::PxMeshPreprocessingFlag::eWELD_VERTICES);
	params.buildTriangleAdjacencies = false;
	params.buildGPUData = true;

	physx::PxDefaultMemoryOutputStream writeBuffer;
	physx::PxTriangleMeshCookingResult::Enum result;

	bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);
	if (!status)
	{
		return;
	}

	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	auto mesh = m_Physics->GetPhysics()->createTriangleMesh(readBuffer);


	// Create actor
	// physx::PxRigidDynamic* m_Body = m_Physics->GetPhysics()->createRigidDynamic(physx::PxTransform(physx::PxVec3(0, 0, 0)));
	m_Body = m_Physics->GetPhysics()->createRigidDynamic(physx::PxTransform(physx::PxVec3(m_Position.x, m_Position.y, m_Position.z)));

	m_Body->setLinearDamping(0.2f);
	m_Body->setAngularDamping(0.1f);

	physx::PxTriangleMeshGeometry geom;
	geom.triangleMesh = mesh;
	geom.scale = physx::PxVec3(1.0f, 1.0f, 1.0f);

	/*m_Body->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_GYROSCOPIC_FORCES, true);
	m_Body->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, true);*/

	physx::PxMaterial* material = m_Physics->GetPhysics()->createMaterial(0.4f, 0.4f, 0.4f);
	physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*m_Body, geom, *material);
	shape->setContactOffset(0.1f);
	shape->setRestOffset(0.02f);

	physx::PxReal density = 100.f;
	physx::PxRigidBodyExt::updateMassAndInertia(*m_Body, density);

	m_Physics->GetScene()->addActor(*m_Body);

	m_Body->setSolverIterationCounts(50, 1);
	m_Body->setMaxDepenetrationVelocity(5.f);
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
