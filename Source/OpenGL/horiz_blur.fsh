/*=============================================================================
	horiz_blur.fsh: Horizontal blur fragment shader.
	Created by Vlad Gordienko, Mar. 2018.
=============================================================================*/

varying vec2 blurCoordsOffset[11]; 

uniform	sampler2D texture;

void main()
{
	vec4 result = vec4( 0.0, 0.0, 0.0, 0.0 );

	result  = texture2D( texture, blurCoordsOffset[0] ) * 0.0093;
    result += texture2D( texture, blurCoordsOffset[1] ) * 0.028002;
    result += texture2D( texture, blurCoordsOffset[2] ) * 0.065984;
    result += texture2D( texture, blurCoordsOffset[3] ) * 0.121703;
    result += texture2D( texture, blurCoordsOffset[4] ) * 0.175713;
    result += texture2D( texture, blurCoordsOffset[5] ) * 0.198596;
    result += texture2D( texture, blurCoordsOffset[6] ) * 0.175713;
    result += texture2D( texture, blurCoordsOffset[7] ) * 0.121703;
    result += texture2D( texture, blurCoordsOffset[8] ) * 0.065984;
    result += texture2D( texture, blurCoordsOffset[9] ) * 0.028002;
    result += texture2D( texture, blurCoordsOffset[10] ) * 0.0093;

	gl_FragColor = result;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/