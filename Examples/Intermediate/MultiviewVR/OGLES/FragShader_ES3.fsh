#version 300 es
uniform sampler2D  sTexture;

in lowp    float  LightIntensity;
in mediump vec2   TexCoord;
in mediump vec3   Normals;

out mediump vec4 oColor;

void main()
{
    oColor.rgb = texture(sTexture, TexCoord).rgb * LightIntensity;
    oColor.a = 1.0;
}
