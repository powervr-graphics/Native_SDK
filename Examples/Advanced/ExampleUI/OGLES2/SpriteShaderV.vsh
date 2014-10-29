attribute highp vec3 inVertex;
attribute highp vec2 inUVs;
attribute mediump float inTransIdx;
attribute lowp vec4	 inRGBA;

uniform mediump mat4 MTransforms[30];
uniform mediump mat4 MVPMatrix;

varying highp vec2 TexCoord;
varying lowp vec4 RGBA;

void main()
{
	TexCoord = inUVs;
	RGBA     = inRGBA;
	
	lowp int iTransIdx = int(inTransIdx);
	highp vec4 position = MTransforms[iTransIdx] * vec4(inVertex, 1.0);
		
	gl_Position = MVPMatrix * position;
}