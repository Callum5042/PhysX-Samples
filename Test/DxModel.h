#pragma once

#include "DxRenderer.h"
#include <DirectXMath.h>
#include "Vertex.h"
#include "PxPhysicsAPI.h"

namespace DX
{
	class Model
	{
	public:
		Model(DX::Renderer* renderer, physx::PxPhysics* physics, physx::PxScene* scene);
		virtual ~Model() = default;

		// Create device
		void Create(float x, float y, float z);

		// Render the model
		void Render();

		void Update();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		// Get actor
		inline physx::PxRigidBody* GetBody() { return m_Body; }

	private:
		DX::Renderer* m_DxRenderer = nullptr;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();

		// Mesh data
		MeshData m_MeshData;

		// Physics
		physx::PxPhysics* m_Physics = nullptr;
		physx::PxScene* m_Scene = nullptr;
		physx::PxRigidBody* m_Body = nullptr;
	};
}