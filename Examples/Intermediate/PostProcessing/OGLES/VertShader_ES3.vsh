#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec3	 inVertex;
layout (location = NORMAL_ARRAY) in mediump vec3	 inNormal;
layout (location = TEXCOORD_ARRAY) in mediump vec2 inTexCoord;

uniform highp   mat4  MVInv;
uniform highp   mat4  MVPMatrix;
uniform mediump vec3  LightDirection;
uniform highp   float  Shininess;

out highp    float  LightIntensity;
out mediump vec2    TexCoord;
void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);	

	// Pass through texcoords
	TexCoord = inTexCoord;
	
	highp vec3 normnorm = normalize(inNormal);
	
	// Simple diffuse lighting in model space
	LightIntensity = max(dot(normnorm, -LightDirection), 0.0)/*ambient*/;
    
	LightIntensity += 0.005;
	
    // if the light is behind no specular reflection
    if(dot(normnorm, -LightDirection) > 0.0)
    {
      vec3 viewDir = -(vec3(normalize((MVInv * vec4(0,0,0,1)) - vec4(inVertex,1.0))));
      LightIntensity += Shininess * 2. * pow(max(0.0, dot(reflect(-LightDirection, normnorm), viewDir)), 32.0);
    }
	LightIntensity *= 2.;
}