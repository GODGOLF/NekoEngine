#pragma once

#ifndef _DX_12_INCLUDE_H_
#define _DX_12_INCLUDE_H_            // Exclude rarely-used stuff from Windows headers.

#include<Windows.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <wrl.h>
#include <shellapi.h>
#include <vector>
#include <dxgi1_6.h>

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>

using namespace std;
using namespace DirectX;
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif
