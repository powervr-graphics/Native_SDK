attribute highp	  vec3 inVertex;		//Vertex coordinates
attribute highp vec2 inTexCoord;		//Texture coordinates in.
varying   highp vec2 t1;				//Texture coordinate passed to fragment.

#ifdef EDGE_DETECTION
uniform mediump vec2 PixelSize;			//Relative size of a pixel (in texels) for this program.
varying highp vec2 t2;				//Texture location for fragment directly above.
varying highp vec2 t3;				//Texture location for fragment directly to the right.
#endif

void main()
{
	//Pass through texture coordinates.
	t1 = inTexCoord;

#ifdef EDGE_DETECTION
	// Sets texture coordinates for surrounding texels (up and right);
	t2 = vec2(inTexCoord.x, inTexCoord.y+PixelSize.y);
	t3 = vec2(inTexCoord.x+PixelSize.x, inTexCoord.y);
#endif

	// Set vertex position.
	gl_Position = vec4(inVertex,  1.0);

}
