/*=============================================================================
	FrGizmo.h: Editor objects manipulator.
	Prototype by Vlad Gordienko, Jul. 2017.
	Created Dec. 2017.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CGizmo.
-----------------------------------------------------------------------------*/

//
// A gizmo tool mode.
//
enum EGizmoMode
{
	GIZMO_Translate,
	GIZMO_Rotate,
	GIZMO_Scale,
	GIZMO_MAX
};


//
// A gizmo active axis.
//
enum EGizmoAxis
{
	GIAX_None,
	GIAX_X,
	GIAX_Y,
	GIAX_Both,
	GIAX_MAX
};


//
// An gizmo itself.
//
class CGizmo
{
public:
	// CGizmo interface.
	CGizmo( Float InSize = 1.f );
	~CGizmo();
	void Reset();

	// Setters.
	void SetLocation( const TVector& NewLoc );
	void SetRotation( TAngle NewAng );
	void Move( const TVector& DeltaMove );
	void Rotate( TAngle DeltaRot );
	void SetMode( EGizmoMode NewMode );
	void SetAxis( EGizmoAxis Selected );

	// World interaction.
	void Render( CCanvas* Canvas );
	EGizmoAxis AxisAt( const TViewInfo& ViewInfo, Int32 Cx, Int32 Cy );
	void Perform
	(
		const TViewInfo& ViewInfo,
		const TVector& CursorPos,
		const TVector& MovementDelta,
		TVector* OutTranslation,
		TAngle* OutRotation,
		TVector* OutScale
	);

private:
	// Gizmo internal.
	EGizmoMode		Mode;
	EGizmoAxis		CurrentAxis;
	Float			Size;
	TVector			Location;
	TAngle			Rotation;
	TVector			Scale;
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/