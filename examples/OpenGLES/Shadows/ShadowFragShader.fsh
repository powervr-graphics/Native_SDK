#version 310 es

precision highp float;

#if defined(SHADOW_TYPE_VSM) || defined(SHADOW_TYPE_EVSM2) || defined(SHADOW_TYPE_EVSM4)

in  vec4 vPosition;

out  vec4 oColor;

#endif

 vec2 GetEVSMExponents( float positiveExponent,  float negativeExponent)
{
    const  float maxExponent = 5.54f;

     vec2 lightSpaceExponents = vec2(positiveExponent, negativeExponent);

    // Clamp to maximum range of fp32/fp16 to prevent overflow/underflow
    return min(lightSpaceExponents, maxExponent);
}

// Applies exponential warp to shadow map depth, input depth should be in [0, 1]
 vec2 WarpDepth( float depth,  vec2 exponents)
{
    // Rescale depth into [-1, 1]
    depth = 2.0f * depth - 1.0f;
     float pos =  exp( exponents.x * depth);
     float neg = -exp(-exponents.y * depth);
    return vec2(pos, neg);
}

const  float PositiveExponent = 40.0f;
const  float NegativeExponent = 5.0f;

void main()
{
#if defined(SHADOW_TYPE_VSM)

     float depth = vPosition.z / vPosition.w;
	depth = depth * 0.5 + 0.5;

	 float moment1 = depth;
	 float moment2 = depth * depth;

	oColor = vec4(moment1, moment2, 0.0, 0.0);

#elif defined(SHADOW_TYPE_EVSM2) || defined(SHADOW_TYPE_EVSM4)

     float depth = vPosition.z * 0.5 + 0.5;

	 vec2 exponents = GetEVSMExponents(PositiveExponent, NegativeExponent);
	 vec2 vsmDepth = WarpDepth(depth, exponents);

	 vec4 average = vec4(vsmDepth.xy, vsmDepth.xy * vsmDepth.xy);

    #if defined(SHADOW_TYPE_EVSM2)
        oColor = average.xzxz;
    #else
        oColor = average;
    #endif
    
#endif
}
