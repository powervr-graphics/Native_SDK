#version 300 es

#ifdef ANDROID
#extension GL_OES_EGL_image_external : enable
uniform samplerExternalOES	Sampler;
#endif
uniform sampler2D        	SamplerY;
uniform sampler2D        	SamplerUV;
#ifdef SCREEN_BANDS
uniform lowp vec2        vScreenBand;
#endif
#ifdef NOISE
uniform sampler2D        SamplerNoise;
in mediump vec2     vNoiseTexCoord;
#endif

in mediump vec2     vTexCoord;

layout(location = 0) out lowp vec4 oColour;

const lowp float c_fInt = 0.7;

void main()
{
#ifdef ANDROID
	mediump vec3 rgb = texture( Sampler, vTexCoord).rgb;
#else	
	mediump vec3 yuv;
	yuv.x  = texture(SamplerY,  vTexCoord).r;
	yuv.yz = texture(SamplerUV, vTexCoord).ra - vec2(0.5, 0.5);

	// BT.709
	lowp vec3 rgb = mat3(1.0,     1.0,      1.0,
						 0.0,     -.18732,  1.8556,
						 1.57481, -.46813,  0.0) * yuv;
#endif	
	
#ifdef GREYSCALE
	rgb = vec3((rgb.r + rgb.b + rgb.g) / 3.0);
#endif

#ifdef SCREEN_BANDS
	if(vTexCoord.y > vScreenBand.x && vTexCoord.y <= vScreenBand.y)
	{
		rgb.g += 0.5;
	}
#endif

#ifdef NOISE
	lowp float fRand = (texture(SamplerNoise, vNoiseTexCoord).r * c_fInt) - 0.1;
	rgb = rgb + fRand;
#endif

	oColour = vec4(rgb, 1.0);
}
