#include "PhysicManager.h"
#define PVD_HOST "Localhost"


using namespace physx;

ControlledActorDesc::ControlledActorDesc() :
	mType(PxControllerShapeType::eFORCE_DWORD),
	mPosition(PxExtendedVec3(0, 0, 0)),
	mSlopeLimit(0.0f),
	mContactOffset(0.0f),
	mStepOffset(0.0f),
	mInvisibleWallHeight(0.0f),
	mMaxJumpHeight(0.0f),
	mRadius(0.0f),
	mHeight(0.0f),
	mCrouchHeight(0.0f),
	mProxyDensity(10.0f),
	//	mProxyScale			(0.8f)
	mProxyScale(0.9f),
	mVolumeGrowth(1.5f),
	mReportCallback(NULL),
	mBehaviorCallback(NULL)
{
}

PhysicManager::PhysicManager():
m_NbThreads(1),
m_Foundation(0),
m_DebugRenderScale(1.0f),
m_recordMemoryAllocations(true),
m_controllerManager(NULL)
{
	
}
PxFilterFlags DefaultFilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	pairFlags = PxPairFlag::eCONTACT_DEFAULT
		| PxPairFlag::eTRIGGER_DEFAULT
		//| PxPairFlag::eNOTIFY_TOUCH_PERSISTS 
		| PxPairFlag::eNOTIFY_CONTACT_POINTS;

	return PxFilterFlag::eDEFAULT;
}
bool PhysicManager::Initial()
{
	m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_DefaultAllocatorCallback,
		m_DefaultErrorCallback);
	if (!m_Foundation) 
	{
		OutputDebugString("PxCreateFoundation failed!\n");
		return false;
	}


	m_transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	//transport = physx::PxDefaultPvdFileTransportCreate("kuy.pvd");
	if (m_transport == NULL)
		return false;

	m_Pvd = PxCreatePvd(*m_Foundation);

	m_Pvd->connect(*m_transport, PxPvdInstrumentationFlag::eALL);
	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation,
		PxTolerancesScale(), m_recordMemoryAllocations, m_Pvd);
	if (!m_Physics) 
	{
		OutputDebugString("PxCreatePhysics failed!");
		return false;
	}
	if (!PxInitExtensions(*m_Physics, m_Pvd)) 
	{
		OutputDebugString("PxInitExtensions failed!\n");
		return false;
	}
	PxCookingParams params(m_scale);
	params.meshWeldTolerance = 0.001f;
	params.buildGPUData = true;
	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
	m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, params);
	if (!m_Cooking) 
	{
		OutputDebugString("PxCreateCooking failed!\n");
		return false;
	}

	m_Physics->registerDeletionListener(*this, PxDeletionEventFlag::eUSER_RELEASE);

	//setup default material...
	m_Material = m_Physics->createMaterial(2.5f, 2.5f, 0.1f);
	if (!m_Material)
		OutputDebugString("createMaterial failed!\n");

	const PxTolerancesScale sc = m_Physics->getTolerancesScale();
	PxSceneDesc sceneDesc(sc);
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	if (!sceneDesc.cpuDispatcher)
	{
		m_CpuDispatcher = PxDefaultCpuDispatcherCreate(m_NbThreads);
		if (!m_CpuDispatcher)
			OutputDebugString("PxDefaultCpuDispatcherCreate failed!\n");
		sceneDesc.cpuDispatcher = m_CpuDispatcher;
	}
	if (!sceneDesc.filterShader)
		sceneDesc.filterShader = DefaultFilterShader;

	sceneDesc.frictionType = PxFrictionType::eTWO_DIRECTIONAL;
	//sceneDesc.frictionType = PxFrictionType::eONE_DIRECTIONAL;
	//sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
	sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
	//sceneDesc.flags |= PxSceneFlag::eENABLE_AVERAGE_POINT;
	sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION;
	//sceneDesc.flags |= PxSceneFlag::eADAPTIVE_FORCE;
	sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
	sceneDesc.sceneQueryUpdateMode = PxSceneQueryUpdateMode::eBUILD_ENABLED_COMMIT_DISABLED;

	//sceneDesc.flags |= PxSceneFlag::eDISABLE_CONTACT_CACHE;
	//sceneDesc.broadPhaseType =  PxBroadPhaseType::eGPU;
	//sceneDesc.broadPhaseType = PxBroadPhaseType::eSAP;
	sceneDesc.gpuMaxNumPartitions = 8;

	sceneDesc.simulationEventCallback = &m_physicCallBack;

	//sceneDesc.solverType = PxSolverType::eTGS;


	m_Scene = m_Physics->createScene(sceneDesc);
	if (!m_Scene)
	{
		OutputDebugString("createScene failed!\n");
		return false;
	}
		

	PxSceneWriteLock scopedLock(*m_Scene);

	PxSceneFlags flag = m_Scene->getFlags();

	PX_UNUSED(flag);
	m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, m_InitialDebugRender ? m_DebugRenderScale : 0.0f);
	m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

	PxPvdSceneClient* pvdClient = m_Scene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	m_controllerManager = PxCreateControllerManager(*m_Scene);
	if (!m_controllerManager)
	{
		return false;
	}

	return true;
}
void PhysicManager::Update(float dt)
{
	m_waitForResults = false;
	if (m_Scene) {

		m_waitForResults = m_DefaultStepper.advance(m_Scene, PxReal(dt));
		if (m_waitForResults) {
			m_DefaultStepper.wait(m_Scene);
		}
	}
}
void PhysicManager::Destroy()
{
	if (m_controllerManager)
	{
		m_controllerManager->release();
		m_controllerManager = NULL;
	}
	if (m_Material) 
	{
		m_Material->release();
		m_Material = NULL;
	}

	if (m_Scene) 
	{
		m_Scene->release();
		m_Scene = NULL;
	}
	if (m_CpuDispatcher) 
	{
		m_CpuDispatcher->release();
		m_CpuDispatcher = NULL;
	}

	PxCloseExtensions();
	if (m_Cooking) 
	{
		m_Cooking->release();
		m_Cooking = NULL;
	}
	if (m_Physics)
	{
		m_Physics->release();
		m_Physics = NULL;
	}
	if (m_Pvd)
	{
		m_Pvd->release();
		m_Pvd = NULL;
	}

	if (m_transport)
	{
		m_transport->release();
		m_transport = NULL;
	}

	if (m_Foundation)
	{
		m_Foundation->release();
		m_Foundation = NULL;
	}
	

}
PX_FORCE_INLINE void SetupDefaultRigidDynamic(PxRigidDynamic& body, bool kinematic = false)
{
	body.setActorFlag(PxActorFlag::eVISUALIZATION, true);
	body.setAngularDamping(0.5f);
	body.setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kinematic);

}
physx::PxRigidDynamic*	PhysicManager::CreateDynamicBox(const physx::PxVec3& pos, const physx::PxVec3& halfDims, const physx::PxVec3* linVel, physx::PxReal density)
{
	PxSceneWriteLock scopedLock(*m_Scene);
	PxRigidDynamic* box = PxCreateDynamic(*m_Physics, PxTransform(pos), PxBoxGeometry(halfDims), *m_Material, density);

	SetupDefaultRigidDynamic(*box);
	m_Scene->addActor(*box);
	//this->object.push_back(obj);

	if (linVel)
		box->setLinearVelocity(*linVel);

	return box;
}
physx::PxRigidStatic* PhysicManager::CreateStaticBox(const physx::PxVec3& pos,const physx::PxVec3& halfDims)
{
	PxSceneWriteLock scopedLock(*m_Scene);
	PxShape* shape = m_Physics->createShape(PxBoxGeometry(halfDims), *m_Material, false);
	PxRigidStatic* box = PxCreateStatic(*m_Physics, PxTransform(pos), *shape);
	if (shape) 
	{
		shape->release();
	}
	m_Scene->addActor(*box);
	return box;
}
physx::PxRigidStatic* PhysicManager::CreateStaticBox(const physx::PxVec3& pos, physx::PxShape* shape)
{
	PxSceneWriteLock scopedLock(*m_Scene);
	PxRigidStatic* box = PxCreateStatic(*m_Physics, PxTransform(pos), *shape);
	m_Scene->addActor(*box);
	return box;
}

physx::PxRigidDynamic*	PhysicManager::CreateDynamicSphere(const physx::PxVec3& pos, physx::PxReal radius, const physx::PxVec3* linVel, physx::PxReal density)
{
	PxSceneWriteLock scopedLock(*m_Scene);
	PxRigidDynamic* sphere = PxCreateDynamic(*m_Physics, PxTransform(pos), PxSphereGeometry(radius), *m_Material, density);
	PX_ASSERT(sphere);
	sphere->setMass(1500);

	SetupDefaultRigidDynamic(*sphere);
	m_Scene->addActor(*sphere);
	if (linVel)
		sphere->setLinearVelocity(*linVel);


	return sphere;
}

physx::PxRigidStatic*	PhysicManager::CreateStaticSphere(const physx::PxVec3& pos, physx::PxReal radius)
{
	PxSceneWriteLock scopedLock(*m_Scene);
	PxRigidStatic* sphere = PxCreateStatic(*m_Physics, PxTransform(pos), PxSphereGeometry(radius), *m_Material);
	PX_ASSERT(sphere);
	m_Scene->addActor(*sphere);
	return sphere;
}
physx::PxRigidStatic* PhysicManager::CreateStaticPlane(physx::PxVec4 normal, bool addToScene)
{
	PxSceneWriteLock scopedLock(*m_Scene);
	PxRigidStatic* plane = PxCreatePlane(*m_Physics, PxPlane(normal.x, normal.y, normal.z, normal.w), *m_Material);
	PxShape* shapes[1];
	plane->getShapes(shapes, 1);
	if (addToScene) {
		m_Scene->addActor(*plane);
	}
	return plane;
}
void PhysicManager::onRelease(const PxBase* observed, void* userData, PxDeletionEventFlag::Enum deletionEvent)
{
	if (!observed->isReleasable()) {
		//OutputDebugString(L"e\n");
	}
}
physx::PxController* PhysicManager::CreateCharacterController(ControlledActorDesc &desc)
{
	const float radius = desc.mRadius;
	float height = desc.mHeight;
	float crouchHeight = desc.mCrouchHeight;

	PxControllerDesc* cDesc;
	PxBoxControllerDesc boxDesc;
	PxCapsuleControllerDesc capsuleDesc;

	if (desc.mType == PxControllerShapeType::eBOX)
	{
		height *= 0.5f;
		height += radius;
		crouchHeight *= 0.5f;
		crouchHeight += radius;
		boxDesc.halfHeight = height;
		boxDesc.halfSideExtent = radius;
		boxDesc.halfForwardExtent = radius;
		cDesc = &boxDesc;
	}
	else
	{
		PX_ASSERT(desc.mType == PxControllerShapeType::eCAPSULE);
		capsuleDesc.height = height;
		capsuleDesc.radius = radius;
		capsuleDesc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
		cDesc = &capsuleDesc;
	}

	cDesc->density = desc.mProxyDensity;
	cDesc->scaleCoeff = desc.mProxyScale;
	cDesc->material = m_Material;
	cDesc->position = desc.mPosition;
	cDesc->slopeLimit = desc.mSlopeLimit;
	cDesc->contactOffset = desc.mContactOffset;
	cDesc->stepOffset = desc.mStepOffset;
	cDesc->invisibleWallHeight = desc.mInvisibleWallHeight;
	cDesc->maxJumpHeight = desc.mMaxJumpHeight;
	//	cDesc->nonWalkableMode		= PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
	cDesc->reportCallback = desc.mReportCallback;
	cDesc->behaviorCallback = desc.mBehaviorCallback;
	cDesc->volumeGrowth = desc.mVolumeGrowth;
	PxController* c = m_controllerManager->createController(*cDesc);
	if (!c)
	{
		return NULL;
	}
	return c;
}
physx::PxMaterial* PhysicManager::CreateMaterial(float staticFriction, float dynamicFriction, float restitution)
{
	PxMaterial* material = m_Physics->createMaterial(staticFriction, dynamicFriction, restitution);
	if (!material)
	{
		return NULL;
	}
	return material;
}