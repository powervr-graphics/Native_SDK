#version 320 es

#define PI 3.1415926535897932384626433832795

layout(set = 0, binding = 0) uniform highp samplerCube envMap;
layout(location = 0) in highp vec3 inPos;
layout(location = 0) out highp vec4 outColor;

layout (constant_id = 0) const int NUM_SAMPLES_PER_DIR = 128;

void main()
{
	highp vec3 N = normalize(inPos);
	const highp float twoPI = PI * 2.0;
	const highp float halfPI = PI * 0.5;

	highp int numSamples = 0;
	highp vec3 out_col_tmp = vec3(0.);
	
	// Ensure we are not missing (too many) texels - taking into consideration bilinear filtering and the fact that we are
	// doing a cubemap, we should be looking at a number of samples on the order of more than one sample per "texel".
	
	// (Cube faces are square anyway)
	highp float tex_size = float(textureSize(envMap, 0).x); 
	highp float lod = max(log2(tex_size / float(NUM_SAMPLES_PER_DIR)) + 1.,0.) ;

	highp float DELTA_THETA = 1./float(NUM_SAMPLES_PER_DIR );
	highp float DELTA_PHI  = 1./float(NUM_SAMPLES_PER_DIR );

	for(highp float theta = 0.0; theta < twoPI; theta += DELTA_THETA)
	{
		for(highp float phi  = 0.; phi < twoPI; phi += DELTA_PHI)
		{
			highp float cosTheta = cos(theta);
			highp float sinPhi = sin(phi);
			highp float sinTheta = sin(theta);
			highp float cosPhi = cos(phi);
			highp vec3 L = normalize(vec3(sinTheta * cosPhi, sinPhi, cosPhi * cosTheta));

			highp float factor = dot(N, L);
			if (factor > 0.0001)
			{
				out_col_tmp += textureLod(envMap, L, lod).rgb * factor;
			}

			numSamples += 1;
		}
	}

	outColor = vec4(out_col_tmp * PI / float(numSamples), 1.);
}
