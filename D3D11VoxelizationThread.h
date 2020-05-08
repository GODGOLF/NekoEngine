#pragma once
#ifndef _D3D11_VOXELIZATION_THREAD_H_
#define _D3D11_VOXELIZATION_THREAD_H_
#include "D3D11RenderThread.h"
#include "D3D11Shader.h"
#include "Camera.h"
#include "ModelInF.h"
#include <map>
#include "D3DModelInF.h"
#include "D3D11MVP.h"
#include "ObjSceneInF.h"
#include "VoxelRenderVariable.h"
#include "LightManager.h"
#include "FrustumCullling.h"
struct VoxelRenderParameter : Parameter
{
	std::vector<ModelInF*>* m_modelDataList;
	std::map<std::string, D3DModelInF*>* m_modelObjectList;
	Camera* pCamera;
	LightManager* pLightManager;
};
class D3D11VoxelizationThread : public D3D11RenderThread, public VoxelRenderVariable
{
public:
	D3D11VoxelizationThread();
	virtual ~D3D11VoxelizationThread();

	virtual HRESULT Initial(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Render(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Update(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Destroy() override;

	void SetGBufferRenderParameter(LightManager* pLightManager, ObjScene* pParameter, Camera* camera);

	ID3D11Buffer* GetVoxelLightRenderCB() 
	{
		return m_voxelLightRenderCB;
	}
	ID3D11ShaderResourceView* GetVoxelLightPassSRV()
	{
		return m_voxelLightPassSRV;
	}


	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}

private:
	virtual void ThreadExcecute() override;

	ID3D11Buffer * m_voxelCB;
	ID3D11Buffer * m_voxelLightRenderCB;
	//voxel GBuffer textures
	ID3D11Texture3D* m_voxelColorRT;
	ID3D11Texture3D* m_voxelNormalRT;
	ID3D11Texture3D* m_voxelSpecPowerRT;
	ID3D11Texture3D* m_voxelLightPassRT;
	

	// GBuffer shader resource views
	ID3D11ShaderResourceView* m_voxelColorSRV;
	ID3D11ShaderResourceView* m_voxelNormalSRV;
	ID3D11ShaderResourceView* m_voxelSpecPowerSRV;
	ID3D11ShaderResourceView* m_voxelLightPassSRV;
	//voxel texture UAV
	ID3D11UnorderedAccessView* m_voxelColorUAV;
	ID3D11UnorderedAccessView* m_voxelNormalUAV;
	ID3D11UnorderedAccessView* m_voxelSpecUAV;
	ID3D11UnorderedAccessView* m_voxelLightPassUAV;

	ID3D11RenderTargetView* m_voxelColorRTV;
	ID3D11RenderTargetView* m_voxelNormalRTV;
	ID3D11RenderTargetView* m_voxelSpecRTV;
	ID3D11RenderTargetView* m_voxelLightPassRTV;

	UINT m_volumeDimension;
	float m_volumeGridSize;
	float m_voxelSize;

	XMMATRIX m_viewProjectionMatrix[3];
	XMMATRIX m_viewProjectionMatrixI[3];

	XMMATRIX m_projection;

	ID3D11RenderTargetView* m_dummyRTV;
	ID3D11DepthStencilView* m_DepthStencilDSV;
	ID3D11Texture2D * m_DepthStencilRT;
	ID3D11Texture2D* m_dummyRT;

	//voxelization
	D3D11Shader m_voxelShader;
	//ansio
	D3D11Shader m_ansioShader;
	D3D11Shader m_injectPropagation;
	
	//cull 
	ID3D11RasterizerState* m_RSCullBack;
	ID3D11DepthStencilState *m_DepthStencilState;

	//nessary variable for main thread
	VoxelRenderParameter* m_RenderParameter;

	D3D11MVP m_mvp;

	D3D11_VIEWPORT m_Vp;

	//voxel light pass
	D3D11Shader m_LightInjection;
	ID3D11Buffer * m_voxelInjectRadianceCB;
	ID3D11Buffer * m_voxelLightResourceCB;

	FrustumCulling m_culling;

private:
	//render all object
	void RenderObj();
	void GenerateVoxel();
	void UpdateVoxelCB();
	void UpdateVoxelLightPassCB();
	void ComputeLightInjection();
	void ComputeAnisoMipMap();
	void ComputeLightPropagation();
};
#endif // !_D3D11_VOXELIZATION_THREAD_H_
