uniform sampler2D  sBloomMapping;

uniform mediump float fBloomIntensity;

varying mediump vec2 TexCoord;

void main()
{
    gl_FragColor = texture2D(sBloomMapping, TexCoord) * fBloomIntensity;
}
