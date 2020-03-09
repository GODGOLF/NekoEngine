

#ifndef _DX_INCLUDE_H_
#define _DX_INCLUDE_H_            // Exclude rarely-used stuff from Windows headers.

#include<Windows.h>
#include <d3d11_1.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <vector>

using namespace std;
using namespace DirectX;
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif
