uniform sampler2D  sTexture;
uniform mediump float BloomIntensity;// Luminance threshol
varying mediump vec2 TexCoord;

mediump float luma(mediump vec3 color)
{	
	return dot(vec3(0.2126,0.7152,0.0722), color); 
}


void main()
{
    mediump float lumThres = .5;
	lowp vec4 oColor = texture2D(sTexture, TexCoord);
    gl_FragColor  = oColor * clamp(luma(oColor.rgb) - lumThres, 0.0,1.0) * 
    					 (1.0 / (1.0 - lumThres)) * BloomIntensity * oColor.a /*multiply by alpha so that only the pixels belongs to the model get bloom*/; 
}
