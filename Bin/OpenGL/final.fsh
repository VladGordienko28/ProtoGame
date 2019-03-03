/*=============================================================================
	final.fsh: Final Post-Processing fragment-shader.
	Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

varying	vec4 textureUV;

uniform	sampler2D texture;
uniform vec3 highlights;
uniform vec3 midTones;
uniform vec3 shadows;
uniform float bwScale;
uniform float aberrationIntensity;

uniform vec3 m_vignette; // x - intensity, y - innerRadius, z - outerRadius
uniform vec4 m_RTSize; // x - width, y - height, z - 1/width, z - 1/height
uniform int m_enableFXAA;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const float FXAA_SPAN_MAX = 8.0;
const float FXAA_REDUCE_MUL = 1.0/8.0;

#define FxaaInt2 ivec2
#define FxaaFloat2 vec2
#define FxaaTexLod0(t, p) texture2DLod(t, p, 0.0)
#define FxaaTexOff(t, p, o, r) texture2D(t, p + o * r)

vec3 FxaaPixelShader( 
  vec4 posPos, // Output of FxaaVertexShader interpolated across screen.
  sampler2D tex, // Input texture.
  vec2 rcpFrame) // Constant {1.0/frameWidth, 1.0/frameHeight}.
{   
/*---------------------------------------------------------*/
    #define FXAA_REDUCE_MIN   (1.0/64.0)
    //#define FXAA_REDUCE_MUL   (1.0/8.0)
    //#define FXAA_SPAN_MAX     8.0
/*---------------------------------------------------------*/
    vec3 rgbNW = FxaaTexLod0(tex, posPos.zw).xyz;
    vec3 rgbNE = FxaaTexOff(tex, posPos.zw, FxaaInt2(1,0), rcpFrame.xy).xyz;
    vec3 rgbSW = FxaaTexOff(tex, posPos.zw, FxaaInt2(0,1), rcpFrame.xy).xyz;
    vec3 rgbSE = FxaaTexOff(tex, posPos.zw, FxaaInt2(1,1), rcpFrame.xy).xyz;
    vec3 rgbM  = FxaaTexLod0(tex, posPos.xy).xyz;
/*---------------------------------------------------------*/
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
/*---------------------------------------------------------*/
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
/*---------------------------------------------------------*/
    vec2 dir; 
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
/*---------------------------------------------------------*/
    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(FxaaFloat2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX), 
          max(FxaaFloat2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), 
          dir * rcpDirMin)) * rcpFrame.xy;
/*--------------------------------------------------------*/
    vec3 rgbA = (1.0/2.0) * (
        FxaaTexLod0(tex, posPos.xy + dir * (1.0/3.0 - 0.5)).xyz +
        FxaaTexLod0(tex, posPos.xy + dir * (2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        FxaaTexLod0(tex, posPos.xy + dir * (0.0/3.0 - 0.5)).xyz +
        FxaaTexLod0(tex, posPos.xy + dir * (3.0/3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);
    if((lumaB < lumaMin) || (lumaB > lumaMax)) return rgbA;
    return rgbB; }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////


vec4 PostFX( sampler2D tex, vec4 uv, float time )
{
  vec4 c = vec4( 0.0 );
  c.rgb = FxaaPixelShader( uv, tex, m_RTSize.zw );
  c.a = 1.0;
  return c;
}


void main()
{
	vec4 sampleColor;

	if( aberrationIntensity > 0.0 )
	{
		sampleColor.r = texture2D( texture, textureUV.xy + vec2(0.001, 0.0007) * aberrationIntensity ).r;
		sampleColor.g = texture2D( texture, textureUV.xy + vec2(-0.0007, 0.001) * aberrationIntensity ).g;
		sampleColor.b = texture2D( texture, textureUV.xy + vec2(-0.001, -0.001) * aberrationIntensity ).b;

		sampleColor.a = texture2D( texture, textureUV.xy ).a;		
	}
	else
	{
		if( m_enableFXAA != 0 )
			sampleColor = PostFX( texture, textureUV.xyzw, 0.0 );
		else
			sampleColor = texture2D( texture, textureUV.xy );
	}

	vec3 result = pow( max( vec3(0,0,0), (sampleColor.rgb-shadows) )*(highlights), midTones );

	if( bwScale + aberrationIntensity != 0.0 )
	{
		float brightness = dot( result, vec3(0.299, 0.587, 0.114) );
		gl_FragColor = vec4( mix( result, vec3( brightness, brightness, brightness ), bwScale ), sampleColor.a );
	}
	else
	{
		gl_FragColor = vec4( result, sampleColor.a );
	}

	if( m_vignette.x > 0.0 )
	{
		float centreDist = clamp( length( textureUV - vec2( 0.5, 0.5 ) ), m_vignette.y, m_vignette.z ) - m_vignette.y;
		float vignetteSize = m_vignette.z - m_vignette.y;

		gl_FragColor *= 1.0 - centreDist * m_vignette.x / vignetteSize;
	}
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/