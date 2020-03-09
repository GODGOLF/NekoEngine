#pragma once
#ifndef _MODEL_DATA_STRUCTURE_H_
#define _MODEL_DATA_STRUCTURE_H_
#include <DirectXMath.h>
#include <vector>

struct Material
{
	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT4 specularColor;
	DirectX::XMFLOAT3 texIndentify;
	char* diffuseTexture;
	char* normalTexture;
	float shiness;
};
struct MaterialInfo {
	Material mat;
	int id;
	int start;
	int count;
};
struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 tex;
	DirectX::XMFLOAT4 tangent;
};
struct VertexAnime : Vertex {
	DirectX::XMUINT4 bones;
	DirectX::XMFLOAT4 width;

};
struct BufferData {

	char texID;
	bool hasBone;
	std::vector<VertexAnime> vertrics;
	bool isTriangleStrip;
	DirectX::XMFLOAT3 tran;
	DirectX::XMFLOAT3 rotate;
	DirectX::XMFLOAT3 scale;
	DirectX::XMMATRIX DefaultMatrix;
	DirectX::XMMATRIX geometricMatrix;
	std::vector<MaterialInfo> material;
};
#endif // !_MODEL_DATA_STRUCTURE_H_
