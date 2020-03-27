#include "common.fx"
Texture2D<float> DepthTexture					: register(t0);
Texture2D<float4> ColorSpecIntTexture			: register(t1);
Texture2D<float3> NormalTexture					: register(t2);
Texture2D<float4> SpecPowTexture				: register(t3);
SamplerState PointSampler						: register(s0);					

cbuffer cbDirLight : register(b2)
{
	float3 DirToLight			: packoffset(c0);
	float4 DirLightColor		: packoffset(c1);
	float intensity				: packoffset(c2.x);
}

struct VS_OUTPUT
{
	float4 position : SV_POSITION; // vertex position 
	float2 tex : TEXCOORD0;
	float2 cpPos	: TEXCOORD1;
};

static const float3 diffuseConeDirections[] =
{
	float3(0.0f, 1.0f, 0.0f),
	float3(0.0f, 0.5f, 0.866025f),
	float3(0.823639f, 0.5f, 0.267617f),
	float3(0.509037f, 0.5f, -0.7006629f),
	float3(-0.50937f, 0.5f, -0.7006629f),
	float3(-0.823639f, 0.5f, 0.267617f)
};
static const float diffuseConeWeights[] =
{
	PI / 4.0f,
	3.0f * PI / 20.0f,
	3.0f * PI / 20.0f,
	3.0f * PI / 20.0f,
	3.0f * PI / 20.0f,
	3.0f * PI / 20.0f,
};

/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////
VS_OUTPUT VSMain(float3 position : POSITION, float2 tex : TEXCOORD)
{
	VS_OUTPUT output;

	float texelSize;
	output.position = float4(position, 1);
	output.tex = tex;
	output.cpPos = output.position.xy;
	return output;
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shaders
/////////////////////////////////////////////////////////////////////////////
//PBR function
float DistributionGGX(float3 N, float3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0f);
	float NdotH2 = NdotH * NdotH;
	float nom = a2;
	float denom = (NdotH2 *(a2 - 1.0f) + 1.0f);
	denom = PI * denom*denom;
	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0f);
	float k = (r*r) / 8.0f;
	float num = NdotV;
	float denom = NdotV * (1.f - k) + k;
	return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float k) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);
	return ggx1 * ggx2;
}
float3 fresnelSchlick(float cosTheta, float3 F0)
{
	cosTheta = min(cosTheta,1.0f);
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	cosTheta = min(cosTheta, 1.0f);
	float invertRoughness = 1.0 - roughness;
	float3 r = invertRoughness;
	return F0 + (max(r, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
half3 EnvBRDFApprox(half3 SpecularColor, half Roughness, half NoV)
{
	const half4 c0 = { -1, -0.0275, -0.572, 0.022 };
	const half4 c1 = { 1, 0.0425, 1.04, -0.04 };
	half4 r = Roughness * c0 + c1;
	half a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
	half2 AB = half2(-1.04, 1.04) * a004 + r.zw;
	return SpecularColor * AB.x + AB.y;
}


// Directional light with PBR
float3 CalcDirectionalLightPBR(float3 position, Material material) {
	// Phong diffuse
	
	float3 N = material.normal;
	float3 V = normalize(EyePosition - position);
	float3 L = normalize(DirToLight);
	float3 H = normalize(V + L);
	float3 R = reflect(-V, N);
	

	//cook-torrance brdf
	float NDF = DistributionGGX(N, H, material.roughness);
	float G = GeometrySmith(N, V, L, material.roughness);
	//use diffuseColor as albedo
	float3 albedo = material.diffuseColor;
	float3 F0 = float3(0.04f,0.04f,0.04f);
	F0 = lerp(F0, albedo, material.metallic);
	float3 F = fresnelSchlick(clamp(dot(H, V), 0.0f,1.0f), F0);
	float3 kS = F;
	float3 kD = float3(1.0f,1.0f,1.0f)- kS;
	//kD *= 1.0 - material.metallic;

	float3 numerator = NDF * G * F;
	float denominator = 4.0f*max(dot(N, V), 0.0f)*max(dot(N, L), 0.0f);
	float3 specular = numerator / max(denominator, 0.001);

	//add to outgoing radiance Lo
	float NDotL = max(dot(N,L),0);
	float3 Lo = (kD * albedo /PI+specular)*NDotL* DirLightColor * intensity;
	

	return Lo;
}

float4 DirectionalLightCommand(VS_OUTPUT In, bool useShadow)
{
	// Unpack the GBuffer
	SURFACE_DATA gbd = UnpackGBuffer_Loc(In.position.xy,
		DepthTexture,
		ColorSpecIntTexture,
		NormalTexture,
		SpecPowTexture,
		PointSampler);

	//// Convert the data into the material structure
	Material mat;
	MaterialFromGBuffer(gbd, mat);

	// Reconstruct the world position
	float3 position = CalcWorldPos(In.cpPos, gbd.LinearDepth);
	
	// Calculate the directional light
	float3 finalColor = CalcDirectionalLightPBR(position, mat) ;
	
	//gamma correction
	float3 gammarValue = 1.0 / 2.2;
	finalColor = max(pow(finalColor, gammarValue),0.0f);
	
	// Return the final color
	return float4(finalColor, 1.0f);
}
float4 PSMain(VS_OUTPUT In) : SV_TARGET
{
	return DirectionalLightCommand(In, false);
}


