#include "PhysicUserCallback.h"

using namespace physx;

bool DefaultStepper::advance(PxScene* scene,
	PxReal dt) {
	PxSceneWriteLock writeLock(*scene);
	scene->simulate(dt);
	return true;
}
void DefaultStepper::wait(PxScene* scene) {

	PxSceneWriteLock writeLock(*scene);
	scene->fetchResults(true, NULL);

}
Stepper::Stepper() : mSimulationTime(0.001f) {

}