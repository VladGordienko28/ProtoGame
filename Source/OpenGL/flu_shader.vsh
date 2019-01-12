/*=============================================================================
    flu_shader.vsh: Flu vertex shader.
    Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

// Shader varying variables.
varying	vec2	WorldPoint;
varying	vec2	TexturePoint;
varying	vec4	Color;


//
// Entry point.
//
void main()
{
	// Interpolated values for frag shader.
	WorldPoint		= gl_Vertex.xy;
	TexturePoint	= gl_MultiTexCoord0.xy;
	Color			= gl_Color;

	// Transform vertex.
	gl_Position		= gl_ModelViewProjectionMatrix * gl_Vertex;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/