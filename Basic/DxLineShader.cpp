#include "DxLineShader.h"
#include <fstream>
#include <vector>

DX::LineShader::LineShader(Renderer* renderer) : m_DxRenderer(renderer)
{
}

void DX::LineShader::Load()
{
	CreateVertexShader();
	CreatePixelShader();
	CreateModelConstantBuffer();
	CreateCameraConstantBuffer();
}

void DX::LineShader::Use()
{
	// Bind the input layout to the pipeline's Input Assembler stage
	m_DxRenderer->GetDeviceContext()->IASetInputLayout(m_VertexLayout.Get());

	// Bind the vertex shader to the pipeline's Vertex Shader stage
	m_DxRenderer->GetDeviceContext()->VSSetShader(m_VertexShader.Get(), nullptr, 0);

	// Bind the pixel shader to the pipeline's Pixel Shader stage
	m_DxRenderer->GetDeviceContext()->PSSetShader(m_PixelShader.Get(), nullptr, 0);

	// Bind world constant buffer to the pipeline
	const int world_buffer_slot = 0;
	m_DxRenderer->GetDeviceContext()->VSSetConstantBuffers(world_buffer_slot, 1, m_ModelConstantBuffer.GetAddressOf());
	m_DxRenderer->GetDeviceContext()->PSSetConstantBuffers(world_buffer_slot, 1, m_ModelConstantBuffer.GetAddressOf());

	// Bind camera constant buffer to the pipeline
	const int camera_buffer_slot = 1;
	m_DxRenderer->GetDeviceContext()->VSSetConstantBuffers(camera_buffer_slot, 1, m_CameraConstantBuffer.GetAddressOf());
}

void DX::LineShader::UpdateModel(DirectX::XMMATRIX world)
{
	DX::WorldBuffer buffer = {};
	buffer.world = DirectX::XMMatrixTranspose(world);
	buffer.worldInverse = DirectX::XMMatrixInverse(nullptr, world);

	m_DxRenderer->GetDeviceContext()->UpdateSubresource(m_ModelConstantBuffer.Get(), 0, nullptr, &buffer, 0, 0);
}

void DX::LineShader::UpdateCamera(const CameraBuffer& buffer)
{
	m_DxRenderer->GetDeviceContext()->UpdateSubresource(m_CameraConstantBuffer.Get(), 0, nullptr, &buffer, 0, 0);
}

void DX::LineShader::CreateVertexShader()
{
	// Load the binary file into memory
	std::ifstream vertex_file("D:\\Sources\\PhysX Samples\\Basic\\Shaders\\LineVertexShader.cso", std::fstream::in | std::fstream::binary);
	std::vector<char> vertex_data((std::istreambuf_iterator<char>(vertex_file)), std::istreambuf_iterator<char>());

	// Create the vertex shader
	DX::Check(m_DxRenderer->GetDevice()->CreateVertexShader(vertex_data.data(), vertex_data.size(), nullptr, m_VertexShader.ReleaseAndGetAddressOf()));

	// Describe the memory layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOUR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT nummber_elements = ARRAYSIZE(layout);
	DX::Check(m_DxRenderer->GetDevice()->CreateInputLayout(layout, nummber_elements, vertex_data.data(), vertex_data.size(), m_VertexLayout.ReleaseAndGetAddressOf()));
}

void DX::LineShader::CreatePixelShader()
{
	// Load the binary file into memory
	std::ifstream pixel_file("D:\\Sources\\PhysX Samples\\Basic\\Shaders\\LinePixelShader.cso", std::fstream::in | std::fstream::binary);
	std::vector<char> pixel_data((std::istreambuf_iterator<char>(pixel_file)), std::istreambuf_iterator<char>());

	// Create pixel shader
	DX::Check(m_DxRenderer->GetDevice()->CreatePixelShader(pixel_data.data(), pixel_data.size(), nullptr, m_PixelShader.ReleaseAndGetAddressOf()));
}

void DX::LineShader::CreateModelConstantBuffer()
{
	// Create world constant buffer - Must set D3D11_USAGE_DYNAMIC and D3D11_CPU_ACCESS_WRITE to be able to with Map
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WorldBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	DX::Check(m_DxRenderer->GetDevice()->CreateBuffer(&bd, nullptr, m_ModelConstantBuffer.ReleaseAndGetAddressOf()));
}

void DX::LineShader::CreateCameraConstantBuffer()
{
	// Create world constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CameraBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	DX::Check(m_DxRenderer->GetDevice()->CreateBuffer(&bd, nullptr, m_CameraConstantBuffer.ReleaseAndGetAddressOf()));
}