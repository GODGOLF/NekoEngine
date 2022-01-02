#pragma once
#ifndef _POST_FX_H_
#define _POST_FX_H_
#include "PostFXInF.h"
struct PostFXIntialParameter : public PostFXParameter
{

};
struct PostFXComputeParameter : public PostFXParameter
{
	void* input;
	void* output;
};
class PostFX : public PostFXInF
{
	virtual HRESULT Initial(DXInF* pDevice, PostFXParameter* pParameter = nullptr) override;
	virtual void Compute(DXInF* pDevice, PostFXParameter* pParameter = nullptr) override;
	virtual void Destroy() override;
};

#endif
