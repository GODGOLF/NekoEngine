#pragma once

#ifndef _FBX_LOADER_H_
#define _FBX_LOADER_H_
#include <Windows.h>
#include <fbxsdk.h>
#include "ModelDataStructure.h"
#include <map>
#include <string>

struct KeyFrame {
	long long mFrameTime;
	DirectX::XMMATRIX transformMatrix;
};
struct FBXModelData : public BufferData
{
	FbxNode *meshNode = NULL;
	std::vector<WORD> index;
};
struct Joint {
	int parentIndex;
	char name[128];
	std::map<std::string, std::vector<KeyFrame>> keyframeAnimation;
};
struct AnimationStack
{
	FbxString name;
	long long start;
	long long end;
	
};

struct AnimationFrame
{
	enum VALUE
	{
		e24,
		e100
	};
};

class FBXLoader
{
public:
	FBXLoader();
	~FBXLoader();
	bool LoadFBX(FbxManager* pFBXManager, char* file, AnimationFrame::VALUE frameRate = AnimationFrame::e24);
	std::vector<FBXModelData>* GetModelList();
	std::vector<Joint>* GetJoint();
	std::vector<AnimationStack>* GetAnimationStacks();
	bool haveAnimation;
	long long ConvertMillisecondToFrameRate(AnimationFrame::VALUE frameRate,long long timeValue);
private:
	std::vector<FBXModelData> m_modelList;
	std::vector<Joint> m_skeleton;
	std::vector<AnimationStack> m_animationStack;
	FbxScene* m_scene;
	AnimationFrame::VALUE m_frameRate;
private:

	void LoadContent(FbxNode* pNode);
	void LoadPolygon(FbxMesh* pMesh);
	void LoadMaterial(FbxSurfaceMaterial* material, BufferData* modelUser);
	void LoadSkeletonHierarchyRecursive(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex);
	int	FindJointUsingName(const char* name);
	void LoadAnimationData();
	FbxTime::EMode ConvertAnimFrameValueToFbxEnum(AnimationFrame::VALUE frameRate);

	struct BoneInfo {
		unsigned int bone;
		float width;
	};
	//temp vertex data
	struct TempVertexData {
		DirectX::XMFLOAT3 vertex;
		std::vector<BoneInfo> bone;
	};
	void LoadBoneData(FBXModelData* mesh, std::vector<TempVertexData> &vertex);
};

#endif
