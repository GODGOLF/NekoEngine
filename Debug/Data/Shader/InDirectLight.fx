//https://computergraphics.stackexchange.com/questions/8418/get-intersection-ray-with-square

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
struct InDirectLightParameter
{
	float voxelScale;
	float voxelDimension;
	float maxTracingDistanceGlobal;
	float3 position;
	float3 normal;
	float3 diffuseColor;
	float samplingFactor;
	float boundStength;
	float3 worldMin;
	float3 worldMax;
	SamplerState clampSampler;
	Texture3D<float4> voxelTexture;
};

bool IntersectRayWithWorldAABB(float3 ro, float3 rd, float3 worldMin, float3 worldMax)
{
	float3 tempMin = (worldMin -ro)/rd;
	float3 tempMax = (worldMax -ro)/rd;
	
	float3 vMax = max (tempMax, tempMin);
    float3 vMin = min (tempMax, tempMin);
	
	float leave = min(vMax.x, min (vMax.y, vMax.z));
	
	float enter = max (max (vMin.x, 0.0), max (vMin.y, vMin.z));
	
	return leave > enter;
	
}
float3	WorldToVoxel(float3 position, float3 worldMinPoint, float voxelScale)
{
    float3 voxelPos = position - worldMinPoint;
    return voxelPos * voxelScale;
}

float3 TraceCone(float3 direction,float aperture,in InDirectLightParameter parameter)
{
	// world space grid voxel size
	float voxelWorldSize = 2.0/(parameter.voxelScale * parameter.voxelDimension);
	
	// move further to avoid self collision
	float dst = voxelWorldSize;
	
	float3 startPosition = parameter.position + parameter.normal * dst;
	// final results
    float3 coneSample = float3(0,0,0);
	
	float maxDistance = parameter.maxTracingDistanceGlobal * (1.0f / parameter.voxelScale);
	
	if(maxDistance <= 0.f)
	{
		maxDistance =1.0f;
	}
	if(aperture <=0.0f)
	{
		aperture = 0.543f;
	}
	float falloff = 0.5f * parameter.voxelScale;
	
	if(!IntersectRayWithWorldAABB(parameter.position, direction,parameter.worldMin,parameter.worldMax))
	{
		return coneSample;
	}
	[loop]
	while(dst <= maxDistance)
	{
		float3 conePosition = startPosition + direction * dst;
		// cone expansion and respective mip level based on diameter
        float diameter = 2.0f * aperture * dst;
		
		float3 coord = WorldToVoxel(conePosition,parameter.worldMin,parameter.voxelScale); 
		
		float mipLevel = log2(diameter/voxelWorldSize);
		
		float4 baseColor = parameter.voxelTexture.SampleLevel(parameter.clampSampler,coord,0).xyzw;
		
		coneSample += (1.f-baseColor.a) * baseColor;
		
		float samplingFactor = (parameter.samplingFactor >0.f) ? parameter.samplingFactor : 0.5f;
		
		dst += diameter * samplingFactor;
		
	}
	return coneSample;
} 

float4 CalInDirectLight(in InDirectLightParameter input)
{
	float3 diffuseTrace = 0.f;
	if(any(input.diffuseColor))
	{
		// diffuse cone setup
        const float aperture = 0.57735f;
		float3 guide = float3(0.0f, 1.0f, 0.0f);
		
		float3 coneDirection = 0.0f;
		
		if (abs(dot(input.normal,guide)) == 1.0f)
        {
            guide = float3(0.0f, 0.0f, 1.0f);
        }
		// Find a tangent and a bitangent
		float3 right = normalize(guide - dot(input.normal, guide) * input.normal);
		float3 up = cross(right, input.normal);
		for(int i=0;i<6;i++)
		{
			coneDirection = input.normal;
			coneDirection += diffuseConeDirections[i].x * right + diffuseConeDirections[i].z * up;
			coneDirection = normalize(coneDirection);
			diffuseTrace += TraceCone(coneDirection,aperture,input)* diffuseConeWeights[i];
		}
		diffuseTrace *= input.diffuseColor;
	}
	float4 result = float4(input.boundStength * diffuseTrace,1.0f);
	return result;
}