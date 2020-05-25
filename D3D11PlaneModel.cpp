#include "DXInclude.h"
#include "D3D11PlaneModel.h"
#include "ModelDataStructure.h"
#include "PlaneObj.h"
#include "FunctionHelper.h"
#define MATERIAL_CB_INDEX	2	
#define TESSA_CB_INDEX		4
struct DisPlacementMapCB
{
	float gMaxTessDistance;
	float gMinTessDistance;
	float gMaxTessFactor;
	float gMinTessFactor;
	XMFLOAT3 gCameraPos;
	float gHeightScale;
};
struct PlaneMaterialConstant
{
	XMFLOAT4 diffuseColor;
	float  metallic;
	float roughness;
	float textureScale;
	float pad;
};
D3D11PlaneModel::D3D11PlaneModel()
{
	m_vertexDataIndex = NULL;
	m_pConstantLighting = NULL;
	m_pConstantTessa = NULL;
}
D3D11PlaneModel::~D3D11PlaneModel()
{
	Destroy();
}
HRESULT D3D11PlaneModel::Initial(char* file, ModelExtraParameter* parameter)
{
	D3D11PlaneModelParameterInitial* pParameter = (D3D11PlaneModelParameterInitial* )parameter;
	m_dimension = pParameter->dimension;
	m_arraySizePow = (int)std::pow((double)pParameter->arraySize,2.0);
	m_arraySize = pParameter->arraySize;
	m_celSize = pParameter->celSize;

	m_diffuseColor = pParameter->diffuseColor;

	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	ID3D11Device* pDevice = pParameter->pDevice->GetDevice();
	

	int initialX = -pParameter->dimension* pParameter->celSize / 2;
	int initialZ = -pParameter->dimension* pParameter->celSize / 2;

	m_vertexDataIndex = new QuadTree<int>((float)initialX, (float)initialX, (float)(pParameter->dimension* pParameter->celSize), (float)(pParameter->dimension* pParameter->celSize),4);

	int blockSize = pParameter->dimension / m_arraySize;
	int blockSize2 = blockSize * pParameter->celSize;

	for (int node = 0; node < m_arraySizePow; node++)
	{
		
		int xIndex = node% m_arraySize;
		int zIndex = node / m_arraySize;
		int startPointX = xIndex* blockSize2;
		int startPointZ = zIndex* blockSize2;
		int celSize = pParameter->celSize;
		std::vector<Vertex> vertexTerrain;
		vertexTerrain.resize(blockSize* blockSize *4);

		m_vertexDataIndex->Insert(node, (float)startPointX + initialX, (float)startPointZ + initialZ);

		int count = 0;
		for (int i = 0; i < blockSize; i++)
		{
			for (int j = 0; j < blockSize; j++)
			{
				//calculate control point
				float xTempPoint = (float)startPointX + ((float)j*pParameter->celSize);
				float zTempPoint = (float)startPointZ + ((float)i*pParameter->celSize);
				float xTempNextPoint = (float)startPointX + ((float)(j+1)*pParameter->celSize);
				float zTempNextPoint = (float)startPointZ + ((float)(i + 1)*pParameter->celSize);

				float xPoint = xTempPoint + initialX;
				float zPoint = zTempPoint + initialZ;
				float xNextPoint = xTempNextPoint + initialX;
				float zNextPoint = zTempNextPoint + initialZ;

				vertexTerrain[count].position = XMFLOAT3(xPoint, 0, zPoint);
				vertexTerrain[count + 1].position = XMFLOAT3(xPoint, 0, zNextPoint);
				vertexTerrain[count+2].position = XMFLOAT3(xNextPoint, 0, zPoint);
				vertexTerrain[count + 3].position = XMFLOAT3(xNextPoint, 0, zNextPoint);

				int size = pParameter->dimension * pParameter->celSize;
				float du = xTempPoint / (float)size;
				float dy = zTempPoint / (float)size;
				float duPlus = xTempNextPoint / (float)size;
				float dyPlus = zTempNextPoint / (float)size;
				vertexTerrain[count].tex = XMFLOAT2(du, dy);
				vertexTerrain[count + 1].tex = XMFLOAT2(du, dyPlus);
				vertexTerrain[count + 2].tex = XMFLOAT2(duPlus,dy);
				vertexTerrain[count + 3].tex = XMFLOAT2(duPlus, dyPlus);
				count += 4;
			}
		}

		D3D11_SUBRESOURCE_DATA InitData;
		m_vertexBuffer.push_back(VertexBuffer());
		//vertex buffer
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(Vertex) * (unsigned int)vertexTerrain.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = &vertexTerrain[0];
		hr = pDevice->CreateBuffer(&bd, &InitData, &m_vertexBuffer.back().pVertexBuffer);
		if (hr != S_OK) 
		{
			return hr;
		}
	}

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(DisPlacementMapCB);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = pDevice->CreateBuffer(&bd, nullptr, &m_pConstantTessa);
	if (FAILED(hr))
		return hr;

	//light constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PlaneMaterialConstant);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = pDevice->CreateBuffer(&bd, nullptr, &m_pConstantLighting);
	if (FAILED(hr))
		return hr;

	return S_OK;
}
void D3D11PlaneModel::Render(void* pDeviceContext, ModelExtraParameter* parameter)
{
	ID3D11DeviceContext* pD3DeviceContext = (ID3D11DeviceContext*)pDeviceContext;
	D3D11PlaneModelParameterRender* pParameter = (D3D11PlaneModelParameterRender*)parameter;

	m_culling.ConstructFrustum(pParameter->pCamera->GetFarValue(), pParameter->pCamera->GetProjection(), pParameter->pCamera->GetView());

	UpdateTesseration(pD3DeviceContext, pParameter);

	//bind MVP buffer
	pParameter->pMVP->BindConstantMVP(pDeviceContext, pParameter->pCamera,
		XMMatrixIdentity(),
		pParameter->pModelInfo->position,
		pParameter->pModelInfo->rotation,
		pParameter->pModelInfo->scale);
	
	//get Index of vertex buffer;
	std::vector<int> index;
	int initialX = -m_dimension* m_celSize / 2;
	int initialZ = -m_dimension* m_celSize / 2;
	m_vertexDataIndex->Query(&m_culling, (float)initialX, (float)initialZ, (float)(m_dimension*m_celSize), (float)(m_dimension*m_celSize), index);
	
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	int blockSize = m_dimension / m_arraySize;

	PlaneMaterialConstant material;
	material.diffuseColor = m_diffuseColor;
	material.roughness = pParameter->pModelInfo->roughness;
	material.metallic = pParameter->pModelInfo->metallic;
	if (DirectXHelper::instanceof<PlaneObj>(pParameter->pModelInfo))
	{
		PlaneObj* plane = (PlaneObj*)pParameter->pModelInfo;
		material.textureScale = plane->TextureScale;
	}
	else
	{
		material.textureScale = 1.0f;
	}
	pD3DeviceContext->UpdateSubresource(m_pConstantLighting, 0, nullptr, &material, 0, 0);
	pD3DeviceContext->PSSetConstantBuffers(MATERIAL_CB_INDEX, 1, &m_pConstantLighting);
	pD3DeviceContext->VSSetConstantBuffers(TESSA_CB_INDEX, 1, &m_pConstantTessa);
	pD3DeviceContext->DSSetConstantBuffers(TESSA_CB_INDEX, 1, &m_pConstantTessa);
	pD3DeviceContext->HSSetConstantBuffers(TESSA_CB_INDEX, 1, &m_pConstantTessa);
	pD3DeviceContext->PSSetConstantBuffers(TESSA_CB_INDEX, 1, &m_pConstantTessa);
	for (unsigned int i = 0; i < index.size(); i++)
	{
		ID3D11Buffer* vertexBuffer = m_vertexBuffer[i].pVertexBuffer;
		pD3DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		pD3DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		pD3DeviceContext->DrawInstanced(blockSize*blockSize*4, 1, 0, 0);
	}
	pParameter->pMVP->UnbindConstantMVP(pD3DeviceContext);
	ID3D11Buffer* nullBuffer = NULL;
	pD3DeviceContext->PSSetConstantBuffers(MATERIAL_CB_INDEX, 1, &nullBuffer);
	pD3DeviceContext->VSSetConstantBuffers(TESSA_CB_INDEX, 1, &nullBuffer);
	pD3DeviceContext->DSSetConstantBuffers(TESSA_CB_INDEX, 1, &nullBuffer);
	pD3DeviceContext->PSSetConstantBuffers(TESSA_CB_INDEX, 1, &nullBuffer);
	pD3DeviceContext->HSSetConstantBuffers(TESSA_CB_INDEX, 1, &nullBuffer);
	std::vector<int>().swap(index);
}
void D3D11PlaneModel::Destroy()
{
	for (unsigned int i = 0; i < m_vertexBuffer.size(); i++)
	{
		SAFE_RELEASE(m_vertexBuffer[i].pVertexBuffer);
	}
	SAFE_RELEASE(m_pConstantTessa);
	SAFE_RELEASE(m_pConstantLighting);
}

void D3D11PlaneModel::UpdateTesseration(ID3D11DeviceContext* pDeviceContext, D3D11PlaneModelParameterRender* pParameter)
{
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pDeviceContext->Map(m_pConstantTessa, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	DisPlacementMapCB* cb = (DisPlacementMapCB*)MappedResource.pData;
	cb->gHeightScale = pParameter->pModelInfo->heightScale;
	cb->gMaxTessDistance = pParameter->pModelInfo->maxTessDistance;
	cb->gMinTessDistance = pParameter->pModelInfo->minTessDistance;
	cb->gMinTessFactor = pParameter->pModelInfo->minTessFactor;
	cb->gMaxTessFactor = pParameter->pModelInfo->maxTessFactor;
	XMStoreFloat3(&cb->gCameraPos, pParameter->pCamera->GetPosition());
	pDeviceContext->Unmap(m_pConstantTessa, 0);
}