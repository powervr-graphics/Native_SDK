uniform sampler2D    sTexture;
uniform sampler2D    sColourSpline;

uniform mediump float uColourSplineIndex;

varying mediump vec2  vTexCoord;

void main()
{
	lowp vec4 fn = texture2D(sTexture, vTexCoord);
	
#if defined(FN0)
	mediump float val = fn.x;
#elif  defined(FN1)
	mediump float val = fn.y;
#elif  defined(FN2)
	mediump float val = fn.z;
#elif  defined(FN3)
	mediump float val = fn.w;
#elif  defined(FN1_MINUS_FN0)
	mediump float val = fn.y - fn.x;
#elif  defined(FN2_MINUS_FN1)
	mediump float val = fn.z * -1.5 - fn.y;
#elif defined(SUM_FN0_FN1_FN2)
	mediump float val = fn.x * -1.0 + fn.y * 0.3 + fn.z * -0.03;
#endif

	lowp vec3 colour = texture2D(sColourSpline, vec2(val, uColourSplineIndex)).rgb;
	gl_FragColor = vec4(colour, 1.0);
}
