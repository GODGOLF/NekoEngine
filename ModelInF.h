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
	bool alphaTranparent;
public:
	ModelInF() : 
		position(0, 0, 0), 
		m_quaternion(0, 0, 0,1), 
		m_rotation(0,0,0),
		scale(1, 1, 1), 
		name(""), 
		metallic(0.1f),
		roughness(0.5f),
		maxTessDistance(100.f),
		minTessDistance(0.f),
		maxTessFactor(1.f),
		minTessFactor(1.f),
		alphaTranparent(false),
		heightScale(0.f){};
	virtual ~ModelInF() {};
	
	std::string GetModelIndex() const;
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT4 GetQuaternion();
	void SetRotation(DirectX::XMFLOAT4 quaternion);
	void SetRotation(DirectX::XMFLOAT3 axis,float angle);
	void SetRotation(float x, float y, float z);
private:
	std::string m_modelIndex;
	friend ObjManager;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMFLOAT4 m_quaternion;
};


#endif // !_MODEL_INTERFACE_H_

