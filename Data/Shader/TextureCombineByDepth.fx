Texture2D shaderTexture1 : register(t0);
Texture2D shaderTexture2 : register(t1);
Texture2D depthTexture1 :  register(t2);
Texture2D depthTexture2 :  register(t3);
SamplerState SampleType : register(s0);

struct PSInput
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

PSInput VSMain(float3 position : POSITION, float2 tex : TEXCOORD)
{
	PSInput output;
	float texelSize;
	output.position = float4(position, 1);
	output.tex = tex;
	return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float4 color1 = shaderTexture1.Sample(SampleType, input.tex);
	float4 color2 = shaderTexture2.Sample(SampleType, input.tex);
	float depth1 = depthTexture1.Sample(SampleType,input.tex).x;
	float depth2 = depthTexture2.Sample(SampleType,input.tex).x;
	
	float4 color = 0;
	if(depth1 <= depth2)
	{
		color = color1;
	}
	else
	{
		color = float4(lerp(color1.xyz,color2.xyz,color2.w),1.f);
	}
	color = float4(lerp(color1.xyz, color2.xyz, color2.w), 1.f);
	//color = color1;
	return color;
}

