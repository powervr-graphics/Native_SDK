uniform sampler2D sTexture;

varying mediump vec2  TexCoord;
varying mediump vec3 vWorldNormal;
varying mediump vec3 vLightDir;
varying mediump float vOneOverAttenuation;

void main()
{
    mediump float lightIntensity = max(dot(normalize(vLightDir), normalize(vWorldNormal)), 0.);
    lightIntensity *= vOneOverAttenuation;
    gl_FragColor = texture2D(sTexture, TexCoord);
    gl_FragColor.xyz = gl_FragColor.xyz * lightIntensity;
    gl_FragColor.a = 1.0;
}
