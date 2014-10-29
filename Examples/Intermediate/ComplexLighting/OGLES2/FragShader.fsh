uniform sampler2D  sTexture;

varying mediump vec2  TexCoord;
varying lowp    vec3  DiffuseLight;
varying lowp    vec3  SpecularLight;

void main()
{
    lowp vec3 texColor  = vec3(texture2D(sTexture, TexCoord));
	lowp vec3 color = (texColor * DiffuseLight) + SpecularLight;
	gl_FragColor = vec4(color, 1.0);
}
