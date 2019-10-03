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
	void SetLocation( const math::Vector& NewLoc );
	void SetRotation( math::Angle NewAng );
	void Move( const math::Vector& DeltaMove );
	void Rotate( math::Angle DeltaRot );
	void SetMode( EGizmoMode NewMode );
	void SetAxis( EGizmoAxis Selected );

	// World interaction.
	void Render( CCanvas* Canvas );
	EGizmoAxis AxisAt( const gfx::ViewInfo& ViewInfo, Int32 Cx, Int32 Cy );
	void Perform
	(
		const gfx::ViewInfo& ViewInfo,
		const math::Vector& CursorPos,
		const math::Vector& MovementDelta,
		math::Vector* OutTranslation,
		math::Angle* OutRotation,
		math::Vector* OutScale
	);

private:
	// Gizmo internal.
	EGizmoMode		Mode;
	EGizmoAxis		CurrentAxis;
	Float			Size;
	math::Vector	Location;
	math::Angle		Rotation;
	math::Vector	Scale;
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/