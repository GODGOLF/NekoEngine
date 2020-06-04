#pragma once
#ifndef _D3D11_PLANE_MODEL_H_
#define _D3D11_PLANE_MODEL_H_
#include "D3DModelInF.h"
#include "TextureLoader.h"
#include "D3D11Class.h"
#include "D3D11MVP.h"
#include "ModelInF.h"
#include <string>
#include <vector>
#include <map>
#include "QuadTree.h"
#include "FrustumCullling.h"
struct D3D11PlaneModelParameterInitial : public ModelExtraParameter
{
	D3D11Class* pDevice;
	int arraySize;
	int dimension;
	int celSize;
	XMFLOAT4 diffuseColor;
};
struct D3D11PlaneModelParameterRender : public ModelExtraParameter
{
	D3D11PlaneModelParameterRender() : pCamera(NULL), pMVP(NULL), pModelInfo(NULL)
	{}
	Camera* pCamera;
	D3D11MVP* pMVP;
	ModelInF* pModelInfo;
	bool tranparent = false;
};

class D3D11PlaneModel :public D3DModelInF
{
public:
	D3D11PlaneModel();
	virtual ~D3D11PlaneModel();
	virtual HRESULT Initial(char* file, ModelExtraParameter* parameter = NULL)  override;
	virtual void Render(void* pDeviceContext, ModelExtraParameter* parameter = NULL)  override;
	virtual void Destroy() override;
private:
	
	struct VertexBuffer
	{
		ID3D11Buffer* pVertexBuffer = nullptr;
	};

	ID3D11Buffer*  m_pConstantLighting;

	ID3D11Buffer*  m_pConstantTessa;

	QuadTree<int>* m_vertexDataIndex;

	std::vector<VertexBuffer> m_vertexBuffer;

	FrustumCulling m_culling;

private:
	void UpdateTesseration(ID3D11DeviceContext* pDeviceContext, D3D11PlaneModelParameterRender* pParameter);

protected:
	int m_dimension;
	int m_arraySizePow;
	int m_arraySize;
	int m_celSize;
	XMFLOAT4 m_diffuseColor;
};


#endif // !_D3D11_MODEL_H_



