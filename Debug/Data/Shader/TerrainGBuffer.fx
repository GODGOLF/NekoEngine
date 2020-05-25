//https://www.google.com/search?q=translate+linear+algebra&rlz=1C1CHBF_enTH716TH716&oq=translate+linear+algebra&aqs=chrome..69i57j0l3j69i64l2.212j0j7&sourceid=chrome&ie=UTF-8
//https://study.com/academy/lesson/the-gram-schmidt-process-for-orthonormalizing-vectors.html
//https://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal
#include "common.fx"
Texture2D txDiffuse : register(t0);
Texture2D txHeightMap : register(t1);
SamplerState samLinear : register(s0);

#define FLT_MAX 3.402823e+38

cbuffer MaterialBufferPS : register(b2)
{
	float4 diffuseColor;
	float  metallic;
	float roughness;
	float textureScale;
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
cbuffer heightInfo : register(b5)
{
	float texCellSpaceU;
	float texCellSpaceV;
	float worldCellSpace;
	float haveTextureDiffuse;
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
	//float3 tangent : TANGENT;
	//float3 binormal : BINORMAL;
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
					  
	Output.normal = float3(0,1,0);
	
	float h = txHeightMap.SampleLevel(samLinear,t,0).r;
	p += (gHeightScale * h)* Output.normal;
	
	Output.position = mul(float4(p,1.f), viewMatrix);
	Output.position = mul(Output.position, projectMatrix);
	Output.tex = t;
	
	return Output;
}


bool FrustumCulling(float3 position, float3 extent)
{
	for(int i=0;i<6;i++)
	{
		float4 plane = frustumPlanes[i];
		float d = dot(extent, abs(plane.xyz));
		float r = dot(position, plane.xyz) + plane.w;
		if((d+r) < 0)
		{
			return false;
		}
	}
	return true;
}
float3 FindCenterTriangle(float3 point0, float3 point1,float3 point2)
{
	//find center
	float3 center;
	center.x = (point0.x + point1.x + point2.x) /3.f;
	center.y = (point0.y + point1.y + point2.y) /3.f;
	center.z = (point0.z + point1.z + point2.z) /3.f;
	return center;
}
float FindRadianFromTriangle(float3 point0, float3 point1,float3 point2, float3 centerPoint)
{
	//first we have to find min,max value
	float radius =0;
	
	float l = abs(length(point0 - centerPoint));
	if(l>radius)
	{
		radius = l;
	}
	l = abs(length(point1 - centerPoint));
	if(l>radius)
	{
		radius = l;
	}
	l = abs(length(point2 - centerPoint));
	if(l>radius)
	{
		radius = l;
	}
	
	return radius;
}
struct PS_GBUFFER_OUT
{
	float4 ColorSpecInt : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 SpecPow : SV_TARGET2;
};
PS_GBUFFER_OUT PackGBuffer(float3 BaseColor, float3 Normal)
{
	PS_GBUFFER_OUT Out;

	// Pack all the data into the GBuffer structure
	Out.ColorSpecInt = float4(BaseColor.rgb, 1.0f);
	Out.Normal = float4(Normal * 0.5 + 0.5, 1.0);
	Out.SpecPow = float4(0, metallic, roughness, 1.0);
	return Out;
}

PS_GBUFFER_OUT PSMain(PSInput input) : SV_TARGET
{
	//calculate normal
	float2 leftTex = input.tex+float2(-texCellSpaceU,0.f);
	float2 rightTex = input.tex+float2(texCellSpaceU,0.f);
	float2 bottomTex = input.tex+float2(0.f,texCellSpaceV);
	float2 topTex = input.tex+float2(0.f,texCellSpaceV);
	
	float leftY = txHeightMap.SampleLevel(samLinear,leftTex,0).r *gHeightScale;
	float rightY = txHeightMap.SampleLevel(samLinear,rightTex,0).r*gHeightScale;
	float bottomY = txHeightMap.SampleLevel(samLinear,bottomTex,0).r*gHeightScale;
	float topY = txHeightMap.SampleLevel(samLinear,topTex,0).r*gHeightScale;
	
	float3 tangent = normalize(float3(2.0f*worldCellSpace,rightY-leftY,0.f));
	float3 bitangent = normalize(float3(0.f,bottomY-topY,-2.0f*worldCellSpace));
	
	float3 normal = cross(tangent,bitangent);
	PS_GBUFFER_OUT output;
	float4 color;
	float4 diffuse = diffuseColor;
	if(haveTextureDiffuse)
	{
		diffuse *= txDiffuse.Sample(samLinear, input.tex*textureScale);
	}
	normal = normalize(normal);
	output = PackGBuffer(diffuse.xyz, normal);
	return output;
}

