#version 450

const highp vec2 positions[4] = vec2[4]
                                (
                                  vec2(-1., -1.),
                                  vec2(-1., 1.),
                                  vec2(1., -1.),
                                  vec2(1., 1.)
                                );

out mediump vec2 vTexCoord;

void main()
{
	highp vec2 position = positions[gl_VertexIndex];
	gl_Position = vec4(position, 0.5, 1.);

	vTexCoord = position * .5 + .5; //Map -1..1->0..1
}
