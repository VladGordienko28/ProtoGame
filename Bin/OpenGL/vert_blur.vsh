/*=============================================================================
	vert_blur.vsh: Vertical blur vertex shader.
	Created by Vlad Gordienko, Mar. 2018.
=============================================================================*/

varying vec2 blurCoordsOffset[11]; 

uniform float targetHeight;


void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	vec2 texCenter = gl_Position * 0.5 + 0.5;
	float pixSize = 1.0 / targetHeight;

	for( int i=-5; i<=5; i++ )
	{
		blurCoordsOffset[i+5] = texCenter + vec2( 0.0, i*pixSize );
	}
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/