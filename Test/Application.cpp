#include "Application.h"

#include <string>
#include <iostream>
#include <vector>
#include <SDL.h>

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

Applicataion::~Applicataion()
{
    SDLCleanup();
}

int Applicataion::Execute()
{
    DirectXSetup();

    /////////// Line stuff
    ComPtr<ID3D11Buffer> m_LineVertexBuffer = nullptr;
    // void CreateVisualisationLineBuffer();
    std::vector<LineVertex> m_LineList;

    // Create vertex buffer
    D3D11_BUFFER_DESC vertexbuffer_desc = {};
    vertexbuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    vertexbuffer_desc.ByteWidth = static_cast<UINT>(sizeof(LineVertex) * 1000000);
    vertexbuffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexbuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    m_DxRenderer->GetDevice()->CreateBuffer(&vertexbuffer_desc, nullptr, m_LineVertexBuffer.ReleaseAndGetAddressOf());

    {
        LineVertex v1;
        v1.x = -10.0f;
        v1.y = 3.0f;
        v1.z = 0.0f;

        v1.r = 1;
        v1.g = 0;
        v1.b = 0;
        v1.a = 1;

        LineVertex v2;
        v2.x = 10.0f;
        v2.y = 3.0f;
        v2.z = 0.0f;

        v2.r = 1;
        v2.g = 0;
        v2.b = 0;
        v2.a = 1;

        m_LineList.push_back(v1);
        m_LineList.push_back(v2);
    }

    ////////////


    // Create physics
    m_Physics = std::make_unique<PX::Physics>();
    m_Physics->Setup();

    // Create models
    m_DynamicModel = std::make_unique<DX::DynamicModel>(m_DxRenderer.get(), m_Physics.get());
    m_DynamicModel->Create(0.0f, 5.0f, 0.0f);

    m_PlaneModel = std::make_unique<DX::PlaneModel>(m_DxRenderer.get(), m_Physics.get());
    m_PlaneModel->Create();

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
            m_DynamicModel->Update();
            m_DxShader->UpdateWorldBuffer(m_DynamicModel->World, m_DynamicModel->Colour);
            m_DynamicModel->Render();

            // Render the floor
            m_DxShader->UpdateWorldBuffer(m_PlaneModel->World, m_PlaneModel->Colour);
            m_PlaneModel->Render();

            // Render lines
            // 
            // Populate lines
            std::vector<LineVertex> vertices;
            const physx::PxRenderBuffer& rb = m_Physics->GetScene()->getRenderBuffer();
            for (physx::PxU32 i = 0; i < rb.getNbLines(); i++)
            {
                const physx::PxDebugLine& line = rb.getLines()[i];

                LineVertex v1;
                v1.x = line.pos0.x;
                v1.y = line.pos0.y;
                v1.z = line.pos0.z;

                v1.r = static_cast<float>(line.color0 & 0x000000ff);
                v1.g = static_cast<float>((line.color0 & 0x0000ff00) >> 8);
                v1.b = static_cast<float>((line.color0 & 0x00ff0000) >> 16);
                v1.a = static_cast<float>((line.color0 & 0xff000000) >> 24);

                LineVertex v2;
                v2.x = line.pos1.x;
                v2.y = line.pos1.y;
                v2.z = line.pos1.z;

                v2.r = static_cast<float>(line.color1 & 0x000000ff);
                v2.g = static_cast<float>((line.color1 & 0x0000ff00) >> 8);
                v2.b = static_cast<float>((line.color1 & 0x00ff0000) >> 16);
                v2.a = static_cast<float>((line.color1 & 0xff000000) >> 24);

                vertices.push_back(v1);
                vertices.push_back(v2);
            }

            m_LineList.clear();
            m_LineList.insert(m_LineList.end(), vertices.begin(), vertices.end());

            // Map new resource
            D3D11_MAPPED_SUBRESOURCE resource = {};
            int memorysize = sizeof(LineVertex) * static_cast<int>(m_LineList.size());

            DX::Check(m_DxRenderer->GetDeviceContext()->Map(m_LineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
            std::memcpy(resource.pData, m_LineList.data(), memorysize);
            m_DxRenderer->GetDeviceContext()->Unmap(m_LineVertexBuffer.Get(), 0);

            // Bind the vertex buffer
            UINT stride = sizeof(LineVertex);
            UINT offset = 0;

            // Bind the vertex buffer to the pipeline's Input Assembler stage
            m_DxRenderer->GetDeviceContext()->IASetVertexBuffers(0, 1, m_LineVertexBuffer.GetAddressOf(), &stride, &offset);

            // Bind the geometry topology to the pipeline's Input Assembler stage
            m_DxRenderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

            DX::CameraBuffer camera_buffer = {};
            camera_buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
            camera_buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());
            camera_buffer.cameraPosition = m_DxCamera->GetPosition();

            // Apply shader
            m_DxLineShader->Use();
            m_DxLineShader->UpdateCamera(camera_buffer);
            m_DxLineShader->UpdateModel(DirectX::XMMatrixIdentity());

            // Render
            m_DxRenderer->GetDeviceContext()->Draw(static_cast<UINT>(m_LineList.size()), 0);

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
    m_SdlWindow = SDL_CreateWindow("PhysX Samples - Testing ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
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

        auto title = "PhysX Samples - Testing - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
        SDL_SetWindowTitle(m_SdlWindow, title.c_str());
    }
}
