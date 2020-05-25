#ifndef _COLLISION_OBJECT_H_
#define _COLLISION_OBJECT_H_
#include "PhysicManager.h"
#include "ModelInF.h"

struct RigidType
{
	enum VALUE
	{
		STATIC,
		DYNAMIC
	};
};
struct RigidModel
{
	enum VALUE
	{
		BOX,
		SPHERE,
		PLANE
	};
};
struct CollisionDesc
{
	RigidModel::VALUE rigidModel;

	struct Box
	{
		Box();
		RigidType::VALUE rigidType;
		DirectX::XMFLOAT3 haftDimension;
		float density;
	};
	Box boxDesc;
	struct Sphere
	{
		RigidType::VALUE rigidType;
		float radius;
		float density;
	};
	Sphere sphereDesc;
	struct Plane
	{
		DirectX::XMFLOAT4 normal;
	};
	Plane planeDesc;

};

class CollisionObject
{
public:
	CollisionObject();
	virtual ~CollisionObject();
	virtual bool InitialCollision(ModelInF* modelInf, PhysicManager* pPhysicmanager, CollisionDesc &desc);
	virtual void Update(ModelInF* modelInf);
	virtual void Destroy();
private:
	physx::PxRigidActor* m_rigidBody;


};
#endif // !_COLLISION_OBJECT_H_

