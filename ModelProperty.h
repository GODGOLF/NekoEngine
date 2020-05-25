#ifndef _MODEL_PROPERTY_H_
#define _MODEL_PROPERTY_H_
#include "ModelInF.h"
#include "CollisionObject.h"

class ModelProperty
{
public:
	ModelProperty() {};
	virtual ~ModelProperty()
	{

	}
	ModelInF* model;
	CollisionObject collision;
	void Update();
	void Destroy();
private:
};


#endif // !_MODEL_PROPERTY_H_


