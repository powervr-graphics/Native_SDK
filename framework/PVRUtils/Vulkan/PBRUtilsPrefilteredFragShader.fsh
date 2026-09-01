#version 320 es

#define PI 3.14159265358

layout (location = 0) in highp vec3 inPos;
layout (location = 0) out highp vec4 outColor0;

layout(set = 0, binding = 0) uniform highp samplerCube envMap;

layout(push_constant) uniform PushConsts {
	highp float roughness;
} pushConsts;

layout(constant_id = 0) const uint numSamples = 1024u;

highp vec2 hammersley(uint i, uint N)
{
	highp float vdc = float(bitfieldReverse(i)) * 2.3283064365386963e-10; // Van der Corput
	return vec2(float(i) / float(N), vdc);
}

// Normal Distribution function
highp float D_GGX(highp float dotNH, highp float roughness)
{
	highp float a = roughness * roughness;
	highp float a2 = a * a;
	highp float denom = dotNH * dotNH * (a2 - 1.0) + 1.0;
	return a2 /(PI * denom * denom);
}

// Sourced from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
highp vec3 importanceSampleCGX(highp vec2 xi, highp float roughness, highp vec3 N)
{
	highp float a = roughness * roughness;
	highp float phi = 2.0 * PI * xi.x;
	highp float cosTheta = sqrt( (1.0f - xi.y) / ( 1.0f + (a*a - 1.0f) * xi.y ));
	highp float sinTheta = sqrt( 1.0f - cosTheta * cosTheta);	
	highp vec3 h = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
	highp vec3 upVector = abs(N.z) < 0.999f ? vec3(0.0f,0.0f,1.0f) : vec3(1.0f,0.0f,0.0f);
	highp vec3 tangentX = normalize( cross( upVector, N ) );
	highp vec3 tangentY = cross( N, tangentX );
	// Tangent to world space
	return (tangentX * h.x) + (tangentY * h.y) + (N * h.z);
}

// Sourced from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
void preFilterEnvMap(highp vec3 R, highp float roughness)
{
	highp vec3 N = R;
	highp vec3 V = R;

	highp vec4 result0 = vec4(0.);
	highp float mapSize = float(textureSize(envMap, 0).x);

	highp float omegaP = 4.0 * PI / (6.0 * mapSize * mapSize);
	highp float mipBias = 1.0f;	// Original paper suggest biasing the mip to improve the results
	
	for(uint i = 0u; i < numSamples; ++i)
	{
		highp vec2 Xi = hammersley(i, numSamples);
		highp vec3 H = importanceSampleCGX(Xi, roughness, N);
		highp vec3 L = 2.0 * dot(V, H) * H - V;

		highp float NoL = max(dot(N, L), 0.0);
		if(NoL > 0.0)
		{
			// We will usually not do roughness == 0. We should start from the first roughness value
			if(roughness == 0.0)
			{
				result0 = vec4(textureLod(envMap, L, 0.0).rgb * NoL, 0.0);
				break;
			}
			
			// optmized: https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
			highp float NoH = max(dot(N, H), 0.0);
			highp float VoH = max(dot(V,H), 0.0);
			highp float NoV = max(dot(N, V), 0.0);
			// Probability Distribution Function
			highp float pdf = D_GGX(NoH, roughness) * NoH / ((4.0f * VoH) + 0.0001) /*avoid division by 0*/;

			// Solid angle represented by this sample
			highp float omegaS = 1.0 / (float(numSamples) * pdf);
			// Solid angle covered by 1 pixel with 6 faces that are EnvMapSize X EnvMapSize

			highp float mipLevel = max(0.5 * log2(omegaS / omegaP) + mipBias, 0.0f);
			
			result0 += vec4(textureLod(envMap, L, mipLevel).rgb * NoL, NoL);
		}
	}
	
	if(result0.w != 0.0)
	{
		result0.rgb = result0.rgb / result0.w;// divide by the weight
	}
	outColor0 = vec4(result0.rgb,1.0);
}

void main()
{
	preFilterEnvMap(normalize(inPos), pushConsts.roughness);
}


