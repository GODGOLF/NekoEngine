
#define PI 3.14159265359

struct MaterialPBR
{
	float3 normal;
	float3 diffuseColor;
	float  metallic;
	float roughness;
	float3 dirLight;
	float3 eyePosition;
	float intensity;
	float3 dirLightColor;
};

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


float3 CalLightPBR(float3 position, MaterialPBR material) {

	float3 N = material.normal;
	float3 V = normalize(material.eyePosition - position);
	float3 L = normalize(-material.dirLight);
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
	float3 Lo = (kD * albedo /PI+specular)*NDotL* material.dirLightColor * material.intensity;
	
	return Lo;
}