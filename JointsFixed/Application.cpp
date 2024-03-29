#include "Application.h"
#include "DxLineManager.h"

#include <string>
#include <iostream>
#include <vector>
#include <SDL.h>

Applicataion::~Applicataion()
{
    SDLCleanup();
}

int Applicataion::Execute()
{
    DirectXSetup();

    // Lines
    std::unique_ptr<DX::LineManager> line_manager = std::make_unique<DX::LineManager>(m_DxRenderer.get());
    line_manager->Create();

    // Create physics
    m_Physics = std::make_unique<PX::Physics>();
    m_Physics->Setup();

    // Create models
    m_DynamicModel1 = std::make_unique<DX::DynamicModel>(m_DxRenderer.get(), m_Physics.get());
    m_DynamicModel1->Create(-2.0f, 5.0f, 0.0f);

    m_DynamicModel2 = std::make_unique<DX::DynamicModel>(m_DxRenderer.get(), m_Physics.get());
    m_DynamicModel2->Create(2.0f, 3.0f, 0.0f);

    m_PlaneModel = std::make_unique<DX::PlaneModel>(m_DxRenderer.get(), m_Physics.get());
    m_PlaneModel->Create();

    // Link objects together
    auto joint = physx::PxFixedJointCreate(*m_Physics->GetPhysics(), m_DynamicModel1->GetBody(), physx::PxTransform(physx::PxVec3(-2, 0, 0)), m_DynamicModel2->GetBody(), physx::PxTransform(physx::PxVec3(2, 0, 0)));
    joint->setConstraintFlag(physx::PxConstraintFlag::eVISUALIZATION, true);

    // Starts the timer
    m_Timer.Start();

    // Main application event loop
    SDL_Event e = {};
    while (e.type != SDL_QUIT)
    {
        if (SDL_PollEvent(&e))
        {
            if (e.type == SDL_WINDOWEVENT)
            {
                // On resize event, resize the DxRender device
                if (e.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    m_DxRenderer->Resize(e.window.data1, e.window.data2);
                    m_DxCamera->UpdateAspectRatio(e.window.data1, e.window.data2);

                    // Update world constant buffer with new camera view and perspective
                    SetCameraBuffer();
                }
            }
            else if (e.type == SDL_MOUSEMOTION)
            {
                if (e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))
                {
                    // Rotate the world 
                    auto pitch = e.motion.yrel * 0.01f;
                    auto yaw = e.motion.xrel * 0.01f;
                    m_DxCamera->Rotate(pitch, yaw);

                    // Update world constant buffer with new camera view and perspective
                    SetCameraBuffer();
                }
            }
            else if (e.type == SDL_MOUSEWHEEL)
            {
                auto direction = static_cast<float>(e.wheel.y);
                m_DxCamera->UpdateFov(-direction);

                // Update world constant buffer with new camera view and perspective
                SetCameraBuffer();
            }
        }
        else
        {
            m_Timer.Tick();
            CalculateFramesPerSecond();

            m_Physics->Simulate(m_Timer.DeltaTime());

            // Clear the buffers
            m_DxRenderer->Clear();

            // Bind the shader to the pipeline
            m_DxShader->Use();

            // Render the model
            m_DynamicModel1->Update();
            m_DxShader->UpdateWorldBuffer(m_DynamicModel1->World, m_DynamicModel1->Colour);
            m_DynamicModel1->Render();

            m_DynamicModel2->Update();
            m_DxShader->UpdateWorldBuffer(m_DynamicModel2->World, m_DynamicModel2->Colour);
            m_DynamicModel2->Render();

            // Render the floor
            m_DxShader->UpdateWorldBuffer(m_PlaneModel->World, m_PlaneModel->Colour);
            m_PlaneModel->Render();

            // Render lines
            line_manager->AddSceneLine(m_Physics.get());

            // Apply shader
            m_DxLineShader->Use();
            m_DxLineShader->UpdateModel(DirectX::XMMatrixIdentity());

            line_manager->Render();
            line_manager->ClearLines();

            // Display the rendered scene
            m_DxRenderer->Present();
        }
    }

    return 0;
}

void Applicataion::DirectXSetup()
{
    // Initialise SDL subsystems and creates the window
    if (!SDLInit())
        throw std::exception("SDLInit failed");

    // Initialise and create the DirectX 11 renderer
    m_DxRenderer = std::make_unique<DX::Renderer>(m_SdlWindow);
    m_DxRenderer->Create();

    // Initialise and create the DirectX 11 shader
    m_DxShader = std::make_unique<DX::Shader>(m_DxRenderer.get());
    m_DxShader->LoadVertexShader("Shaders/VertexShader.cso");
    m_DxShader->LoadPixelShader("Shaders/PixelShader.cso");

    m_DxLineShader = std::make_unique<DX::LineShader>(m_DxRenderer.get());
    m_DxLineShader->Load();

    // Initialise and setup the perspective camera
    auto window_width = 0;
    auto window_height = 0;
    SDL_GetWindowSize(m_SdlWindow, &window_width, &window_height);

    // Setup camera
    m_DxCamera = std::make_unique<DX::Camera>(window_width, window_height);

    SetupDirectionalLight();
}

void Applicataion::SetupDirectionalLight()
{
    float delta_time = static_cast<float>(m_Timer.DeltaTime());

    DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(5.0f, 4.0f, -6.0f);

    // Decompose matrix for position
    DirectX::XMVECTOR scale;
    DirectX::XMVECTOR rotation;
    DirectX::XMVECTOR position;
    DirectX::XMMatrixDecompose(&scale, &rotation, &position, world);

    // Calculate direction (direction looking at the center of the scene)
    DirectX::XMVECTOR center = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    auto direction = DirectX::XMVectorSubtract(center, position);
    direction = DirectX::XMVector4Normalize(direction);

    // Update buffer
    DX::DirectionalLightBuffer buffer = {};
    DirectX::XMStoreFloat4(&buffer.direction, direction);
    m_DxShader->UpdateDirectionalLightBuffer(buffer);
}

void Applicataion::SetCameraBuffer()
{
    DX::CameraBuffer buffer = {};
    buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
    buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());
    buffer.cameraPosition = m_DxCamera->GetPosition();

    m_DxShader->UpdateCameraBuffer(buffer);
    m_DxLineShader->UpdateCamera(buffer);
}

bool Applicataion::SDLInit()
{
    // Initialise SDL subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        auto error = SDL_GetError();
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error, nullptr);
        return false;
    }

    // Create SDL Window
    auto window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED;
    m_SdlWindow = SDL_CreateWindow("PhysX Samples - JointsFixed ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
    if (m_SdlWindow == nullptr)
    {
        std::string error = "SDL_CreateWindow failed: "; 
        error += SDL_GetError();
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error.c_str(), nullptr);
        return false;
    }

    return true;
}

void Applicataion::SDLCleanup()
{
    SDL_DestroyWindow(m_SdlWindow);
    SDL_Quit();
}

void Applicataion::CalculateFramesPerSecond()
{
    // Changes the window title to show the frames per second and average frame time every second

    static double time = 0;
    static int frameCount = 0;

    frameCount++;
    time += m_Timer.DeltaTime();
    if (time > 1.0f)
    {
        auto fps = frameCount;
        time = 0.0f;
        frameCount = 0;

        auto title = "PhysX Samples - JointsFixed - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
        SDL_SetWindowTitle(m_SdlWindow, title.c_str());
    }
}
