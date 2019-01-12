/*=============================================================================
    FrExpImp.h: Exporter & Importer abstract classes.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CExporterBase.
-----------------------------------------------------------------------------*/

//
// An abstract exporter class.
//
class CExporterBase
{
public:
	// CExporterBase interface.
	virtual void ExportByte		( const Char* FieldName, UInt8 Value	) = 0;
	virtual void ExportInteger	( const Char* FieldName, Int32 Value	) = 0;
	virtual void ExportFloat	( const Char* FieldName, Float Value	) = 0;
	virtual void ExportString	( const Char* FieldName, String Value	) = 0;
	virtual void ExportBool		( const Char* FieldName, Bool Value		) = 0;
	virtual void ExportColor	( const Char* FieldName, TColor Value	) = 0;
	virtual void ExportVector	( const Char* FieldName, TVector Value	) = 0;
	virtual void ExportAABB		( const Char* FieldName, TRect Value	) = 0;
	virtual void ExportAngle	( const Char* FieldName, TAngle Value	) = 0;
	virtual void ExportObject	( const Char* FieldName, FObject* Value	) = 0;
};


/*-----------------------------------------------------------------------------
    CImporterBase.
-----------------------------------------------------------------------------*/

//
// An abstract importer class.
//
class CImporterBase
{
public:
	// CImporterBase interface.
	virtual UInt8		ImportByte		( const Char* FieldName ) = 0;
	virtual Int32		ImportInteger	( const Char* FieldName ) = 0;
	virtual Float		ImportFloat		( const Char* FieldName ) = 0;
	virtual String		ImportString	( const Char* FieldName ) = 0;
	virtual Bool		ImportBool		( const Char* FieldName ) = 0;
	virtual TColor		ImportColor		( const Char* FieldName ) = 0;
	virtual TVector		ImportVector	( const Char* FieldName ) = 0;
	virtual TRect		ImportAABB		( const Char* FieldName ) = 0;
	virtual TAngle		ImportAngle		( const Char* FieldName ) = 0;
	virtual FObject*	ImportObject	( const Char* FieldName ) = 0;
};


/*-----------------------------------------------------------------------------
    Macro.
-----------------------------------------------------------------------------*/

#define IMPORT_BYTE(name)		*(UInt8*)&name = Im.ImportByte( L#name );
#define IMPORT_INTEGER(name)	name = Im.ImportInteger( L#name );
#define IMPORT_FLOAT(name)		name = Im.ImportFloat( L#name );
#define IMPORT_STRING(name)		name = Im.ImportString( L#name );
#define IMPORT_BOOL(name)		name = Im.ImportBool( L#name );
#define IMPORT_COLOR(name)		name = Im.ImportColor( L#name );
#define IMPORT_VECTOR(name)		name = Im.ImportVector( L#name );
#define IMPORT_AABB(name)		name = Im.ImportAABB( L#name );
#define IMPORT_ANGLE(name)		name = Im.ImportAngle( L#name );
#define IMPORT_OBJECT(name)		*(FObject**)&name = Im.ImportObject( L#name );


#define EXPORT_BYTE(name)		Ex.ExportByte( L#name, (UInt8)name );
#define EXPORT_INTEGER(name)	Ex.ExportInteger( L#name, name );
#define EXPORT_FLOAT(name)		Ex.ExportFloat( L#name, name );
#define EXPORT_STRING(name)		Ex.ExportString( L#name, name );
#define EXPORT_BOOL(name)		Ex.ExportBool( L#name, name );
#define EXPORT_COLOR(name)		Ex.ExportColor( L#name, name );
#define EXPORT_VECTOR(name)		Ex.ExportVector( L#name, name );
#define EXPORT_ANGLE(name)		Ex.ExportAngle( L#name, name );
#define EXPORT_AABB(name)		Ex.ExportAABB( L#name, name );
#define EXPORT_OBJECT(name)		Ex.ExportObject( L#name, (FObject*)name );


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/