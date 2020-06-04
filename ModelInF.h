#pragma once
#ifndef _MODEL_INTERFACE_H_
#define _MODEL_INTERFACE_H_
#include <DirectXMath.h>
#include <string>
#include <vector>
class ObjManager;

class ModelInF
{
public:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT3 scale;
	std::string name;
	bool haveAnimation;
	float roughness;
	float metallic;
	//displacement mapping
	float maxTessDistance;
	float minTessDistance;
	float maxTessFactor;
	float minTessFactor;
	float heightScale;
	
public:
	ModelInF() : 
		position(0, 0, 0), 
		rotation(0, 0, 0,1), 
		scale(1, 1, 1), 
		name(""), 
		metallic(0.1f),
		roughness(0.5f),
		maxTessDistance(100.f),
		minTessDistance(0.f),
		maxTessFactor(1.f),
		minTessFactor(1.f),
		heightScale(0.f){};
	virtual ~ModelInF() {};
	
	std::string GetModelIndex() const;

	
private:
	std::string m_modelIndex;
	friend ObjManager;
};


#endif // !_MODEL_INTERFACE_H_

