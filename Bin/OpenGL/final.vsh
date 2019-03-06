/*=============================================================================
	final.vsh: Final Post-Processing vertex-shader.
	Copyright Mar.2018 Vlad Gordienko.
=============================================================================*/

// Shader varying variables.
varying	vec4 textureUV;

#define FXAA_SUBPIX_SHIFT (1.0/4.0)

//
// Entry point.
//
void main()
{
	textureUV.xy	= gl_MultiTexCoord0.xy;


	vec2 rcpFrame = vec2(1.0/1600, 1.0/900);

	textureUV.zw	= gl_MultiTexCoord0.xy - 
                  (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT));

	// Transform vertex.
	gl_Position		= gl_ModelViewProjectionMatrix * gl_Vertex;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/