#pragma once

#ifndef _FBX_LOADER_H_
#define _FBX_LOADER_H_
#include <Windows.h>
#include <fbxsdk.h>
#include "ModelDataStructure.h"

struct KeyFrame {
	FbxLongLong mFrameTime;
	DirectX::XMMATRIX globalMatrix;
};
struct FBXModelData : public BufferData
{
	FbxNode *meshNode = NULL;
	std::vector<KeyFrame> keyFrames;
	std::vector<WORD> index;
};
struct Joint {
	int parentIndex;
	char* name;
	FbxAMatrix mGlobalBindposeInverse;
	FbxAMatrix finalTransform;
	FbxAMatrix boneAnimationTranform;
	FbxNode *mNode;
};

class FBXLoader
{
public:
	FBXLoader();
	~FBXLoader();
	bool LoadFBX(FbxManager* pFBXManager, char* file);
	std::vector<FBXModelData>* GetModelList();
	std::vector<Joint>* GetJoint();
private:
	bool m_haveAnimation;
	FbxTime m_start;
	FbxTime m_stop;
	FbxTime m_frameTime;
	FbxTime m_timeCount;

	std::vector<FBXModelData> m_modelList;
	std::vector<Joint> m_skeleton;

	void LoadContent(FbxNode* pNode);
	void LoadPolygon(FbxMesh* pMesh);
	void LoadMaterial(FbxSurfaceMaterial* material, BufferData* modelUser, int &id,const char* file);
	void LoadSkeletonHierachy(FbxNode* rootNode);
	void LoadSkeletonHierarchyRecursive(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex);
	int	FindJointUsingName(const char* name);

	struct BoneInfo {
		unsigned int bone;
		float width;
	};
	//temp vertex data
	struct TempVertexData {
		DirectX::XMFLOAT3 vertex;
		std::vector<BoneInfo> bone;
	};
	void ProcessJointsAndAnimations(FBXModelData* mesh, std::vector<TempVertexData> &vertex);
};

#endif
