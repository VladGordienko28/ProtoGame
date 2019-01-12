/*=============================================================================
    FrColor.h: TColor & color constants.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    TColor.
-----------------------------------------------------------------------------*/

//
// A RGBA color.
//
struct TColor
{
public:
	// Variables.
	union
	{
		struct{ UInt8 R, G, B, A; };
		UInt32 D;
	};

	// Constructor.
	TColor()
		:	D( 0 )
	{}
	TColor( UInt32 InD )
		:	D( InD )
	{}
	TColor( UInt8 InR, UInt8 InG, UInt8 InB, UInt8 InA )
		:	R( InR ), G( InG ), B( InB ), A( InA )
	{}
	TColor( Float InR, Float InG, Float InB )
		:	R( Clamp(Floor(InR*256.f), 0, 255) ), 
			G( Clamp(Floor(InG*256.f), 0, 255) ), 
			B( Clamp(Floor(InB*256.f), 0, 255) ), 
			A( 0xff )
	{}

	// Operators.
	TColor operator-() const
	{
		return TColor( ~D );
	}
	TColor operator+() const
	{
		return TColor( D );
	}
	TColor operator+( const TColor C ) const
	{
		return TColor
					( 
						Min( 0xff, (Int32)R + (Int32)C.R ),
						Min( 0xff, (Int32)G + (Int32)C.G ),
						Min( 0xff, (Int32)B + (Int32)C.B ),
						Min( 0xff, (Int32)A + (Int32)C.A ) 
					);
	}
	TColor operator-( const TColor C ) const
	{
		return TColor
					( 
						Max( 0x00, (Int32)R - (Int32)C.R ),
						Max( 0x00, (Int32)G - (Int32)C.G ),
						Max( 0x00, (Int32)B - (Int32)C.B ),
						Max( 0x00, (Int32)A - (Int32)C.A ) 
					);
	}
	TColor operator*( UInt8 Brig ) const
	{
		return TColor
					( 
						((Int32)R * (Int32)Brig) >> 8,
				   		((Int32)G * (Int32)Brig) >> 8,
						((Int32)B * (Int32)Brig) >> 8,
						((Int32)A * (Int32)Brig) >> 8 
					);
	}
	TColor operator*( const TColor& C ) const
	{
		return TColor( ((Int32)R * (Int32)C.R) >> 8,
				   	   ((Int32)G * (Int32)C.G) >> 8,
					   ((Int32)B * (Int32)C.B) >> 8,
					   ((Int32)A * (Int32)C.A) >> 8 );
	}
	TColor operator*( const Float F ) const
	{
		return TColor
					(
						Clamp( Floor(R*F), 0, 255 ),
						Clamp( Floor(G*F), 0, 255 ),
						Clamp( Floor(B*F), 0, 255 ),
						/*Clamp( Floor(A*F), 0, 255 )*/A
					);	
	}
	TColor operator+=( const TColor C )
	{
		R	= Min( 0xff, (Int32)R + (Int32)C.R );
		G	= Min( 0xff, (Int32)G + (Int32)C.G );
		B	= Min( 0xff, (Int32)B + (Int32)C.B );
		A	= Min( 0xff, (Int32)A + (Int32)C.A );
		return *this;
	}
	TColor operator-=( const TColor C )
	{
		R	= Max( 0x00, (Int32)R - (Int32)C.R );
		G	= Max( 0x00, (Int32)G - (Int32)C.G );
		B	= Max( 0x00, (Int32)B - (Int32)C.B );
		A	= Max( 0x00, (Int32)A - (Int32)C.A );
		return *this;
	}
	TColor operator*=( const TColor C )
	{
		R	= ((Int32)R * (Int32)C.R) >> 8;
		G	= ((Int32)G * (Int32)C.G) >> 8;
		B	= ((Int32)B * (Int32)C.B) >> 8;
		A	= ((Int32)A * (Int32)C.A) >> 8;
		return *this;
	}
	TColor operator*=( Float F )
	{
		R	=	Clamp( Floor(R*F), 0, 255 );
		G	=	Clamp( Floor(G*F), 0, 255 );
		B	=	Clamp( Floor(B*F), 0, 255 );
		A	=	/*Clamp( Floor(A*F), 0, 255 )*/A;
		return *this;
	}

	TColor operator*=( UInt8 Brig )
	{
		R	= ((Int32)R * (Int32)Brig) >> 8;
		G	= ((Int32)G * (Int32)Brig) >> 8;
		B	= ((Int32)B * (Int32)Brig) >> 8;
		A	= ((Int32)A * (Int32)Brig) >> 8;
		return *this;
	}

	Bool operator==( const TColor C ) const
	{
		return D == C.D;
	}
	Bool operator!=( const TColor C ) const
	{
		return D != C.D;
	}

	// Functions.
	friend void Serialize( CSerializer& S, TColor& V )
	{
		Serialize( S, V.R );
		Serialize( S, V.G );
		Serialize( S, V.B );
		Serialize( S, V.A );
	}	
	friend TColor Lerp( TColor A, TColor B, Float Alpha )
	{
		TColor Result;
		Result.R = Float(A.R) + (Float(B.R)-Float(A.R))*Alpha;
		Result.G = Float(A.G) + (Float(B.G)-Float(A.G))*Alpha;
		Result.B = Float(A.B) + (Float(B.B)-Float(A.B))*Alpha;
		Result.A = Float(A.A) + (Float(B.A)-Float(A.A))*Alpha;
		return Result;
	}

	static void RGBToHSL( TColor Color, UInt8& H, UInt8& S, UInt8& L );
	static TColor HSLToRGB( UInt8 H, UInt8 S, UInt8 L );
};


/*-----------------------------------------------------------------------------
    TColor standard constants.
-----------------------------------------------------------------------------*/

// CSS colors.
#define COLOR_AliceBlue				TColor( 0xf0, 0xf8, 0xff, 0xff )
#define COLOR_AntiqueWhite			TColor( 0xfa, 0xeb, 0xd7, 0xff )
#define COLOR_Aqua					TColor( 0x00, 0xff, 0xff, 0xff )
#define COLOR_Aquamarine			TColor( 0x7f, 0xff, 0xd4, 0xff )
#define COLOR_Azure					TColor( 0xf0, 0xff, 0xff, 0xff )
#define COLOR_Beige					TColor( 0xf5, 0xf5, 0xdc, 0xff )
#define COLOR_Bisque				TColor( 0xff, 0xe4, 0xc4, 0xff )
#define COLOR_Black					TColor( 0x00, 0x00, 0x00, 0x00 )
#define COLOR_BlanchedAlmond		TColor( 0xff, 0xeb, 0xcd, 0xff )
#define COLOR_Blue					TColor( 0x00, 0x00, 0xff, 0xff )
#define COLOR_BlueViolet			TColor( 0x8a, 0x2b, 0xe2, 0xff )
#define COLOR_Brown					TColor( 0xa5, 0x2a, 0x2a, 0xff )
#define COLOR_BurlyWood				TColor( 0xde, 0xb8, 0x87, 0xff )
#define COLOR_CadetBlue				TColor( 0x5f, 0x9e, 0xa0, 0xff )
#define COLOR_Chartreuse			TColor( 0x7f, 0xff, 0x00, 0xff )
#define COLOR_Chocolate				TColor( 0xd2, 0x69, 0x1e, 0xff )
#define COLOR_Coral					TColor( 0xff, 0x7f, 0x50, 0xff )
#define COLOR_CornflowerBlue		TColor( 0x64, 0x95, 0xed, 0xff )
#define COLOR_Cornsilk				TColor( 0xff, 0xf8, 0xdc, 0xff )
#define COLOR_Crimson				TColor( 0xdc, 0x14, 0x3c, 0xff )
#define COLOR_Cyan					TColor( 0x00, 0xff, 0xff, 0xff )
#define COLOR_DarkBlue				TColor( 0x00, 0x00, 0x8b, 0xff )
#define COLOR_DarkCyan				TColor( 0x00, 0x8b, 0x8b, 0xff )
#define COLOR_DarkGoldenrod			TColor( 0xb8, 0x86, 0x0b, 0xff )
#define COLOR_DarkGray				TColor( 0xa9, 0xa9, 0xa9, 0xff )
#define COLOR_DarkGreen				TColor( 0x00, 0x64, 0x00, 0xff )
#define COLOR_DarkKhaki				TColor( 0xbd, 0xb7, 0x6b, 0xff )
#define COLOR_DarkMagenta			TColor( 0x8b, 0x00, 0x8b, 0xff )
#define COLOR_DarkOliveGreen		TColor( 0x55, 0x6b, 0x2f, 0xff )
#define COLOR_DarkOrange			TColor( 0xff, 0x8c, 0x00, 0xff )
#define COLOR_DarkOrchid			TColor( 0x99, 0x32, 0xcc, 0xff )
#define COLOR_DarkRed				TColor( 0x8b, 0x00, 0x00, 0xff )
#define COLOR_DarkSalmon			TColor( 0xe9, 0x96, 0x7a, 0xff )
#define COLOR_DarkSeaGreen			TColor( 0x8f, 0xbc, 0x8f, 0xff )
#define COLOR_DarkSlateBlue			TColor( 0x48, 0x3d, 0x8b, 0xff )
#define COLOR_DarkSlateGray			TColor( 0x2f, 0x4f, 0x4f, 0xff )
#define COLOR_DarkTurquoise			TColor( 0x00, 0xce, 0xd1, 0xff )
#define COLOR_DarkViolet			TColor( 0x94, 0x00, 0xd3, 0xff )
#define COLOR_DeepPink				TColor( 0xff, 0x14, 0x93, 0xff )
#define COLOR_DeepSkyBlue			TColor( 0x00, 0xbf, 0xff, 0xff )
#define COLOR_DimGray				TColor( 0x69, 0x69, 0x69, 0xff )
#define COLOR_DodgerBlue			TColor( 0x1e, 0x90, 0xff, 0xff )
#define COLOR_FireBrick				TColor( 0xb2, 0x22, 0x22, 0xff )
#define COLOR_FloralWhite			TColor( 0xff, 0xfa, 0xf0, 0xff )
#define COLOR_ForestGreen			TColor( 0x22, 0x8b, 0x22, 0xff )
#define COLOR_Fuchsia				TColor( 0xff, 0x00, 0xff, 0xff )
#define COLOR_Gainsboro				TColor( 0xdc, 0xdc, 0xdc, 0xff )
#define COLOR_GhostWhite			TColor( 0xf8, 0xf8, 0xff, 0xff )
#define COLOR_Gold					TColor( 0xff, 0xd7, 0x00, 0xff )
#define COLOR_Goldenrod				TColor( 0xda, 0xa5, 0x20, 0xff )
#define COLOR_Gray					TColor( 0x80, 0x80, 0x80, 0xff )
#define COLOR_Green					TColor( 0x00, 0x80, 0x00, 0xff )
#define COLOR_GreenYellow			TColor( 0xad, 0xff, 0x2f, 0xff )
#define COLOR_HoneyDew				TColor( 0xf0, 0xff, 0xf0, 0xff )
#define COLOR_HotPink				TColor( 0xff, 0x69, 0xb4, 0xff )
#define COLOR_IndianRed				TColor( 0xcd, 0x5c, 0x5c, 0xff )
#define COLOR_Indigo				TColor( 0x4b, 0x00, 0x82, 0xff )
#define COLOR_Ivory					TColor( 0xff, 0xff, 0xf0, 0xff )
#define COLOR_Khaki					TColor( 0xf0, 0xe6, 0x8c, 0xff )
#define COLOR_Lavender				TColor( 0xe6, 0xe6, 0xfa, 0xff )
#define COLOR_LavenderBlush			TColor( 0xff, 0xf0, 0xf5, 0xff )
#define COLOR_LawnGreen				TColor( 0x7c, 0xfc, 0x00, 0xff )
#define COLOR_LemonChiffon			TColor( 0xff, 0xfa, 0xcd, 0xff )
#define COLOR_LightBlue				TColor( 0xad, 0xd8, 0xe6, 0xff )
#define COLOR_LightCoral			TColor( 0xf0, 0x80, 0x80, 0xff )
#define COLOR_LightCyan				TColor( 0xe0, 0xff, 0xff, 0xff )
#define COLOR_LightGray				TColor( 0xd3, 0xd3, 0xd3, 0xff )
#define COLOR_LightGreen			TColor( 0x90, 0xee, 0x90, 0xff )
#define COLOR_LightPink				TColor( 0xff, 0xb6, 0xc1, 0xff )
#define COLOR_LightSalmon			TColor( 0xff, 0xa0, 0x7a, 0xff )
#define COLOR_LightSeaGreen			TColor( 0x20, 0xb2, 0xaa, 0xff )
#define COLOR_LightSkyBlue			TColor( 0x87, 0xce, 0xfa, 0xff )
#define COLOR_LightSlateGray		TColor( 0x77, 0x88, 0x99, 0xff )
#define COLOR_LightSteelBlue		TColor( 0xb0, 0xc4, 0xde, 0xff )
#define COLOR_LightYellow			TColor( 0xff, 0xff, 0xe0, 0xff )
#define COLOR_Lime					TColor( 0x00, 0xff, 0x00, 0xff )
#define COLOR_LimeGreen				TColor( 0x32, 0xcd, 0x32, 0xff )
#define COLOR_Linen					TColor( 0xfa, 0xf0, 0xe6, 0xff )
#define COLOR_Magenta				TColor( 0xff, 0x00, 0xff, 0xff )
#define COLOR_Maroon				TColor( 0x80, 0x00, 0x00, 0xff )
#define COLOR_MediumAquamarine		TColor( 0x66, 0xcd, 0xaa, 0xff )
#define COLOR_MediumBlue			TColor( 0x00, 0x00, 0xcd, 0xff )
#define COLOR_MediumOrchid			TColor( 0xba, 0x55, 0xd3, 0xff )
#define COLOR_MediumPurple			TColor( 0x93, 0x70, 0xdb, 0xff )
#define COLOR_MediumSeaGreen		TColor( 0x3c, 0xb3, 0x71, 0xff )
#define COLOR_MediumSlateBlue		TColor( 0x7b, 0x68, 0xee, 0xff )
#define COLOR_MediumSpringGreen		TColor( 0x00, 0xfa, 0x9a, 0xff )
#define COLOR_MediumTurquoise		TColor( 0x48, 0xd1, 0xcc, 0xff )
#define COLOR_MediumVioletRed		TColor( 0xc7, 0x15, 0x85, 0xff )
#define COLOR_MidnightBlue			TColor( 0x19, 0x19, 0x70, 0xff )
#define COLOR_MintCream				TColor( 0xf5, 0xff, 0xf1, 0xff )
#define COLOR_MistyRose				TColor( 0xff, 0xe4, 0xe1, 0xff )
#define COLOR_Moccasin				TColor( 0xff, 0xe4, 0xb5, 0xff )
#define COLOR_NavajoWhite			TColor( 0xff, 0xde, 0xad, 0xff )
#define COLOR_Navy					TColor( 0x00, 0x00, 0x80, 0xff )
#define COLOR_OldLace				TColor( 0xfd, 0xf5, 0xe6, 0xff )
#define COLOR_Olive					TColor( 0x80, 0x80, 0x00, 0xff )
#define COLOR_OliveDrab				TColor( 0x6b, 0x8e, 0x23, 0xff )
#define COLOR_Orange				TColor( 0xff, 0xa5, 0x00, 0xff )
#define COLOR_OrangeRed				TColor( 0xff, 0x45, 0x00, 0xff )
#define COLOR_Orchid				TColor( 0xda, 0x70, 0xd6, 0xff )
#define COLOR_PaleGoldenrod			TColor( 0xee, 0xe8, 0xaa, 0xff )
#define COLOR_PaleGreen				TColor( 0x98, 0xfb, 0x98, 0xff )
#define COLOR_PaleTurquise			TColor( 0xaf, 0xee, 0xee, 0xff )
#define COLOR_PaleVioletRed			TColor( 0xdb, 0x70, 0x93, 0xff )
#define COLOR_PapayaWhip			TColor( 0xff, 0xef, 0xd5, 0xff )
#define COLOR_PeachPuff				TColor( 0xff, 0xda, 0xb9, 0xff )
#define COLOR_Peru					TColor( 0xcd, 0x85, 0x3f, 0xff )
#define COLOR_Pink					TColor( 0xff, 0xc0, 0xcb, 0xff )
#define COLOR_Plum					TColor( 0xdd, 0xa0, 0xdd, 0xff )
#define COLOR_PowderBlue			TColor( 0xb0, 0xe0, 0xe6, 0xff )
#define COLOR_Purple				TColor( 0x80, 0x00, 0x80, 0xff )
#define COLOR_Red					TColor( 0xff, 0x00, 0x00, 0xff )
#define COLOR_RosyBrown				TColor( 0xbc, 0x8f, 0x8f, 0xff )
#define COLOR_RoyalBlue				TColor( 0x41, 0x69, 0xe1, 0xff )
#define COLOR_SaddleBrown			TColor( 0x8b, 0x45, 0x13, 0xff )
#define COLOR_Salmon				TColor( 0xfa, 0x80, 0x72, 0xff )
#define COLOR_SandyBrown			TColor( 0xf4, 0xa4, 0x60, 0xff )
#define COLOR_SeaGreen				TColor( 0x2e, 0x8b, 0x57, 0xff )
#define COLOR_SeaShell				TColor( 0xff, 0xf5, 0xee, 0xff )
#define COLOR_Sienna				TColor( 0xa0, 0x52, 0x2d, 0xff )
#define COLOR_Silver				TColor( 0xc0, 0xc0, 0xc0, 0xff )
#define COLOR_SkyBlue				TColor( 0x87, 0xce, 0xeb, 0xff )
#define COLOR_SlateBlue				TColor( 0x6a, 0x5a, 0xcd, 0xff )
#define COLOR_SlateGray				TColor( 0x70, 0x80, 0x90, 0xff )
#define COLOR_Snow					TColor( 0xff, 0xfa, 0xfa, 0xff )
#define COLOR_SpringGreen			TColor( 0x00, 0xff, 0x7f, 0xff )
#define COLOR_SteelBlue				TColor( 0x46, 0x82, 0xb4, 0xff )
#define COLOR_Tan					TColor( 0xd2, 0xb4, 0x8c, 0xff )
#define COLOR_Teal					TColor( 0x00, 0x80, 0x80, 0xff )
#define COLOR_Thistle				TColor( 0xd8, 0xbf, 0xd8, 0xff )
#define COLOR_Tomato				TColor( 0xff, 0x63, 0x47, 0xff )
#define COLOR_Torquoise				TColor( 0x40, 0xe0, 0xd0, 0xff )
#define COLOR_Violen				TColor( 0xee, 0x82, 0xee, 0xff )
#define COLOR_Wheat					TColor( 0xf5, 0xde, 0xb3, 0xff )
#define COLOR_White					TColor( 0xff, 0xff, 0xff, 0xff )
#define COLOR_WhiteSmoke			TColor( 0xf5, 0xf5, 0xf5, 0xff )
#define COLOR_Yellow				TColor( 0xff, 0xff, 0x00, 0xff )
#define COLOR_YellowGreen			TColor( 0x9a, 0xcd, 0x32, 0xff )

// Special highlight colors.
#define COLOR_Highlight1			TColor( 0x50, 0x50, 0x50, 0xff )
#define COLOR_Highlight2			TColor( 0x96, 0x96, 0x96, 0xff )


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/