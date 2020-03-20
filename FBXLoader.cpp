#ifndef FBXSDK_SHARED
#define FBXSDK_SHARED
#endif // !FBXSDK_SHARED
#include "FBXLoader.h"
#include <windows.h>
#include <string>
#define DIFFUSE_TEXTURE_KEY "DiffuseColor"
#define NORMAL_TEXTURE_KEY "NormalMap"


FBXLoader::FBXLoader()
{

}
FBXLoader::~FBXLoader()
{

}
bool FBXLoader::LoadFBX(FbxManager* pFBXManager, char* file)
{
	pFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL, true);
	pFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, true);
	pFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_LINK, true);
	pFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE, true);
	pFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, true);
	pFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION, true);
	pFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

	// Create an importer using the SDK manager.
	FbxImporter* lImporter = FbxImporter::Create(pFBXManager, "");
	if (!lImporter->Initialize(file, -1, pFBXManager->GetIOSettings())) {
		
		return false;
	}
	if (!lImporter) {
		return false;
	}
	FbxScene* lScene = FbxScene::Create(pFBXManager, "");
	// Import the contents of the file into the scene.
	bool status = lImporter->Import(lScene);
	if (!status) {
		return false;
	}
	lImporter->Destroy();

	int AnimStackNumber = 0;
	FbxArray<FbxString*> AnimStackNameArray;

	lScene->FillAnimStackNameArray(AnimStackNameArray);
	//check whether has animation or not 
	if (AnimStackNameArray.Size() >0) {
		m_haveAnimation = true;
		FbxAnimStack *AnimationStack = lScene->FindMember<FbxAnimStack>(AnimStackNameArray[AnimStackNumber]->Buffer());
		lScene->SetCurrentAnimationStack(AnimationStack);
		FbxTakeInfo *takeInfo = lScene->GetTakeInfo(*(AnimStackNameArray[AnimStackNumber]));
		m_start = takeInfo->mLocalTimeSpan.GetStart();
		m_stop = takeInfo->mLocalTimeSpan.GetStop();
		m_frameTime.SetTime(0, 0, 0, 1, 0, lScene->GetGlobalSettings().GetTimeMode());
		m_timeCount = m_start;
	}
	else {
		m_haveAnimation = false;
	}

	FbxNode*lNode = lScene->GetRootNode();
	if (lNode) {
		for (int i = 0; i < lNode->GetChildCount(); i++) {

			LoadContent(lNode->GetChild(i));
		}

	}
	return true;
}

void FBXLoader::LoadContent(FbxNode* pNode)
{
	FbxNodeAttribute::EType lAttributeType;
	if (pNode->GetNodeAttribute() == NULL)
	{
		OutputDebugString("NULL Node Attribute\n\n");
	}
	else {
		lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());
		switch (lAttributeType)
		{

		case FbxNodeAttribute::eMesh: {
			m_modelList.push_back(FBXModelData());
			FbxMesh * lMesh = (FbxMesh*)pNode->GetNodeAttribute();
			FbxDouble3 lTranslation = pNode->LclTranslation.Get();
			m_modelList.back().tran = DirectX::XMFLOAT3((float)lTranslation[0], (float)lTranslation[1], (float)lTranslation[2]);
			FbxDouble3 lRotation = pNode->LclRotation.Get();
			m_modelList.back().rotate = DirectX::XMFLOAT3((float)lRotation[0], (float)lRotation[1], (float)lRotation[2]);
			FbxDouble3 lScaling = pNode->LclScaling.Get();
			m_modelList.back().scale = DirectX::XMFLOAT3((float)lScaling[0], (float)lScaling[1], (float)lScaling[2]);

			FbxAMatrix& lGlobalTransform = pNode->EvaluateGlobalTransform();

			m_modelList.back().meshNode = pNode;

			FbxVector4 t0 = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
			FbxVector4 r0 = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
			FbxVector4 s0 = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

			FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);

			lGlobalTransform = lGlobalTransform * geometryOffset;
			
			m_modelList.back().DefaultMatrix = DirectX::XMMATRIX((float)lGlobalTransform.Get(0, 0), (float)lGlobalTransform.Get(0, 1), (float)lGlobalTransform.Get(0, 2), (float)lGlobalTransform.Get(0, 3),
				(float)lGlobalTransform.Get(1, 0), (float)lGlobalTransform.Get(1, 1), (float)lGlobalTransform.Get(1, 2), (float)lGlobalTransform.Get(1, 3),
				(float)lGlobalTransform.Get(2, 0), (float)lGlobalTransform.Get(2, 1), (float)lGlobalTransform.Get(2, 2), (float)lGlobalTransform.Get(2, 3),
				(float)lGlobalTransform.Get(3, 0), (float)lGlobalTransform.Get(3, 1), (float)lGlobalTransform.Get(3, 2), (float)lGlobalTransform.Get(3, 3));

			LoadPolygon(lMesh);
		}
		break;

		case FbxNodeAttribute::eSkeleton:
		{
			LoadSkeletonHierarchyRecursive(pNode, 0, 0, -1);
		}
		break;
		default:
			break;
		}
	}
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		LoadContent(pNode->GetChild(i));
	}
}
void FBXLoader::LoadPolygon(FbxMesh* pMesh)
{
	int lPolygonCount = pMesh->GetPolygonCount();
	int lControlCount = pMesh->GetControlPointsCount();
	UINT IndexArray[6] = { 0, 1, 2, 0, 2, 3 };
	if (lControlCount <= 0) {
		return;
	}

	FbxVector4* lControlPoints = pMesh->GetControlPoints();


	int indexTri = 0;

	std::vector<WORD> tempIndex;
	std::vector<TempVertexData> tempVertex;
	std::vector<DirectX::XMFLOAT2> tempUV;
	std::vector<DirectX::XMFLOAT3> tempNormal;
	FBXModelData* model = &m_modelList.back();


	//index 
	for (int i = 0; i < lPolygonCount; i++)
	{

		int lPolygonSize = pMesh->GetPolygonSize(i);
		//index ID

		// 1ポリゴン内の頂点数を取得
		switch (lPolygonSize)
		{
		case 3:  // 三角ポリゴン
			for (int j = 0; j<3; j++)
			{
				// コントロールポイントのインデックスを取得
				model->index.push_back(pMesh->GetPolygonVertex(i, j));
			}
			break;
		case 4: // 四角ポリゴン
			for (int j = 0; j<6; j++)
			{
				model->index.push_back(pMesh->GetPolygonVertex(i, IndexArray[j]));
			}
			break;
		}
		indexTri += lPolygonSize;
	}
	//get control point
	for (int i = 0; i < lControlCount; i++)
	{
		FbxVector4 pValue = lControlPoints[i];
		TempVertexData data;
		data.vertex = DirectX::XMFLOAT3((float)pValue[0], (float)pValue[1], (float)pValue[2]);
		tempVertex.push_back(data);
	}
	//// 法線ベクトルをセット
	{
		int vertexId = 0;
		for (int i = 0; i < lPolygonCount; i++)
		{
			int lPolygonSize = pMesh->GetPolygonSize(i);
			for (int j = 0; j < lPolygonSize; j++)
			{
				for (int l = 0; l < pMesh->GetElementNormalCount(); ++l)
				{
					FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(l);

					if (leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (leNormal->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						{
							FbxVector4 normal = leNormal->GetDirectArray().GetAt(vertexId);
							tempNormal.push_back(DirectX::XMFLOAT3((float)normal[0], (float)normal[1], (float)normal[2] * -1));
						}
						break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leNormal->GetIndexArray().GetAt(vertexId);
							FbxVector4 normal = leNormal->GetDirectArray().GetAt(id);
							tempNormal.push_back(DirectX::XMFLOAT3((float)normal[0] * -1, (float)normal[1] * -1, (float)normal[2] * -1));

						}
						break;
						default:
							break; // other reference modes not shown here!
						}
					}

				}
				//Display3DVector(header, leNormal->GetDirectArray().GetAt(id));
				vertexId++;
			}
		}
	}
	//UV
	{
		for (int l = 0; l < pMesh->GetElementUVCount(); ++l)
		{
			FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
			switch (leUV->GetMappingMode())
			{
			default:
				break;
			case FbxGeometryElement::eByControlPoint:
				switch (leUV->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:

					break;
				case FbxGeometryElement::eIndexToDirect:
				{
					//mada
				}
				break;
				default:
					break; // other reference modes not shown here!
				}
				break;

			case FbxGeometryElement::eByPolygonVertex:
			{


				switch (leUV->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				case FbxGeometryElement::eIndexToDirect:
				{
					//Display2DVector(header, leUV->GetDirectArray().GetAt(lTextureUVIndex));
					for (int i = 0; i < lPolygonCount; i++)
					{
						int lPolygonSize = pMesh->GetPolygonSize(i);
						for (int j = 0; j < lPolygonSize; j++)
						{
							int lTextureUVIndex = pMesh->GetTextureUVIndex(i, j);
							FbxVector2 uv = leUV->GetDirectArray().GetAt(lTextureUVIndex);
							tempUV.push_back(DirectX::XMFLOAT2((float)uv[0], (float)(1 - uv[1])));

						}

					}


				}
				break;
				default:
					break; // other reference modes not shown here!
				}
			}
			break;

			case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
			case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
			case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
				break;
			}
		}
	}

	//load animation

	if (m_haveAnimation) {
		ProcessJointsAndAnimations(model, tempVertex);
	}


	for (unsigned int i = 0; i < model->index.size(); i++) {
		VertexAnime vertex;
		vertex.position = tempVertex[model->index[i]].vertex;
		vertex.normal = tempNormal[i];
		vertex.tangent = DirectX::XMFLOAT4(0, 0, 0, 1);
		vertex.tex = tempUV[i];
		if (tempVertex[model->index[i]].bone.size() >0) {
			vertex.bones.x = tempVertex[model->index[i]].bone[0].bone;
			vertex.bones.y = tempVertex[model->index[i]].bone[1].bone;
			vertex.bones.z = tempVertex[model->index[i]].bone[2].bone;
			vertex.bones.w = tempVertex[model->index[i]].bone[3].bone;

			vertex.width.x = tempVertex[model->index[i]].bone[0].width;
			vertex.width.y = tempVertex[model->index[i]].bone[1].width;
			vertex.width.z = tempVertex[model->index[i]].bone[2].width;
			vertex.width.w = tempVertex[model->index[i]].bone[3].width;
		}
		model->vertrics.push_back(vertex);
		tempIndex.push_back(i);
	}
	//clear index and add new index
	model->index.clear();
	//::CopyMemory(&model->index, &tempIndex,sizeof(WORD)*tempIndex.size());
	model->index = tempIndex;

	//material
	{
		int idCount = 0;
		std::string fileName = "";
		int polyCount = 0;
		FbxLayerElementMaterial* lLayerMaterial = pMesh->GetLayer(0)->GetMaterials();
		if (lLayerMaterial)
		{
			switch (lLayerMaterial->GetMappingMode())
			{
				// １つのメッシュに１つだけマッピングされている
			case FbxLayerElement::eAllSame:
			{
				model->material.push_back(MaterialInfo());
				int lMatId = lLayerMaterial->GetIndexArray().GetAt(0);
				FbxSurfaceMaterial* lMaterial = pMesh->GetNode()->GetMaterial(lMatId);

				if (lMatId >= 0)
				{
					// テクスチャーのロード
					//GetBaseTextureMap(lMaterial, pMeshUser);

					LoadMaterial(lMaterial, model, idCount, fileName.c_str());
					model->material.back().startIndex = 0;
					model->material.back().count = (unsigned int)model->index.size();
				}
			}
			break;
			case FbxLayerElement::eByPolygon:
			{
				model->material.push_back(MaterialInfo());
				for (int i = 0; i < lPolygonCount; i++)
				{
					int lPolygonSize = pMesh->GetPolygonSize(i);
					for (int j = 0; j < pMesh->GetLayerCount(); j++)
					{

						FbxLayerElementMaterial* lLayerMaterial = pMesh->GetLayer(j)->GetMaterials();
						if (lLayerMaterial)
						{
							FbxSurfaceMaterial* lMaterial = NULL;
							int lMatId = -1;
							lMaterial = pMesh->GetNode()->GetMaterial(lLayerMaterial->GetIndexArray().GetAt(i));
							lMatId = lLayerMaterial->GetIndexArray().GetAt(i);

							if (lMatId >= 0)
							{
								// テクスチャーのロード
								int prvID = idCount;
								std::string prevfile = fileName;

								LoadMaterial(lMaterial, model, idCount, fileName.c_str());
								polyCount += (lPolygonSize >3) ? 6 : 3;
								if (prevfile.compare("") == 0) {
									//don't do any thing
									model->material[idCount].startIndex = 0;

								}
								else if (i == lPolygonCount - 1) {
									model->material[idCount].count = polyCount;
								}
								else if (fileName.compare(prevfile) != 0) {
									idCount++;
									model->material[idCount - 1].count = polyCount;
									model->material[idCount].startIndex = i;
									polyCount = 0;
								}

							}
						}


					}
				}

			}
			break;
			}

		}
	}


	std::vector<WORD> clear;
	tempIndex.swap(clear);
}
void FBXLoader::LoadMaterial(FbxSurfaceMaterial* material, BufferData* modelUser, int &id,const char* file)
{
	FbxProperty lProperty = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
	strcpy_s(modelUser->material.back().name, sizeof(modelUser->material.back().name), material->GetName());
	if (lProperty.IsValid())
	{
		int lTextureCount = lProperty.GetSrcObjectCount<FbxTexture>();
		for (int j = 0; j < lTextureCount; ++j)
		{
			//Here we have to check if it's layeredtextures, or just textures:
			FbxLayeredTexture *lLayeredTexture = lProperty.GetSrcObject<FbxLayeredTexture>(j);
			if (lLayeredTexture)
			{
				//FbxLayeredTexture::EBlendMode lBlendMode;
				// now it not support mutiple layer
			}
			else {
				//no layered texture simply get on the property
				FbxTexture* lTexture = lProperty.GetSrcObject<FbxTexture>(j);

				if (lTexture)
				{
					FbxFileTexture *lFileTexture = FbxCast<FbxFileTexture>(lTexture);
					if (std::strcmp(lProperty.GetName(), DIFFUSE_TEXTURE_KEY) == 0) {
						const char* imageFile = lFileTexture->GetFileName();
						strcpy_s(modelUser->material.back().mat.diffuseTexture,sizeof(modelUser->material.back().mat.diffuseTexture), imageFile);
						
					}

				}
			}
		}
	}
	lProperty = material->FindProperty(FbxSurfaceMaterial::sNormalMap);
	if (lProperty.IsValid())
	{
		int lTextureCount = lProperty.GetSrcObjectCount<FbxTexture>();
		for (int j = 0; j < lTextureCount; ++j)
		{
			//Here we have to check if it's layeredtextures, or just textures:
			FbxLayeredTexture *lLayeredTexture = lProperty.GetSrcObject<FbxLayeredTexture>(j);
			if (lLayeredTexture)
			{
				//FbxLayeredTexture::EBlendMode lBlendMode;
				// now it not support mutiple layer
			}
			else {
				//no layered texture simply get on the property
				FbxTexture* lTexture = lProperty.GetSrcObject<FbxTexture>(j);

				if (lTexture)
				{
					FbxFileTexture *lFileTexture = FbxCast<FbxFileTexture>(lTexture);
					if (std::strcmp(lProperty.GetName(), NORMAL_TEXTURE_KEY) == 0) {
						const char* imageFile = lFileTexture->GetFileName();
						strcpy_s(modelUser->material.back().mat.normalTexture, sizeof(modelUser->material.back().mat.normalTexture), imageFile);

					}

				}
			}
		}
	}

	// load material
	FbxPropertyT<FbxDouble3> lKFbxDouble3;
	FbxColor theColor;
	if (material->GetClassId().Is(FbxSurfacePhong::ClassId)) {
		// Display the Ambient Color
		lKFbxDouble3 = ((FbxSurfacePhong *)material)->Ambient;
		theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
		modelUser->material[id].mat.ambientColor = DirectX::XMFLOAT4((float)theColor.mRed, (float)theColor.mGreen, (float)theColor.mBlue, (float)theColor.mAlpha);

		lKFbxDouble3 = ((FbxSurfacePhong *)material)->Diffuse;
		theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
		modelUser->material[id].mat.diffuseColor = DirectX::XMFLOAT4((float)theColor.mRed, (float)theColor.mGreen, (float)theColor.mBlue, (float)theColor.mAlpha);

		lKFbxDouble3 = ((FbxSurfacePhong *)material)->Specular;
		theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
		modelUser->material[id].mat.specularColor = DirectX::XMFLOAT4((float)theColor.mRed, (float)theColor.mGreen, (float)theColor.mBlue, (float)theColor.mAlpha);

		FbxPropertyT<FbxDouble> lKFbxDouble;
		lKFbxDouble = ((FbxSurfacePhong *)material)->Shininess;
		modelUser->material[id].mat.shiness = (float)lKFbxDouble;

	}
}
void FBXLoader::LoadSkeletonHierachy(FbxNode* rootNode)
{

}
void FBXLoader::LoadSkeletonHierarchyRecursive(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex)
{
	FbxSkeleton* lSkeleton = inNode->GetSkeleton();
	if (lSkeleton && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
		Joint joint;
		strcpy_s(joint.name,sizeof(joint.name), inNode->GetName());
		joint.parentIndex = inParentIndex;
		joint.mGlobalBindposeInverse.SetIdentity();

		FbxNode* parentNode = inNode->GetParent();
		if (parentNode != NULL) {
			std::string name = parentNode->GetName();
			int index = FindJointUsingName(name.c_str());
			joint.parentIndex = index;
		}
		m_skeleton.push_back(joint);

	}
}
int FBXLoader::FindJointUsingName(const char* name) {
	for (unsigned int i = 0; i < m_skeleton.size(); i++) {
		if (std::strcmp(m_skeleton[i].name,name) == 0) {
			return i;
		}
	}
	return -1;
}
void FBXLoader::ProcessJointsAndAnimations(FBXModelData* mesh, std::vector<TempVertexData> &vertex)
{
	FbxMesh* currMesh = mesh->meshNode->GetMesh();
	unsigned int numOfDeformers = currMesh->GetDeformerCount();

	FbxVector4 t0 = mesh->meshNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 r0 = mesh->meshNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 s0 = mesh->meshNode->GetGeometricScaling(FbxNode::eSourcePivot);
	FbxAMatrix globalPosition = mesh->meshNode->EvaluateGlobalTransform(m_timeCount);
	FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);



	for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
	{
		FbxSkin *currSkin = (FbxSkin *)currMesh->GetDeformer(0, FbxDeformer::eSkin);
		if (!currSkin)
		{
			continue;
		}
		unsigned int numOfClusters = currSkin->GetClusterCount();
		for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; clusterIndex++) {
			FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
			std::string currJointName = currCluster->GetLink()->GetName();
			int currJointIndex = FindJointUsingName(currJointName.c_str());
			if (currJointIndex == -1) {
				continue;
			}

			m_skeleton[currJointIndex].mNode = currCluster->GetLink();
			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;

			FbxAMatrix vertexTransformMatrix;
			FbxAMatrix globalBindposeInverseMatrix;
			currCluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
			currCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space
			globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryOffset;
			//globalBindposeInverseMatrix.SetIdentity();
			FbxAMatrix currentTransformOffset = globalPosition * geometryOffset;
			FbxAMatrix mGlobalTransform = currentTransformOffset.Inverse() *
				currCluster->GetLink()->EvaluateGlobalTransform(m_timeCount);

			int parentIndex = m_skeleton[currJointIndex].parentIndex;


			FbxAMatrix parentTranform;
			if (parentIndex != -1) {
				parentTranform = m_skeleton[parentIndex].finalTransform;
			}
			else {
				parentTranform.SetIdentity();
			}


			FbxAMatrix mMattranForm = mGlobalTransform * parentTranform;

			m_skeleton[currJointIndex].boneAnimationTranform = mMattranForm;

			vertexTransformMatrix = globalBindposeInverseMatrix * mMattranForm;


			/*XMFLOAT4X4 fVertexTranform = XMFLOAT4X4((float)vertexTransformMatrix.Get(0, 0), (float)vertexTransformMatrix.Get(0, 1), (float)vertexTransformMatrix.Get(0, 2), (float)vertexTransformMatrix.Get(0, 3),
			(float)vertexTransformMatrix.Get(1, 0), (float)vertexTransformMatrix.Get(1, 1), (float)vertexTransformMatrix.Get(1, 2), (float)vertexTransformMatrix.Get(1, 3),
			(float)vertexTransformMatrix.Get(2, 0), (float)vertexTransformMatrix.Get(2, 1), (float)vertexTransformMatrix.Get(2, 2), (float)vertexTransformMatrix.Get(2, 3),
			(float)vertexTransformMatrix.Get(3, 0), (float)vertexTransformMatrix.Get(3, 1), (float)vertexTransformMatrix.Get(3, 2), (float)vertexTransformMatrix.Get(3, 3));


			XMFLOAT4X4 fBindpose = XMFLOAT4X4((float)globalBindposeInverseMatrix.Get(0, 0), (float)globalBindposeInverseMatrix.Get(0, 1), (float)globalBindposeInverseMatrix.Get(0, 2), (float)globalBindposeInverseMatrix.Get(0, 3),
			(float)globalBindposeInverseMatrix.Get(1, 0), (float)globalBindposeInverseMatrix.Get(1, 1), (float)globalBindposeInverseMatrix.Get(1, 2), (float)globalBindposeInverseMatrix.Get(1, 3),
			(float)globalBindposeInverseMatrix.Get(2, 0), (float)globalBindposeInverseMatrix.Get(2, 1), (float)globalBindposeInverseMatrix.Get(2, 2), (float)globalBindposeInverseMatrix.Get(2, 3),
			(float)globalBindposeInverseMatrix.Get(3, 0), (float)globalBindposeInverseMatrix.Get(3, 1), (float)globalBindposeInverseMatrix.Get(3, 2), (float)globalBindposeInverseMatrix.Get(3, 3));



			XMMATRIX mFinalTranform =   XMLoadFloat4x4(&fVertexTranform);
			XMMATRIX mBindPose = XMLoadFloat4x4(&fBindpose);*/
			m_skeleton[currJointIndex].finalTransform = vertexTransformMatrix.Transpose();
			m_skeleton[currJointIndex].mGlobalBindposeInverse = globalBindposeInverseMatrix;



			unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();

			for (unsigned int i = 0; i < numOfIndices; i++)
			{

				int controlPointIndices = currCluster->GetControlPointIndices()[i];
				BoneInfo info;
				info.bone = currJointIndex;
				info.width = (float)currCluster->GetControlPointWeights()[i];
				vertex[controlPointIndices].bone.push_back(info);
			}

			//FbxTime mAnimationLength = stop.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
		}
	}
	for (unsigned int i = 0; i < vertex.size(); i++) {
		if (vertex[i].bone.size() < 4) {
			for (unsigned int j = (unsigned int)vertex[i].bone.size(); j < 4; j++) {
				BoneInfo info;
				info.bone = 0;
				info.width = 0.f;
				vertex[i].bone.push_back(info);
			}
		}
	}
}
std::vector<FBXModelData>* FBXLoader::GetModelList()
{
	return &m_modelList;
}
std::vector<Joint>* FBXLoader::GetJoint()
{
	return &m_skeleton;
}