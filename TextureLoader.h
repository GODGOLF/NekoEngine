#pragma once
#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include "DXInF.h"
namespace Texture {
	struct TextureRSV {
		ID3D11ShaderResourceView* texture;
		TextureRSV() : texture(NULL) {};
	};
	struct TextureData {
		int width;
		int height;
		std::vector<XMFLOAT4> data;
		DXGI_FORMAT format;
	};
	struct TextureInfo {
		int width;
		int height;
		DXGI_FORMAT format;
	};
	HRESULT LoadTexture(DXInF* device, const char* file, TextureRSV &res);
	HRESULT LoadTexture(DXInF* device, const char* file, TextureRSV &res, TextureInfo* output);
	HRESULT LoadTexture(const char* file, TextureData* output);
	HRESULT GenTextureCubeMap(DXInF* device, std::vector<const char*> file, TextureRSV &res, bool mipLevel = false);
};
#endif // !_TEXTURE_H_

