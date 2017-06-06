#version 300 es
precision mediump sampler2DArray;
precision mediump float;
precision mediump int;
uniform sampler2DArray  sTexture;
uniform int layerIndex;
in mediump vec2   HighResTexCoord;
in mediump vec2   LowResTexCoord;
out mediump vec4 oColor;
void main()
{
    vec4 highResSample = texture(sTexture, vec3(HighResTexCoord,layerIndex + 2));
    vec4 lowResSample = texture(sTexture, vec3(LowResTexCoord,layerIndex));
    // calculate the squared distance to middle of screen
    // center of the screen: (-.5 + 1.5) / 2 = .5
    vec2 dist = vec2(0.5) - HighResTexCoord;
    float squareDist = dot(dist,dist);
    // High resoultion texture is used when distance from center is less than 0.5(0.25 is 0.5 squared) in texture coordinates. 
    // When the distance is less than 0.2 (0.04 is 0.2 squared), only the high res texture will be used.
    float lerp = smoothstep(-.25, -.04,-squareDist);
    oColor = mix(lowResSample * vec4(1.,.5,.5,1.) + vec4(.2,0.,0.,0.), highResSample * vec4(.5,.5,1.,1.) + vec4(0.,0.,.2,0.), lerp);
    oColor.a = 1.0;
}
