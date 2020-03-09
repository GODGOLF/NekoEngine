#include "DXInclude.h"
#include "D3D11Model.h"


HRESULT D3D11Model::initial(char* file, ModelExtraParameter* parameter)
{
	D3D11ModelParameterInitial* d3dParameter = (D3D11ModelParameterInitial*)parameter;
	m_model.LoadFBX(d3dParameter->fbxManager, file);
	std::vector<FBXModelData>* list = m_model.GetModelList();
	ID3D11Device* device = d3dParameter->device->GetDevice();

	//allocate memmory
	m_modelBuffer.resize(list->size());
	for (int i = 0; i < list->size(); i++)
	{
		HRESULT hr;
		D3D11_BUFFER_DESC bd;
		//vertex buffer
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(VertexAnime) * (unsigned int)list->operator[](i).vertrics.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		D3D11_SUBRESOURCE_DATA InitData;
		//ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = &list->operator[](i).vertrics[0];
		hr = device->CreateBuffer(&bd, &InitData, &m_modelBuffer[i].g_pVertexBuffer);

		//index buffer
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(WORD) * list->operator[](i).index.size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = &list->operator[](i).index[0];
		hr = device->CreateBuffer(&bd, &InitData, &m_modelBuffer[i].g_pIndexBuffer);
	}
	
}
void D3D11Model::render(DXInF* deviceContext, ModelExtraParameter* parameter)
{

}
void D3D11Model::destroy()
{

}