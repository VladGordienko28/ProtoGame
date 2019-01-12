/*=============================================================================
    FrTest.cpp: File for research.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"


// Temporal input system!!!!!!!!!!!!!!
// Funky shit!!
//Bool	GInput[512];		// Kill it!!! 


//
// Special macro to invoke function.
//
#define INVOKE( func )\
	Int32 Invoke_##func()\
	{\
		func();\
		return 0;\
	}\
	static Int32 var_##func = Invoke_##func();\


// New string code testing.
Int32 tmp()
{

	





	/*
	String S = L"Vlad";
	String S1 = S + L"best";

	TArray<String> Arr;
	Arr.Push( S1 );


	String A = Arr.Pop();

	*/



	/*
	CoolString S, T;

	S = L"Vlad";

	T = S;

	T[3] = 'y';

	Char C = T[3];
	*/
	return 0;
}

INVOKE(tmp);




//
// @Note:
// Press <Ctrl>+<Alt>+<D> - to open disassembler.
//


/*
bool somefunc() {return false;};


bool a = false, b = somefunc(), c = true;



class EClass
{
public:
	EClass() { log(L"EClass::EClass"); };
	virtual ~EClass() { log(L"EClass::~EClass"); };
};




class AClass
{
public:
	AClass() { log(L"AClass::AClass"); };
	virtual ~AClass() { log(L"AClass::~AClass"); };
};

class BClass : public AClass
{
public:
	BClass() { log(L"BClass::BClass"); };
 ~BClass() { log(L"BClass::~BClass"); };
};

class DClass : public BClass, public EClass
{
public:
	DClass() { log(L"DClass::DClass"); };
	 ~DClass() { log(L"DClass::~DClass"); };
};


bool MyFunc()
{

	AClass* x = new DClass;
	delete x;


	return 0;
	*/


	/*
	switch (3)
	{
	default:
		break;

//	default:
	//	break;
	case 1:
		break;
//	case 1:
//		break;
	}


	
	return a ? b : c;*/
/*
}
*/

/*
DWord Screen[1920], Bitmap[128*128];
Integer X = 100;
QWord Walk, DWalk, VMask;



bool MyFunc()
{
	DWord *Dest=Screen, *Src=Bitmap;

	while( X-- > 0 )
	{
		*Dest++ = Src[(Walk>>57) + ((Walk>>25) & VMask)];
		Walk += DWalk;
	}

	return 0;

}


INVOKE( MyFunc )
*/



/*		
					PURE FOR RESEARCH EXPORTER.

class CDebugExporter: public CExporterBase
{
public:
	void ExportByte		( const String FieldName, Byte Value		) 
	{
		log( L"%s = %d", *FieldName, Value );
	}
	void ExportInteger	( const String FieldName, Integer Value		)
	{
		log( L"%s = %d", *FieldName, Value );
	}
	void ExportFloat	( const String FieldName, Float Value		)
	{
		log( L"%s = %f", *FieldName, Value );
	}
	void ExportString	( const String FieldName, String Value		)
	{
		log( L"%s = \"%s\"", *FieldName, *Value );
	}
	void ExportBool		( const String FieldName, Bool Value		)
	{
		log( L"%s = %s", *FieldName, Value ? L"true" : L"false" );
	}
	void ExportColor	( const String FieldName, TColor Value		)
	{
		log( L"%s = [%h %h %h %h]", *FieldName, Value.R, Value.G, Value.B, Value.A );
	}
	void ExportVector	( const String FieldName, TVector Value		)
	{
		log( L"%s = [%f %f]", *FieldName, Value.X, Value.Y );
	}
	void ExportAABB		( const String FieldName, TRect Value		) 
	{
	}
	void ExportAngle	( const String FieldName, TAngle Value		)
	{
	}
	void ExportObject	( const String FieldName, FObject* Value	)
	{
	}
};*/



/*		STORED TRANSFORMATION CODE!!!!


void COpenGLCanvas::SetTransform( const TViewInfo& Info )
{

		glMatrixMode( GL_PROJECTION );
	glLoadIdentity();


	// Computecells.

	TVector SScale, SOffset;


	
	TVector WinToGL;


	WinToGL.X = 2.f / ScreenWidth;
	WinToGL.Y = 2.f / ScreenHeight;


	SOffset.X =  WinToGL.X * (Info.X + (Info.Width/2)) - 1.f; 
	SOffset.Y =  1.f-WinToGL.Y * (Info.Y + (Info.Height/2)) ; 



	SScale.X = (Info.Width / ScreenWidth);
	SScale.Y = (Info.Height / ScreenHeight);



glTranslatef( SOffset.X, SOffset.Y, 0.0 );	

	
	glScalef( SScale.X, SScale.Y, 1 );

	






	GLfloat M[4][4];
	Float XFOV2, YFOV2;

	// Reset old matrix.
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// Precompute.
	XFOV2	= 2.f / (Info.FOV.X * Info.Zoom);
	YFOV2	= 2.f / (Info.FOV.Y * Info.Zoom);

	// Store this coords system.
	View			= Info;

	// Computecells.
    M[0][0]	= XFOV2 * +Info.Coords.XAxis.X;
    M[0][1] = YFOV2 * -Info.Coords.XAxis.Y;
    M[0][2] = 0.f;
    M[0][3] = 0.f;

    M[1][0] = XFOV2 * -Info.Coords.YAxis.X;
    M[1][1] = YFOV2 * +Info.Coords.YAxis.Y;
    M[1][2] = 0.f;
    M[1][3] = 0.f;

    M[2][0] = 0.f;
    M[2][1] = 0.f;
    M[2][2] = 1.f;
    M[2][3] = 0.f;

    M[3][0] = -(Info.Coords.Origin.X*M[0][0] + Info.Coords.Origin.Y*M[1][0]);
    M[3][1] = -(Info.Coords.Origin.X*M[0][1] + Info.Coords.Origin.Y*M[1][1]);
    M[3][2] = 0.f;
    M[3][3] = 1.f;

	
    // Send this matrix to OpenGL.
    glLoadMatrixf( (GLfloat*)M );






}


*/
//
// Setup OpenGL's projection matrix for render coords
// system. Coords.Origin - is a center of camera,
// Coords.XAxis, Coords.YAxis - is a axis vectors,
// Coords.XAxis.Size() is a FOV.X value
//

//
// Setup OpenGL's projection matrix for render node's coords
// system. Note: To draw routines give vertices in
// ERendTransform coords system!
//


// 	// pure for research.
// 	TVector SphereMapping( TVector Center, TVector Vert, TVector Eye )
// 	{
// 		
// 		TVector Normal = Vert - Center;
// 		Normal.Normalize();
// 
// 		Eye.Normalize();
// 
// 		TVector Refl = Eye - (Normal*(Normal*Eye))*2;
// 		Float P = 2*Sqrt( Refl.X*Refl.X + Refl.Y*Refl.Y +1);
// 		return TVector( Refl.X/P+0.5, Refl.Y/P+0.5 );
// 		
// 
// 		/*
// 		TVector Normal = Vert - Center;
// 		Normal += TVector( Sin(Eye.X+GPlat->TimeStamp()), Cos(Eye.Y+GPlat->TimeStamp()) );
// 
// 		Normal.Normalize();
// 		Normal += TVector(1, 1);
// 		Normal = Normal * 0.5;
// 
// 
// 		return Normal;
// 		*/
// 
// 		/*
// 		Float Time = GPlat->TimeStamp();
// 		TVector Res;
// 		TVector Dir = Vert + Eye;
// 
// 		Res.X = Cos(Time*0.2+Dir.X*0.1) + Cos(Time*0.5+Dir.X*0.01); 
// 		Res.Y = Sin(Time*0.2+Dir.Y*0.1) + Sin(Time*0.5+Dir.Y*0.01); 
// 		return Res;
// 		*/
// 
// 		/*
// 		TVector Normal = Eye+ Vert - Center;
// 		Normal.Normalize();
// 		Normal += TVector(1, 1);
// 		Normal = Normal * 0.5;
// 		return Normal;
// 		*/
// 	}





/*
void CSGUnion( FBrushComponent* Brush, FLevel* Level )
{
	assert(Brush);
	assert(Level);

	log( L"CSG: Union" );

	// Prepare.
	Integer NumEnts = Level->Entities.Num();
	Bool bHasAffect = false;
	TCSGPoly Poly, OtherPoly;
	TRect Rect;

	Poly.FromBrush( Brush );
	Rect = Brush->GetAABB();

	// Test with all the first overlap brush in the level.
	for( Integer i=0; i<NumEnts; i++ )
	{
		// Simple tests to reject.
		if( (Level->Entities[i]->Base == Brush) ||
			(!Level->Entities[i]->Base->IsA(FBrushComponent::MetaClass) ) ||
			(Level->Entities[i]->Base->bDestroyed) )
				continue;
			
		FBrushComponent* Other = (FBrushComponent*)Level->Entities[i]->Base;

		// Another cheap test.
		if( !Rect.IsOverlap( Other->GetAABB() ) )
			continue;

		OtherPoly.FromBrush(Other);

		// Here we split poly by otherpoly and process each chunks.
		TVector V1 = OtherPoly.Vertices[OtherPoly.NumVerts-1];
		for( Integer j=0; j<OtherPoly.NumVerts; j++ )
		{
			TVector V2 = OtherPoly.Vertices[j];

			if( Poly.TestSegment( V1, V2 ) )
			{
				// This segment intersect poly, split it now.
				TCSGPoly Front, Back;				
				bHasAffect = true;

				Poly.Split( V1, V2, Front, Back );

				// Add front chunk to world and process it.
				if( Front.NumVerts >= 3 )
				{
					FBrushComponent* FrontBrush = Front.ToBrush( Level, Brush );
					CSGUnion( FrontBrush, Level );
				}

				// Continue process back.
				Poly = Back;
			}

			V1 = V2;
		}

		// Check special case, all vertices are totally inside.
		if( !bHasAffect )
		{
			Integer NumIns = 0;
			for( Integer j=0; j<OtherPoly.NumVerts; j++ )
				if( IsPointInsidePoly( OtherPoly.Vertices[j], Poly.Vertices, Poly.NumVerts ) )
					NumIns++;

			bHasAffect = NumIns == OtherPoly.NumVerts;
		}

		if( bHasAffect )
			break;
	}

	// Remove parent.
	if( bHasAffect )
		Level->DestroyEntity( Brush->Entity );
}
*/



/*

//
// Merge all overlaps vertices.
//
void TCSGPoly::MergeOverlap()
{
	for( Integer v=0; v<NumVerts; v++ )
	{
		TVector&	V1 = Vertices[v],
					V2 = Vertices[(v+1) % NumVerts];

		if( (V1-V2).SizeSquared() <= 0.25f )
		{
			for( Integer i=v; i<NumVerts-1; i++ )
				Vertices[i] = Vertices[i+1];

			v--;
			NumVerts--;
		}
	}
}


//
// Compute area of the CSG poly.
//
Float TCSGPoly::Area()
{
	Float	Area	= 0.f;
	TVector	Edge1	= Vertices[1] - Vertices[0], Edge2;

	for( Integer i=2; i<NumVerts; i++ )
	{
		Edge2 = Vertices[i] - Vertices[0];
		Area	+= Abs(Edge1 / Edge2);
		Edge1 = Edge2;
	}

	return Area;
}
*/







/*

///////////////////////////////////////////////////////////////////////////////////
//
// Draw a text.
//
void CCanvas::DrawText(	const String& S, 
						FFont* Font, 
						TColor Color,
						const TVector Start, 
						const TVector Scale )
{
	assert(Font);

	//CLIPPING + IS REALLY VISIBLE/


	// Preinitialize.
	TRenderRect R;
	R.Color			= Color;
	R.Flags			= POLY_Unlit;
	R.Rotation		= 0.f;

	TVector Walk = Start;


	// add handle whitespace and new line!!!


	// For each character in string.
	for( Integer i=0; i<S.Len(); i++ )
	{
		if( S[i] == L' ' )
		{
			TGlyph& Glyph = Font->GetGlyph(L'A');
			Walk.X += Glyph.W * Scale.X;
			continue;
		}

		TGlyph& Glyph = Font->GetGlyph(S[i]);
		TVector CharSize = TVector( Glyph.W * Scale.X, Glyph.H * Scale.Y );

		


		R.Bitmap		= Font->Bitmaps[Glyph.iBitmap];

		R.Bounds.Min	= Walk;
		R.Bounds.Max	= Walk + CharSize;

		R.TexCoords.Min = TVector( Glyph.X, Glyph.Y )* (1/ 256.f);
		R.TexCoords.Max = TVector( Glyph.X+Glyph.W, Glyph.Y+Glyph.H )* (1/ 256.f);





		DrawRect(R);



		Walk.X += CharSize.X;
	}
}/////////////////////////////////////////////////////////////////////////////////
*/



/*
				assert(String::Pos( L"Component", Class->Alt ) != -1);
				ExtraList->AddItem( String::Copy( Class->Alt, 0, Class->Alt.Len()-9 ), Class );

*/


	/*
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"C++ Rocks!", COLOR_Lime );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"C++ Rocks!", COLOR_Lime );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"Delphi cool too", COLOR_Red );
	Output->AddRecord( L"C++ Rocks!", COLOR_Lime );

	*/


/*

//
// Pathnode serialization.
//
void Serialize( CSerializer& S, TPathNode& V )
{
	Serialize( S, V.Location );
	Serialize( S, V.Marker );

	for( Integer i=0; i<array_length(V.iEdges); i++ )
		Serialize( S, V.iEdges[i] );
}


//
// Path edge serialization.
//
void Serialize( CSerializer& S, TPathEdge& V )
{
	Serialize( S, V.iStart );
	Serialize( S, V.iFinish );
	SerializeEnum( S, V.PathType );
	Serialize( S, V.Cost );
	Serialize( S, V.Height );
}

*/



/*
//
// A byte stream.
//
struct TStream
{
public:
	Byte*		Data;
	Integer		Pos;
	Integer		Size;

	TStream( void* InData, DWord InSize )
		:	Data((Byte*)InData), Size(InSize), Pos(0)
	{}
	void operator++()
	{
		Pos	= Min( Pos+1, Size-1 );
	}
	void operator--()
	{
		Pos	= Max( Pos-1, 0 );
	}
	Byte& operator*()
	{
		return Data[Pos];
	}
	const Byte& operator*() const
	{
		return Data[Pos];
	}
};
*/

	//@fix me Vlad: I should find the way to import/export the dynamic array of the wires!
/*
	for( Integer i=0; i<MAX_LOGIC_PLUGS; i++ )
	{
		for( Integer j=0; j<Plugs[i].Num(); j++ )
		{
			Plugs[i][j].iJack = Im.ImportInteger( *String::Format( L"Plugs[%d][%d].iJack", i, j ) );
			Plugs[i][j].Target = (FLogicComponent*)Im.ImportObject( *String::Format( L"Plugs[%d][%d].Target", i, j ) );
		}
	}
*/



/*
	// Without horrible whitespace after frames, for 
	// power-of-two size.
	Integer FrmPerX	= Floor((Float)(Sheet->USize) / (Float)(FrameW));		
	Integer FrmPerY	= Floor((Float)(Sheet->VSize) / (Float)(FrameH));

	for( Integer Y=0; Y<FrmPerY; Y++ )
	for( Integer X=0; X<FrmPerX; X++ )
	{
		TRect R;

		R.Min.X	= (Float)((X+0)*FrameW) / (Float)(Sheet->USize);
		R.Min.Y	= (Float)((Y+0)*FrameH) / (Float)(Sheet->VSize);

		R.Max.X	= (Float)((X+1)*FrameW) / (Float)(Sheet->USize);
		R.Max.Y	= (Float)((Y+1)*FrameH) / (Float)(Sheet->VSize);

		// Flip V.
		Exchange( R.Min.Y, R.Max.Y );

		Frames.Push( R );
	}*/



// #if 0
// 
// 	//////////////////////////////////////////////////
// 	WComboBox* Combo = new WComboBox( GUIWindow, GUIWindow );
// 
// 
// 	Combo->SetLocation( 10, 10 );
// 
// 	Combo->AddItem( L"One", 0 );
// 	Combo->AddItem( L"Two", 0 );
// 	Combo->AddItem( L"Three", 0 );
// 	Combo->AddItem( L"Four", 0 );
// 	Combo->AddItem( L"Five", 0 );
// 	Combo->AddItem( L"Six", 0 );
// 
// 
// 	//////////////////////////////////////////////////
// 
// #endif

	/*

	{
		Byte Buffer[] = { 1, 2, 7, 7, 7, 7, 7, 7, 5, 9, 6, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
		void* Out, *Dest;
		DWord OutSize, DstSize;

		CLZWCompressor lzw;
		lzw.Encode( Buffer, sizeof(Buffer), Out, OutSize );

		lzw.Decode( Out, OutSize, Dest, DstSize );


		for( Integer i=0; i<DstSize; i++ )
			wprintf( L"%d ", ((Byte*)Dest)[i] );
	
		mem::free( Out );
		mem::free(Dest);
	}

	*/


/*

FScript*	CameraScript;
FScript*	BrushScript;
void InitScripts()
{
	FComponent* Com;

FScript*	ZoneScript;
FScript*	BoxScript;

FScript*	SkyScript;
FScript*	ModelScript;
FScript*	WarpScript;



	// Camera.
	CameraScript	= NewObject<FScript>(L"Camera");
	Com				= NewObject<FCameraComponent>( L"CameraComponent", CameraScript ); 
	Com->InitForScript( CameraScript );


		// Zone.
	ZoneScript	= NewObject<FScript>(L"Zone");
	Com				= NewObject<FZoneComponent>( L"ZoneComponent", ZoneScript ); 
	Com->InitForScript( ZoneScript );


	// Warp.
	WarpScript	= NewObject<FScript>(L"Warp");
	Com				= NewObject<FWarpComponent>( L"WarpComponent", WarpScript ); 
	Com->InitForScript( WarpScript );

	// Mirror.
	FScript* MirrorScript	= NewObject<FScript>(L"Mirror");
	Com				= NewObject<FMirrorComponent>( L"Mirror", MirrorScript ); 
	Com->InitForScript( MirrorScript );




			// Sky.
	SkyScript	= NewObject<FScript>(L"Sky");
	Com				= NewObject<FSkyComponent>( L"SkyComponent", SkyScript ); 
	Com->InitForScript( SkyScript );


			// Model.
	ModelScript	= NewObject<FScript>(L"Model");
	Com				= NewObject<FModelComponent>( L"ModelComponent", ModelScript ); 
	Com->InitForScript( ModelScript );


	// Brush.
	BrushScript	= NewObject<FScript>(L"Brush");
	Com				= NewObject<FBrushComponent>( L"BrushComponent", BrushScript ); 
	Com->InitForScript( BrushScript );
	FBrushComponent* Car = (FBrushComponent*)Com;

	
		Car->Type	= BRUSH_NotSolid;
		Car->NumVerts = 3;
		Car->Vertices[2] = TVector( +4.f, 4.f );
		Car->Vertices[1] = TVector( -4.f, +4.f );
		Car->Vertices[0] = TVector( -4.f, -4.f );
		


	FScript* Emitter	= NewObject<FScript>(L"Emitter");
	Com				= NewObject<FRectComponent>( L"", Emitter ); 
	Com->InitForScript( Emitter );
	Com				= NewObject<FSpriteComponent>( L"", Emitter ); 
	Com->InitForScript( Emitter );
	Com				= NewObject<FWeatherEmitterComponent>( L"", Emitter ); 
	Com->InitForScript( Emitter );



// gLIDER.
	FScript* Glider	= NewObject<FScript>(L"Glider");
	Com				= NewObject<FRectComponent>( L"", Glider ); 
	Com->InitForScript( Glider );
	Com				= NewObject<FSpriteComponent>( L"", Glider ); 
	Com->InitForScript( Glider );
	Com				= NewObject<FKeyframeComponent>( L"", Glider ); 
	Com->InitForScript( Glider );




	// Box.
	BoxScript	= NewObject<FScript>(L"Box");
	Com				= NewObject<FRectComponent>( L"RectComponent", BoxScript ); 
	BoxScript->FileName	= L"Box.flu";

	Com->InitForScript( BoxScript );



	Com		= NewObject<FDecoComponent>(L"DecoComponent", BoxScript);
	Com->InitForScript(BoxScript);
	//((FSpriteComponent*)Com)->Offset = TVector( 0.f, 0.f ); 






	{
		CTextReader Reader(GDirectory + L"\\Box.fsr");

		while( !Reader.IsEOF() )
		{
			String Line = Reader.ReadLine();

			// remove tabulation.
			for( Integer i=0; i<Line.Len(); i++ )
				if( Line[i] == '\t' )
				{
					Line[i] = ' ';
				}


			BoxScript->Text.Push(Line);

		}
	}
	  
	BoxScript->bHasText = true;
	BoxScript->InstanceBuffer	= new CInstanceBuffer(BoxScript);




}

FBrushComponent* Brush; 
*/


#if 0
/*-----------------------------------------------------------------------------
    String.
-----------------------------------------------------------------------------*/

//
// An Unicode string.
//
class String
{
private:
	// Variables.
	Char*		Data;
	Integer		Length;

public:
	// Constructor.
	String()
		: Length( 0 ),
		  Data( L"\0" )
	{}
	String( const String& Other )
	{
		Length	= Other.Length;
		Data	= new Char[Length + 1]();
		for( Integer i=0; i < Length; i++ )
			Data[i] = Other.Data[i];
	}
	String( const Char* InStr )
	{
		Length	= wcslen( InStr );
		Data	= new Char[Length + 1]();
		for( Integer i=0; i < Length; i++ )
			Data[i] = InStr[i];
	}
	String( const Char* InStr, Integer InLen )
	{
		Length	= InLen;
		if( Length )
		{
			Data = new Char[Length + 1]();
			for( Integer i=0; i < Length; i++ )
				Data[i] = InStr[i];
			Data[Length] = '\0';
		}
		else
			Data = L"\0";
	}

	// Destructor.
	~String()
	{
		if( Length )
			delete[] Data;
		Length	= 0;
		Data	= nullptr;
	}

	// Functions.
	DWord HashCode() const
	{
		DWord Hash = 2139062143;
		for( Char* C = Data; *C; C++ )
			Hash = 37 * Hash + *C;
		return Hash;
	}
	inline Integer Len() const
	{
		return Length;
	}
	Bool ToInteger( Integer& OutValue, Integer Default = 0 ) const;
	Bool ToFloat( Float& OutValue, Float Default = 0.f ) const;

	// Operators.
	Char& operator[]( Integer i )
	{
		return Data[i];
	}
	const Char& operator[]( Integer i ) const
	{
		return Data[i];
	}
	Char* operator*() const
	{
		return Data;
	}
	String& operator=( const Char* Str )
	{
		if( Length )
			delete[] Data;
		Length = wcslen(Str);
		if( Length )
		{
			Data = new Char[Length+1]();
			for( Integer i=0; i<=Length; i++ )
				Data[i] = Str[i];	
		}
		else
		{
			Data = L"\0";
		}
		return *this;
	}
	String& operator=( const String& Other )
	{
		if( Length )
			delete[] Data;
		Length = Other.Length;
		if( Length )
		{
			Data = new Char[Length+1]();
			for( Integer i=0; i<Length; i++ )
				Data[i] = Other.Data[i];
		}
		else
		{
			Data = L"\0";
		}
		return *this;
	}
	Bool operator==( const String& Other ) const
	{
		return Data && Other.Data && wcscmp( Data, Other.Data ) == 0;
	}
	Bool operator==( const Char* Str ) const
	{
		return Data && Str && wcscmp( Data, Str ) == 0;
	}
	Bool operator!=( const String& Other ) const
	{
		return Data && Other.Data && wcscmp( Data, Other.Data ) != 0;
	}
	Bool operator!=( const Char* Str ) const
	{
		return Data && Str && wcscmp( Data, Str ) != 0;
	}
	String& operator+=( const Char* Str )
	{
		Integer StrLen = wcslen(Str);
		if( StrLen )
		{
			Length += StrLen;
			Char* NewData = new Char[Length+1]();
			wcscpy( NewData, Data );
			wcscat( NewData, Str );
			if( Length != StrLen )
				delete[] Data;
			Data = NewData;
		}
		return *this;
	}
	String& operator+=( const String& Other )
	{
		return operator+=(*Other);
	}
	String operator+( const Char* Str ) 
	{
		return String(*this) += Str;   
	}
	String operator+( const String& Other )
	{
		return operator+(*Other);
	}
	operator Bool() const
	{
		return Length > 0;
	}

	// Statics.
	static String Format( const String Fmt, ... );
	static String Copy( const String& Source, Integer StartChar, Integer Count );
	static Integer Pos( const String& Needle, const String& HayStack );
	static String UpperCase( const String& Str );
	static String LowerCase( const String& Str );
	static String Delete( const String& Str, Integer StartChar, Integer Count );
	static Integer CompareText( const String& Str1, const String& Str2 );

	// Friends.
	friend void Serialize( CSerializer& S, String& V );
};
#endif

#if 0
//
// String serialization.
//
void Serialize( CSerializer& S, String& V )
{
	if( S.GetMode() == SM_Load )
	{
		// Load string.
		if( V.Length > 0 )
			delete[] V.Data;

		Serialize( S, V.Length );
		
		if( V.Length > 0 )
		{
			V.Data = new Char[V.Length+1];
			V[V.Length] = '\0';
			S.SerializeData( *V, sizeof(Char) * V.Length );
		}
		else
		{
			V.Data = L"\0";
		}
	}
	else
	{
		// Store or count string.
		Integer Len = V.Len();
		Serialize( S, Len );

		if( Len > 0 )
			S.SerializeData( *V, sizeof(Char)*Len );
	}
}


//
// Convert string into integer value, return true if 
// converted successfully, otherwise return false and out value
// will be set default.
//
Bool String::ToInteger( Integer& Value, Integer Default ) const
{
	if (!Length)
		return false;

	Integer		iChar	= 0;
	Bool		bNeg	= false;
	Value				= Default;

	// Detect sign.
	if (Data[0] == L'-')
	{
		iChar++;
		bNeg = true;
	}
	else if (Data[0] == L'+')
	{
		iChar++;
		bNeg = false;
	}

	// Parse digit by digit.
	Integer Result = 0;
	for (Integer i = iChar; i < Length; i++)
	if (Data[i] >= L'0' && Data[i] <= L'9')
	{
		Result *= 10;
		Result += (Integer)(Data[i] - L'0');
	}
	else
		return false;

	Value = bNeg ? -Result : Result;
	return true;
}


//
// Convert string into float value, return true if 
// converted successfully, otherwise return false and out value
// will be set default.
//
Bool String::ToFloat( Float& Value, Float Default ) const
{
	if (!Length)
		return false;

	Integer		iChar	= 0;
	Bool		bNeg	= false;
	Float		Frac = 0.f, Ceil = 0.f;
	Value				= Default;

	// Detect sign.
	if (Data[0] == L'-')
	{
		iChar++;
		bNeg = true;
	}
	else if (Data[0] == L'+')
	{
		iChar++;
		bNeg = false;
	}

	if( iChar < Length )
	{
		if( Data[iChar] == L'.' )
		{
			// Parse fractional part.
		ParseFrac:
			iChar++;
			Float m = 0.1f;
			for( ; iChar < Length; iChar++ )
				if( Data[iChar] >= L'0' && Data[iChar] <= L'9' )
				{
					Frac += (Integer)(Data[iChar] - L'0') * m;
					m /= 10.f;
				}
				else
					return false;
		}
		else if( Data[iChar] >= L'0' && Data[iChar] <= L'9' )
		{
			// Parse ceil part.
			for( ; iChar < Length; iChar++ )
				if( Data[iChar] >= L'0' && Data[iChar] <= L'9' )
				{
					Ceil *= 10.f;
					Ceil += (Integer)(Data[iChar] - L'0');
				}
				else if( Data[iChar] == L'.' )
				{
					goto ParseFrac;
				}
				else
					return false;
		}
		else
			return false;
	}
	else
		return false;

	Value = bNeg ? -(Ceil+Frac) : +(Ceil+Frac);
	return true;
}


//
// Format string.
//
String String::Format( const String Fmt, ... )
{
	static Char Dest[2048] = {};
	va_list ArgPtr;
	va_start( ArgPtr, Fmt );
	_vsnwprintf( Dest, 2048, *Fmt, ArgPtr );
	va_end( ArgPtr );
	return Dest;
}


//
// Copy substring.
//
String String::Copy( const String& Source, Integer StartChar, Integer Count )
{
	StartChar = Clamp( StartChar, 0, Source.Length-1 );
	Count = Clamp( Count, 0, Source.Length-StartChar );
	return String( &Source.Data[StartChar], Count );
}


//
// Search needle in haystack :), of string
// of course. if not found return -1.
//
Integer String::Pos( const String& Needle, const String& HayStack )
{
	const Char* P = wcsstr( HayStack.Data, Needle.Data );
	return P ? ((Integer)P - (Integer)HayStack.Data)/sizeof(Char) : -1;
}


//
// Return string copy with upper case.
//
String String::UpperCase( const String& Str )
{
	String S = Str;
	for( Char* C=S.Data; *C; C++ )
		*C = towupper( *C );
	return S;
}


//
// Return string copy with lower case.
//
String String::LowerCase( const String& Str )
{
	String S = Str;
	for( Char* C=S.Data; *C; C++ )
		*C = towlower( *C );
	return S;
}


//
// Remove Count symbols from StartChar position. 
//
String String::Delete( const String& Str, Integer StartChar, Integer Count )
{
	StartChar	= Clamp( StartChar, 0, Str.Length-1 );
	Count		= Clamp( Count, 0, Str.Length-StartChar );
	return Copy( Str, 0, StartChar ) + Copy( Str, StartChar+Count, Str.Length-StartChar-Count );	
}


//
// String comparison.
// Return:
//   < 0: Str1 < Str2.
//   = 0: Str1 = Str2.
//	 > 0: Str1 > Str2.
//
Integer String::CompareText( const String& Str1, const String& Str2 )
{
	return wcscmp( *Str1, *Str2 );
}
#endif


#if 0
	switch (Event)
	{
		case WPE_KeyDown:
		case WPE_KeyUp:
		case WPE_CharType:
			if( Focused && Focused != this )
				Focused->WidgetProc( Event, Parms );
			WContainer::WidgetProc( Event, Parms );

			// System keys.
			if( Event == WPE_KeyDown || Event == WPE_KeyUp )
			{
				if( Parms.Key == VK_Alt )		bAlt	= Event == WPE_KeyDown;
				if( Parms.Key == VK_Ctrl )		bCtrl	= Event == WPE_KeyDown;
				if( Parms.Key == VK_Shift )		bShift	= Event == WPE_KeyDown;
			}
			break;

		case WPE_MouseScroll:
			if( Highlight )
				Highlight->WidgetProc( WPE_MouseScroll, Parms );
			break;

		case WPE_MouseMove:
		case WPE_DblClick:  
			if( !Drag.bDrag && IsCapture() && Focused && Focused != this )
			{
				TPoint Client = Focused->WindowToClient( TPoint( Parms.X, Parms.Y ) );
				Focused->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
			}
			else
				WContainer::WidgetProc( Event, Parms );

			MousePos	= TPoint( Parms.X, Parms.Y );
			break;

		case WPE_MouseDown:
			if( Parms.Button == MB_Left )	bLMouse = true;
			if( Parms.Button == MB_Right )	bRMouse = true;
			if( Parms.Button == MB_Middle )	bMMouse = true;
			WContainer::WidgetProc( Event, Parms );
			break;

		case WPE_MouseUp:
			if( !Drag.bDrag && Focused && Focused != this )
			{
				TPoint Client = Focused->WindowToClient( TPoint( Parms.X, Parms.Y ) );
				Focused->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
			}
			else
				WContainer::WidgetProc( Event, Parms );

			if( Parms.Button == MB_Left )	bLMouse = false;
			if( Parms.Button == MB_Right )	bRMouse = false;
			if( Parms.Button == MB_Middle )	bMMouse = false;

			break;
	
		case WPE_Paint:
			WContainer::WidgetProc( Event, Parms );
			DrawTooltip( Parms.Render );
			break;

		default:
			WContainer::WidgetProc( Event, Parms );
			break;
	}
#endif



// stored phys.
#if 0

//
// Complex physics - handles rotation, friction, restitution,
// portals, touches compute forces and so on. It's pretty
// expensive, so don't use it too often.
//
void CPhysics::PhysicComplex( FPhysicComponent* Body, Float Delta )
{
	// Don't process unmovable body.
	if( Body->Mass <= 0.f )
		return;

	// Setup pointers.
	Level	= Body->Level;

	// Unhash body to perform movement.
	// And return to it back.
	Level->CollHash->RemoveFromHash( Body );
	{
		// Prepare.
		FZoneComponent*	DetectedZone	= nullptr;
		TVector			OldLocation		= Body->Location;
		BodyInvMass		= GetInvMass(Body);
		BodyInvIner		= GetInvInertia(Body);


		if( Abs(Body->AngVelocity) < 0.1f )	Body->AngVelocity	= 0.f;
		if( Body->Velocity.Size() < 0.1f ) Body->Velocity	= TVector( 0.f, 0.f );

		// Integrate translate forces.
		Body->Velocity	+=	Body->Forces * (BodyInvMass * Delta);/////////////////////////////////////////////////////////////
		Body->Forces	=	TVector( 0.f, 0.f );//////////////////////////////////////////////////////////////////////////////

		//
		// Here we clamp delta, to avoid very long distances.
		// It's reduce situations when body fall, or pass
		// through obstacles. I think max delta it's 95%
		// of body's size.
		//
		TVector VelDelta	= Body->Velocity * Delta;
		VelDelta.X	= Clamp( VelDelta.X, -Body->Size.X*0.95f, +Body->Size.X*0.95f );
		VelDelta.Y	= Clamp( VelDelta.Y, -Body->Size.Y*0.95f, +Body->Size.Y*0.95f );

		Body->Location	+= VelDelta;

		// Integrate rotation forces.
		Body->AngVelocity	+=	Body->Torque * (BodyInvIner * Delta);
		Body->Rotation		+=	TAngle( Body->AngVelocity * Delta );
		Body->Torque		=	0.f;

		// Get list of potential collide bodies, using cheap
		// AABB test.
		TRect OtherAABB, BodyAABB = Body->GetAABB();
		Level->CollHash->GetOverlapped( BodyAABB, NumOthers, Others );

		// Convert Body to polygon.
		TVector	PolyOrig	= Body->Location;
		BodyToPoly( Body, AVerts, ANorms, ANum );

		// Test collision with all bodies.
		for( Integer i=0; i<NumOthers; i++ )
		{
			// Figure out is body still overlaps with
			// Others[i] due position correction.
			Other		=	Others[i];
			BodyAABB	=	Body->GetAABB();
			OtherAABB	=	Other->GetAABB();

			if( !BodyAABB.IsOverlap(OtherAABB) )
				continue;

			// See if body already touch other.
			if( IsTouch( Body, Other ) )
				continue;

			// Polys for collision.
			BodyToPoly( Other, BVerts, BNorms, BNum );
			if( Body->Location != PolyOrig )
				BodyToPoly( Body, AVerts, ANorms, ANum );

			// Compute collsion.
			DetectComplexCollision();
			HitSide	= NormalToSide(HitNormal);

			// No collision?
			if( NumConts == 0 )
				continue;

			// Handle zones.
			if( Other->IsA(FZoneComponent::MetaClass) )
			{
				DetectedZone	= (FZoneComponent*)Other;
				continue;
			}

			// Compute other's physics properties.
			FPhysicComponent* PhysOther = nullptr;
			if( Other->IsA(FPhysicComponent::MetaClass) )
			{
				PhysOther		= (FPhysicComponent*)Other;
				OtherInvMass	= GetInvMass(PhysOther);
				OtherInvIner	= GetInvInertia(PhysOther);
			}
			else
			{
				OtherInvMass	= 0.f;
				OtherInvIner	= 0.f;
			}

			// Ask script how to handle hit.
			Solution	= HSOL_None;
			bBrake		= false;
			Body->Entity->CallEvent( L"OnCollide", Other->Entity, (Byte)HitSide );
			Other->Entity->CallEvent( L"OnCollide", Body->Entity, (Byte)OppositeSide(HitSide) );

			if
				(
					(Solution == HSOL_Solid) ||
					(Solution == HSOL_Oneway && HitSide == HSIDE_Top && Body->Velocity.Y < 0.f)
				)
			{
				// Handle solid collision.

				// Process REAL physics interaction.
				Float SFriction	= !PhysOther ? GMaterials[Body->Material].SFriction : MixFriction
				(
					GMaterials[Body->Material].SFriction,
					GMaterials[PhysOther->Material].SFriction
				);
				Float DFriction	= !PhysOther ? GMaterials[Body->Material].DFriction : MixFriction
				(
					GMaterials[Body->Material].DFriction,
					GMaterials[PhysOther->Material].DFriction
				);

				for( Integer k=0; k<NumConts; k++ )
				{
					// Radii vectors.
					TVector RadBody		= Contacts[k] - Body->Location;
					TVector	RadOther	= Contacts[k] - Other->Location;

					// Relative velocity.
					TVector RelVel;
					if( PhysOther )
						RelVel	=	PhysOther->Velocity + (RadOther / PhysOther->AngVelocity) -
									Body->Velocity		- (RadBody / Body->AngVelocity);
					else
						RelVel	= -Body->Velocity - (RadBody / Body->AngVelocity);

					// Project velocity along hit normal.
					Float ProjVel	= RelVel * HitNormal;

					// Collide only when velocity along hit normal
					// and satisfied script's solution about this hit.
					if 
						(
							(ProjVel <= 0.f) &&
							(	( Solution == HSOL_Solid )||
								( Solution == HSOL_Oneway && HitSide == HSIDE_Top ) )
						)
					{
						Float	RACrossN	= RadBody / HitNormal;
						Float	RBCrossN	= RadOther / HitNormal;

						Float	InvMassTotal	=	BodyInvMass + OtherInvMass +
													Sqr(RACrossN) * BodyInvIner +
													Sqr(RBCrossN) * OtherInvIner;

						// Pick elasticity for hit solving.
						Float	e	=	!PhysOther ? GMaterials[Body->Material].Elasticity :
										Min( GMaterials[Body->Material].Elasticity, GMaterials[PhysOther->Material].Elasticity );

						// Scalar impulse.
						Float	j	= -(1 + e) * ProjVel / (InvMassTotal*NumConts);
						TVector	TotalImpulse	= HitNormal * j;

						// Apply impulse friction.
						TVector	Tangent	= RelVel - (HitNormal * (RelVel*HitNormal));
						Tangent.Normalize();
						Float jt	= -(RelVel * Tangent) / (InvMassTotal*NumConts);

						// Don't apply too small friction.
						if( ( jt <= -0.00001f )&&( jt <= +0.00001f ) )//-------------------------why? maybe its cause failure.
						{
							if( Abs(jt) < j*SFriction )
							{
								// Static friction.
								TotalImpulse	+=	Tangent * jt;

							}
							else
							{
								// Dynamic friction.
								TotalImpulse	+=	Tangent * (-j*DFriction);
							}
						}

						// Apply sum of impulses to bodies movement.
						Body->Velocity -= TotalImpulse * BodyInvMass;
						if( PhysOther )
							PhysOther->Velocity	+= TotalImpulse * OtherInvMass;

						// Apply sum of impulses to bodies rotation.
						Body->AngVelocity -= (RadBody/TotalImpulse)*BodyInvIner;
						if( PhysOther )
							PhysOther->AngVelocity += (RadOther/TotalImpulse)*OtherInvIner;
					}
				}

				// Apply correction to bodies location.
				TVector Correct = HitNormal * Max( 0.f, HitTime-0.05f ) *
										( 0.5f / (BodyInvMass+OtherInvMass) );
				Body->Location -= Correct * BodyInvMass;

				if( PhysOther )
				{
					Level->CollHash->RemoveFromHash(PhysOther);
					PhysOther->Location += Correct * OtherInvMass;
					Level->CollHash->AddToHash(PhysOther);
				}

				// Handle floor.
				if( HitSide == HSIDE_Bottom )
				{
					// Body get floor slab.
					FMoverComponent* Mover = As<FMoverComponent>(Other);
					if( Mover )
						Mover->AddRider( Body );
					Body->Floor		= Other->Entity;
				}
				else if( HitSide == HSIDE_Top )
				{
					// Other get floor slab.
					if( Other->IsA(FPhysicComponent::MetaClass) )
						((FPhysicComponent*)Other)->Floor	= Body->Entity;
				}
			}
			else
			{
				// Bodies don't want to collide, so
				// touch 'em.
				if( Solution != HSOL_Oneway )
				{
					BeginTouch( Body, Other );
					BodyAABB = Body->GetAABB();
				}
			}
		}

		//
		// See if no more touch touched actors.
		//
		{
			BodyToPoly( Body, AVerts, ANorms, ANum );
			for( Integer i=0; i<array_length(Body->Touched); i++ )
				if( Body->Touched[i] )
				{
					Other	= Body->Touched[i]->Base;
					BodyToPoly( Other, BVerts, BNorms, BNum );

					if( !PolysIsOverlap( AVerts, ANum, BVerts, BNum ) )
						EndTouch( Body, Other );
				}
		}

		// Process zone.
		SetBodyZone( Body, DetectedZone );

		// Process portal pass.
		HandlePortals( Body, OldLocation );	
	}
	Level->CollHash->AddToHash( Body );		
}

#endif



// Here's list of natural WinApi constant's without my stuff.
#if 0
enum
{
	KEY_None,			KEY_LButton,			KEY_RButton,			KEY_AV0x03,
	KEY_MButton,		KEY_AV0x05,				KEY_AV0x06,				KEY_AV0x07,
	KEY_Backspace,		KEY_Tab,				KEY_AV0x0a,				KEY_AV0x0b,
	KEY_AV0x0c,			KEY_Return,				KEY_AV0x0e,				KEY_AV0x0f,
	KEY_Shift,			KEY_Ctrl,				KEY_Alt,				KEY_Pause,
	KEY_CapsLock,		KEY_AV0x15,				KEY_AV0x16,				KEY_AV0x17,
	KEY_AV0x18,			KEY_AV0x19,				KEY_AV0x1a,				KEY_Escape,
	KEY_AV0x1c,			KEY_AV0x1d,				KEY_AV0x1e,				KEY_AV0x1f,
	KEY_Space,			KEY_PageUp,				KEY_PageDown,			KEY_End,
	KEY_Home,			KEY_Left,				KEY_Up,					KEY_Right,
	KEY_Down,			KEY_Select,				KEY_Print,				KEY_Execute,
	KEY_PrintScrn,		KEY_Insert,				KEY_Delete,				KEY_Help,
	KEY_0,				KEY_1,					KEY_2,					KEY_3,
	KEY_4,				KEY_5,					KEY_6,					KEY_7,
	KEY_8,				KEY_9,					KEY_AV0x3a,				KEY_AV0x3b,
	KEY_AV0x3c,			KEY_AV0x3d,				KEY_AV0x3e,				KEY_AV0x3f,
	KEY_AV0x40,			KEY_A,					KEY_B,					KEY_C,
	KEY_D,				KEY_E,					KEY_F,					KEY_G,
	KEY_H,				KEY_I,					KEY_J,					KEY_K,
	KEY_L,				KEY_M,					KEY_N,					KEY_O,
	KEY_P,				KEY_Q,					KEY_R,					KEY_S,
	KEY_T,				KEY_U,					KEY_V,					KEY_W,
	KEY_X,				KEY_Y,					KEY_Z,					KEY_AV0x5b,
	KEY_AV0x5c,			KEY_AV0x5d,				KEY_AV0x5e,				KEY_AV0x5f,
	KEY_NumPad0,		KEY_NumPad1,			KEY_NumPad2,			KEY_NumPad3,
	KEY_NumPad4,		KEY_NumPad5,			KEY_NumPad6,			KEY_NumPad7,
	KEY_NumPad8,		KEY_NumPad9,			KEY_Multiply,			KEY_Add,
	KEY_Separator,		KEY_Subtract,			KEY_Decimal,			KEY_Divide,
	KEY_F1,				KEY_F2,					KEY_F3,					KEY_F4,
	KEY_F5,				KEY_F6,					KEY_F7,					KEY_F8,
	KEY_F9,				KEY_F10,				KEY_F11,				KEY_F12,
	KEY_F13,			KEY_F14,				KEY_F15,				KEY_F16,
	KEY_F17,			KEY_F18,				KEY_F19,				KEY_F20,
	KEY_F21,			KEY_F22,				KEY_F23,				KEY_F24,
	KEY_AV0x88,			KEY_AV0x89,				KEY_AV0x8a,				KEY_AV0x8b,
	KEY_AV0x8c,			KEY_AV0x8d,				KEY_AV0x8e,				KEY_AV0x8f,
	KEY_NumLock,		KEY_ScrollLock,			KEY_AV0x92,				KEY_AV0x93,
	KEY_AV0x94,			KEY_AV0x95,				KEY_AV0x96,				KEY_AV0x97,
	KEY_AV0x98,			KEY_AV0x99,				KEY_AV0x9a,				KEY_AV0x9b,
	KEY_AV0x9c,			KEY_AV0x9d,				KEY_AV0x9e,				KEY_AV0x9,
	KEY_LShift,			KEY_RShift,				KEY_LControl,			KEY_RControl,
	KEY_AV0xa4,			KEY_AV0xa5,				KEY_AV0xa6,				KEY_AV0xa7,
	KEY_AV0xa8,			KEY_AV0xa9,				KEY_AV0xaa,				KEY_AV0xab,
	KEY_AV0xac,			KEY_AV0xad,				KEY_AC0xae,				KEY_AV0xaf,
	KEY_AV0xb0,			KEY_AV0xb1,				KEY_AV0xb2,				KEY_AV0xb3,
	KEY_AV0xb4,			KEY_AV0xb5,				KEY_AV0xb6,				KEY_AV0xb7,
	KEY_AV0xb8,			KEY_AV0xb9,				KEY_Semicolon,			KEY_Equals,
	KEY_Comma,			KEY_Minus,				KEY_Period,				KEY_Slash,
	KEY_Tilde,			KEY_AV0xc1,				KEY_AV0xc2,				KEY_AV0xc3,
	KEY_AV0xc4,			KEY_AV0xc5,				KEY_AV0xc6,				KEY_AV0xc7,
	KEY_AV0xc8,			KEY_AV0xc9,				KEY_AV0xca,				KEY_AV0xcb,
	KEY_AV0xcc,			KEY_AV0xcd,				KEY_AV0xce,				KEY_AV0xcf,
	KEY_AV0xd0,			KEY_AV0xd1,				KEY_AV0xd2,				KEY_AV0xd3,
	KEY_AV0xd4,			KEY_AV0xd5,				KEY_AV0xd6,				KEY_AV0xd7,
	KEY_AV0xd8,			KEY_AV0xd9,				KEY_AV0xda,				KEY_LeftBracket,
	KEY_Backslash,		KEY_RightBracket,		KEY_SingleQuote,		KEY_AV0xdf,
	KEY_AV0xe0,			KEY_AV0xe1,				KEY_AV0xe2,				KEY_AV0xe3,
	KEY_AV0xe4,			KEY_AV0xe5,				KEY_AV0xe6,				KEY_AV0xe7,
	KEY_AV0xe8,			KEY_AV0xe9,				KEY_AV0xea,				KEY_AV0xeb,
	KEY_AV0xec,			KEY_AV0xed,				KEY_AV0xee,				KEY_AV0xef,
	KEY_AV0xf0,			KEY_AV0xf1,				KEY_AV0xf2,				KEY_AV0xf3,
	KEY_AV0xf4,			KEY_AV0xf5,				KEY_Attn,				KEY_CrSel,
	KEY_ExSel,			KEY_ErEof,				KEY_Play,				KEY_Zoom,
	KEY_NoName,			KEY_PA1,				KEY_OEMClear,			KEY_MAX
};
#endif



#if 0

/// Is old in commentary!!!
	{
		//
		// Modal dialog processing.
		//

		// Make sure modal dialog are ALWAYS last in the children list.
		// Modal should be at top of the top.
		if( Modal != Children.Last() )
		{
			Integer iModal = Children.FindItem(Modal);
			assert(iModal != -1);
			while( iModal < Children.Num()-1 )
			{
				Children.Swap( iModal, iModal+1 );
				iModal++;
			}
		}

		switch (Event)
		{
			case WPE_KeyDown:
			case WPE_KeyUp:
			case WPE_CharType:
				if( Focused && Focused != this && Focused->IsChildOf(Modal) )
					Focused->WidgetProc( Event, Parms );

				Modal->WidgetProc( Event, Parms );

				// System keys.
				if( Event == WPE_KeyDown || Event == WPE_KeyUp )
				{
					if( Parms.Key == VK_Alt )		bAlt	= Event == WPE_KeyDown;
					if( Parms.Key == VK_Ctrl )		bCtrl	= Event == WPE_KeyDown;
					if( Parms.Key == VK_Shift )		bShift	= Event == WPE_KeyDown;
				}
				break;

			case WPE_MouseScroll:
				if( Highlight && Highlight->IsChildOf(Modal) )
					Highlight->WidgetProc( WPE_MouseScroll, Parms );
				break;

			case WPE_MouseMove:
			case WPE_DblClick:
				if( !Drag.bDrag && IsCapture() && Focused && Focused!=this && Focused->IsChildOf(Modal) )
				{
					TPoint Client = Focused->WindowToClient( TPoint( Parms.X, Parms.Y ) );
					Focused->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
				}
				else
				{
					WWidget* Below = GetWidgetAt(TPoint(Parms.X, Parms.Y));
					if( Below && Below->IsChildOf(Modal) )
					{
						TPoint Client = Below->WindowToClient( TPoint( Parms.X, Parms.Y ) );
						Below->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
					}
				}
				/*
				if( GetWidgetAt(TPoint(Parms.X, Parms.Y)) == Modal || Event == WPE_MouseMove )
				{
					TPoint Client = Modal->WindowToClient( TPoint( Parms.X, Parms.Y ) );
					Modal->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
				}*/
				MousePos	= TPoint( Parms.X, Parms.Y );
				break;

			case WPE_MouseDown:
				if( Parms.Button == MB_Left )	bLMouse = true;
				if( Parms.Button == MB_Right )	bRMouse = true;		
				if( Parms.Button == MB_Middle )	bMMouse = true;
				{
					WWidget* Below = GetWidgetAt(TPoint(Parms.X, Parms.Y));
					if( Below && Below->IsChildOf(Modal) )
					{
						TPoint Client = Below->WindowToClient( TPoint( Parms.X, Parms.Y ) );
						Below->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
					}
				}
					/*
				if( GetWidgetAt(TPoint(Parms.X, Parms.Y)) == Modal )
				{
					TPoint Client = Modal->WindowToClient( TPoint( Parms.X, Parms.Y ) );
					Modal->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
				}*/
				break;

			case WPE_MouseUp:
				if( !Drag.bDrag && Focused && Focused != this && Focused->IsChildOf(Modal) )
				{
					TPoint Client = Focused->WindowToClient( TPoint( Parms.X, Parms.Y ) );
					Focused->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
				}
				/*
				if( GetWidgetAt(TPoint(Parms.X, Parms.Y)) == Modal )
				{
					TPoint Client = Modal->WindowToClient( TPoint( Parms.X, Parms.Y ) );
					Modal->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
				}*/

				if( Parms.Button == MB_Left )	bLMouse = false;
				if( Parms.Button == MB_Right )	bRMouse = false;
				if( Parms.Button == MB_Middle )	bMMouse = false;
				break;
	
			case WPE_Paint:
				for( Integer i=0; i<Children.Num(); i++ )
					if( Children[i]->bVisible )
					{
						Parms.Render->SetBrightness( Children[i] == Modal ? 1.f : 0.5f );
						Children[i]->WidgetProc( Event, Parms );
					}

				DrawTooltip( Parms.Render );
				break;

			default:
				WContainer::WidgetProc( Event, Parms );
				break;
		}
	}
#endif


// Pure for single use.
void MakeConsoleFile()
{
#if 0
	CFileSaver  Saver(GDirectory+L"\\Console.fcr");	// flu console resource.

	// Bitmap data.
	{
		FBitmap* Bitmap = WWindow::Font1->Bitmaps[0];
		Byte* Data = (Byte*)Bitmap->GetData();
		CRLECompressor rle;
		void* OutData;
		DWord OutSize;
		rle.Encode( Data, 65536, OutData, OutSize ); 

		Serialize( Saver, OutSize );
		Saver.SerializeData( OutData, OutSize );
		Serialize( Saver, Bitmap->Palette );
	}

	// Font tables.
	{
		Integer NumGly = WWindow::Font1->Glyphs.Num();
		Serialize( Saver, WWindow::Font1->Height );
		Serialize( Saver, WWindow::Font1->Remap );
		Serialize( Saver, NumGly );
		Saver.SerializeData( &WWindow::Font1->Glyphs[0], sizeof(TGlyph)*WWindow::Font1->Glyphs.Num() );

	}
#endif
}



/*
FLevel* MakeLevel()
{
	//FLevel* Level = new FLevel();
	FLevel* Level = NewObject<FLevel>( L"MyLevel" );
	{
		Level->CreateEntity( (FScript*)GObjectDatabase->FindObject( L"Camera", FScript::MetaClass, nullptr ), L"Camera", TVector(0.f, 0.f) );




		//Level->RndFlags |= RND_Other | RND_Logic | RND_Grid;
		Level->RndFlags	= RND_Editor;
	}
	return Level;
}
*/

/*

	// TMap crash test!!!
	TMap<Integer, String> Map;
	Map.Put( 3, L"France" );
	Map.Put( 4, L"Italy" );
	Map.Put( 1, L"Germany" );
	Map.Put( 2, L"USA" );
	Map.Put( 0, L"russia" );


	log( L"Country is '%s'", **Map.Get(2) );	/// USA, USA, USA!!!
*/



// Hi, from FrGraph.cpp!
#if 0

//
//                      "Pins method"
//	Heres the full algorithm of automation path building:
//
//	At begging place a pins to the walkable surfaces on each brush.
//	There is two types of pins lie leftward and rightward, relative to
//	the brush's surface center. Next step is trace line downward from
//	the each placed pin to make 'fall' spot pin. After it add pins
//	relative with markers, since they used in graph too.

//@@@@@@@@@@@@@@@@ add normal description!!!!!!!!!!

//  When this steps are passed, should merge the near pins into one.
//  This operation shrink pins count pretty well. Now we have
//  a list of pins, lets start link 'em. Firstly link all walkable
//  paths, first - check for reachable and with constant step check
//  for valid floor. Next need to compute jump/fall paths. It's
//  just a line tracing with little modification. Graph now computed
//  so it's time to convert it into list of TPathNode's and TPathEdges.
//  And finally compute 'mark area' for each node and pass height
//  for each graph edge. This algorithm should be extended for
//  teleports, portals, jumpers, ladders and etc. Also new idea:
//  sort edges list in each node by cost, most cheaper should be
//  firstly, its optimize real-time graph travers operation.
//


//
// Path building constants. It's should be
// adjusted more precisely.
//
#define PIN_BASE			0.55f		// Height of pin above surface.
#define PIN_DROP_BIAS		4.f			// Bias from pin to drop pin downward.
#define MAX_FALL_DIST		64.f		// How long maximum fall path.
#define PIN_SAME_THRESH		2.f			// Threshold to merge pins.
#define WALK_STEP			2.f			// Constant step size to determine floor.
#define WALK_HEIGHT			1.2f		// Walk height.
#define MAX_JUMP_X			16.f		// Maximum length of X jump.
#define JUMP_WEIGHT			1.7f		// How jump affect path cost of edge.


/*-----------------------------------------------------------------------------
	TPathPin.
-----------------------------------------------------------------------------*/

//
// A temporal pin in the path.
//
struct TPathPin
{
public:
	// Variables.
	DWord				bLeft	: 1;		
	DWord				bRight	: 1;
	DWord				bMiddle	: 1;
	TVector				Location;			// Pin location.
	TVector				Floor;				// Normal of floor below pin.
	FBrushComponent*	Slab;				// Pin's slab.
	FBaseComponent*		MyMarker;			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! warp????	base_component.
	Integer				NumEdges;
	TPathEdge			Edges[8];
	Integer				Remap[4];

	// TPathPin initialization.
	TPathPin()
		:	Location( 0.f, 0.f ),
			Floor( 0.f, 1.f ),
			bLeft( false ),
			bRight( false ),
			bMiddle( false ),
			Slab( nullptr ),
			MyMarker( nullptr ),
			NumEdges( 0 )
	{
		for( Integer i=0; i<array_length(Remap); i++ )
			Remap[i] = -1;
	}

	// Convert pin to the complete path node.
	TPathNode ToPathNode() const
	{
		TPathNode Result;
		Result.Location	= Location;
		Result.Marker	= MyMarker;

		for( Integer i=0; i<array_length(Remap); i++ )
			Result.iEdges[i]	= Remap[i];

		return Result;
	}

	// Add a new edge to the list.
	Integer AddEdge( const TPathEdge& InEdge )
	{
		if( NumEdges < array_length(Edges) )
		{
			Edges[NumEdges]	= InEdge;
			NumEdges++;
		}
		else
			return -1;

		// Sort by cost, bubble sort are good here.
		for( Integer i=0; i<NumEdges-1; i++ )
		for( Integer j=0; j<NumEdges-i-1; j++ )
			if( Edges[j].Cost > Edges[j+1].Cost )
				Exchange( Edges[j], Edges[j+1] );

		return NumEdges-1;
	}
};


/*-----------------------------------------------------------------------------
	CPathBuilder.
-----------------------------------------------------------------------------*/

//
// A path builder.
//
class CPathBuilder
{
public:
	// Variables.
	FLevel*				Level;
	CNavigator*			Navigator;
	TArray<TPathPin>	Pins;

	// CPathBuilder interface.
	CPathBuilder( FLevel* InLevel );
	~CPathBuilder();
	void BuildNetwork();

private:
	// Support functions.
	FBrushComponent* TestPoint( const TVector& P );
	FBrushComponent* TestLine( Bool bFast, const TVector& A, const TVector& B, TVector& V, TVector& Normal );

	// Pins functions.
	void PlaceBrushPins();
	void PlaceDropPins();
	void PlaceMarkerPins();
	void MergePins();

	// Edges functions.
	void PaveWalk();
	void PaveJumpFall();
	void PaveSpecial();
	Bool IsLinked( Integer iPin1, Integer iPin2 );





};





/*

        procedure ComputeHeight( Edge: PPathEdge );
    end;

*/


//
// Setup builder for level.
//
CPathBuilder::CPathBuilder( FLevel* InLevel )
	:	Level( InLevel )
{
	// Destroy old if any.
	freeandnil(InLevel->Navigator);

	// Create new one.
	Navigator			= new CNavigator( Level );
	Level->Navigator	= Navigator;
}


//
// Build network!
//
void CPathBuilder::BuildNetwork()
{
	// Cleanup previous paths.
	Pins.Empty();

	// Initialize statistics.
	Integer	PinsCountNoMerge	= 0,
			PinsCount			= 0,
			EdgesCount			= 0,
			EdgesPostCount		= 0;

	// Place pins and merge them.
	PlaceBrushPins();
	PlaceDropPins();
	PlaceMarkerPins();
	PinsCountNoMerge	= Pins.Num();
	MergePins();
	PinsCount			= Pins.Num();

	// Link pins into path.
	PaveWalk();
	PaveJumpFall();




	//....

/*

    LinkJumpFall();
    LinkSpecial();

*/



	// Add edges to the navigator.
	for( Integer i=0; i<Pins.Num(); i++ )
	{
		TPathPin& Pin	= Pins[i];
		EdgesCount		+= Pin.NumEdges;
		Pin.NumEdges	= Min( Pin.NumEdges, 4 );

		for( Integer j=0; j<Pin.NumEdges; j++ )
			Pin.Remap[j] = Navigator->Edges.Push( Pin.Edges[j] );
	}
	EdgesPostCount	= Navigator->Edges.Num();

	// Add nodes to the navigator.
	for( Integer i=0; i<Pins.Num(); i++ )
	{
		TPathPin& Pin	= Pins[i];
		Navigator->Nodes.Push( Pin.ToPathNode() );
	}




	/*


    // Compute hulls.
    for i := 0 to Navigator.Edges.Count-1 do
        ComputeHeight(Navigator.GetPathEdge(i));




*/

	// Output info.
	log( L"**Path Builder**" );
	log( L"Path: Created %i pins, merged to %i", PinsCountNoMerge, PinsCount );
	log( L"Path: Created %i edges, reduced to %i", EdgesCount, EdgesPostCount );
}


//
// Path builder destructor.
//
CPathBuilder::~CPathBuilder()
{
	Level		= nullptr;
	Navigator	= nullptr;
	Pins.Empty();
}


//
// Place pins over walkable brush's surfaces.
//
void CPathBuilder::PlaceBrushPins()
{
	for( Integer iEntity=0; iEntity<Level->Entities.Num(); iEntity++ )
		if( Level->Entities[iEntity]->Base->IsA(FBrushComponent::MetaClass) )
		{
			FBrushComponent* Brush = (FBrushComponent*)Level->Entities[iEntity]->Base;
			
			if( Brush->Type == BRUSH_NotSolid )
				continue;

			// Winding the brush.
			TVector P1, P2;
			P1 = Brush->Vertices[Brush->NumVerts-1];
			for( Integer iVert=0; iVert<Brush->NumVerts; iVert++ )
			{
				// Get edge params.
				P2	= Brush->Vertices[iVert];
				TVector Tangent = P2 - P1;
				Tangent.Normalize();
				TVector Normal = Tangent.Cross();

				// Place only at walkable surface.
				if( IsWalkable(Normal) )
				{
					// Left pin.
					TVector SpotL = P1 + (Normal+Tangent) * PIN_BASE + Brush->Location;
					if( !TestPoint(SpotL) )
					{
						TPathPin Pin;
						Pin.Location	= SpotL;
						Pin.bLeft		= true;
						Pin.Floor		= Normal;
						Pin.Slab		= Brush;
						Pins.Push( Pin );
					}

					// Right pin.
					TVector SpotR = P2 + (Normal-Tangent) * PIN_BASE + Brush->Location;
					if( !TestPoint(SpotR) )
					{
						TPathPin Pin;
						Pin.Location	= SpotR;
						Pin.bRight		= true;
						Pin.Floor		= Normal;
						Pin.Slab		= Brush;
						Pins.Push( Pin );
					}
				}

				P1 = P2;
			}
		}
}


//
// Drop line from all pins down and test
// intersection.
//
void CPathBuilder::PlaceDropPins()
{
	Integer NumSource = Pins.Num();
	for( Integer i=0; i<NumSource; i++ )
	{
		TPathPin& Source	= Pins[i];
		TVector Tangent		= -Source.Floor.Cross();

		if( Source.bLeft )
		{
			// Drop from left pin.
			TVector A = Source.Location - Tangent * PIN_DROP_BIAS;
			TVector B = A - TVector( 0.f, MAX_FALL_DIST );

			// Trace line.
			TVector HitSpot, HitNormal;
			FBrushComponent* HitBrush = TestLine( false, A, B, HitSpot, HitNormal );

			if( HitBrush && HitBrush!=Source.Slab && IsWalkable(HitNormal) )
			{
				TPathPin Pin;
				Pin.Location	= HitSpot + HitNormal * PIN_BASE;
				Pin.bMiddle		= true;
				Pin.Floor		= HitNormal;
				Pin.Slab		= HitBrush;
				Pins.Push( Pin );
			}
		}
		else if( Source.bRight )
		{
			// Drop from right pin.
			TVector A = Source.Location + Tangent * PIN_DROP_BIAS;
			TVector B = A - TVector( 0.f, MAX_FALL_DIST );

			// Trace line.
			TVector HitSpot, HitNormal;
			FBrushComponent* HitBrush = TestLine( false, A, B, HitSpot, HitNormal );

			if( HitBrush && HitBrush!=Source.Slab && IsWalkable(HitNormal) )
			{
				TPathPin Pin;
				Pin.Location	= HitSpot + HitNormal * PIN_BASE;
				Pin.bMiddle		= true;
				Pin.Floor		= HitNormal;
				Pin.Slab		= HitBrush;
				Pins.Push( Pin );
			}
		}
		else
			assert( false );
	}
}


//
// Place all pins relative to markers.
//
void CPathBuilder::PlaceMarkerPins()
{
	// no implemented :(
	return;
}


//
// Merge all close pins.
//
void CPathBuilder::MergePins()
{
	Bool bWasMerge;
	do 
	{
		bWasMerge	= false;

		for( Integer i=0; i<Pins.Num(); i++ )
			for( Integer j=i+1; j<Pins.Num(); j++ )
			{
				TPathPin& Pin1 = Pins[i];
				TPathPin& Pin2 = Pins[j];

				if( (Pin1.Location-Pin2.Location).SizeSquared() <= PIN_SAME_THRESH*PIN_SAME_THRESH )
				{
					// Merge them!
					if( Pin1.MyMarker && Pin2.MyMarker )
					{
						// Two close markers, don't merge them!
						log
							( 
								L"Path: Markers '%s' and '%s' are too close", 
								*Pin1.MyMarker->GetFullName(), 
								*Pin2.MyMarker->GetFullName() 
							);
					}
					else
					{
						// Merge Pin2 into Pin1.
						Pin1.Location	= (Pin1.Location + Pin2.Location) * 0.5f;
						Pin1.Floor		= (Pin1.Floor + Pin2.Floor) * 0.5f;
						Pin1.bLeft		= Pin1.bLeft | Pin2.bLeft;
						Pin1.bRight		= Pin1.bRight | Pin2.bRight;
						Pin1.bMiddle	= Pin1.bMiddle | Pin2.bMiddle;
						Pin1.Slab		= Pin1.Slab ? Pin1.Slab : Pin2.Slab;

						// Remove Pin2.
						Pins.Remove( j );
						bWasMerge	= true;
					}
				}
			}

	} while( bWasMerge );
}


//
// Pave the walkable way.
//
void CPathBuilder::PaveWalk()
{
	FBrushComponent *Obstacle, *Ledge, *Floor;
	TVector HitSpot, HitNormal;

	for( Integer i=0; i<Pins.Num(); i++ )
	for( Integer j=i+1; j<Pins.Num(); j++ )
	{
		// Test each pair.
		TPathPin& Pin1 = Pins[i];
		TPathPin& Pin2 = Pins[j];

		// Trace a line along dir.
		Obstacle	= TestLine
							(
								true,
								Pin1.Location,
								Pin2.Location,
								HitSpot,
								HitNormal
							);

		// Path obstructed, no walking.
		if( Obstacle )
			continue;

		// Walk along the dir and check for floor below edge.
		TVector PathDelta = Pin2.Location - Pin1.Location;
		Integer NumSteps = Max( 1, ::Floor(PathDelta.Size() / WALK_STEP) );
		PathDelta *= 1.f/NumSteps;
		TVector V = Pin1.Location;
		Floor = nullptr;

		for( Integer k=0; k<NumSteps; k++ )
		{
			Floor	= TestLine
							(
								true,
								V,
								V - TVector( 0.f, WALK_HEIGHT ),
								HitSpot,
								HitNormal
							);

			if( Distance( HitSpot, V ) < PIN_BASE )
				Floor	= nullptr;

			if( !Floor )
				break;

			V	+= PathDelta;
		}

		if( !Floor )
		{
			// No floor below path, its abyss.
		}
		else
		{
			// Floor is valid, so it's walkable.
			// But check for ledges.
			if( Abs(Pin1.Location.Y - Pin2.Location.Y) <= 0.51f )
			{
				// Same height, so its walkable.
				TPathEdge Edge;
				Edge.iStart		= i;
				Edge.iFinish	= j;
				Edge.PathType	= (EPathType)Random(PATH_MAX);;
				Edge.Cost		= ::Floor(Abs(Pin1.Location.X - Pin2.Location.X));

				Pin1.AddEdge( Edge );
				Exchange( Edge.iStart, Edge.iFinish );
				Pin2.AddEdge( Edge );
			}
			else
			{
				// Check for ledge.
				// Sort by height.
				Integer iLower, iUpper;
				TPathPin *Lower, *Upper;

				if( Pin1.Location.Y > Pin2.Location.Y )
				{
					Upper	= &Pin1;	iUpper	= i;
					Lower	= &Pin2;	iLower	= j;
				}
				else
				{
					Upper	= &Pin2;	iUpper	= j;
					Lower	= &Pin1;	iLower	= i;
				}

				// Trace line from lower node to target.
				TVector Target( Upper->Location.X, Lower->Location.Y );
				Ledge	= TestLine
								(
									false,
									Lower->Location,
									Target,
									HitSpot,
									HitNormal
								);

				if( !Ledge || IsWalkable(HitNormal) )
				{
					// Everything is fine, just a slope.
					TPathEdge Edge;
					Edge.iStart		= iUpper;
					Edge.iFinish	= iLower;
					Edge.PathType	= (EPathType)Random(PATH_MAX);;
					Edge.Cost		= ::Floor(Abs(Pin1.Location.X - Pin2.Location.X));

					Upper->AddEdge( Edge );
					Exchange( Edge.iStart, Edge.iFinish );
					Lower->AddEdge( Edge );
				}
				else
				{
					// No! Used jump/fall combo.
					TPathEdge Edge;
					Edge.iStart		= iUpper;
					Edge.iFinish	= iLower;
					Edge.PathType	= (EPathType)Random(PATH_MAX);;
					Edge.Cost		= ::Floor(Abs(Pin1.Location.X - Pin2.Location.X));

					Upper->AddEdge( Edge );

					Exchange( Edge.iStart, Edge.iFinish );
					Edge.PathType	= (EPathType)Random(PATH_MAX);;
					Lower->AddEdge( Edge );
				}
			}
		}
	}
}


//
// Pave the paths with fall/jump flag.
//
void CPathBuilder::PaveJumpFall()
{
	TVector HitSpot, HitNormal;

	for( Integer i=0; i<Pins.Num(); i++ )
	for( Integer j=i+1; j<Pins.Num(); j++ )
	{
		// Don't link pins again.
		if( IsLinked( i, j ) )
			continue;

		// Test each pair.
		TPathPin& Pin1 = Pins[i];
		TPathPin& Pin2 = Pins[j];

		// Sort by height.
		Integer iLower, iUpper;
		TPathPin *Lower, *Upper;

		if( Pin1.Location.Y > Pin2.Location.Y )
		{
			Upper	= &Pin1;	iUpper	= i;
			Lower	= &Pin2;	iLower	= j;
		}
		else
		{
			Upper	= &Pin2;	iUpper	= j;
			Lower	= &Pin1;	iLower	= i;
		}

		if( Upper->bLeft )
		{
			// Jump off to left.
			TVector Tangent = -Upper->Floor.Cross();
			TVector	JumpSpot = Upper->Location + (Upper->Floor - Tangent) * PIN_BASE * 1.f;
			TVector	Dir	= Lower->Location - Upper->Location;

			// Make sure jump to left and not too far.
			if	(	Dir.X < 0.f &&
					Abs(Dir.X) <= MAX_JUMP_X &&
					!TestPoint(JumpSpot) 
				)
			{
				// Test trajectory.
				FBrushComponent* Obstacle = TestLine
												(	
													true,
													JumpSpot,
													Lower->Location,
													HitSpot,
													HitNormal
												);

				if( !Obstacle )
				{
					// Path is not obstructed.
					TPathEdge Edge;
					Edge.iStart		= iLower;
					Edge.iFinish	= iUpper;
					Edge.PathType	= (EPathType)Random(PATH_MAX);
					Edge.Cost		= ::Floor(JUMP_WEIGHT*(Abs(Dir.X)+Abs(Dir.Y)));
					Lower->AddEdge( Edge );

					Edge.iStart		= iUpper;
					Edge.iFinish	= iLower;
					Edge.PathType	= (EPathType)Random(PATH_MAX);
					Edge.Cost		= ::Floor(JUMP_WEIGHT*(Abs(Dir.X)+Abs(Dir.Y)));
					Lower->AddEdge( Edge );
				}
			}
		}

		if( Upper->bRight )
		{
			// Jump off to right.
			TVector Tangent = -Upper->Floor.Cross();
			TVector	JumpSpot = Upper->Location + (Upper->Floor + Tangent) * PIN_BASE * 1.f;
			TVector	Dir	= Lower->Location - Upper->Location;

			// Make sure, jump to right only, and not too far.
			if	(	Dir.X > 0.f &&
					Abs(Dir.X) <= MAX_JUMP_X &&
					!TestPoint(JumpSpot) 
				)
			{
				// Test trajectory.
				FBrushComponent* Obstacle = TestLine
												(	
													true,
													JumpSpot,
													Lower->Location,
													HitSpot,
													HitNormal
												);

				if( !Obstacle )
				{
					// Path is not obstructed.
					TPathEdge Edge;
					Edge.iStart		= iLower;
					Edge.iFinish	= iUpper;
					Edge.PathType	= (EPathType)Random(PATH_MAX);;
					Edge.Cost		= ::Floor(JUMP_WEIGHT*(Abs(Dir.X)+Abs(Dir.Y)));
					Lower->AddEdge( Edge );

					Edge.iStart		= iUpper;
					Edge.iFinish	= iLower;
					Edge.PathType	= (EPathType)Random(PATH_MAX);;
					Edge.Cost		= ::Floor(JUMP_WEIGHT*(Abs(Dir.X)+Abs(Dir.Y)));
					Lower->AddEdge( Edge );
				}
			}
		}
	}
}


//
// Pave a markers paths, such as teleports
// or ladders.
//
void CPathBuilder::PaveSpecial()
{
	//write it!!!!
}


//
// Return true, if pins are connected DIRECTLY!
// Otherwise return false.
//
Bool CPathBuilder::IsLinked( Integer iPin1, Integer iPin2 )
{
	TPathPin& P1 = Pins[iPin1];
	TPathPin& P2 = Pins[iPin2];

	for( Integer i=0; i<P1.NumEdges; i++ )
		if( P1.Edges[i].iFinish == iPin2 )
			return true;

	for( Integer i=0; i<P2.NumEdges; i++ )
		if( P2.Edges[i].iFinish == iPin1 )
			return true;

	return false;
}


//
// Test point with level's geometry. Return brush where
// point reside, or null if point is outside of any brush.
//
FBrushComponent* CPathBuilder::TestPoint( const TVector& P )
{
	assert(Level);

	for( Integer iEntity=0; iEntity<Level->Entities.Num(); iEntity++ )
		if( Level->Entities[iEntity]->Base->IsA(FBrushComponent::MetaClass) )
		{
			FBrushComponent* Brush = (FBrushComponent*)Level->Entities[iEntity]->Base;

			// Only for pure solid brushes.
			if( Brush->Type == BRUSH_Solid )
			{
				// Transform test point to brush local coords.
				TVector Local = P - Brush->Location;
				if( IsPointInsidePoly( Local, Brush->Vertices, Brush->NumVerts ) )
					return Brush;
			}
		}

	return nullptr;
}


//
// Test a line with level geometry, if no intersection return
// null, otherwise return brush, with hit normal and location
// of hit, hits are sorted by distance to A.
// bFast mean user need much close object, or just fact of
// collision.
//
FBrushComponent* CPathBuilder::TestLine( Bool bFast, const TVector& A, const TVector& B, TVector& V, TVector& Normal )
{
	FBrushComponent* Result		= nullptr;
	Float TestDist, BestDist	= 100000.f;
	TVector Vt, Nt;

	for( Integer iEntity=0; iEntity<Level->Entities.Num(); iEntity++ )
		if( Level->Entities[iEntity]->Base->IsA(FBrushComponent::MetaClass) )
		{
			FBrushComponent* Brush = (FBrushComponent*)Level->Entities[iEntity]->Base;

			if( Brush->Type != BRUSH_NotSolid )
			{
				// Test collision with solid/semi-solid brush.
				TVector LocalA = A - Brush->Location;
				TVector LocalB = B - Brush->Location;

				// Oh my God, why it's make slow test?
#if 0
				// Fast AABB test.
				TVector RectV, RectNormal;
				Float Time;
				if( !Brush->GetAABB().LineIntersect( A, B, RectV, RectNormal, Time ) )
					continue;
#endif	

				// More detail test.
				if( LineIntersectPoly( LocalA, LocalB, Brush->Vertices, Brush->NumVerts, Vt, Nt ) )
				{
					if( Brush->Type == BRUSH_Solid ||
						( Brush->Type == BRUSH_SemiSolid && IsWalkable(Nt) ) )
					{
						Vt			+= Brush->Location;
						TestDist	= (Vt - A).SizeSquared();

						if( TestDist < BestDist )
						{
							V			= Vt;
							Normal		= Nt;
							BestDist	= TestDist;
							Result		= Brush;

							if( bFast )
								return Result;
						}
					}
				}
			}
		}

	return Result;
}


/*-----------------------------------------------------------------------------
    Global path routines.
-----------------------------------------------------------------------------*/

//
// Build navigation network for entire level.
//
void CEditor::BuildPaths( FLevel* Level )
{
benchmark_begin(pathbuilder)//////////////////////////////////////////////////////////////////////////////

	CPathBuilder Builder( Level );
	Builder.BuildNetwork();

benchmark_end;
}


//
// Destroy level's navigation network.
//
void CEditor::DestroyPaths( FLevel* Level )
{
	freeandnil( Level->Navigator );
}

#endif


#if 0


/*-----------------------------------------------------------------------------
    TMap.
-----------------------------------------------------------------------------*/

//
// A naive dictionary.
//
template<class K, class V> class TMap
{
public:
	// Constructors.
	TMap()
		:	Entries()
	{}
	TMap( const TMap<K, V>& Other )
		:	Entries(Other.Entries)
	{}

	// Destructor.
	~TMap()
	{
		Entries.Empty();
	}

	// Clear map.
	void Clear()
	{
		Entries.Empty();
	}

	// Return true, if map has this key.
	Bool ContainsKey( const K& Key ) const
	{
		return FindEntry(Key) != -1;
	}

	// Return true, if map has this value.
	Bool ContainsValue( const V& Value ) const
	{
		for( Integer i=0; i<Entries.Num(); i++ )
			if( Entries[i].Value == Value )
				return true;
		return false;
	}

	// Get a value by key, if value not found return
	// nullptr.
	V* Get( const K& Key )
	{
		Integer i = FindEntry(Key);
		return i != -1 ? &Entries[i].Value : nullptr;
	}
	const V* Get( const K& Key ) const
	{
		Integer i = FindEntry(Key);
		return i != -1 ? &Entries[i].Value : nullptr;
	}

	// Return true, if map are empty.
	Bool IsEmpty() const
	{
		return Entries.Num() == 0;
	}

	// Add a new entry to map, if it found,
	// just override old.
	void Put( const K& Key, const V& Value )
	{
		Integer i = FindEntry(Key);
		if( i != -1 )
			Entries[i].Value = Value;
		else
			Add( Key, Value );
	}

	// Return list of keys.
	TArray<K> KeySet() const
	{
		TArray<K> Keys(Entries.Num());
		for( Integer i=0; i<Entries.Num(); i++ )
			Keys[i] = Entries[i].Key;
		return Keys;
	}

	// Return list of values.
	TArray<V> Values() const
	{
		TArray<V> Vals(Entries.Num());
		for( Integer i=0; i<Entries.Num(); i++ )
			Vals[i] = Entries[i].Value;
		return Vals;
	}

	// Remove an entry by key. Return false, if
	// no key found.
	Bool Remove( const K& Key )
	{
		Integer i = FindEntry(Key);
		if( i != -1 )
			Entries.Remove(i);
		return i != -1;
	}

	// Return map size.
	Integer Size() const
	{
		return Entries.Num();
	}

	// Serialization.
	friend void Serialize( CSerializer& S, TMap<K, V>& Map )
	{
		Serialize( S, Map.Entries );
	}

	// Operators.
	Bool operator==( const TMap<K, V>& Other ) const
	{
		return Entries == Other.Entries;
	}
	Bool operator!=( const TMap<K, V>& Other ) const
	{
		return Entries != Other.Entries;
	}
	TMap<K, V>& operator=( const TMap<K, V>& Other )
	{
		Entries = Other.Entries;
		return *this;
	}

private:
	// Pair struct.
	class TEntry
	{
	public:
		K		Key;
		V		Value;
		TEntry()
		{}
		TEntry( const K& InKey, const V& InValue )
			:	Key(InKey), Value(InValue)
		{}
		friend void Serialize( CSerializer& S, TEntry& Ent )
		{
			Serialize( S, Ent.Key );
			Serialize( S, Ent.Value );
		}
	};

	// List of pairs.
	TArray<TEntry>		Entries;

	// Find an index of entry by the key, if
	// no entry found return -1.
	Integer FindEntry( const K& Key ) const
	{
		for( Integer i=0; i<Entries.Num(); i++ )
			if( Entries[i].Key == Key )
				return i;
		return -1;
	}

	// Add a new entry to map.
	void Add( const K& Key, const V& Value )
	{
		Entries.Push(TEntry( Key, Value ));
	}
};
#endif



//
// OUTDATED LZW CODE, CRASHES SOMETIMES :(((((
//
#if 0
/*-----------------------------------------------------------------------------
    Bits manipulation.
-----------------------------------------------------------------------------*/

//
// Bits masks.
//
static const Byte GBits[8]		= { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static const Byte GNotBits[8]	= { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };


//
// A bit address struct.
//
union TBitAddr
{
public:
	// Variables.
	struct
	{
		DWord	iBit	: 3;
		DWord	iByte	: 29;
	};
	DWord		Addr;

	// TBitAddr interface.
	TBitAddr( DWord i )
		:	Addr(i)
	{}
};


//
// Write a single bit to the array, and 
// goto next bit.
//
inline void WriteBit( Byte* Array, TBitAddr& i, DWord Bit )
{
	if( Bit & 1 )
		Array[i.iByte]	|= GBits[i.iBit];
	else
		Array[i.iByte]	&= GNotBits[i.iBit];
	i.Addr++;
}


//
// Write a NumBits to the array.
//
inline void WriteBits( Byte* Array, TBitAddr& i, DWord Bits, DWord NumBits )
{
	for( Integer j=0; j<NumBits; j++ )
		WriteBit( Array, i, (Bits >> j) & 1 );
}


//
// Read a single bit from the array, and 
// goto next bit.
//
inline DWord ReadBit( Byte* Array, TBitAddr& i )
{
	DWord Tmp = Array[i.iByte] & GBits[i.iBit];
	i.Addr++;
	return Tmp != 0 ? 1 : 0;
}


//
// Read a NumBits from the array.
//
inline DWord ReadBits( Byte* Array, TBitAddr& i, DWord NumBits )
{
	DWord Tmp = 0;
	for( Integer j=0; j<NumBits; j++ )
		Tmp	|= ReadBit( Array, i ) << j;
	return Tmp;
}


/*-----------------------------------------------------------------------------
    CLZWCompressor.
-----------------------------------------------------------------------------*/

//
// LZW constants.
//
#define LZW_BITS			12
#define LZW_CODE_MAX		((1<<LZW_BITS)-1)
#define LZW_CODE_BEGIN		257
#define LZW_CODE_END		256
#define LZW_TABLE_SIZE		5021


//
// LZW algorithm table entry.
//
struct TLZWEntry
{
public:
	Byte		Value;
	Integer		Code;
	Integer		Prefix;
} GLZWTable[LZW_TABLE_SIZE];
static Byte GLZWStack[LZW_TABLE_SIZE]; 


//
// Find a lzw entry in global table.
//
Integer FindLZWEntry( Integer Prefix, Integer Value )
{
	Integer iEntry	= Prefix ^ (Value << (LZW_BITS-8));
	Integer Bias	= iEntry == 0 ? 1 : LZW_TABLE_SIZE-iEntry;
	for( ; ; )
	{
		if( GLZWTable[iEntry].Code == -1 )
			return iEntry;
		if	( 
				(GLZWTable[iEntry].Prefix == Prefix) && 
				(GLZWTable[iEntry].Value == (Byte)Value) 
			)
				return iEntry;

		iEntry	-= Bias;
		if( iEntry < 0 )
			iEntry	+= LZW_TABLE_SIZE;
	}
}


//
// Unpack the chunk of LZW data.
//
DWord DecodeString( DWord Count, DWord Code )
{
	while( Code > 255 )
	{
		assert(Count>=0 && Count<array_length(GLZWStack));
		assert(Code>=0 && Code<array_length(GLZWTable));
		GLZWStack[Count++] = GLZWTable[Code].Value;
		Code	= GLZWTable[Code].Prefix;
	}
	assert(Count>=0 && Count<array_length(GLZWStack));
	GLZWStack[Count++] = (Byte)Code;
	return Count;
}


//
// LZW constructor.
//
CLZWCompressor::CLZWCompressor()
{
}


//
// Encode data using LZW compression.
//
void CLZWCompressor::Encode( const void* InBuffer, DWord InSize, void*& OutBuffer, DWord& OutSize )
{
	// Allocate out buffer, with some extra memory
	// just in case.
	OutBuffer	= MemMalloc(InSize+InSize*2+8);
	OutSize		= 0;

	Byte*	In	= (Byte*)InBuffer;	
	Byte*	Out	= (Byte*)OutBuffer;

	Integer		iWalk = 0;
	TBitAddr	oWalk = 0;

	// Output length of source data.
	*(Integer*)Out	= InSize;
	oWalk.iByte		+= sizeof(Integer);

	// Setup LZW table.
	for( Integer i=0; i<LZW_TABLE_SIZE; i++ )
		GLZWTable[i].Code	= -1;

	Integer	NextCode = LZW_CODE_BEGIN, Code=0, Value=0;

	// Read first code.
	Code	= InSize > 0 ? In[iWalk++] : LZW_CODE_END;

	// Read code by code.
	while( iWalk < InSize )
	{
		Value			= In[iWalk++];
		Integer iEntry	= FindLZWEntry( Code, Value );

		if( GLZWTable[iEntry].Code != -1 )
		{
			// Entry found.
			Code	= GLZWTable[iEntry].Code;
		}
		else
		{
			// Entry not found.
			// Add to dictionary.
			if( NextCode <= LZW_CODE_MAX )
			{
				GLZWTable[iEntry].Code		= NextCode++;
				GLZWTable[iEntry].Prefix	= Code;
				GLZWTable[iEntry].Value		= Value;
			}

			// Write bits.
			WriteBits( Out, oWalk, Code, LZW_BITS );
			Code	= Value;
		}
	}

	// Write markers.
	WriteBits( Out, oWalk, Code, LZW_BITS );
	WriteBits( Out, oWalk, LZW_CODE_END, LZW_BITS );

	// Set out buffer length.
	OutSize		= oWalk.iByte + 1;
	OutBuffer	= MemRealloc( OutBuffer, OutSize );
}


//
// Decode LZW compressed data.
//
void CLZWCompressor::Decode( const void* InBuffer, DWord InSize, void*& OutBuffer, DWord& OutSize )
{
	// Setup buffers.
	Byte*		In		= (Byte*)InBuffer;
	TBitAddr	iWalk	= 0;

	OutSize		= *(Integer*)In;
	iWalk.iByte	+= sizeof(Integer);

	OutBuffer	= mem::alloc(OutSize);
	Byte*	Out		= (Byte*)OutBuffer;
	Integer	oWalk	= 0;
	
	// Setup LZW table.
	for( Integer i=0; i<LZW_TABLE_SIZE; i++ )
	{
		GLZWTable[i].Code	= -1;
		GLZWTable[i].Prefix	= 0;
		GLZWTable[i].Value	= 0;
	}
	mem::zero( GLZWStack, sizeof(GLZWStack) );

	// Setup LZW.
	Integer Value=0;
	DWord	NextCode=0, NewCode=0, OldCode=0;
	DWord	Count=0;

	NextCode	= LZW_CODE_BEGIN;
	OldCode	= ReadBits( In, iWalk, LZW_BITS );
	if( OldCode == LZW_CODE_END )
		return;
	Value	= OldCode;

	Out[oWalk++]	= OldCode;
	while( (NewCode = ReadBits(In, iWalk, LZW_BITS)) != LZW_CODE_END )
	{
		// Check for some problems.
		if( NewCode >= NextCode )
		{
			GLZWStack[0]	= (Byte)Value;
			Count	= DecodeString( 1, OldCode );
		}
		else
			Count	= DecodeString( 0, NewCode );

		assert(Count>=1 && Count<array_length(GLZWStack));
		Value	= GLZWStack[Count-1];

		// Write unpacked string.
		assert(Count>0 && Count<=array_length(GLZWStack));
		while( Count > 0 )
		{
			Out[oWalk++]	= GLZWStack[--Count];
			//assert(oWalk<OutSize);
		}


		// Update lzw table.
		if( NextCode <= LZW_CODE_MAX )
		{assert(NextCode>=0&&NextCode<array_length(GLZWTable));
			GLZWTable[NextCode].Prefix	= OldCode;
			GLZWTable[NextCode].Value	= Value;
			NextCode++;
		}
		OldCode	= NewCode;
	}
}


//
// Forecast the size of the source data 
// after a LZW compression.
//
DWord CLZWCompressor::ForecastSize( const void* InBuffer, DWord InSize )
{ 
	Byte*	In	= (Byte*)InBuffer;	
	Integer		iWalk = 0;

	DWord TotalSize	= 0;
	TotalSize	+= 32;

	// Setup LZW table.
	for( Integer i=0; i<LZW_TABLE_SIZE; i++ )
		GLZWTable[i].Code	= -1;

	Integer	NextCode = LZW_CODE_BEGIN, Code, Value;

	// Read first code.
	Code	= InSize > 0 ? In[iWalk++] : LZW_CODE_END;

	// Read code by code.
	while( iWalk < InSize )
	{
		Value			= In[iWalk++];
		Integer iEntry	= FindLZWEntry( Code, Value );

		if( GLZWTable[iEntry].Code != -1 )
		{
			// Entry found.
			Code	= GLZWTable[iEntry].Code;
		}
		else
		{
			// Entry not found.
			// Add to dictionary.
			if( NextCode <= LZW_CODE_MAX )
			{
				GLZWTable[iEntry].Code		= NextCode++;
				GLZWTable[iEntry].Prefix	= Code;
				GLZWTable[iEntry].Value		= Value;
			}

			// Write bits.
			TotalSize	+= LZW_BITS;
			Code	= Value;
		}
	}

	// Write markers.
	TotalSize	+= LZW_BITS + LZW_BITS;

	// Set out buffer length.
	TotalSize	+= 8;

	return TotalSize/8;
}


#endif


/*
	//
	// LZW test.
	//
	for( Integer i=0; i<10; i++ )
	{
	char Message[50000] = "LZW Super cool!!!  Araefmlsdkfmsdklfml                    ";
	void* Buffer;
	DWord OutSize, OutSize1;
	void* Result;

	CLZWCompressor LZW;
	LZW.Encode( Message, sizeof(Message), Buffer, OutSize );
	/*

	for( Integer i=4; i<OutSize; i++ )
	{
		log( L"[%i]: %x", i-4, ((Byte*)Buffer)[i] );
	}* /


	
	LZW.Decode( Buffer, OutSize, Result, OutSize1 );



	//log( L"%s", Result );
	printf("%s", Result);
	log(L"!!!!!");}
*/


/*
/*-----------------------------------------------------------------------------
	Level entities destruction.
-----------------------------------------------------------------------------* /

//
// Serializer to cleanup references.
//
class CRefsCleaner: public CSerializer
{
public:
	FObject*	Target;

	// CRefsCleaner interface.
	CRefsCleaner( FObject* InTarget )
		:	Target( InTarget )
	{
		Mode = SM_Undefined;
	}
	~CRefsCleaner()
	{}

	// CSerializer interface.
	void SerializeData( void* Mem, DWord Count )
	{}
	void SerializeRef( FObject*& Obj )
	{
		if( Obj == Target )
			Obj = nullptr;
	}
};


//
// Serialize level and all it objects. 
// 
void SerializeEntireLevel( CSerializer& S, FLevel* Lev )
{
	Lev->SerializeThis( S );

	for( Integer i=0; i<Lev->Entities.Num(); i++ )
	{
		FEntity* Ent = Lev->Entities[i];
		Ent->SerializeThis( S );
		Ent->Base->SerializeThis( S );

		for( Integer e=0; e<Ent->Components.Num(); e++ )
			Ent->Components[e]->SerializeThis( S );
	}
}


//
// Destroy a level such as entitity or component.
//
void DestroyLevelObject( FObject* Obj, FLevel* Level )
{
	if( !Level->bIsPlaying )
	{
		DestroyObject( Obj, true );
		return;
	}

	CRefsCleaner Cleaner( Obj );
	SerializeEntireLevel( Cleaner, Level );
	Obj->SerializeThis( Cleaner );

	if( Obj->IsA(FEntity::MetaClass) )
	{
		FEntity* Entity = (FEntity*)Obj;
		Entity->Base->SerializeThis( Cleaner );
		for( Integer e=0; e<Entity->Components.Num(); e++ )
			Entity->Components[e]->SerializeThis( Cleaner );
	}

	// Totally kill object.
	DestroyObject( Obj, false );
}
*/



/*
; Here you/player can remap keyboard keys, it's can
; changed manually via this file, or in game.
; here's litle example of remap:
;JoyUp=37	; When user clicked JoyUp it will be redirected to 37 button(<up>).
;A=66	; uhh.
*/



/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/