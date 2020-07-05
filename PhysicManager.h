#pragma once
#ifndef _PHYSIC_MANAGER_H_
#define _PHYSIC_MANAGER_H_
#define PX_PHYSX_CHARACTER_STATIC_LIB

#include <PxPhysicsAPI.h>
#include <extensions/PxExtensionsAPI.h>
#include "PhysicUserCallback.h"
#include "PhysicCallbackManager.h"

struct ControlledActorDesc
{
	ControlledActorDesc();

	physx::PxControllerShapeType::Enum		mType;
	physx::PxExtendedVec3					mPosition;
	float							mSlopeLimit;
	float							mContactOffset;
	float							mStepOffset;
	float							mInvisibleWallHeight;
	float							mMaxJumpHeight;
	float							mRadius;
	float							mHeight;
	float							mCrouchHeight;
	float							mProxyDensity;
	float							mProxyScale;
	float							mVolumeGrowth;
	physx::PxUserControllerHitReport*		mReportCallback;
	physx::PxControllerBehaviorCallback*	mBehaviorCallback;
};

class PhysicManager : public physx::PxDeletionListener
{
public:
	PhysicManager();
	bool Initial();
	void Update(float dt);
	void Destroy();

	physx::PxRigidDynamic*	CreateDynamicBox(const physx::PxVec3& pos, const physx::PxVec3& halfDims, const physx::PxVec3* linVel = NULL, physx::PxReal density = 1.0f);
	physx::PxRigidStatic*	CreateStaticBox(const physx::PxVec3& pos,const physx::PxVec3& halfDims);
	physx::PxRigidStatic*	CreateStaticBox(const physx::PxVec3& pos, physx::PxShape* shape);
	physx::PxRigidDynamic*	CreateDynamicSphere(const physx::PxVec3& pos, physx::PxReal radius, const physx::PxVec3* linVel = NULL, physx::PxReal density = 1.0f);
	physx::PxRigidStatic*	CreateStaticSphere(const physx::PxVec3& pos, physx::PxReal radius);
	physx::PxRigidStatic*	CreateStaticPlane(physx::PxVec4 normal, bool addToScene = true);
	physx::PxController*    CreateCharacterController(ControlledActorDesc &desc);
	physx::PxMaterial*		CreateMaterial(float staticFriction,float dynamicFriction,float restitution);

private:
	virtual	void onRelease(const physx::PxBase* observed, void* userData, physx::PxDeletionEventFlag::Enum deletionEvent) override;

private:
	physx::PxFoundation * m_Foundation;
	physx::PxDefaultAllocator m_DefaultAllocatorCallback;
	physx::PxPvd* m_Pvd;
	physx::PxPvdTransport* m_transport;
	physx::PxPhysics* m_Physics;
	physx::PxCooking* m_Cooking;
	physx::PxScene* m_Scene;
	physx::PxDefaultCpuDispatcher*	m_CpuDispatcher;
	physx::PxTolerancesScale m_scale;
	physx::PxMaterial*	m_Material;
	physx::PxReal m_DebugRenderScale;
	physx::PxU32 m_NbThreads;
	physx::PxControllerManager* m_controllerManager;
	PxUserErrorCallback m_DefaultErrorCallback;
	bool m_recordMemoryAllocations;
	DefaultStepper m_DefaultStepper;
	PhysicCallBackManager m_physicCallBack;
	bool m_InitialDebugRender;
	bool m_waitForResults;

};
#endif // !_PHYSIC_MANAGER_H_

