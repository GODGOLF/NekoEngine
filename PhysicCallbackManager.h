#ifndef _PHYSIC_CALL_BACK_MANAGER_H_
#define _PHYSIC_CALL_BACK_MANAGER_H_

#include <PxPhysicsAPI.h>
#include <vector>

class PhysicCallBackManager : public physx::PxSimulationEventCallback
{
public:
	// Implements PxSimulationEventCallback
	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)override;
	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
	virtual void onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32)override;
	virtual void onWake(physx::PxActor**, physx::PxU32)override;
	virtual void onSleep(physx::PxActor**, physx::PxU32)override;
	virtual void onAdvance(const physx::PxRigidBody*const*, const physx::PxTransform*, const physx::PxU32)override;

private:
	//std::vector<ObjCallback*> objectCallback;
};

#endif // !_PHYSIC_CALL_BACK_MANAGER_H_

