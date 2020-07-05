#include "CollisionObject.h"

using namespace physx;
CollisionObject::CollisionObject() :m_rigidBody(NULL)
{

}
CollisionObject::~CollisionObject()
{
}
bool CollisionObject::InitialCollision(ModelInF* modelInf,PhysicManager* pPhysicmanager, CollisionDesc &desc)
{
	if (m_rigidBody)
	{
		m_rigidBody->release();
		m_rigidBody = NULL;
	}
	switch (desc.rigidModel)
	{
	case RigidModel::SPHERE:
	{
		PxVec3 pxPos;
		pxPos.x = modelInf->position.x;
		pxPos.y = modelInf->position.y;
		pxPos.z = modelInf->position.z;
		if (desc.sphereDesc.rigidType == RigidType::STATIC)
		{
			
			m_rigidBody = pPhysicmanager->CreateStaticSphere(pxPos, desc.sphereDesc.radius);
		}
		else
		{
			m_rigidBody = pPhysicmanager->CreateDynamicSphere(pxPos, desc.sphereDesc.radius, 0, desc.sphereDesc.density);
		}
		
		break;
	}
	case RigidModel::PLANE:
	{
		PxVec4 pxNormal;
		pxNormal.x = desc.planeDesc.normal.x;
		pxNormal.y = desc.planeDesc.normal.y;
		pxNormal.z = desc.planeDesc.normal.z;
		pxNormal.w = desc.planeDesc.normal.w;
		m_rigidBody = pPhysicmanager->CreateStaticPlane(pxNormal, true);
		break;
	}
	case RigidModel::BOX:
	{
		PxVec3 pxPos;
		pxPos.x = modelInf->position.x;
		pxPos.y = modelInf->position.y;
		pxPos.z = modelInf->position.z;
		PxVec3 pxHalfVec;
		pxHalfVec.x = desc.boxDesc.haftDimension.x;
		pxHalfVec.y = desc.boxDesc.haftDimension.y;
		pxHalfVec.z = desc.boxDesc.haftDimension.z;
		
		if (desc.boxDesc.rigidType == RigidType::STATIC)
		{

			m_rigidBody = pPhysicmanager->CreateStaticBox(pxPos, pxHalfVec);
		}
		else
		{
			m_rigidBody = pPhysicmanager->CreateDynamicBox(pxPos, pxHalfVec, 0, desc.boxDesc.density);
		}
		break;
	}
	default:
		break;
	}
	if (m_rigidBody)
		return true;
	else
		return false;
}
void CollisionObject::Update(ModelInF* modelInf)
{
	PxTransform trans = m_rigidBody->getGlobalPose();
	PxVec3 pos = trans.p;
	modelInf->position.x = pos.x;
	modelInf->position.y = pos.y;
	modelInf->position.z = pos.z;
	PxQuat pxRotation = trans.q;
	modelInf->SetRotation(DirectX::XMFLOAT4(pxRotation.x, pxRotation.y, pxRotation.z, pxRotation.w));
}
void CollisionObject::Destroy()
{
	if (m_rigidBody)
	{
		m_rigidBody->release();
		m_rigidBody = NULL;
	}
	
}
CollisionDesc::Box::Box() : rigidType(RigidType::DYNAMIC), density(100.0f)
{

}