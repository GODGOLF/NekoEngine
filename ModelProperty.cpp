#include "ModelProperty.h"


void ModelProperty::Update()
{
	if (model != NULL)
	{
		collision.Update(model);
	}
}
void ModelProperty::Destroy()
{
	if (model)
	{
		delete model;
		model = NULL;
	}
	collision.Destroy();
}