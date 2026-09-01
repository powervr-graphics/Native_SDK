#extension GL_OES_EGL_image_external : require

uniform samplerExternalOES	sSampler;
varying mediump vec2		sTexCoord;

void main()
{
    gl_FragColor = vec4(texture2D( sSampler, sTexCoord ).xyz, 1.);
}
