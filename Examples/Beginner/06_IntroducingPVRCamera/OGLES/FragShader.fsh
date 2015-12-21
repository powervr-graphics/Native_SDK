#if defined(ANDROID)

#extension GL_OES_EGL_image_external:require
uniform samplerExternalOES Sampler;

#elif defined(IOS)

uniform sampler2D        	SamplerY;
uniform sampler2D        	SamplerUV;

#else

uniform sampler2D        	Sampler;

#endif

varying mediump vec2     vTexCoord;

void main()
{
    mediump vec3 color;
#if !defined(IOS)
	color = texture2D(Sampler, vTexCoord).rgb;
#else
    mediump vec3 yuv;
    yuv.x  = texture2D(SamplerY,  vTexCoord).r;
    yuv.yz = texture2D(SamplerUV, vTexCoord).ra - vec2(0.5, 0.5);

    // BT.709
    color = mat3(1.0,     1.0,      1.0,
			 0.0,     -.18732,  1.8556,
			 1.57481, -.46813,  0.0) * yuv;
#endif
//Any effect can be applied here - invert the colors...
	gl_FragColor.xyz = vec3(1.) - color;
    gl_FragColor.w = 1.0;
}
