#include "common.fx"
Texture2D txDiffuse : register(t0);
Texture2D ObjNormMap: register(t1);
Texture2D ObjDisplacementMap : register(t2);
TextureCube txDiffuseCubeType : register(t3);
SamplerState samLinear : register(s0);

#define FLT_MAX 3.402823e+38

cbuffer cbPerMesh : register(b2)
{
	matrix g_mConstBoneWorld[MAX_BONE_MATRICES];
};

cbuffer MaterialBufferPS : register(b3)
{
	float4 diffuseColor;
	float4 specularColor;
	float3 haveTexture;
	float specExp;
	float  metallic;
	float roughness;
	float shaderTypeID;
}

cbuffer FrustumCulling : register(b4)
{
	float4 frustumPlanes[6];
}
cbuffer cbPerFrame : register(b5)
{
	float gMaxTessDistance		: packoffset(c0.x);
	float gMinTessDistance		: packoffset(c0.y);
	float gMaxTessFactor		: packoffset(c0.z);
	float gMinTessFactor		: packoffset(c0.w);
	float3 gCameraPosition		: packoffset(c1);
	float gHeightScale			: packoffset(c1.w);
	float haveDisplacementMap	: packoffset(c2.x);
}
struct HSInput
{
	float4 position 	: SV_POSITION;
	float3 normal 		: NORMAL;
	float2 tex 			: TEXCOORD0;
	float3 tangent 		: TANGENT;
	float3 binormal		: BINORMAL;
	float  tessFactor  		: TESS;
};
struct DSInput
{
	float4 position 	: SV_POSITION;
	float3 normal 		: NORMAL;
	float2 tex 			: TEXCOORD0;
	float3 tangent 		: TANGENT;
	float3 binormal		: BINORMAL;
};
struct GSInput
{
	float4 position 	: SV_POSITION;
	float3 normal 		: NORMAL;
	float2 tex 			: TEXCOORD0;
	float3 tangent 		: TANGENT;
	float3 binormal		: BINORMAL;
};


struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 wPos : TEXCOORD1;
};
HSInput VSMain(float3 position : POSITION,
	float3 normal : NORMAL,
	float2 tex : TEXCOORD,
	float4 tangent : TANGENT,
	float4 binormal : BINORMAL,
	uint4 bone : BONEINDICES,
	float4 weight : WEIGHTS)
{
	float4 vInputPos = float4(position, 1.f);
	float4 vWorldPos = float4(0.f, 0.f, 0.f, 0.f);
	float3 vNormal = normal.xyz;
	
	if (bone.x == -1) {
		vWorldPos = vInputPos;
	}
	else {
		matrix boneTransform = g_mConstBoneWorld[bone.x] * weight.x;
		boneTransform += g_mConstBoneWorld[bone.y] * weight.y;
		boneTransform += g_mConstBoneWorld[bone.z] * weight.z;
		boneTransform += g_mConstBoneWorld[bone.w] * weight.w;
		vWorldPos = mul(vInputPos, boneTransform);
		vNormal = mul(normal, (float3x3)boneTransform);
	}
	HSInput result;
	result.position = mul(vWorldPos, worldMatrix);
	result.normal = mul(vNormal, (float3x3)worldInverse);
	result.normal = normalize(result.normal);
	result.tex = tex;
	//tangent 
	result.tangent = mul(tangent.xyz, (float3x3)worldInverse);
	result.binormal = mul(binormal.xyz,(float3x3)worldInverse);
	
	float d = distance(result.position.xyz, gCameraPosition);
	
	//0 if d>= gMinTessDistance and 1 if d<=gMaxTessDistance
	float tess = saturate((gMinTessDistance -d) / (gMinTessDistance - gMaxTessDistance));
	//invern
	tess = 1.0f - tess;	
	result.tessFactor = gMinTessFactor + tess * (gMaxTessFactor - gMinTessFactor);
	return result;
}

struct PatchTess 
{ 
	float EdgeTess[3] : SV_TessFactor; 
	float InsideTess  : SV_InsideTessFactor; 
}; 
PatchTess PatchHS(InputPatch<HSInput,3> patch, uint patchID : SV_PrimitiveID) 
{
	PatchTess pt;
	// Average vertex tessellation factors along edges. 
	pt.EdgeTess[0] = 0.5f*(patch[1].tessFactor + patch[2].tessFactor); 
	pt.EdgeTess[1] = 0.5f*(patch[2].tessFactor + patch[0].tessFactor); 
	pt.EdgeTess[2] = 0.5f*(patch[0].tessFactor + patch[1].tessFactor); 
	
	// Pick an edge tessellation factor for the interior tessellation. 
	pt.InsideTess = pt.EdgeTess[0]; 
	return pt;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
DSInput HSMain(InputPatch<HSInput,3> p, uint i : SV_OutputControlPointID,  uint PatchID : SV_PrimitiveID)
{
	DSInput Output;
	
	Output.position = p[i].position;
	Output.normal 	= p[i].normal;
	Output.tex 		= p[i].tex;
	Output.tangent 	= p[i].tangent;
	Output.binormal	= p[i].binormal;
	return Output;
}
[domain("tri")]
GSInput DSMain(PatchTess input, float3 location : SV_DomainLocation, const OutputPatch<DSInput, 3> tri)
{
	GSInput Output;
	Output.position = location.x *tri[0].position + 
					  location.y *tri[1].position +
					  location.z *tri[2].position;
	Output.normal 	= location.x *tri[0].normal + 
					  location.y *tri[1].normal +
					  location.z *tri[2].normal;
	Output.tex 		= location.x *tri[0].tex + 
					  location.y *tri[1].tex +
					  location.z *tri[2].tex;
	Output.tangent 	= location.x *tri[0].tangent + 
					  location.y *tri[1].tangent +
					  location.z *tri[2].tangent;
	Output.binormal	= location.x *tri[0].binormal + 
					  location.y *tri[1].binormal +
					  location.z *tri[2].binormal;
					  
	Output.normal = normalize(Output.normal);
	
	if(haveDisplacementMap == 1)
	{
		float h = ObjDisplacementMap.SampleLevel(samLinear,Output.tex,0).r;
		Output.position.xyz += (gHeightScale * (h-1.0f))*Output.normal;
	}
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
[maxvertexcount(3)]
void GSMain(triangle GSInput input[3], inout TriangleStream<PSInput> triStream)
{
	float3 center = FindCenterTriangle(input[0].position.xyz,input[1].position.xyz,input[2].position.xyz);
	float3 radius = FindRadianFromTriangle(input[0].position.xyz,input[1].position.xyz,input[2].position.xyz,center);
	
	bool isInside = FrustumCulling(center,radius);

	if(!isInside)
	{
		return;
	}
	PSInput output;
	for (int j = 0; j < 3; ++j)
	{
		output.position = mul(input[j].position, viewMatrix);
		output.position = mul(output.position, projectMatrix);
		output.normal =  input[j].normal;
		output.tex = input[j].tex;
		output.tangent = input[j].tangent;
		output.binormal = input[j].binormal;
		output.wPos = output.position.xyz;
		triStream.Append(output);
		
	}
	triStream.RestartStrip();
}
struct PS_GBUFFER_OUT
{
	float4 ColorSpecInt : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 SpecPow : SV_TARGET2;
};
PS_GBUFFER_OUT PackGBuffer(float3 BaseColor, float3 Normal, float SpecPower)
{
	PS_GBUFFER_OUT Out;

	// Normalize the specular power
	float SpecPowerNorm = max(0.0001, (SpecPower - g_SpecPowerRange.x) / g_SpecPowerRange.y);

	// Pack all the data into the GBuffer structure
	Out.ColorSpecInt = float4(BaseColor.rgb, 1.0f);
	Out.Normal = float4(Normal * 0.5 + 0.5, 1.0);
	Out.SpecPow = float4(SpecPowerNorm, metallic, roughness, shaderTypeID);
	return Out;
}

PS_GBUFFER_OUT PSMain(PSInput input) : SV_TARGET
{
	PS_GBUFFER_OUT output;
	float4 color;
	float4 textureColor = diffuseColor;
	float3 newNormal = 0;
	if (haveTexture.x == 1) {
		textureColor = diffuseColor*txDiffuse.Sample(samLinear, input.tex);
	}
	//cube map for skyybox
	else if(haveTexture.y == 2)
	{
		textureColor = diffuseColor*txDiffuseCubeType.Sample(samLinear, input.wPos);
	}
	if(textureColor.w == 0.f)
	{
		discard;
	}
	//If material has a normal map, we can compute it now
	if (haveTexture.y != 0)
	{
		//Load normal from normal map
		float4 normalMap = ObjNormMap.Sample(samLinear, input.tex);

		//Change normal map range from [0, 1] to [-1, 1]
		normalMap = (2.0f*normalMap) - 1.0f;

		newNormal = (normalMap.x* input.tangent) + (normalMap.y* input.binormal) + (normalMap.z * input.normal);
	
		newNormal = normalize(newNormal);
	
	}
	else
	{
		newNormal= normalize(input.normal);
	}
	output = PackGBuffer(textureColor.xyz, newNormal, specExp);
	return output;
}

