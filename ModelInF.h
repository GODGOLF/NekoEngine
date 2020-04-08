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
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;
	std::string name;
	bool haveAnimation;
	struct AnimationStackInfo
	{
		std::string name;
		long long start;
		long long end;
	};
	void SetAnimationStackIndex(int index);
	int GetAnimationStackIndex();
	void SetAnimationTime(long long time);
	long long GetAnimationTime();
public:
	ModelInF() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1), name(""), m_curAnimationStrackIndex(-1), m_curAnimationTime(-1) {};
	virtual ~ModelInF() {};
	AnimationStackInfo GetAnimationStack(unsigned int i) const;
	unsigned int GetAnimationCount() const;
	std::string GetModelIndex() const;
private:
	std::vector<AnimationStackInfo> m_animationStacks;
	std::string m_modelIndex;
	friend ObjManager;
	int m_curAnimationStrackIndex;
	long long m_curAnimationTime;
};


#endif // !_MODEL_INTERFACE_H_

