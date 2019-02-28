/*=============================================================================
	final.vsh: Final Post-Processing vertex-shader.
	Copyright Mar.2018 Vlad Gordienko.
=============================================================================*/

// contants
#define FXAA_SUBPIX_SHIFT (1.0/4.0)

// uniforms
uniform vec4 m_RTSize; // x - width, y - height, z - 1/width, z - 1/height

// Shader varying variables.
varying	vec4 textureUV;

//
// Entry point.
//
void main()
{
	textureUV.xy	= gl_MultiTexCoord0.xy;

	textureUV.zw	= gl_MultiTexCoord0.xy - 
                  (m_RTSize.zw * (0.5 + FXAA_SUBPIX_SHIFT));

	// Transform vertex.
	gl_Position		= gl_ModelViewProjectionMatrix * gl_Vertex;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/