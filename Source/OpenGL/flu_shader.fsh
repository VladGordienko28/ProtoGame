/*=============================================================================
    flu_shader.fsh: Flu fragment shader.
    Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

//
// An information about single lightsource.
//
struct TLight
{
	int		Effect;
	vec4	Color;
	float	Brightness;
	float	Radius;
	vec2	Location;
	float	Rotation;
};


//
// Shader varying variables.
//
varying	vec2		WorldPoint;
varying	vec2		TexturePoint;
varying	vec4		Color;


//
// Shader uniform variables.
//
uniform	int			bUnlit;
uniform int			bRenderLightmap;
uniform	sampler2D	Bitmap;
uniform	float		Saturation;
uniform	TLight		ALights[16];
uniform	TLight		MLights[16];
uniform	int			ANum, MNum;
uniform	float		Time;
uniform	vec3		AmbientLight;


//
// Return the light value at current fragment.
//
vec3 LightValue( bool bAdditive )
{
	vec3	Result		= bAdditive ? vec3( 0.0, 0.0, 0.0 ) : AmbientLight;
	int		NumLights	= bAdditive ? ANum : MNum;

	for( int i=0; i<NumLights; i++ )
	{
		TLight	Source	= bAdditive ? ALights[i] : MLights[i];
	
		// Precompute.
		vec2	Ray		= WorldPoint.xy - Source.Location;
		float	Dist	= length(Ray);

		if( Dist > Source.Radius )
			continue;
			
		// Attenuation.
		float	Value	= 1.0 - Dist / Source.Radius;

		// Apply cool lighting effect.
		switch( Source.Effect )
		{
			case 4:
			{
				// LIGHT_SlowWave.
				Value	*= 0.7 + 0.3*sin(Dist-Time*5.0);
				break;
			}
			case 5:
			{
				// Fast wavy effect.
				Value	*= 0.7 + 0.3*sin(Dist/4.0-Time*10.0);
				break;
			}
			case 6:
			{
				// LIGHT_SpotLight.
				float Offset	= Source.Rotation*4;
				float Angle		= mod( Offset + 4.0*atan(Ray.x, Ray.y), 25.1327 );
				if( Angle>=3.1415 && Angle<=9.4247 )
					Value	*= 0.5 + 0.5*cos(Angle);
				else
					Value	= 0.0;
				break;
			}
			case 7:
			{
				// LIGHT_Searchlight.
				float Offset	= (Time+Source.Rotation) * 4.0;
				float Angle		= mod( Offset + 4.0*atan(Ray.x, Ray.y), 25.1327 );
				if( Angle>=3.1415 && Angle<=9.4247 )
					Value	*= 0.5 + 0.5*cos(Angle);
				else
					Value	= 0.0;
				break;
			}
			case 8:
			{
				// LIGHT_Fan.
				float Angle		= atan( Ray.x, Ray.y )*6.0 + Time*3.5;
				Value	*= 0.5*(cos(Angle)+1.0);
				break;
			}
			case 9:
			{
				// LIGHT_Disco.
				float Angle1	= atan( Ray.x, Ray.y )*11.0;
				float Angle2	= atan( Dist, 8.0 )*-11.0;
				float Scale1	= 0.5 + 0.5*cos(Time*5.0 + Angle1);
				float Scale2	= 0.5 + 0.5*cos(Time*5.0 + Angle2);
				Value	*= 1.0 - (Scale1+Scale2 - Scale1*Scale2);
				break;
			}
			case 10:
			{
				// LIGHT_Flower.
				float Angle	= atan( Ray.x, Ray.y )*11.0 - Time*3.2;
				float Scale	= 0.5*(sin(Angle)+1.0);
				Value	*= 0.1 + 0.9*(0.5+sin(Scale+Dist-Time*5.0)*0.5);
				break;
			}
			case 11:
			{
				// LIGHT_Hypnosis.
				float Angle	= atan( Ray.x, Ray.y ) + Time*3.5;
				Value	*= 0.5*(cos(Angle+Dist)+1.0);
				break;
			}
			case 12:
			{
				// LIGHT_Whirligig.
				float Angle	= atan( Ray.x, Ray.y )*5.0 - Time*3.5;
				Value	*= 0.5*(cos(Angle+Dist)+1.0);
				break;
			}
		}

		// Add light's value to result.
		Result	+= Source.Color.xyz * Value * Source.Brightness;
	}

	return Result;
}


//
// Entry point.
//
void main()
{
	if( bRenderLightmap == 0 )
	{
		// Render geometry.
		if( bUnlit == 1 )
		{
			//
			// Draw unlit bitmap.
			//
			vec4 Source = texture2D( Bitmap, TexturePoint ) * Color * Saturation; 
			gl_FragColor	= vec4( Source.xyz, Source.w );
		}
		else
		{
			//
			// Draw lit bitmap.
			//
			vec4 Source	= texture2D( Bitmap, TexturePoint ) * Color * Saturation; 
			vec3 Color	= Source.xyz * LightValue(false);		

			gl_FragColor	= vec4( Color, Source.w );
		}
	}
	else
	{
		//
		// Render lightmap.
		//
		gl_FragColor	= vec4( LightValue(true), 1.0 );
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/