#version 310 es

layout (location = 0) out mediump vec2 vTexCoords;

void main()
{
    vec2 positions[6] = vec2[6](
        vec2(-1.0, -1.0),   // bottom left
        vec2( 1.0, -1.0),   // bottom right
        vec2(-1.0,  1.0),   // top    left
    
        vec2(-1.0,  1.0),   // top    left
        vec2( 1.0, -1.0),   // bottom right
        vec2( 1.0,  1.0)    // top    right
);

    vec2 pos = positions[gl_VertexID];
    gl_Position = vec4(pos, 0.0, 1.0);
    
    // Convert from normalized device coordinates (NDC) [-1,1] to texture [0,1]
    vTexCoords = pos * 0.5 + 0.5;
}
