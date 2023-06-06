#pragma once

#include "DxRenderer.h"
#include <DirectXMath.h>
#include "Vertex.h"
#include "Physics.h"

namespace DX
{
	class Floor
	{
	public:
		Floor(DX::Renderer* renderer, PX::Physics* physics);
		virtual ~Floor() = default;

		// Create device
		void Create();

		// Render the model
		void Render();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
		
		// Colour
		DirectX::XMFLOAT4 Colour = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

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
		PX::Physics* m_Physics = nullptr;
		physx::PxRigidBody* m_Body = nullptr;
	};
}