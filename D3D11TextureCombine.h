#ifndef _D3D11_TEXTURE_COMBINE_INTERFACE_H_
#define _D3D11_TEXTURE_COMBINE_INTERFACE_H_
#include "TextureCombineInF.h"
#include "D3D11TextureRecord.h"
#include "D3D11Shader.h"
class D3D11TextureCombine :public TextureCombineInF
{
public:
	D3D11TextureCombine();
	virtual ~D3D11TextureCombine();
	HRESULT Initial(DXInF* pD3Device, int width, int height) override;
	void Render(void* pDeviceContext,
		void* textureInput1,
		void*textureInput2,
		void* depthInput1,
		void* depthInput2) override;
	void Destroy() override;
	void* GetOutput() override;
private:
	D3D11TextureRecord m_record;
	D3D11Shader m_shader;
	ID3D11SamplerState* m_pSamplerState;

	ID3D11Buffer * m_indexBuffer;
	ID3D11Buffer* m_vertBuffer;
};

#endif


