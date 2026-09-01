#version 320 es

layout(set = 0, binding = 0) uniform mediump samplerCube skybox;

layout(std140, set = 0, binding = 1) uniform nnUniformBuffer
{
    highp mat4 inverseViewProjectionMatrix;
    highp mat4 projectionMatrix;
    highp mat4 viewMatrixCurrentFrame;
    highp mat4 viewMatrixPreviousFrame;
    highp vec3 cameraPosition;
    highp float exposure;
    highp int screenWidth;
    highp int screenHeight;
    mediump float emissiveIntensity;
    mediump float textureLODBias;
    highp vec2 jitter;
};

layout(location = 0) in mediump vec3 RayDir;
layout(location = 1) in highp vec4 clipPosition;
layout(location = 2) in highp vec4 clipPositionPreviousFrame;

layout(location = 0) out mediump vec4 outColor;
layout(location = 1) out highp vec2 outMotion;

void main()
{
    mediump vec3 toneMappedColor = min(texture(skybox, RayDir).rgb, 50. / exposure);
    toneMappedColor *= exposure;

    // http://filmicworlds.com/blog/filmic-tonemapping-operators/
    // Our favorite is the optimized formula by Jim Hejl and Richard Burgess-Dawson
    // We particularly like its high contrast and the fact that it is very cheap, with
    // only 4 mads and a reciprocal.
    mediump vec3 x = max(vec3(0.), toneMappedColor - vec3(0.004));
    toneMappedColor = (x * (6.2 * x + .49)) / (x * (6.175 * x + 1.7) + 0.06);

    outColor = vec4(toneMappedColor, 1.0);

    highp vec2 clipPositionCorrected = clipPosition.xy / clipPosition.w;
    highp vec2 clipPositionPreviousFrameCorrected = clipPositionPreviousFrame.xy / clipPositionPreviousFrame.w;
    highp vec2 viewportPosition = (clipPositionCorrected.xy + 1.0f) / 2.0f;
    highp vec2 viewportPositionPreviousFrame = (clipPositionPreviousFrameCorrected.xy + 1.0f) / 2.0f;

    outMotion = viewportPositionPreviousFrame - viewportPosition;
    outMotion.xy = vec2(outMotion.y, outMotion.x);
    outMotion *= vec2(1080, 1920);
    outMotion.xy += 2.0 * vec2(gl_FragCoord.yx - vec2(0.5));
}
