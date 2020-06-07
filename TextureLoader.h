#pragma once
#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include "DXInF.h"
namespace Texture {
	struct TextureRSV {
		ID3D11ShaderResourceView* texture = NULL;
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
	HRESULT LoadTexture(DXInF* device, const char* file, TextureRSV &output);
	HRESULT LoadTexture(DXInF* device, const char* file, TextureRSV &output, TextureInfo* outputInfo);
	HRESULT LoadTexture(const char* file, TextureData* output);
	//right, left, top, bottom, front, back.
	HRESULT GenTextureCubeMap(DXInF* device, std::vector<const char*> file, TextureRSV &res, bool mipLevel = false);
};
#endif // !_TEXTURE_H_

