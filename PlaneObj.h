#ifndef _PLANE_OBJ_H_
#define _PLANE_OBJ_H_

#include "ModelInF.h"

class PlaneObj : public ModelInF
{
public:
	PlaneObj() : TextureScale(1.0f), alphaTranparent(false) {};
	virtual ~PlaneObj() {};
	float TextureScale;
	bool alphaTranparent;
private:

};
#endif // !_TERRAIN_OBJ_H_

