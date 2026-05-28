#version 320 es

layout (input_attachment_index = 0, set = 0, binding = 0) highp uniform subpassInput sBaseTex;
   
layout(location = 0) in highp vec2 TexCoords;

layout(location = 0) out mediump vec4 outputYUV;

highp vec3 RGBToYUVMatrixRow0 = vec3( 4899.0,   9617.0,  1868.0);
highp vec3 RGBToYUVMatrixRow1 = vec3(-2411.0,  -4733.0,  7143.0);
highp vec3 RGBToYUVMatrixRow2 = vec3(10076.0,  -8438.0, -1639.0);

int round_half_up(int inp, int right_shift)
{
    return ((inp >> (right_shift - 1)) + 1) >> 1;
}

void main()
{
   ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);
   highp vec4 textureValue = subpassLoad(sBaseTex);
   highp float alpha = float(0xFF);
   highp float Y = float(round_half_up(int(dot(textureValue.xyz, RGBToYUVMatrixRow0)), 10));
   highp float U = float(round_half_up(int(dot(textureValue.xyz, RGBToYUVMatrixRow1)), 10));
   highp float V = float(round_half_up(int(dot(textureValue.xyz, RGBToYUVMatrixRow2)), 10));
   outputYUV = vec4(Y, U, V, alpha);
}
