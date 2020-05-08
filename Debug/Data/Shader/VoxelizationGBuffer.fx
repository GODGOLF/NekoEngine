//https://jose-villegas.github.io/post/deferred_voxel_shading/
//https://developer.nvidia.com/content/basics-gpu-voxelization
//http://www.ambrsoft.com/TrigoCalc/Plan3D/Plane3D_.htm
#include "common.fx"
Texture2D txDiffuse : register(t0);
Texture2D ObjNormMap: register(t1);
SamplerState samLinear : register(s0);
RWTexture3D<float4> baseColor : register(u3);
RWTexture3D<float4> normalColor : register(u4);
RWTexture3D<float4> emissionColor : register(u5);

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
}
cbuffer VoxelCB : register(b4)
{
	matrix VP[3];	
	matrix VPI[3];
	float3 worldMinPoint;
	float voxelScale;
	uint volumeDimension;
}
struct GSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	float4 tangent : TANGENT;
};
struct PSInput
{
	float4 position : SV_POSITION;
	float3 wPosition : TEXCOORD0;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD1;
	float4 tangent : TANGENT;
	float3 wsPosition : TEXCOORD2;
	float4 triangleAABB : TEXCOORD3;
};
GSInput VSMain(float3 position : POSITION,
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
	else 
	{
		matrix boneTransform = g_mConstBoneWorld[bone.x] * weight.x;
		boneTransform += g_mConstBoneWorld[bone.y] * weight.y;
		boneTransform += g_mConstBoneWorld[bone.z] * weight.z;
		boneTransform += g_mConstBoneWorld[bone.w] * weight.w;
		vWorldPos = mul(vInputPos, boneTransform);
		vNormal = mul(normal, (float3x3)boneTransform);
	}
	GSInput result;
	result.position = mul(vWorldPos, worldMatrix);
	result.normal = mul(vNormal, (float3x3)worldInverse);
	result.normal = normalize(result.normal);
	result.tex = tex;
	//tangent 
	result.tangent = mul(tangent, worldInverse);


	return result;
}
int CalculateAxis(GSInput input[3])
{
	//calculate edge vectors in voxel coordinate space
	const float3 p1 = input[1].position.xyz - input[0].position.xyz;
	const float3 p2 = input[2].position.xyz - input[0].position.xyz;
	const float3 p = cross(p1, p2);
	
	float nDX = abs(p.x);
	float nDY = abs(p.y);
	float nDZ = abs(p.z);

	if (nDX > nDY && nDX > nDZ)
	{
		return 0;
	}
	else if (nDY > nDX && nDY > nDZ)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}
float4 AxisAlignedBoundingBox(float4 pos[3], float2 pixelDiagonal)
{
	float4 aabb;

	aabb.xy = min(pos[2].xy, min(pos[1].xy, pos[0].xy));
	aabb.zw = max(pos[2].xy, max(pos[1].xy, pos[0].xy));

	// enlarge by half-pixel
	aabb.xy -= pixelDiagonal;
	aabb.zw += pixelDiagonal;

	return aabb;
}

[maxvertexcount(3)]
void GSMain(triangle GSInput input[3], inout TriangleStream<PSInput> triStream)
{
	PSInput output;
	int selectedIndex = CalculateAxis(input);
	matrix viewProjection = VP[selectedIndex];
	matrix viewProjectionI = VPI[selectedIndex];

	float4 pos[3];
	pos[0] = mul(input[0].position,viewProjection);
	pos[1] = mul(input[1].position,viewProjection);
	pos[2] = mul(input[2].position,viewProjection);

	float4 trianglePlane;
	trianglePlane.xyz = cross(pos[1].xyz - pos[0].xyz, pos[2].xyz - pos[0].xyz);
	trianglePlane.xyz = normalize(trianglePlane.xyz);
	trianglePlane.w = -dot(pos[0].xyz, trianglePlane.xyz);

	float2 texCoord[3];
	for (int j = 0; j < 3; j++)
	{
		texCoord[j] = input[j].tex;
	}

	//change winding, otherwise there are artifacts for the back faces;
	if (dot(trianglePlane.xyz, float3(0, 0, 1)) < 0.0)
	{
		float4 vertexTemp = pos[2];
		float2 texCoordTemp = texCoord[2];

		pos[2] = pos[1];
		texCoord[2] = texCoord[1];

		pos[1] = vertexTemp;
		texCoord[1] = texCoordTemp;
	}
	float2 halfPixel = 1.0f / volumeDimension;

	if (trianglePlane.z == 0.0f) {
		return;
	}
	//expand aabb for triangle
	output.triangleAABB = AxisAlignedBoundingBox(pos, halfPixel);

	// calculate the plane through each edge of the triangle
	// in normal form for dilatation of the triangle
	float3 planes[3];
	planes[0] = cross(pos[0].xyw - pos[2].xyw, pos[2].xyw);
	planes[1] = cross(pos[1].xyw - pos[0].xyw, pos[0].xyw);
	planes[2] = cross(pos[2].xyw - pos[1].xyw, pos[1].xyw);
	planes[0].z -= dot(halfPixel, abs(planes[0].xy));
	planes[1].z -= dot(halfPixel, abs(planes[1].xy));
	planes[2].z -= dot(halfPixel, abs(planes[2].xy));

	// calculate intersection between translated planes
	float3 intersection[3];
	intersection[0] = cross(planes[0], planes[1]);
	intersection[1] = cross(planes[1], planes[2]);
	intersection[2] = cross(planes[2], planes[0]);
	intersection[0] /= intersection[0].z;
	intersection[1] /= intersection[1].z;
	intersection[2] /= intersection[2].z;

	float z[3];
	z[0] = -(intersection[0].x * trianglePlane.x + intersection[0].y * trianglePlane.y + trianglePlane.w) / trianglePlane.z;
	z[1] = -(intersection[1].x * trianglePlane.x + intersection[1].y * trianglePlane.y + trianglePlane.w) / trianglePlane.z;
	z[2] = -(intersection[2].x * trianglePlane.x + intersection[2].y * trianglePlane.y + trianglePlane.w) / trianglePlane.z;

	pos[0].xyz = float3(intersection[0].xy, z[0]);
	pos[1].xyz = float3(intersection[1].xy, z[1]);
	pos[2].xyz = float3(intersection[2].xy, z[2]);
	
	for (unsigned int i = 0; i < 3; i++)
	{
		float4 voxelPos = mul(pos[i],viewProjectionI);
		voxelPos.xyz /= voxelPos.w;
		voxelPos.xyz -= worldMinPoint;
		voxelPos *= voxelScale;

		output.position = pos[i];
		output.wPosition = pos[i].xyz;
		output.normal = input[i].normal;
		output.tex = texCoord[i];
		output.tangent = input[i].tangent;
		output.wsPosition = voxelPos.xyz * volumeDimension;
		triStream.Append(output);
	}

	triStream.RestartStrip();

}


float4 PSMain(PSInput input) : SV_TARGET
{
	if (input.wPosition.x < input.triangleAABB.x || input.wPosition.y < input.triangleAABB.y ||
	input.wPosition.x > input.triangleAABB.z || input.wPosition.y > input.triangleAABB.w)
	{
		discard;
	}
	float4 color;
	float4 textureColor = diffuseColor;
	if (haveTexture.x != 0) 
	{
		textureColor = diffuseColor * txDiffuse.Sample(samLinear, input.tex);
	}
	//If material has a normal map, we can set it now
	if (haveTexture.y != 0)
	{
		//Load normal from normal map
		float4 normalMap = ObjNormMap.Sample(samLinear, input.tex);

		//Change normal map range from [0, 1] to [-1, 1]
		normalMap = (2.0f*normalMap) - 1.0f;

		//Make sure tangent is completely orthogonal to normal
		float3 newTangent = normalize(input.tangent.xyz - dot(input.tangent.xyz, input.normal)*input.normal);

		//Create the biTangent
		float3 biTangent = cross(input.normal.xyz, newTangent.xyz) * input.tangent.w;

		//Create the "Texture Space"
		float3x3 texSpace = float3x3(newTangent, biTangent.xyz, input.normal.xyz);

		//Convert normal from normal map to texture space and store in input.normal
		input.normal = mul(normalMap.xyz, texSpace);
	}

	float3 position = input.wsPosition;
	// Store voxels which are inside voxel-space boundary.
	baseColor[position] = textureColor;
	normalColor[position] = float4(input.normal * 0.5 + 0.5, 1.0);
	// Normalize the specular power
	float SpecPowerNorm = max(0.0001, (specExp - g_SpecPowerRange.x) / g_SpecPowerRange.y);
	emissionColor[position] = float4(SpecPowerNorm, metallic, roughness, 1.0f);

	return textureColor;
}

