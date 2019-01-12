/*=============================================================================
	final.vsh: Final Post-Processing vertex-shader.
	Copyright Mar.2018 Vlad Gordienko.
=============================================================================*/

// Shader varying variables.
varying	vec2 textureUV;

//
// Entry point.
//
void main()
{
	textureUV	= gl_MultiTexCoord0.xy;

	// Transform vertex.
	gl_Position		= gl_ModelViewProjectionMatrix * gl_Vertex;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/