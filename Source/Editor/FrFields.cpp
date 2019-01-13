/*=============================================================================
    FrFields.cpp: WObjectInspector.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

//
// Inspector values limit.
//
#define SPINNER_INTEGER_MIN		-999999
#define SPINNER_INTEGER_MAX		+999999
#define SPINNER_INTEGER_SCALE	1

#define SPINNER_FLOAT_MIN		-99999.9999f
#define SPINNER_FLOAT_MAX		+99999.9999f
#define	SPINNER_FLOAT_SCALE		0.01f


/*-----------------------------------------------------------------------------
    Inspector colors.
-----------------------------------------------------------------------------*/

#define COLOR_INSPECTOR_BACKDROP		TColor( 0x3f, 0x3f, 0x3f, 0xff )
#define COLOR_INSPECTOR_BORDER			TColor( 0x66, 0x66, 0x66, 0xff )
#define COLOR_INSPECTOR_ITEM			TColor( 0x30, 0x30, 0x30, 0xff )
#define COLOR_INSPECTOR_ITEM_COMPON		TColor( 0x25, 0x25, 0x25, 0xff )
#define COLOR_INSPECTOR_ITEM_BORDER		TColor( 0x3f, 0x3f, 0x3f, 0xff )
#define COLOR_INSPECTOR_ITEM_SEL		TColor( 0x22, 0x22, 0x22, 0xff )
#define COLOR_INSPECTOR_ITEM_WAIT		COLOR_IndianRed
#define COLOR_INSPECTOR_ITEM_NOEDIT		TColor( 0x37, 0x37, 0x37, 0xff )


/*-----------------------------------------------------------------------------
    CInspectorItemBase implementation.
-----------------------------------------------------------------------------*/

//
// Base item constructor.
//
CInspectorItemBase::CInspectorItemBase( WObjectInspector* InInspector, UInt32 InDepth )
	:	bHidden( false ),
		bExpanded( false ),
		Caption( L"" ),
		Inspector( InInspector ),
		Children(),
		Objects(),
		Depth( 0 ),
		Top( 0 )
{
	assert(Inspector);
}


//
// Base item destructor.
//
CInspectorItemBase::~CInspectorItemBase()
{
	// Don't destroy children, they actually 
	// owned by inspector.
	Children.Empty();
	Objects.Empty();
}


//
// Collapse all children.
//
void CInspectorItemBase::CollapseAll()
{
	for( Int32 i=0; i<Children.Num(); i++ )
	{
		Children[i]->CollapseAll();
		Children[i]->bHidden	= true;
	}

	bExpanded	= false;
	Inspector->UpdateChildren();
}


//
// Expand all children.
//
void CInspectorItemBase::ExpandAll()
{
	// Don't expand children of children.
	for( Int32 i=0; i<Children.Num(); i++ )
		Children[i]->bHidden	= false;

	bExpanded	= true;
	Inspector->UpdateChildren();
}


//
// Draw the base item, should be implemented in
// derived classes.
//
void CInspectorItemBase::Paint( TPoint Base, CGUIRenderBase* Render )
{
}


//
// Mouse press base item.
//
void CInspectorItemBase::MouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
}


//
// Mouse up base item.
//
void CInspectorItemBase::MouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
}


//
// Mouse move base item.
//
void CInspectorItemBase::MouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
}


//
// When user drag something above item.
//
void CInspectorItemBase::DragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept )
{
	bAccept	= false;
}


//
// User drop something on item.
//
void CInspectorItemBase::DragDrop( void* Data, Int32 X, Int32 Y )
{
}


//
// Item has lost the focus.
//
void CInspectorItemBase::Unselect()
{
}


/*-----------------------------------------------------------------------------
    CPropertyItem.
-----------------------------------------------------------------------------*/

//
// Property item.
//
class CPropertyItem: public CInspectorItemBase
{
public:
	// Variables.
	CTypeInfo		TypeInfo;
	Bool			bExpandable;
	SizeT			AddrOffset;
	UInt32			Flags;
	UInt8*			CustomAddr;

	// Temporal and optional widgets.
	WButton*		Button;
	WSpinner*		Spinner;
	WEdit*			Edit;
	WComboBox*		ComboBox;

	// Dynamic array buttons.
	WButton*		AddButton;
	WButton*		RemoveButton;
	Int32			iDynFirst;
	Int32			iDynLast;

	// CPropertyItem interface.
	CPropertyItem
	( 
		WObjectInspector* InInspector,
		UInt32 InDepth,
		UInt8* InCustomAddr,
		String InCaption,
		const CTypeInfo& InType,
		UInt32 InFlags
	);
	CPropertyItem	
	(	
		WObjectInspector* InInspector, 
		UInt32 InDepth, 
		const TArray<FObject*>& InObjs, 
		String InCaption, 
		const CTypeInfo& InType, 
		SizeT InAddrOffset, 
		UInt32 InFlags 
	);

	~CPropertyItem();
	Bool IsAtSeparator( Int32 X ) const;
	Bool IsAtSign( Int32 X ) const;
	inline void* GetAddress( Int32 iObject );

	// Event from temporal widgets.
	void OnSomethingChange( WWidget* Sender );
	void OnPickClick( WWidget* Sender );

	// Events from Object Inspector.
	void Paint( TPoint Base, CGUIRenderBase* Render );
	void MouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void MouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void MouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void DragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept );
	void DragDrop( void* Data, Int32 X, Int32 Y );
	void Unselect();

	// Dynamic array functions.
	void OnButtonAddClick( WWidget* Sender );
	void OnButtonRemoveClick( WWidget* Sender );
	void RebuildDynamicArrayList();
};


// Dynamic array crud!
static CPropertyItem* GDynArrayItem = nullptr;


//
// Custom property constructor.
//
CPropertyItem::CPropertyItem
( 
	WObjectInspector* InInspector,
	UInt32 InDepth,
	UInt8* InCustomAddr,
	String InCaption,
	const CTypeInfo& InType,
	UInt32 InFlags
)	
	:	CInspectorItemBase( InInspector, InDepth ),
		Button( nullptr ),
		ComboBox( nullptr ),
		Edit( nullptr ),
		Spinner( nullptr ),
		AddButton( nullptr ),
		RemoveButton( nullptr ),
		Flags( InFlags ),
		TypeInfo( InType ),
		AddrOffset( 0 ),
		CustomAddr( InCustomAddr ),
		bExpandable( false ),
		iDynFirst( -1 ),
		iDynLast( -1 )
{
	Caption	= InCaption;
	Depth	= InDepth;

	// Add to inspector.
	Inspector->Children.Push(this);

	if( TypeInfo.ArrayDim > 1 )
	{
		// Item is an array property.
		CTypeInfo InnerType = TypeInfo;
		InnerType.ArrayDim	= 1;

		// Add each time.
		for( Int32 i=0; i<TypeInfo.ArrayDim; i++ )
			Children.Push( new CPropertyItem
										(
											Inspector,
											Depth + 1,
											InCustomAddr + i*InnerType.TypeSize(),
											String::Format( L"[%d]", i ),
											InnerType,
											Flags
										) );

		bExpandable	= true;
	}
	else if( TypeInfo.ArrayDim == -1 )
	{
		// Item is a dynamic array property.
		bExpandable	= true;
		RebuildDynamicArrayList();
	}
	else
	{
		// Scalar value.
		if( TypeInfo.Type == TYPE_Color )
		{
			// Color struct.
			static const Char* MemNames[4] = { L"R", L"G", L"B", L"A" };

			for( Int32 i=0; i<4; i++ )
				Children.Push( new CPropertyItem
											(
												Inspector,
												Depth + 1,
												CustomAddr + (SizeT)((UInt8*)&(&((TColor*)nullptr)->R)[i] -(UInt8*)nullptr),
												MemNames[i],
												TYPE_Byte,
												Flags
											) );

			bExpandable	= true;
		}
		else if( TypeInfo.Type == TYPE_Vector )
		{
			// Vector struct.
			static const Char* MemNames[2] = { L"X", L"Y" };

			for( Int32 i=0; i<2; i++ )
				Children.Push( new CPropertyItem
											(
												Inspector,
												Depth + 1,
												CustomAddr + (SizeT)((UInt8*)&(&((TVector*)nullptr)->X)[i] -(UInt8*)nullptr),
												MemNames[i],
												TYPE_Float,
												Flags
											) );

			bExpandable	= true;
		}
		else if( TypeInfo.Type == TYPE_AABB )
		{
			// AABB struct.
			static const Char* MemNames[2] = { L"Min", L"Max" };

			for( Int32 i=0; i<2; i++ )
				Children.Push( new CPropertyItem
											(
												Inspector,
												Depth + 1,
												CustomAddr + (SizeT)((UInt8*)&(&((TRect*)nullptr)->Min)[i] -(UInt8*)nullptr),
												MemNames[i],
												TYPE_Vector,
												Flags
											) );

			bExpandable	= true;
		}
		else if( TypeInfo.Type == TYPE_Struct )
		{
			// Generic struct.
			CStruct* Struct = TypeInfo.Struct;
			assert(Struct);

			for( Int32 i=0; i<Struct->Members.Num(); i++ )
				Children.Push( new CPropertyItem
				(
					Inspector,
					Depth + 1,
					CustomAddr + Struct->Members[i]->Offset,
					Struct->Members[i]->Name,
					*Struct->Members[i],
					Flags
				) );

			bExpandable	= true;
		}
	}

	bExpanded = false;
	CollapseAll();
}


//
// Property item constructor.
//
CPropertyItem::CPropertyItem
(	
	WObjectInspector* InInspector, 
	UInt32 InDepth, 
	const TArray<FObject*>& InObjs, 
	String InCaption, 
	const CTypeInfo& InType, 
	SizeT InAddrOffset, 
	UInt32 InFlags 
)
	:	CInspectorItemBase( InInspector, InDepth ),
		Button( nullptr ),
		ComboBox( nullptr ),
		Edit( nullptr ),
		Spinner( nullptr ),
		AddButton( nullptr ),
		RemoveButton( nullptr ),
		Flags( InFlags ),
		TypeInfo( InType ),
		AddrOffset( InAddrOffset ),
		bExpandable( false ),
		CustomAddr( nullptr ),
		iDynFirst( -1 ),
		iDynLast( -1 )
{
	Caption	= InCaption;
	Depth	= InDepth;

	// Add to inspector.
	Inspector->Children.Push(this);
	Objects = InObjs;

	if( TypeInfo.ArrayDim > 1 )
	{
		// Item is an array property.
		CTypeInfo InnerType = TypeInfo;
		InnerType.ArrayDim	= 1;

		// Add each time.
		for( Int32 i=0; i<TypeInfo.ArrayDim; i++ )
			Children.Push( new CPropertyItem
										(
											Inspector,
											Depth + 1,
											InObjs,
											String::Format( L"[%d]", i ),
											InnerType,
											InAddrOffset + i*InnerType.TypeSize(),
											Flags
										) );

		bExpandable	= true;
	}
	else if( TypeInfo.ArrayDim == -1 )
	{
		// Item is a dynamic array property.
		if( InObjs.Num() == 1 )
		{
			bExpandable	= true;
			RebuildDynamicArrayList();
		}
	}
	else
	{
		// Scalar value.
		if( TypeInfo.Type == TYPE_Color )
		{
			// Color struct.
			static const Char* MemNames[4] = { L"R", L"G", L"B", L"A" };

			for( Int32 i=0; i<4; i++ )
				Children.Push( new CPropertyItem
											(
												Inspector,
												Depth + 1,
												InObjs,
												MemNames[i],
												TYPE_Byte,
												InAddrOffset + (SizeT)((UInt8*)&(&((TColor*)nullptr)->R)[i] -(UInt8*)nullptr),
												Flags
											) );

			bExpandable	= true;
		}
		else if( TypeInfo.Type == TYPE_Vector )
		{
			// Vector struct.
			static const Char* MemNames[2] = { L"X", L"Y" };

			for( Int32 i=0; i<2; i++ )
				Children.Push( new CPropertyItem
											(
												Inspector,
												Depth + 1,
												InObjs,
												MemNames[i],
												TYPE_Float,
												InAddrOffset + (SizeT)((UInt8*)&(&((TVector*)nullptr)->X)[i] -(UInt8*)nullptr),
												Flags
											) );

			bExpandable	= true;
		}
		else if( TypeInfo.Type == TYPE_AABB )
		{
			// AABB struct.
			static const Char* MemNames[2] = { L"Min", L"Max" };

			for( Int32 i=0; i<2; i++ )
				Children.Push( new CPropertyItem
											(
												Inspector,
												Depth + 1,
												InObjs,
												MemNames[i],
												TYPE_Vector,
												InAddrOffset + (SizeT)((UInt8*)&(&((TRect*)nullptr)->Min)[i] -(UInt8*)nullptr),
												Flags
											) );

			bExpandable	= true;
		}
		else if( TypeInfo.Type == TYPE_Struct )
		{
			// Generic struct.
			CStruct* Struct = TypeInfo.Struct;
			assert(Struct);

			for( Int32 i=0; i<Struct->Members.Num(); i++ )
				Children.Push( new CPropertyItem
				(
					Inspector,
					Depth + 1,
					InObjs,
					Struct->Members[i]->Name,
					*Struct->Members[i],
					InAddrOffset + Struct->Members[i]->Offset,
					Flags
				) );

			bExpandable	= true;
		}
	}

	bExpanded = false;
	CollapseAll();
}


//
// Property item destructor.
//
CPropertyItem::~CPropertyItem()
{
	// Release the controls.
	freeandnil( Button );
	freeandnil( Spinner );
	freeandnil( Edit );
	freeandnil( ComboBox );

	freeandnil( AddButton );
	freeandnil( RemoveButton );
}


//
// Insert a dynamic array items.
//
void CPropertyItem::RebuildDynamicArrayList()
{
	assert(bExpandable);

	// Destroy own items. It is hacky way to manipulate inspector
	// here, but it only way to do it.
	for( Int32 i=0; i<Children.Num(); i++ )
	{
		Inspector->Children.RemoveUnique(Children[i]);
		freeandnil(Children[i]);
	}
	Children.Empty();
	Inspector->UpdateChildren();

	CTypeInfo InnerType( TypeInfo.Type, 1, TypeInfo.Inner );
	iDynFirst = Inspector->Children.Num();

	if( CustomAddr )
	{
		// Items being custom item.
		TArrayBase* Array = (TArrayBase*)CustomAddr;

		for( Int32 i=0; i<Array->Count; i++ )
			Children.Push( new CPropertyItem
			(
				Inspector,
				Depth + 1,
				(UInt8*)Array->Data + i*InnerType.TypeSize(),
				String::Format( L"[%d]", i ),
				InnerType,
				Flags
			) );
	}
	else
	{
		// Items being object.
		assert(Objects.Num() == 1);
		TArrayBase* Array = (TArrayBase*)GetAddress(0);

		// Add this items as custom. So no EditChange notification :(
		for( Int32 i=0; i<Array->Count; i++ )
			Children.Push( new CPropertyItem
			(
				Inspector,
				Depth + 1,
				(UInt8*)Array->Data + i*InnerType.TypeSize(),
				String::Format( L"[%d]", i ),
				InnerType,
				Flags
			) );
	}

	iDynLast = Inspector->Children.Num()-1;

	// fixme!
/*
	// Insert children after this items.
	Integer iThis = Inspector->Children.FindItem(this);
	assert(iThis != -1);

	Inspector->Children.Insert( iThis+1, Children.Num() );
	for( Integer i=0; i<Children.Num(); i++ )
		Inspector->Children[iThis+i+1] = Inspector->Children[Inspector->Children.Num()-Children.Num()-1];
	Inspector->Children.SetNum( Inspector->Children.Num() - Children.Num() );
	Inspector->UpdateChildren();
*/
}


//
// Let user pick the entity.
//
void CPropertyItem::OnPickClick( WWidget* Sender )
{
	assert(TypeInfo.Type == TYPE_Entity);
	Inspector->BeginWaitForPick( this );

	if( Inspector->bWaitForPick )
	{
		Button->bEnabled	= false;
		Button->bDown		= true;
	}
}


//
// User drop resource here.
//
void CPropertyItem::DragDrop( void* Data, Int32 X, Int32 Y )
{
	FObject* Res = (FObject*)Data;
	assert( Res && Res->IsA(FResource::MetaClass) );

	if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackEnter();
	{
		// Set value.
		for( Int32 i=0; i<Objects.Num(); i++ )
			*(FObject**)GetAddress(i) = Res;	

		// Notify.
		for( Int32 i=0; i<Objects.Num(); i++ )
			Objects[i]->EditChange();
		for( Int32 i=0; i<Inspector->CustomHandlers.Num(); i++ )
			Inspector->CustomHandlers[i]( Inspector );
	}
	if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackLeave();
}


//
// User drag over something, decide it is correct object or not.
//
void CPropertyItem::DragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept )
{
	bAccept =	!( Flags & PROP_Const )&& 
				( TypeInfo.ArrayDim == 1 )&& 
				( TypeInfo.Type == TYPE_Resource )&& 
				( Data )&&
				((FObject*)Data)->IsA(TypeInfo.Class);
}	


//
// Return absolute address of the property of this item for
// the i'th object.
//
inline void* CPropertyItem::GetAddress( Int32 iObject )
{
	assert((CustomAddr != nullptr) ^ (Objects.Num() != 0));

	if( CustomAddr )
	{
		return CustomAddr;
	}
	else
	{
		assert( iObject>=0 && iObject<Objects.Num() );

		// Hack a little.
		if( Objects[0]->IsA(FEntity::MetaClass) )
		{
			// Entity property from the CInstanceBuffer.
			FEntity* Entity = (FEntity*)Objects[iObject];
			return (void*)((UInt8*)(&Entity->InstanceBuffer->Data[0]) + AddrOffset);
		}
		else if( Objects[0]->IsA(FScript::MetaClass) && !(Flags & PROP_Native) )
		{
			// Script property from the instance buffer.
			FScript* Script = (FScript*)Objects[iObject];
			return (void*)((UInt8*)(&Script->InstanceBuffer->Data[0]) + AddrOffset);
		}
		else
		{
			// Regular property.
			return (void*)(((UInt8*)Objects[iObject]) + AddrOffset);
		}
	}
}


//
// Called when some fields editor control value has been modified.
//
void CPropertyItem::OnSomethingChange( WWidget* Sender )
{
	assert(TypeInfo.ArrayDim == 1);

	switch( TypeInfo.Type )
	{
		case TYPE_Bool:
		{
			// Boolean value.
			Int32 iFlag = ComboBox->ItemIndex;

			if( iFlag != 0 && iFlag != 1 )
				return;

			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackEnter();
			{
				if( CustomAddr ) 
					*(Bool*)CustomAddr = iFlag != 0;

				for( Int32 i=0; i<Objects.Num(); i++ )
					*(Bool*)GetAddress(i) = iFlag != 0;
			}
			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackLeave();

			break;
		}
		case TYPE_Float:
		{
			// Float value.
			Float Value = Spinner->GetFloatValue();

			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackEnter();
			{
				if( CustomAddr ) 
					*(Float*)CustomAddr = Value;

				for( Int32 i=0; i<Objects.Num(); i++ )
					*(Float*)GetAddress(i) = Value;
			}
			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackLeave();
			break;
		}
		case TYPE_Byte:
		{
			// Byte value.
			Int32 Value = Spinner ? Spinner->GetIntValue() : ComboBox->ItemIndex;

			if( Value > 255 || Value < 0 )
				return;

			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackEnter();
			{
				if( CustomAddr ) 
					*(UInt8*)CustomAddr = Value;

				for( Int32 i=0; i<Objects.Num(); i++ )
					*(UInt8*)GetAddress(i) = Value;
			}
			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackLeave();

			break;
		}
		case TYPE_String:
		{
			// String value.
			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackEnter();
			{
				if( CustomAddr ) 
					*(String*)CustomAddr = Edit->Text;

				for( Int32 i=0; i<Objects.Num(); i++ )
					*(String*)GetAddress(i) = Edit->Text;
			}
			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackLeave();
			break;
		}
		case TYPE_Integer:
		{
			// Integer value.
			Int32 Value = Spinner->GetIntValue();
			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackEnter();
			{
				if( CustomAddr ) 
					*(Int32*)CustomAddr = Value;

				for( Int32 i=0; i<Objects.Num(); i++ )
					*(Int32*)GetAddress(i) = Value;
			}
			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackLeave();
			break;
		}
		case TYPE_Angle:
		{
			// Angle value.
			Float Value = Spinner->GetFloatValue();

			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackEnter();
			{
				if( CustomAddr ) 
					*(TAngle*)CustomAddr = TAngle::FromDegs(Value);

				for( Int32 i=0; i<Objects.Num(); i++ )
					*(TAngle*)GetAddress(i) = TAngle::FromDegs(Value);
			}
			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackLeave();
			break;
		}
		case TYPE_Resource:
		{
			// Resource ref.
			CClass* ResClass = TypeInfo.Class;
			assert(ResClass && ResClass->IsA(FResource::MetaClass));
			FObject* ResRef = GProject->FindObject( Edit->Text, ResClass, nullptr );

			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackEnter();
			{
				if( CustomAddr ) 
					*(FResource**)CustomAddr = (FResource*)ResRef;

				for( Int32 i=0; i<Objects.Num(); i++ )
					*(FResource**)GetAddress(i) = (FResource*)ResRef;
			}
			if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackLeave();
			break;
		}
		case TYPE_Entity:
		{
			// Entity ref.
			WEditorPage* Page = (WEditorPage*)GEditor->EditorPages->GetActivePage();
			if( Page && Page->PageType == PAGE_Level )
			{
				FLevel* Level = ((WLevelPage*)Page)->Level;
				assert(Level);
				FEntity* Gotten = Level->FindEntity( Edit->Text );
				FEntity* Result = nullptr;

				if( Gotten )
					Result = TypeInfo.Script ? ( TypeInfo.Script == Gotten->Script ? Gotten : nullptr ) : Gotten;
				else
					Result = nullptr;

				if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackEnter();
				{
					if( CustomAddr ) 
						*(FEntity**)CustomAddr = (FEntity*)Result;

					for( Int32 i=0; i<Objects.Num(); i++ )
						*(FEntity**)GetAddress(i) = (FEntity*)Result;
				}
				if( Inspector->LevelPage )	Inspector->LevelPage->Transactor->TrackLeave();
			}
			break;
		}
		default:
			fatal( L"Unsupported property type %d", (UInt8)TypeInfo.Type );
	}

	// Notify all objects.
	for( Int32 i=0; i<Objects.Num(); i++ )
		Objects[i]->EditChange();
	for( Int32 i=0; i<Inspector->CustomHandlers.Num(); i++ )
		Inspector->CustomHandlers[i]( Inspector );
}


//
// Return true, if x value is at collapse/expand
// button.
//
Bool CPropertyItem::IsAtSign( Int32 X ) const
{ 
	return Abs( X - (18+Depth*20) ) < 20;
}


//
// User has clicked item.
//
void CPropertyItem::MouseDown( EMouseButton MouseButton, Int32 X, Int32 Y )
{
	// Maybe just slide separator.
	if( IsAtSeparator(X) )
	{
		Inspector->bMoveSep	= true;
		return;
	}

	// Unselect all.
	Inspector->UnselectAll();

	if( bExpandable )
	{
		// A struct or array.
		if( IsAtSign(X) )
		{
			// Toggle children.
			if( bExpanded )
				CollapseAll();
			else
				ExpandAll();
		}
		else
		{
			// Color dialog.
			if( TypeInfo.Type == TYPE_Color && TypeInfo.ArrayDim==1 && X > Inspector->Size.Width-54 )
			{
				WColorChooser::SharedColor	= *(TColor*)GetAddress(0);
				Inspector->WaitColor		= this;
				new WColorChooser( Inspector->Root, false, TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::ColorSelected ) );
			}
			else if( TypeInfo.ArrayDim == -1 )
			{
				// Dynamic array buttons.
				GDynArrayItem = this;

				AddButton = new WButton( Inspector, Inspector->Root );
				AddButton->SetSize( 32, INSPECTOR_ITEM_HEIGHT-0 );
				AddButton->Location = TPoint( Inspector->Size.Width - 11 - AddButton->Size.Width, Top+0 );
				AddButton->EventClick = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::DynArrayAddClick );
				AddButton->Caption = L"Add";

				RemoveButton = new WButton( Inspector, Inspector->Root );
				RemoveButton->SetSize( 50, INSPECTOR_ITEM_HEIGHT-0 );
				RemoveButton->Location = TPoint( Inspector->Size.Width - 10 - RemoveButton->Size.Width - AddButton->Size.Width, Top+0 );
				RemoveButton->EventClick = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::DynArrayRemoveClick );
				RemoveButton->Caption = L"Remove";
			}
		}
	}
	else if( TypeInfo.ArrayDim == 1 && !(Flags & PROP_Const) && TypeInfo.Type != TYPE_Delegate )
	{
		// Simple value.
		Inspector->Selected = this;

		// Test value in all object,s they are total matched?
		Bool bMatched = true;
		for( Int32 i=1; i<Objects.Num(); i++ )
			if( !TypeInfo.CompareValue( GetAddress(0), GetAddress(i) ) )
			{
				bMatched	= false;
				break;
			}

		switch( TypeInfo.Type )
		{
			case TYPE_Bool:
			{
				// Boolean value.
				ComboBox		= new WComboBox( Inspector, Inspector->Root );
				ComboBox->SetSize( Inspector->Size.Width-Inspector->Separator-11, INSPECTOR_ITEM_HEIGHT );
				ComboBox->Location = TPoint( Inspector->Separator, Top );
				ComboBox->EventChange = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::SomethingChange );
				ComboBox->AddItem( L"False", nullptr );
				ComboBox->AddItem( L"True" , nullptr );
				if( bMatched )
					ComboBox->SetItemIndex( *(Bool*)GetAddress(0) ? 1 : 0, false );
				else
					ComboBox->SetItemIndex( -1, false );
				break;
			}
			case TYPE_Byte:
			{
				// Byte value.
				if( TypeInfo.Enum )
				{
					// Enumeration.
					ComboBox		= new WComboBox( Inspector, Inspector->Root );
					ComboBox->SetSize( Inspector->Size.Width-Inspector->Separator-11, INSPECTOR_ITEM_HEIGHT );
					ComboBox->Location = TPoint( Inspector->Separator, Top );
					ComboBox->EventChange = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::SomethingChange );

					for( Int32 i=0; i<TypeInfo.Enum->Elements.Num(); i++ )
						ComboBox->AddItem( TypeInfo.Enum->GetAliasOf(i), nullptr );

					ComboBox->SetItemIndex( bMatched ? *(UInt8*)GetAddress(0) : -1, false );
					break;
				}
				else
				{
					// Simple value.
					Spinner		= new WSpinner( Inspector, Inspector->Root );
					Spinner->SetSize( Inspector->Size.Width-Inspector->Separator-14, INSPECTOR_ITEM_HEIGHT-2 );
					Spinner->Location = TPoint( Inspector->Separator+1, Top+1 );
					Spinner->SetRange( 0, 255, 1 );
					Spinner->EventChange = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::SomethingChange );
					if( bMatched )
						Spinner->SetValue(*(UInt8*)GetAddress(0), false);
				}
				break;
			}
			case TYPE_Float:
			{
				// Float value.
				Spinner		= new WSpinner( Inspector, Inspector->Root );
				Spinner->SetSize( Inspector->Size.Width-Inspector->Separator-14, INSPECTOR_ITEM_HEIGHT-2 );
				Spinner->Location = TPoint( Inspector->Separator+1, Top+1 );
				Spinner->SetRange( SPINNER_FLOAT_MIN, SPINNER_FLOAT_MAX, SPINNER_FLOAT_SCALE );
				Spinner->EventChange = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::SomethingChange );
				if( bMatched )
					Spinner->SetValue(*(Float*)GetAddress(0), false);
				break;
			}
			case TYPE_String:
			{
				// String value.
				Edit		= new WEdit( Inspector, Inspector->Root );
				Edit->SetSize( Inspector->Size.Width-Inspector->Separator-14, INSPECTOR_ITEM_HEIGHT-2 );
				Edit->Location = TPoint( Inspector->Separator+1, Top+1 );
				Edit->EditType = EDIT_String;
				Edit->EventChange = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::SomethingChange );
				if( bMatched )
					Edit->SetText( *(String*)GetAddress(0), false );
				break;
			}
			case TYPE_Integer:
			{
				// Integer value.
				Spinner		= new WSpinner( Inspector, Inspector->Root );
				Spinner->SetSize( Inspector->Size.Width-Inspector->Separator-14, INSPECTOR_ITEM_HEIGHT-2 );
				Spinner->Location = TPoint( Inspector->Separator+1, Top+1 );
				Spinner->SetRange( SPINNER_INTEGER_MIN, SPINNER_INTEGER_MAX, SPINNER_INTEGER_SCALE );
				Spinner->EventChange = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::SomethingChange );
				if( bMatched )
					Spinner->SetValue(*(Int32*)GetAddress(0), false);
				break;
			}
			case TYPE_Angle:
			{
				// Angle value.
				Spinner		= new WSpinner( Inspector, Inspector->Root );
				Spinner->SetSize( Inspector->Size.Width-Inspector->Separator-14, INSPECTOR_ITEM_HEIGHT-2 );
				Spinner->Location = TPoint( Inspector->Separator+1, Top+1 );
				Spinner->SetRange( 0.f, 360.f, 0.1f );
				Spinner->EventChange = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::SomethingChange );
				if( bMatched )
					Spinner->SetValue((*(TAngle*)GetAddress(0)).ToDegs(), false);
				break;
			}
			case TYPE_Resource:
			{
				// Resource value.
				Edit		= new WEdit( Inspector, Inspector->Root );
				Edit->SetSize( Inspector->Size.Width-Inspector->Separator-14, INSPECTOR_ITEM_HEIGHT-2 );
				Edit->Location = TPoint( Inspector->Separator+1, Top+1 );
				Edit->EditType = EDIT_String;
				Edit->EventChange = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::SomethingChange );
				if( bMatched )
					Edit->SetText( TypeInfo.ToString(GetAddress(0)) );
				break;
			}
			case TYPE_Entity:
			{
				// Entity value, available only for pure entity, not
				// scripts.
				if( !Inspector->Objects[0]->IsA(FEntity::MetaClass) )
					break;

				// Entity pick button.
				Button			= new WButton( Inspector, Inspector->Root );
				Button->SetSize( 50, INSPECTOR_ITEM_HEIGHT-1 );
				Button->Location = TPoint( Inspector->Size.Width - 11 - Button->Size.Width, Top+0 );
				Button->EventClick	= TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::PickClick );
				Button->Caption		= L"Pick";

				// Entity edit.
				Edit		= new WEdit( Inspector, Inspector->Root );
				Edit->SetSize( Inspector->Size.Width-Inspector->Separator-12 - Button->Size.Width, INSPECTOR_ITEM_HEIGHT-2 );
				Edit->Location = TPoint( Inspector->Separator+1, Top+1 );
				Edit->EditType = EDIT_String;
				Edit->EventChange = TNotifyEvent( Inspector, (TNotifyEvent::TEvent)&WObjectInspector::SomethingChange );
				if( bMatched )
					Edit->SetText( TypeInfo.ToString(GetAddress(0)) );
				break;
			}
			default:
				fatal( L"Unsupported property type %d", (UInt8)TypeInfo.Type );
		}
	}
}


//
// Draw a property item.
//
void CPropertyItem::Paint( TPoint Base, CGUIRenderBase* Render )
{
	// Don't draw invisible.
	if( bHidden )
		return;
	/*
	// Draw background.
	Render->DrawRegion
				( 
					TPoint( Base.X+15, Base.Y+Top ), 
					TSize( Inspector->Size.Width-27, INSPECTOR_ITEM_HEIGHT ), 
					Inspector->WaitItem == this ?	COLOR_INSPECTOR_ITEM_WAIT : 
													Inspector->Selected == this ?	COLOR_INSPECTOR_ITEM_SEL :  
																					COLOR_INSPECTOR_ITEM, 
					COLOR_INSPECTOR_ITEM_BORDER, 
					BPAT_Solid 
				);
	*/

	// Draw background.
	Render->DrawRegion
				( 
					TPoint( Base.X+15, Base.Y+Top ), 
					TSize( Inspector->Size.Width-26, INSPECTOR_ITEM_HEIGHT ), 
					Inspector->WaitItem == this ?	COLOR_INSPECTOR_ITEM_WAIT : 
					Inspector->Selected == this ?	COLOR_INSPECTOR_ITEM_SEL :  
					Flags & PROP_Editable ?			COLOR_INSPECTOR_ITEM :
													COLOR_INSPECTOR_ITEM_NOEDIT,								 
					COLOR_INSPECTOR_ITEM_BORDER, 
					BPAT_Solid 
				);

	// Separator.
	Render->DrawRegion
				( 
					TPoint( Base.X + Inspector->Separator, Base.Y+Top ), 
					TSize( 0, INSPECTOR_ITEM_HEIGHT ),		
					COLOR_INSPECTOR_ITEM_BORDER, 
					COLOR_INSPECTOR_ITEM_BORDER, 
					BPAT_None 
				);

	// Sign.
	if( bExpandable )
		Render->DrawPicture
					( 
						TPoint( Base.X + 18 + Depth*20, Base.Y + Top + 6 ), 
						TSize( 9, 9 ), 
						TPoint( bExpanded ? 30 : 21, 0 ), 
						TSize( 9, 9 ), 
						Inspector->Root->Icons 
					);

	// Property name.
	Render->DrawText
				( 
					TPoint( Base.X + 30 + Depth*20, Base.Y+Top+3  ), 
					Caption, 
					GUI_COLOR_TEXT,  
					Inspector->Root->Font1 
				);

	// Draw property value, only
	// if not selected.
	if( Inspector->Selected != this )
	{
		String Value;

		if( TypeInfo.Type == TYPE_AABB )
		{
			// AABB value.
			Value	= L"(...)";
		}
		else if( TypeInfo.ArrayDim == 1 )
		{
			// Pick value from first.
			Value = TypeInfo.ToString( GetAddress(0) );

			// Check with others, if mismatched, nothing to put.
			for( Int32 i=1; i<Objects.Num(); i++ )
				if( !TypeInfo.CompareValue( GetAddress(0), GetAddress(i) ) )
				{
					// Mismatched.
					Value = L"";
					break;
				}
		}
		else
		{
			// Array value.
			Value = L"[...]";
		}

		// Put it!
		Render->DrawText
					( 
						TPoint( Base.X + Inspector->Separator + 5, Base.Y + Top + 4 ),  
						Value, 
						GUI_COLOR_TEXT, 
						Inspector->Root->Font1  
					);

		// Draw color rect.
		if( TypeInfo.Type == TYPE_Color && TypeInfo.ArrayDim==1 && Value )
			Render->DrawRegion
						( 
							TPoint( Base.X + Inspector->Size.Width - 34, Top + Base.Y + 2 ),
							TSize( 20, INSPECTOR_ITEM_HEIGHT-4 ),
							*(TColor*)GetAddress(0),
							COLOR_Black,
							BPAT_Solid
						);
	}
}


//
// Called when item lost focus.
//
void CPropertyItem::Unselect()
{
	freeandnil( Spinner );
	freeandnil( Edit );
	freeandnil( Button );
	freeandnil( ComboBox );

	freeandnil( AddButton );
	freeandnil( RemoveButton );
}


//
// On dynamic array add item click.
//
void CPropertyItem::OnButtonAddClick( WWidget* Sender )
{
	assert(TypeInfo.ArrayDim == -1);

	TArrayBase* Array = (TArrayBase*)GetAddress(0);
	Int32 i = Array->Count + 1;
	if( i >= DYNAMIC_ARRAY_MAX )
	{
		Inspector->Root->ShowMessage( L"Dynamic array length exceed maximum", L"Object Inspector" );
		return;
	}

	TypeInfo.SetArrayLength( Array, i );
	RebuildDynamicArrayList();
}


//
// On dynamic array remove item click.
//
void CPropertyItem::OnButtonRemoveClick( WWidget* Sender )
{
	assert(TypeInfo.ArrayDim == -1);

	TArrayBase* Array = (TArrayBase*)GetAddress(0);
	if( Array->Count == 0 )
	{
		Inspector->Root->ShowMessage( L"Dynamic array is devastated", L"Object Inspector" );
		return;
	}
	 
	Int32 i = Array->Count - 1;
	TypeInfo.SetArrayLength( Array, i );
	RebuildDynamicArrayList();
}


//
// Mouse up on this item.
//
void CPropertyItem::MouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	Inspector->bMoveSep	= false;
}


//
// Mouse hover item.
//
void CPropertyItem::MouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	if( Inspector->bMoveSep )
	{
		// Move separator.
		Inspector->Separator	= X;
		Inspector->Separator	= Clamp( Inspector->Separator, 100, Inspector->Size.Width-100 );
		
		Inspector->UnselectAll();
	}

	// Change cursor style.
	Inspector->Cursor = IsAtSeparator(X) ? CR_HSplit : CR_Arrow;
}


//
// Return true if cursor is near separator.
//
Bool CPropertyItem::IsAtSeparator( Int32 X ) const
{
	return Abs(Inspector->Separator - X) < 5;
}


/*-----------------------------------------------------------------------------
    CComponentItem.
-----------------------------------------------------------------------------*/

//
// A component item.
//
class CComponentItem: public CInspectorItemBase
{
public:
	// CComponentItem interface.
	CComponentItem( WObjectInspector* InInspector, const TArray<FObject*>& InObjs );
	~CComponentItem();
	Bool IsAtSign( Int32 X ) const;

	// Events from Object Inspector.
	void Paint( TPoint Base, CGUIRenderBase* Render );
	void MouseDown( EMouseButton Button, Int32 X, Int32 Y );
};


//
// Component item constructor.
//
CComponentItem::CComponentItem( WObjectInspector* InInspector, const TArray<FObject*>& InObjs )
	:	CInspectorItemBase( InInspector, 0 )
{
	// Add to inspector.
	Inspector->Children.Push( this );
	Objects = InObjs;
	Caption	= Objects[0]->GetName();

	// Check for problems.
	for( Int32 i=1; i<Objects.Num(); i++ )
		assert(Objects[i]->GetClass() == Objects[0]->GetClass());

	// Add all children.
	Bool bScript = Inspector->Objects[0]->IsA(FScript::MetaClass);
	for( CClass* C = Objects[0]->GetClass(); C; C = C->Super )
	{
		for( Int32 iProp=0; iProp<C->Properties.Num(); iProp++ )
		{
			CProperty* Prop = C->Properties[iProp];

			if( Prop->Flags & PROP_Editable || bScript )
				Children.Push( new CPropertyItem( Inspector, 1, Objects, bScript ? Prop->Name : Prop->GetAliasName(), *Prop, Prop->Offset, Prop->Flags ) );
		}
	}

	// Collapse by default.
	bExpanded = true;
	ExpandAll();
}


//
// Component item destructor.
//
CComponentItem::~CComponentItem()
{
}


//
// Draw component item. 
//
void CComponentItem::Paint( TPoint Base, CGUIRenderBase* Render )
{
	// Draw background.
	Render->DrawRegion
				( 
					TPoint( Base.X+15, Base.Y+Top ), 
					TSize( Inspector->Size.Width-26, INSPECTOR_ITEM_HEIGHT ), 
					COLOR_INSPECTOR_ITEM_COMPON,
					COLOR_INSPECTOR_ITEM_BORDER, 
					BPAT_Solid 
				);

	// Sign.
	Render->DrawPicture
				( 
					TPoint( Base.X + 3, Base.Y + Top + 6 ), 
					TSize( 9, 9 ), 
					TPoint( bExpanded ? 30 : 21, 0 ), 
					TSize( 9, 9 ), 
					Inspector->Root->Icons 
				);

	// Component name.
	Render->DrawText
				( 
					TPoint( Base.X + 30, Base.Y+Top+3  ), 
					Caption, 
					GUI_COLOR_TEXT,  
					Inspector->Root->Font1 
				);
}


//
// User has clicked item.
//
void CComponentItem::MouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	if( IsAtSign(X) )
	{
		// Toggle show children.
		if( bExpanded )
			CollapseAll();
		else
			ExpandAll();

		Inspector->UnselectAll();
	}
}


//
// Return true, if X is near component sign.
//
Bool CComponentItem::IsAtSign( Int32 X ) const
{
	return X < 15;
}


/*-----------------------------------------------------------------------------
    WObjectInspector implementation.
-----------------------------------------------------------------------------*/

//
// Object inspector constructor.
//
WObjectInspector::WObjectInspector( WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		Separator( 130 ),
		bMoveSep( false ),
		Children(),
		Objects(),
		Selected( nullptr ),
		TopClass( FObject::MetaClass ),
		bWaitForPick( false ),
		WaitItem( nullptr )
{
	// Allocate scrollbar.
	ScrollBar		= new WSlider( this, InRoot );
	ScrollBar->SetOrientation( SLIDER_Vertical );
	ScrollBar->EventChange	= TNotifyEvent( this, (TNotifyEvent::TEvent)&WObjectInspector::ScrollChange );
	ScrollBar->SetSize( 12, 50 );
	ScrollBar->Align	= AL_Right;

	// Set own default size.
	SetSize( 300, 300 );
	Padding	= TArea( INSPECTOR_HEADER_SIZE, 0, 0, 0 );

	Caption	= L"Inspector";
}


//
// Object inspector destructor.
//
WObjectInspector::~WObjectInspector()
{
	// Don't destroy scrollbar, 
	// WContainer do this job.

	// Destroy all children.
	for( Int32 i=0; i<Children.Num(); i++ )
		delete Children[i];
}


//
// Empty inspector.
//
void WObjectInspector::Empty()
{
	ScrollBar->Value	= 0;
	Selected			= nullptr;
	WaitItem			= nullptr;
	UnselectAll();
	for( Int32 i=0; i<Children.Num(); i++ )
		delete Children[i];
	Children.Empty();
	Objects.Empty();
	CustomHandlers.Empty();
}


//
// Set an objects list for edit.
//
void WObjectInspector::SetEditObjects( TArray<FObject*>& Objs )
{
	// Cleanup old stuff.
	Empty();

	if( Objs.Num() > 0 )
	{
		// Maybe current page is level edit and selected entity, so
		// we should use transactor here.
		if	( 
				GEditor->GetActivePage() && 
				GEditor->GetActivePage()->PageType == PAGE_Level &&
				Objs[0]->IsA(FEntity::MetaClass)
			)
			LevelPage	= (WLevelPage*)GEditor->GetActivePage();
		else
			LevelPage	= nullptr;


		if( Objs[0]->IsA(FEntity::MetaClass) )				
		{
			// Its a list of entities.
			FEntity*	First = (FEntity*)Objs[0];
			FScript*	Script = First->Script;
			TArray<FBaseComponent*>	Bases;
			Bases.Push( First->Base );
			TopClass	= First->Base->GetClass();

			// Add similar objects to edit.
			for( Int32 i=1; i<Objs.Num(); i++ )
			{
				assert(Objs[i]->IsA(FEntity::MetaClass));
				FEntity* Other = (FEntity*)Objs[i];

				if( Other->Script == Script )
				{
					assert(Other->Base->GetClass() == TopClass);
					Bases.Push( Other->Base );
				}
			}

			// Store list of source objects, doesn't full copy,
			// since some input objects are rejected due script.
			for( Int32 i=0; i<Bases.Num(); i++ )
				Objects.Push(Bases[i]->Entity);

			// Own properties is owned by Base.
			for( CClass* C = First->Base->GetClass(); C; C = C->Super )
			{
				for( Int32 iProp=0; iProp<C->Properties.Num(); iProp++ )
				{
					CProperty* Prop = C->Properties[iProp];

					// Property item will add to list its self, also
					// add children and hide them as default.
					if( Prop->Flags & PROP_Editable )
						new CPropertyItem( this, 0, *(TArray<FObject*>*)&Bases, Prop->GetAliasName(), *Prop, Prop->Offset, Prop->Flags );
				}
			}

			// InstanceBuffer properties.
			for( Int32 i=0; i<First->Script->Properties.Num(); i++ )
			{
				CProperty*Prop = First->Script->Properties[i];

				if( Prop->Flags & PROP_Editable )
					new CPropertyItem( this, 0, *(TArray<FObject*>*)&Objects, Prop->GetAliasName(), *Prop, Prop->Offset, Prop->Flags );
			}

			// Add component items.
			for( Int32 i=0; i<Bases[0]->Entity->Components.Num(); i++  )
			{
				TArray<FExtraComponent*> Comps;

				// Collect same components from different entities.
				for( Int32 iOwner=0; iOwner<Bases.Num(); iOwner++ )
					Comps.Push( Bases[iOwner]->Entity->Components[i] );

				CComponentItem* Item =  new CComponentItem( this, *(TArray<FObject*>*)&Comps );

				// Don't add empty component item.
				if( Item->Children.Num() == 0 )
				{
					Children.RemoveShift(Children.FindItem(Item));
					delete Item;
				}
			}

			// Set caption.
			Caption	= String::Format( L"Inspector [%s]", Objects.Num()==1 ? *Bases[0]->Entity->GetName() : *Bases[0]->Entity->Script->GetName() );
		}
		else if( Objs[0]->IsA(FScript::MetaClass) )
		{
			// Script resource.
			assert(Objs.Num() == 1);
			FScript* Script = (FScript*)Objs[0];
			TArray<FComponent*> TmpArr;
			TmpArr.SetNum( 1 );

			for( Int32 i=0; i<Objs.Num(); i++ )
				Objects.Push(Objs[i]);

			TopClass	= Objects[0]->GetClass();

			// FScript properties.
			for( CClass* C = TopClass; C; C = C->Super )
			{
				for( Int32 iProp=0; iProp<C->Properties.Num(); iProp++ )
				{
					CProperty* Prop = C->Properties[iProp];

					// Property item will add to list its self, also
					// add children and hide them as default.
					new CPropertyItem( this, 0, Objects, Prop->Name, *Prop, Prop->Offset, Prop->Flags );
				}
			}

			// Base component properties.
			if( !Script->IsStatic() )
			{
				for( CClass* C = Script->Base->GetClass(); C; C = C->Super )
				{
					for( Int32 iProp=0; iProp<C->Properties.Num(); iProp++ )
					{
						CProperty* Prop = C->Properties[iProp];

						// Property item will add to list its self, also
						// add children and hide them as default.
						TmpArr[0] = Script->Base;
						new CPropertyItem( this, 0, *(TArray<FObject*>*)&TmpArr, Prop->Name, *Prop, Prop->Offset, Prop->Flags );
					}
				}

				// InstanceBuffer properties.
				for( Int32 i=0; i<Script->Properties.Num(); i++ )
				{
					CProperty* Prop = Script->Properties[i];

					if( Prop->Flags & PROP_Editable )
						new CPropertyItem( this, 0, *(TArray<FObject*>*)&Objects, Prop->Name, *Prop, Prop->Offset, Prop->Flags );
				}

				// Components.
				for( Int32 i=0; i<Script->Components.Num(); i++ )
				{
					TmpArr[0] = Script->Components[i];
					CComponentItem* Item = new CComponentItem( this, *(TArray<FObject*>*)&TmpArr );

					// Don't add empty component item.
					if( Item->Children.Num() == 0 )
					{
						Children.RemoveShift(Children.FindItem(Item));
						delete Item;
					}
				}
			}

			// Set caption.
			Caption	= String::Format( L"Inspector [%s: %s]", *TopClass->GetAltName(), *Objects[0]->GetName() );
		}
		else if( Objs[0]->IsA(FResource::MetaClass) )
		{
			// Its a list of resources.
			assert(Objs.Num() == 1);	

			for( Int32 i=0; i<Objs.Num(); i++ )
				Objects.Push( Objs[i] );

			TopClass	= Objects[0]->GetClass();

			// Own properties.
			for( CClass* C = TopClass; C; C = C->Super )
			{
				for( Int32 iProp=0; iProp<C->Properties.Num(); iProp++ )
				{
					CProperty* Prop = C->Properties[iProp];

					// Property item will add to list its self, also
					// add children and hide them as default.
					new CPropertyItem( this, 0, Objects, Prop->GetAliasName(), *Prop, Prop->Offset, Prop->Flags );
				}
			}

			// Set caption.
			Caption	= String::Format( L"Inspector [%s: %s]", *TopClass->GetAltName(), *Objects[0]->GetName() );
		}
		else if( Objs[0]->IsA(FModifier::MetaClass) )
		{
			// Its a list of modifiers.
			assert(Objs.Num() == 1);

			for( Int32 i=0; i<Objs.Num(); i++ )
				Objects.Push( Objs[i] );

			TopClass	= Objects[0]->GetClass();

			// Own properties.
			for( CClass* C = TopClass; C; C = C->Super )
			{
				for( Int32 iProp=0; iProp<C->Properties.Num(); iProp++ )
				{
					CProperty* Prop = C->Properties[iProp];

					// Property item will add to list its self, also
					// add children and hide them as default.
					new CPropertyItem( this, 0, Objects, Prop->GetAliasName(), *Prop, Prop->Offset, Prop->Flags );
				}
			}
			
			// Set caption.
			Caption	= String::Format( L"Inspector [%s: %s]", *TopClass->GetAltName(), *Objects[0]->GetName() );	
		}
		else
		{
			// Something unsupported.
			fatal( L"Unsupported class \"%s\" to edit.", Objs[0]->GetClass()->Name );
		}
	}
	else
	{
		// No objects to edit.
		// Object inspector will blank.
		Caption	= L"Inspector";
	}

	// Organize the items.
	UpdateChildren();
}


//
// User drag something, delegate decision to the item.
//
void WObjectInspector::OnDragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept )
{
	bAccept	= false;
	if( !bWaitForPick && Y > INSPECTOR_HEADER_SIZE )
	{
		Int32 NewY;
		CInspectorItemBase* Item = GetItemAt( Y, NewY );

		if( Item )
			Item->DragOver( Data, X, Y, bAccept );
	}
}


//
// User drop something, delegate decision to the item.
// Lets item deal with it :3
//
void WObjectInspector::OnDragDrop( void* Data, Int32 X, Int32 Y )
{
	if( !bWaitForPick && Y > INSPECTOR_HEADER_SIZE )
	{
		Int32 NewY;
		CInspectorItemBase* Item = GetItemAt( Y, NewY );

		if( Item )
			Item->DragDrop( Data, X, Y );
	}
}


//
// Entity has been picked.
//
void WObjectInspector::ObjectPicked( FEntity* Picked )
{
	if( !bWaitForPick || !WaitItem )
		return;

	// Compare scripts.
	CPropertyItem* Waiter = (CPropertyItem*)WaitItem;
	if( Picked && Waiter->TypeInfo.Script && Picked->Script != Waiter->TypeInfo.Script )
		Picked	= nullptr;

	// Set value.
	if( LevelPage )	LevelPage->Transactor->TrackEnter();
	{
		for( Int32 i=0; i<WaitItem->Objects.Num(); i++ )
			*(FEntity**)((CPropertyItem*)WaitItem)->GetAddress(i) = Picked;
	}
	if( LevelPage )	LevelPage->Transactor->TrackLeave();

	// Notify.
	for( Int32 i=0; i<WaitItem->Objects.Num(); i++ )
		WaitItem->Objects[i]->EditChange();
	for( Int32 i=0; i<CustomHandlers.Num(); i++ )
		CustomHandlers[i]( this );

	// Reset it.
	bWaitForPick		= false;
	WaitColor			= nullptr;
	WaitItem			= nullptr;
	ScrollBar->bEnabled	= true;
	UnselectAll();
}


//
// Begin wait for pick.
//
void WObjectInspector::BeginWaitForPick( CInspectorItemBase* Waiter )
{
	assert(!bWaitForPick)

	WEditorPage* Page = (WEditorPage*)GEditor->EditorPages->GetActivePage();
	if( Page && Page->PageType == PAGE_Level )
	{
		WLevelPage* LevPag = (WLevelPage*)Page;

		bWaitForPick		= true;
		WaitItem			= Waiter;
		ScrollBar->bEnabled	= false;

		LevPag->SetTool( LEV_PickEntity );
	}
}


//
// Recompute offset for each children.
//
void WObjectInspector::UpdateChildren()
{
	// Count non hidden, required for scroll.
	Int32 NonHidden = 0;
	for( Int32 i=0; i<Children.Num(); i++ )
		if( !Children[i]->bHidden )
			NonHidden++;

	// Compute scrolling.
	Int32 NumVisible		= Floor( (Float)(Size.Height-Padding.Top)  / (Float)INSPECTOR_ITEM_HEIGHT );
	Int32 NumInvis		= Max( NonHidden-NumVisible, 0);
	Int32 Scroll		= (NumInvis * ScrollBar->Value * INSPECTOR_ITEM_HEIGHT) / 100;

	Int32 WalkY	= INSPECTOR_HEADER_SIZE + 1 - Scroll;

	for( Int32 i=0; i<Children.Num(); i++ )
	{
		CInspectorItemBase* Item	= Children[i];

		if( !Item->bHidden )
		{
			// Item is visible.
			Item->Top	= WalkY;
			WalkY		+= INSPECTOR_ITEM_HEIGHT-1;
		}
		else
		{
			// Item  is invisible.
			Item->Top	= 0;
		}
	}
}


//
// Return item at specified location, if no item found here
// return nullptr, LocalY is an Y value in items local coords.
//
CInspectorItemBase* WObjectInspector::GetItemAt( Int32 ParentY, Int32& LocalY )
{ 
	for( Int32 i=0; i<Children.Num(); i++ )
	{
		if( Children[i]->bHidden ) 
			continue;

		Int32 Y1 = Children[i]->Top;
		Int32 Y2 = Y1 + INSPECTOR_ITEM_HEIGHT;

		if( ParentY >= Y1 && ParentY <= Y2 )
		{
			// Found.
			LocalY	= ParentY - Y1;
			return Children[i];
		}
	}

	// Nothing found.
	return nullptr;
}


//
// Mouse move on inspector.
//
void WObjectInspector::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	if( !bWaitForPick && Y > INSPECTOR_HEADER_SIZE )
	{
		Int32 NewY;
		CInspectorItemBase* Item = GetItemAt( Y, NewY );

		if( Item )
			Item->MouseMove( Button, X, NewY );	
	}
}


//
// Mouse down on inspector.
//
void WObjectInspector::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	if( !bWaitForPick && Y > INSPECTOR_HEADER_SIZE )
	{
		Int32 NewY;
		CInspectorItemBase* Item = GetItemAt( Y, NewY );

		if( Item )
			Item->MouseDown( Button, X, NewY );	
	}
}


//
// Mouse up on inspector.
//
void WObjectInspector::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	if( !bWaitForPick && Y > INSPECTOR_HEADER_SIZE )
	{
		Int32 NewY;
		CInspectorItemBase* Item = GetItemAt( Y, NewY );

		if( Item )
			Item->MouseUp( Button, X, NewY );	
	}
}


//
// Scroll inspector via mouse wheel.
//
void WObjectInspector::OnMouseScroll( Int32 Delta )
{
	ScrollBar->Value	= Clamp
							( 
								ScrollBar->Value-Delta/40, 
								0, 
								100 
							);
	ScrollChange( this );
}


//
// Draw inspector and its children.
//
void WObjectInspector::OnPaint( CGUIRenderBase* Render )
{
	WContainer::OnPaint( Render );

	TPoint Base = ClientToWindow(TPoint::Zero);

	// Draw frame.
	Render->DrawRegion
				(
					Base,
					Size,
					COLOR_INSPECTOR_BACKDROP,
					COLOR_INSPECTOR_BORDER,
					BPAT_Solid
				);
	
	// Draw header.
	Render->DrawRegion
				(
					TPoint( Base.X+1, Base.Y ),
					TSize( Size.Width-2, INSPECTOR_HEADER_SIZE ),
					GUI_COLOR_FORM_HEADER,//TColor( 0x33, 0x33, 0x33, 0xff ),
					COLOR_INSPECTOR_BORDER,
					BPAT_Diagonal

				);
	Render->DrawText
				( 
					TPoint( Base.X + 5, Base.Y+(INSPECTOR_HEADER_SIZE-Root->Font1->Height)/2 ), 
					Caption, 
					GUI_COLOR_TEXT, 
					Root->Font1 
				);

	// Draw items.
	Render->SetClipArea( Base + TPoint( 0, Padding.Top+2 ), TSize( Size.Width-12, Size.Height-Padding.Top-3 ) );
	for( Int32 i=0; i<Children.Num(); i++ )
	{
		CInspectorItemBase* Item = Children[i];

		if	(	!Item->bHidden && 
				( Item->Top + INSPECTOR_ITEM_HEIGHT ) > INSPECTOR_HEADER_SIZE 
			)
		{
			Item->Paint( Base, Render );
		}

		// End of visible.
		if( Item->Top > Size.Height )
			break;
	}
}


//
// When inspector scrolled.
//
void WObjectInspector::ScrollChange( WWidget* Sender )
{
	UnselectAll();
	UpdateChildren();
}


//
// Some of the item widget changed.
//
void WObjectInspector::SomethingChange( WWidget* Sender )
{
	//!! not really good solution :(
	assert(Selected);
	((CPropertyItem*)Selected)->OnSomethingChange( Sender );
}


//
// Pick button has been clicked.
//
void WObjectInspector::PickClick( WWidget* Sender )
{
	//!! not really good solution :(
	assert(Selected);
	((CPropertyItem*)Selected)->OnPickClick( Sender );
}


//
// Add item to dynamic array clicked.
//
void WObjectInspector::DynArrayAddClick( WWidget* Sender )
{
	assert(GDynArrayItem);
	GDynArrayItem->OnButtonAddClick( Sender );
}


//
// Remove item from dynamic array clicked.
//
void WObjectInspector::DynArrayRemoveClick( WWidget* Sender )
{
	assert(GDynArrayItem);
	GDynArrayItem->OnButtonRemoveClick( Sender );
}


//
// User just pick color.
//
void WObjectInspector::ColorSelected( WWidget* Sender )
{
	assert(WaitColor);
	CPropertyItem* Item = (CPropertyItem*)WaitColor;

	// Set new color for all objects.
	if( LevelPage )	LevelPage->Transactor->TrackEnter();
	{
		if( Item->CustomAddr )
			*(TColor*)Item->CustomAddr	= WColorChooser::SharedColor;

		for( Int32 i=0; i<Item->Objects.Num(); i++ )
			*(TColor*)Item->GetAddress(i)	= WColorChooser::SharedColor;
	}
	if( LevelPage )	LevelPage->Transactor->TrackLeave();

	WaitColor	= nullptr;
}


//
// Inspector has been resized.
//
void WObjectInspector::OnResize()
{
	// Clamp separator.
	Separator	= Clamp( Separator, 100, Size.Width-100 );

	// Reset focus.
	UnselectAll();

	// Update scroll.
	UpdateChildren();
}


//
// Unselect all items.
//
void WObjectInspector::UnselectAll()
{
	for( Int32 i=0; i<Children.Num(); i++ )
		Children[i]->Unselect();

	Selected	= nullptr;
}


//
// Set custom inspector caption.
//
void WObjectInspector::SetCustomCaption( String NewCaption )
{
	Caption = NewCaption;
}


//
// Add a new custom property to inspector.
//
void WObjectInspector::AddCustomProperty( String PropName, const CTypeInfo& Type, void* Addr )
{
	new CPropertyItem( this, 0, (UInt8*)Addr, PropName, Type, PROP_Editable );
	UpdateChildren();
}


//
// Set an object to edit.
//
void WObjectInspector::SetEditObject( FObject* Obj )
{
	assert(Obj);
	TArray<FObject*> Tmp;
	Tmp.Push(Obj);
	SetEditObjects( Tmp );
}


//
// Count all references in inspector.
//
void WObjectInspector::CountRefs( CSerializer& S )
{
	Serialize( S, Objects );
	CLEANUP_ARR_NULL(Objects);

	// Also cleanup in children.
	for( Int32 i=0; i<Children.Num(); i++ )
	{
		CInspectorItemBase* Item = Children[i];

		Serialize( S, Item->Objects );
		CLEANUP_ARR_NULL(Item->Objects);
	}

	// Maybe cleanup all.
	if( Objects.Num() == 0 )
	{
		TArray<FObject*> Tmp;
		SetEditObjects( Tmp );
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/