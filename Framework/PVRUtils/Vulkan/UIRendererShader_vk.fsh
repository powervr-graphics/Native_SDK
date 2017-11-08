#version 450
layout(set = 0, binding = 0) uniform sampler2D fontTexture;
layout(std140,set = 2, binding = 0)uniform Material
{
	highp mat4 uvMatrix;
    mediump vec4 varColor;
    bool alphaMode;
};
layout(location = 0)in mediump vec2 texCoord;
layout(location = 0)out lowp vec4 oColor;
void main()
{
    mediump vec4 vTex = texture(fontTexture, texCoord);
    if(alphaMode)
    {
        oColor = vec4(varColor.rgb, varColor.a * vTex.a);
    }
    else
    {
        oColor = vec4(varColor * vTex);
    }
}
