attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;
attribute mediump vec2  inTexCoord;

uniform highp   mat4  MVInv;
uniform highp   mat4  MVPMatrix;
uniform mediump vec3  LightDirection;
uniform highp   float  Shininess;

varying lowp    float  LightIntensity;
varying mediump vec2   TexCoord;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);	

	// Pass through texcoords
	TexCoord = inTexCoord;
	
	// Simple diffuse lighting in model space
	LightIntensity = max(dot(inNormal, -LightDirection),0.0) + .2/*ambient*/;
    
    // if the light is behind no specular reflection
    if(dot(normalize(inNormal),-LightDirection) > 0.0)
    {
      vec3 viewDir = vec3(normalize((MVInv * vec4(0,0,0,1)) - vec4(inVertex,1.0)));
        LightIntensity += Shininess * 2. * pow(max(0.0, dot(reflect(-LightDirection, inNormal), viewDir)),32.0);
    } 
    
}