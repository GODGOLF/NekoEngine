#include "DXInclude.h"
#include "TextureLoader.h"
#include <DirectXTex.h>
#include "FunctionHelper.h"
#include "D3D11Class.h"
//https://stackoverflow.com/questions/19364012/d3d11-creating-a-cube-map-from-6-images
//http://richardssoftware.net/Home/Post/26

using namespace DirectXHelper;

HRESULT Texture::LoadTexture(DXInF* device, const char* file, TextureRSV &output) {
	HRESULT hr = S_OK;
	TexMetadata metadata;
	DirectX::ScratchImage image;

	hr = LoadFromWICFile(ConvertStringToWstring(string(file)).c_str(), 0, &metadata, image);
	if (FAILED(hr)) {
		std::string sFile(file);
		sFile += " Failed to download picture\n";
		OutputDebugString(sFile.c_str());
		return hr;
	}
	D3D11Class* directX = (D3D11Class*)device;
	hr = CreateShaderResourceView(directX->GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &output.texture);
	image.Release();
	if (FAILED(hr)) {
		std::string sFile(file);
		sFile += " Failed to create shader Resource\n";
		OutputDebugString(sFile.c_str());
		return hr;
	}
	return hr;
}
HRESULT Texture::LoadTexture(const char* file,
	TextureData* output) {
	HRESULT hr = S_OK;
	TexMetadata metadata;
	ScratchImage image;
	hr = LoadFromWICFile(ConvertStringToWstring(string(file)).c_str(), 0, &metadata, image);
	if (FAILED(hr)) {
		std::string sFile(file);
		sFile += " Failed to download picture\n";
		OutputDebugString(sFile.c_str());
		return hr;
	}
	output->width = metadata.width;
	output->height = metadata.height;
	output->format = metadata.format;
	int next = 0;
	for (int i = 0; i < (int)image.GetPixelsSize(); i += next) {
		switch (metadata.format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		{
			next = 4;
			XMFLOAT4 data = XMFLOAT4(image.GetPixels()[i],
				image.GetPixels()[i + 1],
				image.GetPixels()[i + 2],
				image.GetPixels()[i + 3]);
			output->data.push_back(data);
		}
		break;
		case DXGI_FORMAT_R16_UNORM:
		{
			next = 2;
			XMFLOAT4 data = XMFLOAT4(image.GetPixels()[i],
				image.GetPixels()[i + 1],
				0,
				0);
			output->data.push_back(data);
		}
		break;
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:

		{
			next = 1;
			XMFLOAT4 data = XMFLOAT4(image.GetPixels()[i],
				0,
				0,
				0);
			output->data.push_back(data);
		}
		break;
		default:
			break;
		}
		if (next == 0) {
			return S_FALSE;
		}
	}
	image.Release();
	return hr;
}
HRESULT Texture::LoadTexture(DXInF* device, const char* file, TextureRSV &res, TextureInfo* output)
{
	HRESULT hr = S_OK;
	TexMetadata metadata;
	DirectX::ScratchImage image;

	hr = LoadFromWICFile(ConvertStringToWstring(string(file)).c_str(), 0, &metadata, image);

	if (FAILED(hr)) {
		std::string sFile(file);
		sFile += " Failed to download picture\n";
		OutputDebugString(sFile.c_str());
		return hr;
	}
	output->height = metadata.height;
	output->width = metadata.width;
	output->format = metadata.format;
	D3D11Class* directX = (D3D11Class*)device;
	hr = CreateShaderResourceView(directX->GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &res.texture);
	image.Release();
	if (FAILED(hr)) {
		std::string sFile(file);
		sFile += " Failed to download picture\n";
		OutputDebugString(sFile.c_str());
		return hr;
	}
	return hr;
}
//right, left, top, bottom, front, back.
HRESULT Texture::GenTextureCubeMap(DXInF* device, std::vector<const char*> file, TextureRSV &res, bool haveMipLevel) {
	HRESULT hr = S_OK;
	ID3D11Resource* srcTex[6];
	TexMetadata metadata;
	D3D11Class* directX = (D3D11Class*)device;
	DirectX::ScratchImage image;
	for (int i = 0; i < 6; i++) {
		hr = LoadFromWICFile(ConvertStringToWstring(std::string(file[i])).c_str(), 0, &metadata, image);
		if (FAILED(hr)) {
			std::string sFile(file[i]);
			sFile += " Failed to download picture\n";
			OutputDebugString(sFile.c_str());
			image.Release();
			return hr;
		}
		ID3D11ShaderResourceView* data = NULL;
		hr = CreateShaderResourceView(directX->GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &data);
		if (FAILED(hr)) {
			OutputDebugString("Failed to create resource\n");
			return hr;
		}
		data->GetResource(&srcTex[i]);
		if (data != NULL) {
			data->Release();
		}
		image.Release();
	}
	D3D11_TEXTURE2D_DESC texElementDesc;
	((ID3D11Texture2D*)srcTex[0])->GetDesc(&texElementDesc);

	int mipLevels = 0;
	if (haveMipLevel) {
		UINT width = texElementDesc.Width;
		UINT height = texElementDesc.Height;
		while (true)
		{
			if (width == 0 || height == 0)
				break;
			width = width >> 1;
			height = height >> 1;
			mipLevels++;
		}
	}
	else {
		mipLevels = 1;
	}
	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = texElementDesc.Width;
	texArrayDesc.Height = texElementDesc.Height;
	texArrayDesc.MipLevels = mipLevels;
	texArrayDesc.ArraySize = 6;
	texArrayDesc.Format = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;

	texArrayDesc.CPUAccessFlags = 0;

	if (mipLevels >1) {
		texArrayDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texArrayDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}
	else {
		texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texArrayDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	}

	ID3D11Texture2D* texArray = 0;
	if (FAILED(directX->GetDevice()->CreateTexture2D(&texArrayDesc, 0, &texArray))) {
		return false;
	}
	// Copy individual texture elements into texture array.
	ID3D11DeviceContext* pd3dContext;
	directX->GetDevice()->GetImmediateContext(&pd3dContext);
	D3D11_BOX sourceRegion;
	//Here i copy the mip map levels of the textures
	for (UINT x = 0; x < 6; x++)
	{
		sourceRegion.left = 0;
		sourceRegion.right = texArrayDesc.Width;
		sourceRegion.top = 0;
		sourceRegion.bottom = texArrayDesc.Height;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		//test for overflow
		if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
			break;

		pd3dContext->CopySubresourceRegion(texArray, D3D11CalcSubresource(0, x, texArrayDesc.MipLevels), 0, 0, 0, srcTex[x], 0, &sourceRegion);
	}
	// Create a resource view to the texture array.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	viewDesc.TextureCube.MostDetailedMip = 0;
	viewDesc.TextureCube.MipLevels = texArrayDesc.MipLevels;

	if (FAILED(directX->GetDevice()->CreateShaderResourceView(texArray, &viewDesc, &res.texture)))
		return false;

	for (UINT x = 0; x < 6; x++)
	{
		if (srcTex[x] != NULL) {
			srcTex[x]->Release();
		}
	}
	if (texArray) {
		texArray->Release();
	}
	if (pd3dContext) {
		pd3dContext->Release();
	}
	//genertate mipmap
	if (haveMipLevel) {
		pd3dContext->GenerateMips(res.texture);
	}
	return false;
}