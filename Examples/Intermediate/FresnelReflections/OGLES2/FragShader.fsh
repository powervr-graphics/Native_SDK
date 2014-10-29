uniform sampler2D  sBaseTex;
uniform sampler2D  sReflectTex;

varying mediump vec2   ReflectCoord;
varying mediump vec2   TexCoord;
varying lowp    float  ReflectRatio;

void main()
{
	lowp vec3 baseColor = vec3(texture2D(sBaseTex, TexCoord));
	lowp vec3 reflection = vec3(texture2D(sReflectTex, ReflectCoord));
	lowp vec3 color = mix(baseColor, reflection, ReflectRatio);
	gl_FragColor = vec4(color, 1.0);
}
