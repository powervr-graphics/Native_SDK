#version 320 es

// Rather than produce a input assembler, hardcore the values for a triangle
// that covers the entire screen
const vec2 positions[3] = vec2[](vec2(-1.0, 1.0), // bottom left
                                 vec2(3.0, 1.0),  // bottom right
                                 vec2(-1.0, -3.0) // top left
);

const vec2 TexCoord[3] =
    vec2[](vec2(0.0, 1.0), vec2(2.0, 1.0), vec2(0.0, -1.0));

layout(location = 0) out highp vec2 vTexCoord;

void main() {
  // Pass variabes onto the fragment shader
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
  vTexCoord = TexCoord[gl_VertexIndex];
}