#include "Physics.h"
constexpr auto PVD_HOST = "127.0.0.1";

//// Collision callback
//class SimulationCallback : public physx::PxSimulationEventCallback
//{
//public:
//    virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {}
//    virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override {}
//    virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override {}
//    virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override {}
//    virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override {}
//    virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override {}
//};

PX::Physics::~Physics()
{
    if (m_Physics != nullptr) m_Physics->release();
    if (m_Foundation != nullptr) m_Foundation->release();
}

void PX::Physics::Setup()
{
    CreateFoundationAndPhysics();
    CreateScene();

    m_Scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
    m_Scene->setVisualizationParameter(physx::PxVisualizationParameter::eACTOR_AXES, 2.0f);
    m_Scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
}

void PX::Physics::Simulate(double delta_time)
{
    m_Scene->simulate(static_cast<physx::PxReal>(delta_time));
    m_Scene->fetchResults(true);
}

void PX::Physics::CreateFoundationAndPhysics()
{
    // Physics
    m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_DefaultAllocatorCallback, m_DefaultErrorCallback);
    if (m_Foundation == nullptr)
    {
        throw std::exception("PxCreateFoundation failed!");
    }

    // Create PVD client
    m_Pvd = PxCreatePvd(*m_Foundation);
    m_Transport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    m_Pvd->connect(*m_Transport, physx::PxPvdInstrumentationFlag::eALL);

    // Create physics
    m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, physx::PxTolerancesScale(), true, m_Pvd);
    if (m_Physics == nullptr)
    {
        throw std::exception("PxCreatePhysics failed!");
    }
}

void PX::Physics::CreateScene()
{
    // Create CPU dispatcher
    physx::PxDefaultCpuDispatcher* dispatcher = physx::PxDefaultCpuDispatcherCreate(2);

    // Create scene
    physx::PxSceneDesc scene_desc(m_Physics->getTolerancesScale());
    scene_desc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    scene_desc.cpuDispatcher = dispatcher;
    scene_desc.filterShader = physx::PxDefaultSimulationFilterShader;
    // scene_desc.simulationEventCallback = this;

    m_Scene = m_Physics->createScene(scene_desc);

    m_ControllerManager = PxCreateControllerManager(*m_Scene);
}