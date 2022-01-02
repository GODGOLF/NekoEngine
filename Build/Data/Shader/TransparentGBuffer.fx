Texture2D<float4> txDiffuse : register(t0);
Texture2D<float3> txNormalMap: register(t1);
Texture2D<float4> txDataMap : register(t2);
Texture2D<float> txDepth : register(t3);
Texture2D<float> txDepthNonTransparent : register(t4);

RWTexture3D<unorm float4> txDiffuse3D : register(u0);
RWTexture3D<float3> txNormal3D : register(u1);
RWTexture3D<unorm float4> txData3D : register(u2);
RWTexture3D<float> txDepth3D : register(u3);

#define LAYER 18

cbuffer TransparentCB :register(b0)
{
	uint2 res : packoffset(c0);
}

[numthreads(1024, 1, 1)]
void CSMain(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint2 curPixel = uint2(dispatchThreadID.x % res.x, dispatchThreadID.x / res.x);
	if (curPixel.y < res.y)
	{
		float4 diffuseColor = txDiffuse[curPixel];
		if (diffuseColor.w == 0)
		{
			return;
		}
		float nonTransDepthTex = txDepthNonTransparent[curPixel].r;
		float curDepthTex = txDepth[curPixel];
		float3 curNormalTex = txNormalMap[curPixel];
		float4 curDataMap = txDataMap[curPixel];
		if (curDepthTex > nonTransDepthTex)
		{
			return;
		}
		
		for (int i = 0; i < LAYER; i++)
		{
			if (curDepthTex == 0.f)
			{
				break;
			}
			uint3 pos = uint3(curPixel, i);
			//get cur data from Texture
			float curDepthLayer = txDepth3D[pos].r;
			float4 diffuseColorLayer = txDiffuse3D[pos];
			float4 dataLayer = txData3D[pos];
			float3 normalColorLayer = txNormal3D[pos];
			//compare
			if (curDepthTex < curDepthLayer)
			{
				//store data
				txDepth3D[pos] = curDepthTex;
				txDiffuse3D[pos] = diffuseColor;
				txData3D[pos] = curDataMap;
				txNormal3D[pos] = curNormalTex;
				//save prev data
				curDepthTex = curDepthLayer;
				diffuseColor = diffuseColorLayer;
				curNormalTex = normalColorLayer;
				curDataMap = dataLayer;
			}
			
		}
	}
}

