#ifdef USE_SHADOW_SAMPLERS
#	extension GL_EXT_shadow_samplers : require
    uniform sampler2DShadow sShadow;
#else   
    uniform highp sampler2D sShadow;
#endif

uniform sampler2D       sTexture;

varying highp vec4      vProjCoord;
varying mediump vec2    texCoord;
varying lowp vec3       LightIntensity;

void main ()
{	
    const lowp float fAmbient = 0.4;
    
#ifdef USE_SHADOW_SAMPLERS
    // Don't completely darken the shadowed areas, assume some ambient light
    highp float shadowVal = shadow2DProjEXT(sShadow, vProjCoord) * 0.6 + fAmbient;
#else
    // Subtract a small magic number to account for floating-point error
    highp float comp = (vProjCoord.z / vProjCoord.w) - 0.03;
	highp float depth = texture2DProj(sShadow, vProjCoord).r;
    lowp float shadowVal = comp <= depth ? 1.0 : fAmbient;
#endif
	lowp vec3 color = texture2D(sTexture, texCoord).rgb * LightIntensity * shadowVal;
	gl_FragColor = vec4(color, 1.0);
}

