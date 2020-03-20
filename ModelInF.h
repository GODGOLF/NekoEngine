#pragma once
#ifndef _MODEL_INTERFACE_H_
#define _MODEL_INTERFACE_H_
#include <DirectXMath.h>
#include <string>
class ModelInF
{
public:
	ModelInF() : position(0,0,0), rotation(0,0,0),scale(1,1,1),name(""),modelIndex(""){};
	virtual ~ModelInF() {};
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;
	std::string name;
	std::string modelIndex;
	
};


#endif // !_MODEL_INTERFACE_H_

