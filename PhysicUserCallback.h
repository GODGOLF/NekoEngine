/*
PhysicCallback.h

Author: Zhuang Theerapong

*/
#ifndef PHYSIC_CALLBACK_H
#define PHYSIC_CALLBACK_H

#include <PxPhysicsAPI.h>
#include <string>
#include <Windows.h>
class Stepper : public physx::PxDefaultAllocator
{
public:
	Stepper();
	virtual					~Stepper() {}

	virtual	bool			advance(physx::PxScene* scene, physx::PxReal dt) = 0;
	virtual	void			wait(physx::PxScene* scene) = 0;
	virtual	void			renderDone() {}
	virtual	void			shutdown() {}

	physx::PxReal			getSimulationTime()	const { return mSimulationTime; }


protected:
	physx::PxReal			mSimulationTime;
};
class DefaultStepper : public Stepper {
public:
	bool advance(physx::PxScene* scene,
		physx::PxReal dt);
	void wait(physx::PxScene* scene);
};

class PxUserErrorCallback : public physx::PxErrorCallback
{
public:
	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file,
		int line)
	{
		// error processing implementation
		std::string codeEnum = "code " + std::to_string(code) + "\n";
		OutputDebugString(codeEnum.c_str());
		OutputDebugString(message);
	}
};
class Crab : public physx::PxLightCpuTask, public physx::PxDefaultAllocator
{
public:
	Crab(const physx::PxVec3& crabPos) {};
	~Crab() {};


	// Implements LightCpuTask
	virtual  const char*    getName() const { return "Crab AI Task"; }
	virtual  void           run();
};


#endif // !PX_CALLBACK_H
