//-----------------------------------------------------------------------------
//	Colored.hlsl: A simple shader for flat shaded primitives
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

cbuffer Transforms : register( b0 )
{
	float4x4 projectionMatrix;
}

cbuffer Shading : register ( b1 )
{
	float4 color;
}

struct VsInput
{
	float2 position : POSITION;
};

struct VsOutput
{
	float4 position : SV_Position;
};

VsOutput vsMain( in VsInput input )
{
	VsOutput output;
	output.position = mul( float4( input.position, 0.f, 1.f ), projectionMatrix );
	return output;
}

float4 psMain( in VsOutput input ) : SV_Target
{
	return color;
}