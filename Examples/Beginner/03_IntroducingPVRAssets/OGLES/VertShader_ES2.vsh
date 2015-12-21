attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;
attribute mediump vec2  inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform mediump vec3  LightDirection;
uniform highp   mat4  WorldViewIT;

varying mediump    float  LightIntensity;
varying mediump vec2   TexCoord;
varying mediump vec3   Normals;


void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
    Normals = normalize(mat3(WorldViewIT) * inNormal);
    
	// Pass through texcoords
	TexCoord = inTexCoord;
	
	// Simple diffuse lighting in model space with a touch of ambient
	LightIntensity = max(dot(Normals, -LightDirection),0.0);
}