Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

cbuffer ConstantBuffer : register(b0)
{
	matrix MVP;
	matrix projectMatrix;
	matrix worldMatrix;
	matrix viewMatrix;
	matrix worldInverse;

}
cbuffer ScreenSizeBuffer : register(b1)
{
	float screenHeight;
	float3 padding;
};
struct PSInput
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR0;
};

PSInput VSMain(float3 position : POSITION, float2 tex : TEXCOORD, float4 color :COLOR)
{
	PSInput output;
	float texelSize;
	output.position = float4(position, 1);
	output.tex = tex;
	output.color = color;
	return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float4 color = shaderTexture.Sample(SampleType, input.tex) * input.color;
	return color;
}

