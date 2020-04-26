#ifndef _D3D11_SHADOW_MANAGER_THREAD_H_
#define _D3D11_SHADOW_MANAGER_THREAD_H_

#define DIRECTIONAL_LIGHT_TEXTURE_SIZE	2048

#include "D3D11RenderThread.h"
#include "CascadedMatrixSet.h"
#include <map>
#include "LightManager.h"
#include "D3DModelInF.h"
#include "ObjSceneInF.h"
#include "D3D11Shader.h"
#include "D3D11MVP.h"

struct ShadowInitialParameter : Parameter
{
	int width;
	int height;
};
struct ShadowRenderParameter : Parameter
{
	LightManager* lightmanager;
	Camera* pCamera;
	std::vector<ModelInF*>* m_modelDataList;
	std::map<std::string, D3DModelInF*>* m_modelObjectList;
};

struct ShadowData
{
	ShadowData() : needDestroy(true),lightIndex(-1)
	{
	}
	virtual ~ShadowData();
	ID3D11DepthStencilView* depthStencilDSV;
	ID3D11ShaderResourceView* depthStencilSRV;
	int lightIndex;
	bool needDestroy;
};
struct DirectionalLightSahdow : ShadowData
{
	DirectionalLightSahdow() : pCascadedViewProjCB(NULL){}
	virtual ~DirectionalLightSahdow();
	CascadedMatrixSet cascadedMatrix;
	ID3D11Buffer* pCascadedViewProjCB;
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}
};


class D3D11ShadowManagerThread : public D3D11RenderThread
{
public:
	D3D11ShadowManagerThread();
	virtual ~D3D11ShadowManagerThread();

	virtual HRESULT Initial(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Render(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Update(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Destroy() override;

	void SetShadowDepthRenderParameter(ObjScene* pParameter,LightManager* pLightmanager,Camera* pCamera);

	ShadowData* GetShadowData(int id);
private:
	virtual void ThreadExcecute() override;
	
private:
	std::map<int, ShadowData*> m_dataArray;

	ShadowRenderParameter* m_shadowParameter;

	HRESULT InitialDirectionalLightShadow(DirectionalLightSahdow* pShadow);

	D3D11MVP m_mvp;

	//Shader
	D3D11Shader m_DirectionalLightShadowShader;

	ID3D11RasterizerState* m_RSCullBack;
	ID3D11DepthStencilState* m_DepthStencilState;
	
	ID3D11BlendState* m_pBlendState;

	ID3D11RasterizerState *m_directionLightRS;
private:

	void RenderObj();

	//shadow render
	void RenderDirectionalDepthTexture(DirectionalLightSahdow* pLightSource);

};
#endif // !_D3D11_SHADOW_MANAGER_THREAD_H_

