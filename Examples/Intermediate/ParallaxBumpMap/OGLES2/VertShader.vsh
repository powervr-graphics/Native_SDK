attribute highp vec3	vertPos;
attribute highp vec3	vertNormal;
attribute highp vec2	vertUV;
attribute highp vec3	vertTangent;

uniform highp mat4	mModelView;
uniform highp mat4	mModelViewProj;
uniform highp mat3	mNormal;
uniform highp vec3	vLightEyeSpacePos;

varying lowp  vec3	lightDir;
varying lowp  vec3	viewDir;
varying lowp  vec2	texCoord;

const lowp float fParallaxScale = 0.065;

void main(void)
{	
	// Create a Matrix to transform from eye space to tangent space
	// Start by calculating the normal, tangent and binormal.
	highp vec3 n = normalize(mNormal * vertNormal);
	highp vec3 t = normalize(mNormal * vertTangent);
	highp vec3 b = cross(n,t);

	// Create the matrix from the above
	highp mat3 mEyeToTangent = mat3( t.x, b.x, n.x,
							   t.y, b.y, n.y,
							   t.z, b.z, n.z);
	
	// Write gl_pos
	highp vec4 tempPos = vec4(vertPos, 1.0);				   
	gl_Position = mModelViewProj * tempPos;
	
	// Translate the view direction into Tangent Space
	// Translate the position into eye space
	tempPos = mModelView * tempPos;
	// Get the vector from the eye to the surface, this is the inverse of tempPos
	viewDir = tempPos.xyz;
	// Then translate that into Tangent Space (multiplied by parallax scale as only has to
	// be done once per surface, not per fragment)
	viewDir = normalize(mEyeToTangent * viewDir) * fParallaxScale;
	
	// Translate the light dir from eye space into Tangent Space
	lightDir = normalize(vLightEyeSpacePos - tempPos.xyz);
	
	// Finally set the texture co-ords
	texCoord = vertUV;
}