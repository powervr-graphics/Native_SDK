#version 450

layout(location = 0) out highp vec2 TexCoords;
void main()
{
    highp vec2 texCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    highp vec2 vViewDirVS = vec2(texCoord * 2.0 + -1.0);
    gl_Position = vec4(vViewDirVS, 0.0, 1.0);
    TexCoords = texCoord;
}