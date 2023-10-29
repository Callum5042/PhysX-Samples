#pragma once

#include "DxRenderer.h"
#include "Physics.h"
#include "DxLineShader.h"
#include <vector>

namespace DX
{
	struct LineVertex
	{
		// Position
		float x = 0;
		float y = 0;
		float z = 0;

		// Colour
		float r = 0;
		float g = 0;
		float b = 0;
		float a = 0;
	};

	class LineManager
	{
	public:
		LineManager(Renderer* renderer);
		virtual ~LineManager() = default;

		void Create();

		// Clear line list
		void ClearLines();

		// Add line
		void AddLine(LineVertex a, LineVertex b);

		// Add lines from scene
		void AddSceneLine(PX::Physics* physics);

		// Render lines
		void Render();

	private:
		Renderer* m_Renderer = nullptr;

		ComPtr<ID3D11Buffer> m_LineVertexBuffer = nullptr;
		std::vector<LineVertex> m_LineList;
	};
}