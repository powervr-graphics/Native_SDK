#version 320 es

#define PI 3.14159265358
const mediump float HDR_SCALER = 100.0;

layout (location = 0) in highp vec3 inPos;
layout (location = 0) out mediump vec4 outColor0;
layout (location = 1) out mediump vec4 outColor1;

layout(set = 0, binding = 0) uniform mediump samplerCube envMap;

layout(push_constant) uniform PushConsts {
    mediump float roughness;
} pushConsts;

mediump vec2 hammersley(uint i, uint N)
{
    mediump float vdc = float(bitfieldReverse(i)) * 2.3283064365386963e-10; // Van der Corput
    return vec2(float(i) / float(N), vdc);
}

// Normal Distribution function
mediump float D_GGX(mediump float dotNH, mediump float roughness)
{
    mediump float a = roughness * roughness;
    mediump float a2 = a * a;
    mediump float denom = dotNH * dotNH * (a2 - 1.0) + 1.0;
    return a2 /(PI * denom * denom);
}

// Sourced from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
mediump vec3 importanceSampleCGX(mediump vec2 xi, mediump float roughness, mediump vec3 N)
{
    mediump float a = roughness * roughness;
    mediump float phi = 2.0 * PI * xi.x;
    mediump float cosTheta = sqrt( (1.0f - xi.y) / ( 1.0f + (a*a - 1.0f) * xi.y ));
    mediump float sinTheta = sqrt( 1.0f - cosTheta * cosTheta);    
    mediump vec3 h = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
    mediump vec3 upVector = abs(N.z) < 0.999f ? vec3(0.0f,0.0f,1.0f) : vec3(1.0f,0.0f,0.0f);
    mediump vec3 tangentX = normalize( cross( upVector, N ) );
    mediump vec3 tangentY = cross( N, tangentX );
    // Tangent to world space
    return (tangentX * h.x) + (tangentY * h.y) + (N * h.z);
}

//http://graphicrants.blogspot.com/2009/04/rgbm-color-encoding.html
mediump vec4 RGBMEncode(mediump vec3 color) 
{
  mediump vec4 rgbm;
  color *= 1.0 / 6.0;
  rgbm.a = clamp( max( max( color.r, color.g ), max( color.b, 1e-6 ) ), 0.0, 1.0);
  rgbm.a = ceil(rgbm.a * 255.0) / 255.0;
  rgbm.rgb = color / rgbm.a;
  return rgbm;
}

// Sourced from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
void preFilterEnvMap(mediump vec3 R, mediump float roughness)
{
    mediump vec3 N = R;
    mediump vec3 V = R;

    mediump vec4 result0 = vec4(0.);
    mediump vec4 result1 = vec4(0.);
    mediump const uint numSamples = 10000u;
    mediump float mapSize = float(textureSize(envMap, 0).x);

    // color 1: output the mip mapping.
    {
        mediump vec2 Xi = hammersley(0u, numSamples);
        mediump vec3 H = importanceSampleCGX(Xi, roughness, N);
        mediump vec3 L = 2.0 * dot(V, H) * H - V;
        mediump float NoL = clamp(dot(N, L), 0.0, 1.0);
        
        mediump vec4 tmpEnv = vec4(texture(envMap, L).rgb * NoL, 0.0);
        mediump float tmpLight = tmpEnv.a;
        tmpLight = HDR_SCALER * tmpLight * tmpLight + 1.0;
        result1 = tmpEnv * tmpLight;
    }

	mediump float omegaP = 4.0 * PI / (6.0 * mapSize * mapSize);
	mediump float mipBias = 1.0f;    // Original paper suggest biasing the mip to improve the results
    
    for(uint i = 0u; i < numSamples; ++i)
    {
        mediump vec2 Xi = hammersley(i, numSamples);
        mediump vec3 H = importanceSampleCGX(Xi, roughness, N);
        mediump vec3 L = 2.0 * dot(V, H) * H - V;

        mediump float NoL = max(dot(N, L), 0.0);
        if(NoL > 0.0)
        {              
            if(roughness == 0.0)
            {
                mediump vec4 tmpEnv = vec4(textureLod(envMap, L, 0.0).rgb * NoL, 0.0);
                
                mediump float tmpLight = tmpEnv.a;
                
                tmpLight = HDR_SCALER * tmpLight * tmpLight + 1.0;
                result0 = tmpEnv * tmpLight;
                break;
            }
            
            // optmized: https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
            mediump float NoH = max(dot(N, H), 0.0);
            mediump float VoH = max(dot(V,H), 0.0);
            mediump float NoV = max(dot(N, V), 0.0);
            // Probability Distribution Function
            mediump float pdf = D_GGX(NoH, roughness) * NoH / ((4.0f * VoH) + 0.0001) /*avoid division by 0*/;

            // Solid angle represented by this sample
            mediump float omegaS = 1.0 / (float(numSamples) * pdf);
            // Solid angle covered by 1 pixel with 6 faces that are EnvMapSize X EnvMapSize

            mediump float mipLevel = max(0.5 * log2(omegaS / omegaP) + mipBias, 0.0f);
            
            mediump vec4 tmpEnv = textureLod(envMap, L, mipLevel);
            tmpEnv.rgb = tmpEnv.rgb * NoL;
            mediump float tmpLight = tmpEnv.a * NoL;
            tmpLight = HDR_SCALER * tmpLight * tmpLight + 1.0;
            result0 += vec4(tmpEnv.rgb * tmpLight, NoL);
        }
    }
    
    if(result0.w != 0.0)
    {
        result0.rgb = result0.rgb / result0.w;// divide by the weight
    }
    outColor0 = RGBMEncode(result0.rgb);
    outColor1 = RGBMEncode(result1.rgb);
}

void main()
{
    preFilterEnvMap(normalize(inPos), pushConsts.roughness);
}


