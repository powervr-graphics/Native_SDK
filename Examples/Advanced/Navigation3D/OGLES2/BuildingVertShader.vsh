attribute highp   vec3  inVertex;
attribute highp   vec3  inNormal;
attribute mediump vec2  inTexCoord;

uniform highp mat4    ModelViewProjMatrix;
uniform highp vec3    LightDirection;

varying highp   float   vDiffuse;
varying mediump vec2    vTexCoord;


void main()
{	
	vDiffuse = 0.4 + max(dot(inNormal, LightDirection), 0.0) * 0.6;
	vTexCoord = inTexCoord;
	gl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);	
}
