#pragma once

#include "DxRenderer.h"
#include "DxShader.h"
#include <DirectXMath.h>
#include "DynamicLockedModel.h"

namespace DX
{
	class LineShader
	{
	public:
		LineShader(Renderer* renderer);
		virtual ~LineShader() = default;

		// Load the shader
		void Load();

		// Apply shader to pipeline
		void Use();

		// Update model constant buffer
		void UpdateModel(DirectX::XMMATRIX world);

		// Update camera constant buffer
		void UpdateCamera(const CameraBuffer& buffer);

	private:
		Renderer* m_DxRenderer = nullptr;

		// Vertex shader
		ComPtr<ID3D11InputLayout> m_VertexLayout = nullptr;
		ComPtr<ID3D11VertexShader> m_VertexShader = nullptr;
		void CreateVertexShader();

		// Pixel shader
		ComPtr<ID3D11PixelShader> m_PixelShader = nullptr;
		void CreatePixelShader();

		// Model constant buffer
		ComPtr<ID3D11Buffer> m_ModelConstantBuffer = nullptr;
		void CreateModelConstantBuffer();

		// Camera constant buffer
		ComPtr<ID3D11Buffer> m_CameraConstantBuffer = nullptr;
		void CreateCameraConstantBuffer();
	};
}