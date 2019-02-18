/*=============================================================================
	FrDbgDraw.h: Debug Drawing helper class.
	Created by Vlad Gordienko, May. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CDebugDrawHelper.
-----------------------------------------------------------------------------*/

//
// A debug lines and points draw helper.
//
class CDebugDrawHelper
{
public:
	// Constants.
	enum{ MAX_LINES = 1024 };
	enum{ MAX_POINTS = 512 };

	// CDebugDrawHelper interface.
	Bool DrawLine( const math::Vector& A, const math::Vector& B, TColor Color, Float LifeTime=0.f );
	Bool DrawPoint( const math::Vector& P, TColor Color, Float Size, Float LifeTime=0.f );
	void Render( CCanvas* Canvas );
	void Tick( Float Delta );
	void Reset();

	// Instance.
	static CDebugDrawHelper& Instance();

private:
	// Internal interface.
	CDebugDrawHelper();

	// Structs.
	struct TTempPoint
	{
		math::Vector Position;
		TColor Color;
		Float Size;
		Float Life;
	};
	struct TTempLine
	{
		math::Vector PFrom;
		math::Vector PTo;
		TColor Color;
		Float Life;
	};

	Array<TTempPoint> Points;
	Array<TTempLine> Lines;
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/