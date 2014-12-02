#version 300 es
uniform sampler2D		Texture;

uniform mediump vec2 	RcpWindowSize;

layout (location = 0) out lowp vec4 oColour;

void main()
{	
	mediump vec2 vTexCoord = gl_FragCoord.xy * RcpWindowSize;
	oColour = texture(Texture, vTexCoord);
}
