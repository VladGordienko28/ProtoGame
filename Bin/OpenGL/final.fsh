/*=============================================================================
	final.fsh: Final Post-Processing fragment-shader.
	Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

varying	vec2 textureUV;

uniform	sampler2D texture;
uniform vec3 highlights;
uniform vec3 midTones;
uniform vec3 shadows;
uniform float bwScale;
uniform float aberrationIntensity;

void main()
{
	vec4 sampleColor;
	
	if( aberrationIntensity > 0.0 )
	{
		sampleColor.r = texture2D( texture, textureUV + vec2(0.001, 0.0007) * aberrationIntensity ).r;
		sampleColor.g = texture2D( texture, textureUV + vec2(-0.0007, 0.001) * aberrationIntensity ).g;
		sampleColor.b = texture2D( texture, textureUV + vec2(-0.001, -0.001) * aberrationIntensity ).b;

		sampleColor.a = texture2D( texture, textureUV ).a;		
	}
	else
	{
		sampleColor = texture2D( texture, textureUV );
	}

	vec3 result = pow( max( vec3(0,0,0), (sampleColor.rgb-shadows) )*(highlights), midTones );

	if( bwScale != 0.0 )
	{
		float brightness = dot( result, vec3(0.299, 0.587, 0.114) );
		gl_FragColor = vec4( mix( result, vec3( brightness, brightness, brightness ), bwScale ), sampleColor.a );
	}
	else
	{
		gl_FragColor = vec4( result, sampleColor.a );
	}
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/