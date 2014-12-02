varying lowp vec3  DiffuseIntensity; 
varying lowp vec3  SpecularIntensity;

const lowp vec3 cBaseColor = vec3(0.9, 0.1, 0.1); 
 
void main() 
{ 
	gl_FragColor = vec4((cBaseColor * DiffuseIntensity) + SpecularIntensity, 1.0); 
}
