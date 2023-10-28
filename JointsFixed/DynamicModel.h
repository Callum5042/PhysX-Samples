#pragma once

#include "DxRenderer.h"
#include <DirectXMath.h>
#include "Vertex.h"
#include "Physics.h"

namespace DX
{
	class DynamicModel
	{
	public:
		DynamicModel(DX::Renderer* renderer, PX::Physics* physics);
		virtual ~DynamicModel() = default;

		// Create device
		void Create(float x, float y, float z);
		void Create(float x, float y, float z, float width, float height, float depth);

		// Render the model
		void Render();

		// Update model
		void Update();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		// Colour
		DirectX::XMFLOAT4 Colour = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		// Get actor
		inline physx::PxRigidBody* GetBody() { return m_Body; }

	private:
		DX::Renderer* m_DxRenderer = nullptr;

		DirectX::XMFLOAT3 m_Position = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMFLOAT3 m_Dimensions = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();

		// Mesh data
		MeshData m_MeshData;

		// Physics
		PX::Physics* m_Physics = nullptr;
		physx::PxRigidBody* m_Body = nullptr;
		void CreatePhysicsActor();
	};
}