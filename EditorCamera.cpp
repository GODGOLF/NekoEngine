#include "EditorCamera.h"

using namespace DirectX;

EditorCamera::EditorCamera(int width, int height) 
	: PerspectiveCamera(width,height,45,XMFLOAT3(0,1,0), 
		XMFLOAT3(0, 1.7f, -1), XMFLOAT3(0, 1.7f, 1), 0.1f, 1000.0f)
{
	
}
EditorCamera::~EditorCamera()
{

}
void EditorCamera::Update()
{

}