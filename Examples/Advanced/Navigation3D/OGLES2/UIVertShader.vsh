attribute highp   vec2  inVertex;
attribute mediump vec2  inTexCoord;

uniform   highp   mat2  RotationMatrix;

varying   mediump vec2  vTexCoord;

void main()
{	
	highp vec2 vertex = RotationMatrix * inVertex;
	gl_Position = vec4(vertex, 0.0, 1.0);	
	vTexCoord = inTexCoord;
}
