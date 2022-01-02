//https://catlikecoding.com/unity/tutorials/flow/waves/
#include "common.fx"
Texture2D normalTexture 		: register(t0);
Texture2D roughnessTexture 		: register(t1);
Texture2D metalTexture			: register(t2);
SamplerState samLinear 			: register(s0);

#define FLT_MAX 3.402823e+38
#define WAVE_COUNT	3

cbuffer MaterialBufferPS : register(b2)
{
	float4 diffuseColor;
	float  metallic;
	float roughness;
	float textureScale;
	float tranparent;
	float shaderTypeID;
}

cbuffer FrustumCulling : register(b3)
{
	float4 frustumPlanes[6];
}
cbuffer cbPerFrame : register(b4)
{
	float gMaxTessDistance		: packoffset(c0.x);
	float gMinTessDistance		: packoffset(c0.y);
	float gMaxTessFactor		: packoffset(c0.z);
	float gMinTessFactor		: packoffset(c0.w);
	float3 gCameraPosition		: packoffset(c1);
	float gHeightScale			: packoffset(c1.w);
}
struct WaveData
{
	float2 direction;
	float waveLength;
	float steepness;
};
cbuffer OceanInfo : register(b5)
{
	float time	: packoffset(c0.x);
	float haveNormalTexture : packoffset(c0.y);
	float haveRoughnessTexture : packoffset(c0.z);
	float haveMetalTexture : packoffset(c0.w);
	WaveData data[3]: packoffset(c1);
}
struct HSInput
{
	float4 position 	: SV_POSITION;
	float3 normal 		: NORMAL;
	float2 tex 			: TEXCOORD0;
};
struct DSInput
{
	float4 position 	: SV_POSITION;
	float3 normal 		: NORMAL;
	float2 tex 			: TEXCOORD0;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};
HSInput VSMain(float3 position : POSITION,
	float3 normal : NORMAL,
	float2 tex : TEXCOORD)
{
	float4 vInputPos = float4(position, 1.f);
	float3 vNormal = normal.xyz;
	HSInput result;
	result.position = mul(vInputPos, worldMatrix);
	result.normal = mul(vNormal, (float3x3)worldInverse);
	result.normal = normalize(result.normal);
	result.tex = tex;
	
	float d = distance(result.position.xyz, gCameraPosition);
	
	return result;
}

struct PatchTess 
{ 
	float EdgeTess[4] : SV_TessFactor; 
	float InsideTess[2]  : SV_InsideTessFactor; 
}; 

float CalTessFactor(float3 p)
{
	float d = distance(p,gCameraPosition);
	float s  = saturate((d-gMinTessDistance)/ (gMaxTessDistance-gMinTessDistance));
	return pow(2,(lerp(gMaxTessFactor,gMinTessFactor,s)));
}
PatchTess PatchHS(InputPatch<HSInput,4> patch, uint patchID : SV_PrimitiveID) 
{
	PatchTess pt;
	// Average vertex tessellation factors along edges. 
	
	float3 e0 = 0.5f*(patch[0].position.xyz + patch[2].position.xyz);
	float3 e1 = 0.5f*(patch[0].position.xyz + patch[1].position.xyz);
	float3 e2 = 0.5f*(patch[1].position.xyz + patch[3].position.xyz);
	float3 e3 = 0.5f*(patch[2].position.xyz + patch[3].position.xyz);
	
	float3 c = 0.25f*(patch[0].position.xyz + patch[1].position.xyz + patch[2].position.xyz + patch[3].position.xyz);
	pt.EdgeTess[0] = CalTessFactor(e0);
	pt.EdgeTess[1] = CalTessFactor(e1);
	pt.EdgeTess[2] = CalTessFactor(e2);
	pt.EdgeTess[3] = CalTessFactor(e3);
	
	// Pick an edge tessellation factor for the interior tessellation. 
	pt.InsideTess[0] = CalTessFactor(c);
	pt.InsideTess[1] = pt.EdgeTess[0]; 
	
	return pt;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchHS")]
DSInput HSMain(InputPatch<HSInput,4> p, uint i : SV_OutputControlPointID,  uint PatchID : SV_PrimitiveID)
{
	DSInput Output;
	Output.position = p[i].position;
	Output.normal 	= p[i].normal;
	Output.tex 		= p[i].tex;
	return Output;
}
float3 GerstnerWave(float4 wave,float3 p, inout float3 tangent, inout float3 binormal)
{
	float2 direction = wave.xy;
	float waveLength = wave.z;
	float steepness = wave.w;
	float k = 2*PI/waveLength;
	float c = sqrt(9.8/k);
	float2 d = normalize(direction);
	float f = k*(dot(d,p.xz)- c *time);
	float a = steepness/k;
	
	tangent += float3(-d.x*d.x*(steepness*sin(f))
								,d.x*(steepness*cos(f))
								,-d.x*d.y*(steepness*sin(f)));
								
	binormal += float3(-d.x*d.y*(steepness*sin(f))
							,d.y*(steepness*cos(f))
							,-d.y*d.y*(steepness*sin(f)));
							
	return float3(d.x * (a * cos(f)),
			a * sin(f),
			d.y * (a*cos(f)));
	
}

[domain("quad")]
PSInput DSMain(PatchTess input, float2 uv : SV_DomainLocation, const OutputPatch<DSInput, 4> tri)
{
	PSInput Output;
	float3 v1 = lerp(tri[0].position.xyz,tri[1].position.xyz,uv.x);
	float3 v2 = lerp(tri[2].position.xyz, tri[3].position.xyz, uv.x);
	float3 p = lerp(v1, v2, uv.y);

	float2 t1 = lerp(tri[0].tex, tri[1].tex, uv.x);
	float2 t2 = lerp(tri[2].tex, tri[3].tex, uv.x);
	float2 t = lerp(t1, t2, uv.y);
	
	float3 tangent = float3(1,0,0);
	float3 binormal = float3(0,0,1);
	
	float3 tempPoint = p;
	for(int i =0;i<WAVE_COUNT;i++)
	{
		float4 waveDirectioData = 0.f;
		waveDirectioData.xy = data[i].direction;
		waveDirectioData.z = data[i].waveLength;
		waveDirectioData.w = data[i].steepness;
		p +=  GerstnerWave(waveDirectioData,tempPoint,tangent,binormal);
	}
	
	Output.position = mul(float4(p,1.f), viewMatrix);
	Output.position = mul(Output.position, projectMatrix);
	Output.tex = t;

	
	Output.normal = normalize(cross(binormal,tangent));
	Output.tangent = tangent;
	Output.binormal = binormal;
	
	return Output;
}



struct PS_GBUFFER_OUT
{
	float4 ColorSpecInt : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 SpecPow : SV_TARGET2;
};
PS_GBUFFER_OUT PackGBuffer(float4 BaseColor, float3 Normal, float metalValue, float roughnessValue)
{
	PS_GBUFFER_OUT Out;
	// Pack all the data into the GBuffer structure
	if(tranparent == 1.f)
	{
		Out.ColorSpecInt = BaseColor;
	}
	else
	{
		Out.ColorSpecInt = float4(BaseColor.rgb, 1.0f);
	}
	Out.Normal = float4(Normal * 0.5 + 0.5, 1.0);
	Out.SpecPow = float4(0, metalValue, roughnessValue, shaderTypeID);
	return Out;
}

PS_GBUFFER_OUT PSMain(PSInput input) : SV_TARGET
{
	PS_GBUFFER_OUT output;
	float4 color;
	float4 diffuse = diffuseColor;
	float3 normal = input.normal;
	if(haveNormalTexture == 1.f)
	{
		float3 normalVector = normalTexture.Sample(samLinear, (input.tex*textureScale)+ (time*0.01)).xyz;
		float3 normal2Vector = normalTexture.Sample(samLinear, (input.tex*textureScale)- (time*0.01)).xyz;
		//normalVector = normalVector + normal2Vector;
		//normalVector = normalize(normalVector);
		//Change normal map range from [0, 1] to [-1, 1]
		normalVector = (2.0f*normalVector) - 1.0f;
		
		
		normal = (normalVector.x * input.tangent) + (normalVector.y * input.binormal) + (normalVector.z * input.normal);
		
		
		normal = normalize(normal);
	}
	float roughnessValue = roughness;
	float metalValue = metallic;
	if(haveRoughnessTexture == 1.0f)
	{
		roughnessValue = roughnessTexture.Sample(samLinear, (input.tex*textureScale) + (time*0.01)).r;
	}
	if(haveMetalTexture == 1.0f)
	{
		metalValue = metalTexture.Sample(samLinear, (input.tex*textureScale) +(time*0.01)).r;
	}
	
	output = PackGBuffer(diffuse, normal,metalValue,roughnessValue);
	return output;
}

