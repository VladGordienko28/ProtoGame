/*=============================================================================
    FrComponent.h: Basic components classes.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    FComponent.
-----------------------------------------------------------------------------*/

//
// An abstract component class.
//
class FComponent: public FObject
{
REGISTER_CLASS_H(FComponent);
public:
	// Variables.
	FScript*		Script;
	FEntity*		Entity;
	FLevel*			Level;

	// FComponent interface.
	FComponent();
	~FComponent();
	virtual void InitForEntity( FEntity* InEntity );
	virtual void InitForScript( FScript* InScript );
	virtual Float GetLayer() const;
	virtual void BeginPlay();
	virtual void EndPlay();

	// Tick methods.
	virtual void PreTick( Float Delta ){}
	virtual void Tick( Float Delta ){}
	virtual void TickNonPlay( Float Delta ){}

	// Render functions.
	virtual void Render( CCanvas* Canvas ){}

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

protected:
	// Extended behaviour.
	Bool	bTickable;
	Bool	bRenderable;
};


/*-----------------------------------------------------------------------------
    FExtraComponent.
-----------------------------------------------------------------------------*/

//
// An addition component for entity.
//
class FExtraComponent: public FComponent
{
REGISTER_CLASS_H(FExtraComponent)
public:
	// Variables.
	Float				DrawOrder;
	FBaseComponent*		Base;

	// FExtraComponent interface.
	FExtraComponent();

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );
	void InitForScript( FScript* InScript );
	Float GetLayer() const;

	// FObject interface.
	void PostLoad();
};


/*-----------------------------------------------------------------------------
    FBaseComponent.
-----------------------------------------------------------------------------*/

//
// A base entity component.
//
class FBaseComponent: public FComponent
{
REGISTER_CLASS_H(FBaseComponent)
public:
	// Flags.
	Bool			bDestroyed;
	Bool			bFixedAngle;
	Bool			bFixedSize;
	Bool			bHashable;
	
	// Editor flags.
	Bool			bSelected;
	Bool			bFrozen;

	// World info.
	math::Vector	Location;
	math::Angle		Rotation;
	math::Vector	Size;
	Float			Layer;

	// FBaseComponent interface.
	FBaseComponent();
	virtual math::Rect GetAABB();

	// FComponent interface.
	void InitForEntity( FEntity* InEntity );
	void InitForScript( FScript* InScript );
	Float GetLayer() const;
	void BeginPlay();
	void EndPlay();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();

	// Accessors.
	inline math::Coords ToWorld() const
	{
		return math::Coords( Location, Rotation ).transpose();
	}
	inline math::Coords ToLocal() const
	{
		return math::Coords( Location, Rotation );
	}
	inline Bool IsHashed() const
	{
		return bHashed;
	}


private:
	// Collision hash internal.
	friend CCollisionHash;
	Bool		bHashed;
	UInt32		HashMark;
	math::Rect	HashAABB;

	// Natives.
	void nativeSetLocation( CFrame& Frame );
	void nativeSetSize( CFrame& Frame );
	void nativeSetRotation( CFrame& Frame );
	void nativeMove( CFrame& Frame );
	void nativePlayAmbient( CFrame& Frame );
	void nativeStopAmbient( CFrame& Frame );
	void nativeIsBasedOn( CFrame& Frame );
	void nativeGetAABB( CFrame& Frame );
};


/*-----------------------------------------------------------------------------
    Component macro.
-----------------------------------------------------------------------------*/

//
// Add component to the level's list of components.
//
#define com_add( list )\
{\
	if( Level )\
	{\
		this->Next##list = Level->First##list;\
		Level->First##list = this;\
	}\
}\


//
// Remove component from the level's list.
//
#define com_remove( list )\
{ \
	if( Level )\
	{ \
		auto ComPtr = &Level->First##list;\
		while( *ComPtr )\
		{\
			if( *ComPtr != this )\
			{\
				ComPtr = &((*ComPtr)->Next##list);\
			}\
			else\
			{\
				*ComPtr = (*ComPtr)->Next##list;\
				break;\
			}\
		}\
	}\
}\


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/