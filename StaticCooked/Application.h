#pragma once

#include <iostream>
#include <memory>
#include <SDL_video.h>
#include "Timer.h"
#include "DxRenderer.h"
#include "DxShader.h"
#include "DxLineShader.h"
#include "DxCamera.h"

#include "StaticModel.h"
#include "PlaneModel.h"

#include "Physics.h"

class Applicataion
{
public:
	Applicataion() = default;
	virtual ~Applicataion();

	int Execute();

private:
	void DirectXSetup();

	// SDL window
	bool SDLInit();
	void SDLCleanup();
	SDL_Window* m_SdlWindow = nullptr;

	// Timer
	Timer m_Timer;
	void CalculateFramesPerSecond();

	// Direct3D 11 renderer
	std::unique_ptr<DX::Renderer> m_DxRenderer = nullptr;
	
	// Direct3D 11 model
	std::unique_ptr<DX::StaticModel> m_StaticModel = nullptr;
	std::unique_ptr<DX::PlaneModel> m_PlaneModel = nullptr;

	// Direct3D 11 shader
	std::unique_ptr<DX::Shader> m_DxShader = nullptr;
	std::unique_ptr<DX::LineShader> m_DxLineShader = nullptr;

	// Direct3D 11 perspective camera
	std::unique_ptr<DX::Camera> m_DxCamera = nullptr;

	// Update buffers
	void SetCameraBuffer();

	// Setup directional light
	void SetupDirectionalLight();

	std::unique_ptr<PX::Physics> m_Physics = nullptr;
};