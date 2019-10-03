/*=============================================================================
    FrProjFile.cpp: Project file functions.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    Project shutdown.
-----------------------------------------------------------------------------*/

//
// Close the project.
//
Bool CEditor::CloseProject( Bool bAsk )
{
	// Test the system.
	assert((Project == GProject) && (Project == GObjectDatabase));
	if( !Project )
		return false;

	if( bAsk )
	{
		int S = MessageBox
		( 
			hWnd, 
			*String::format( L"Save changes to project '%s'?", *Project->ProjName ), 
			L"Confirm", 
			MB_YESNOCANCEL | MB_ICONASTERISK | MB_TASKMODAL 
		);

		if( S == IDCANCEL )
			return false;

		if( S == IDYES )
		{
			if( !SaveProject() )
				return false;
		}
	}

	// Close all projects pages.
	for( Int32 i=0; i<EditorPages->Pages.size();  )
		if( ((WEditorPage*)EditorPages->Pages[i])->PageType != PAGE_Hello )
		{
			EditorPages->CloseTabPage( i, true );	
		}
		else
			i++;

	// Flash all resources.
	Flush();

	// Destroy the project.
	delete Project;
	GObjectDatabase = nullptr;
	GProject		= nullptr;
	Project			= nullptr;

	// Refresh an editor panels.
	Array<FObject*> EmptyArr;
	Inspector->SetEditObjects( EmptyArr );
	Browser->Refresh();

	// Update caption.
	SetCaption(L"Fluorine Engine");

	return true;
}


/*-----------------------------------------------------------------------------
    Project construction.
-----------------------------------------------------------------------------*/

//
// Create a new project.
//
Bool CEditor::NewProject()
{
	// Unload last project.
	if( Project && !CloseProject() )
		return false;

	// Allocate project and it objects.
	Project					= new CProject();
	Project->Info			= NewObject<FProjectInfo>();
	Project->BlockMan		= new CBlockManager();
	Project->FileName		= L"";
	Project->ProjName		= L"Unnamed";

	// Load default resources for every project.
	for( Int32 iDef=0; true; iDef++ )
	{
		String ResName = Config->ReadString
		( 
			L"Editor",
			L"Project", 
			*String::format( L"Res[%d]", iDef ) 
		);

		if( !ResName )
			break;

		PreloadResource( ResName );
	}

	// Update caption.
	SetCaption(String::format(L"%s - Fluorine Engine", *Project->ProjName));

	// Refresh an editor panels.
	Array<FObject*> EmptyArr;
	Inspector->SetEditObjects( EmptyArr );
	Browser->Refresh();

	return true;
}


/*-----------------------------------------------------------------------------
    Project saving.
-----------------------------------------------------------------------------*/

//
// Text exporter.
//
class CExporter: public CExporterBase
{
public:
	// Variables.
	CTextWriter&	Writer;
	Char			Whitespace[64];
	
	// Exporter constructor.
	CExporter( CTextWriter&	InWriter )
		:	Writer( InWriter )
	{}

	// Exporter destructor.
	~CExporter()
	{}

	// Set nest level to save data as a tree.
	void SetNestLevel( Int32 InNestLevel )
	{
		assert(InNestLevel*4 < 64);
		mem::zero( Whitespace, sizeof(Whitespace) );
		for( Int32 i=0; i<InNestLevel*4; i++ )
			Whitespace[i] = L' ';
	}

	// Byte export.
	void ExportByte( const Char* FieldName, UInt8 Value )
	{
		if( Value )
			Writer.WriteString(String::format
			( 
				L"%s%s = %d", 
				Whitespace, 
				FieldName, 
				Value 
			));
	}

	// Integer Export.
	void ExportInteger( const Char* FieldName, Int32 Value )
	{
		if( Value )
			Writer.WriteString(String::format
			( 
				L"%s%s = %d", 
				Whitespace, 
				FieldName, 
				Value 
			));
	}

	// Float export.
	void ExportFloat( const Char* FieldName, Float Value )
	{
		if( Value != 0.f )
			Writer.WriteString(String::format
			( 
				L"%s%s = %.4f", 
				Whitespace, 
				FieldName, 
				Value 
			));
	}

	// String export.
	void ExportString( const Char* FieldName, String Value )
	{
		if( Value )
			Writer.WriteString(String::format
			( 
				L"%s%s = \"%s\"", 
				Whitespace, 
				FieldName, 
				*Value 
			));
	}

	// Bool export.
	void ExportBool( const Char* FieldName, Bool Value ) 
	{
		if( Value )
			Writer.WriteString(String::format
			( 
				L"%s%s = %s", 
				Whitespace, 
				FieldName, 
				Value ? L"true" : L"false" 
			));
	}

	// Color export.
	void ExportColor( const Char* FieldName, math::Color Value ) 
	{
		if( Value != math::colors::BLACK )
			Writer.WriteString(String::format
			( 
				L"%s%s = #%02x%02x%02x%02x", 
				Whitespace, 
				FieldName, 
				Value.r, Value.g, Value.b, Value.a 
			));
	}

	// Vector export.
	void ExportVector( const Char* FieldName, math::Vector Value ) 
	{
		if( Value.x!=0.f || Value.y!=0.f )
			Writer.WriteString(String::format
			( 
				L"%s%s = [%.4f; %.4f]", 
				Whitespace, 
				FieldName, 
				Value.x, Value.y 
			));
	}

	// Rect export.
	void ExportAABB( const Char* FieldName, math::Rect Value ) 
	{
		Writer.WriteString(String::format
		( 
			L"%s%s = (%.4f; %.4f; %.4f; %.4f)", 
			Whitespace, 
			FieldName, 
			Value.min.x, Value.min.y, 
			Value.max.x, Value.max.y 
		));
	}

	// Angle export.
	void ExportAngle( const Char* FieldName, math::Angle Value ) 
	{
		if( Value )
			Writer.WriteString(String::format
			( 
				L"%s%s = %d", 
				Whitespace, 
				FieldName, 
				(Int32)Value 
			));
	}

	// Object export.
	void ExportObject( const Char* FieldName, FObject* Value ) 
	{
		if( Value )
			Writer.WriteString(String::format
			( 
				L"%s%s = %s::%s", 
				Whitespace, 
				FieldName, 
				*Value->GetClass()->Name, 
				*Value->GetFullName() 
			));
	}
};


//
// Single resource save.
//
void SaveResource( FResource* R, String Directory )	
{
	assert(R != nullptr);

	// Open file.
	CTextWriter Writer(String::format( L"%s\\Objects\\%s.fr", *Directory, *R->GetName()));
	CExporter Exporter(Writer);
	Exporter.SetNestLevel(0);

	// File header.
	Writer.WriteString( String::format( L"BEGIN_RESOURCE %s %s", *R->GetClass()->Name, *R->GetName() ) );
	{
		if( R->IsA(FLevel::MetaClass) )
		{
			//
			// Level.
			//
			FLevel* Level = (FLevel*)R;
			Exporter.SetNestLevel(1);
			Level->Export(Exporter);

			// Each entity.
			for( Int32 iEntity=0; iEntity<Level->Entities.size(); iEntity++ )
			{
				FEntity* Entity = Level->Entities[iEntity];
				Writer.WriteString( String::format
				( 
					L"    BEGIN_ENTITY %s %s", 
					*Entity->Script->GetName(), 
					*Entity->GetName() 
				));
				Exporter.SetNestLevel(2);
				Entity->Export(Exporter);
				
				// Entity's components.
				{
					Exporter.SetNestLevel(3);

					// Base
					Writer.WriteString( String::format
					( 
						L"        BEGIN_COMPONENT %s %s", 
						*Entity->Base->GetClass()->Name, 
						*Entity->Base->GetName() ) 
					);
					Entity->Base->Export( Exporter );
					Writer.WriteString( L"        END_COMPONENT" );

					// Extra.
					for( Int32 iCom=0; iCom<Entity->Components.size(); iCom++ )
					{
						FComponent* Component = Entity->Components[iCom];
						Writer.WriteString( String::format
						( 
							L"        BEGIN_COMPONENT %s %s", 
							*Component->GetClass()->Name, 
							*Component->GetName() ) 
						);
						Component->Export( Exporter );
						Writer.WriteString( L"        END_COMPONENT" );
					}
				}

				// Entity instance buffer.
				assert(!Entity->Script->IsStatic());
				if( Entity->Script->IsScriptable() )
				{
					Writer.WriteString( String::format( L"        BEGIN_INSTANCE" ) );
						Entity->InstanceBuffer->ExportValues( Exporter );
					Writer.WriteString( L"        END_INSTANCE" );
				}

				Writer.WriteString( L"    END_ENTITY" );
			}
		}
		else if( R->IsA(FScript::MetaClass) )
		{
			//
			// Script.
			//
			FScript* Script = (FScript*)R;
			Exporter.SetNestLevel(1);
			Script->Export(Exporter);

			// Script's components.
			{
				Exporter.SetNestLevel(3);

				if( !Script->IsStatic() )
				{
					// Base
					Writer.WriteString( String::format
					( 
						L"    BEGIN_COMPONENT %s %s", 
						*Script->Base->GetClass()->Name, 
						*Script->Base->GetName() ) 
					);
					Script->Base->Export(Exporter);
					Writer.WriteString( L"    END_COMPONENT" );

					// Extra.
					for( Int32 iCom=0; iCom<Script->Components.size(); iCom++ )
					{
						FComponent* Component = Script->Components[iCom];
						Writer.WriteString( String::format
						( 
							L"    BEGIN_COMPONENT %s %s", 
							*Component->GetClass()->Name, 
							*Component->GetName() ) 
						);
						Component->Export( Exporter );
						Writer.WriteString( L"    END_COMPONENT" );
					}
				}
			}

			// Instance buffer.
			if( Script->IsScriptable() && !Script->IsStatic() )
			{
				Writer.WriteString( String::format( L"    BEGIN_INSTANCE" ) );
				Script->InstanceBuffer->ExportValues( Exporter );
				Writer.WriteString( L"    END_INSTANCE" );
			}

			// Store script text.
			if( Script->IsScriptable() )
				GEditor->ExportResource( Script, Directory+L"\\Scripts", true );
		}
#if MATERIAL_ENABLED
		else if( R->IsA(FMaterial::MetaClass) )
		{
			//
			// Material.
			//
			FMaterial* Material = (FMaterial*)R;
			Exporter.SetNestLevel(1);
			Material->Export(Exporter);

			// Layers.
			Exporter.SetNestLevel(3);
			for( Int32 iLayer=0; iLayer<Material->Layers.size(); iLayer++ )
			{
				FMaterialLayer* Layer = Material->Layers[iLayer];

				Writer.WriteString( String::format
				( 
					L"    BEGIN_LAYER %s %s", 
					*Layer->GetClass()->Name, 
					*Layer->GetName() ) 
				);
				Layer->Export( Exporter );
				Writer.WriteString( L"    END_LAYER" );
			}
		}
#endif
		else
		{
			//
			// Resource.
			//
			Exporter.SetNestLevel(1);
			R->Export(Exporter);

			// Save also bitmaps and sounds.
			if( R->IsA(FBitmap::MetaClass) )
			{
				//FBitmap* Bitmap	= As<FBitmap>(R);
				//if( Bitmap->IsValidBlock() )
					//GEditor->ExportResource( R, Directory+L"\\Bitmaps", false );			
			}
			else if( R->IsA(FSound::MetaClass) )
				GEditor->ExportResource( R, Directory+L"\\Sounds", false );
		}
	}
	// File footer.
	Writer.WriteString( L"END_RESOURCE" );
}


//
// Save entire project.
//
Bool CEditor::SaveProject()
{
	if( !Project )
		return false;

	if( !Project->FileName )
	{
		// Project has no FileName, so Save it As.
		return SaveAsProject();
	}

	// Use dialog to show processing.
	IProgressIndicator::THolder Ind( TaskDialog, L"Project Saving" );

	String	Directory	= fm::getFilePath( *Project->FileName );

	// Create directories for imported resources.
	CreateDirectory( *(Directory+L"\\Objects"), nullptr );
	CreateDirectory( *(Directory+L"\\Bitmaps"), nullptr );
	CreateDirectory( *(Directory+L"\\Scripts"), nullptr );
	CreateDirectory( *(Directory+L"\\Sounds"), nullptr );
	CreateDirectory( *(Directory+L"\\Music"), nullptr );

	// Wrap list of files in block.
	JSon::Ptr projectFile = JSon::createObjectNode();

	JSon::Ptr includeList = JSon::createArrayNode();
	projectFile->addField( L"includes", includeList );
	{
		// Save all scripts.
		Ind.UpdateDetails(L"Saving Scripts");
		Ind.SetProgress( 1, 4 );
		for( Int32 i=0; i<Project->GObjects.size(); i++ )
			if( Project->GObjects[i] )
			{
				FObject* Obj = Project->GObjects[i];

				// Save to file.
				if( Obj->IsA(FScript::MetaClass) )
				{
					includeList->insertElement( JSon::createStringNode( Obj->GetName() + L".fr" ) );
					SaveResource( (FResource*)Obj, Directory ); 
				}
			}

		// Save all bitmaps, due thier nesting.
		Ind.UpdateDetails(L"Saving Bitmaps");
		Ind.SetProgress( 2, 4 );
		for( Int32 i=0; i<Project->GObjects.size(); i++ )
			if( Project->GObjects[i] )
			{
				FObject* Obj = Project->GObjects[i];

				// Save to file.
				if( Obj->IsA(FBitmap::MetaClass) )
				{
					includeList->insertElement( JSon::createStringNode( Obj->GetName() + L".fr" ) );
					SaveResource( (FResource*)Obj, Directory ); 
				}
			}

		// Save all resources, except scripts and bitmaps.
		Ind.UpdateDetails(L"Saving Resources");
		Ind.SetProgress( 3, 4 );
		for( Int32 i=0; i<Project->GObjects.size(); i++ )
			if( Project->GObjects[i] )
			{
				FObject* Obj = Project->GObjects[i];

				// Save to file.
				if	( 
						Obj->IsA(FResource::MetaClass) && 
						!Obj->IsA(FScript::MetaClass) && 
						!Obj->IsA(FBitmap::MetaClass) && 
						!(Obj->IsA(FLevel::MetaClass) && ((FLevel*)Obj)->IsTemporal()) 
					)
				{
					includeList->insertElement( JSon::createStringNode( Obj->GetName() + L".fr" ) );
					SaveResource( (FResource*)Obj, Directory ); 
				}
			}
	}

	// Editor pages.
	Ind.UpdateDetails(L"Saving Pages");
	Ind.SetProgress( 4, 4 );

	JSon::Ptr pagesList = JSon::createArrayNode();
	projectFile->addField( L"editorPages", pagesList );
	{
		for( Int32 iPage=0; iPage<EditorPages->Pages.size(); iPage++ )
		{
			WEditorPage* Page	= (WEditorPage*)EditorPages->Pages[iPage];
			FObject* Res = nullptr;

			if( Page->PageType!=PAGE_Hello && Page->PageType!=PAGE_Play )
				Res	= Page->GetResource();

			if( Res )
				pagesList->insertElement( JSon::createStringNode( Res->GetName() ) );
		}
	}

	String saveError;
	Text::Ptr projectFileText = JSon::saveToText( projectFile, &saveError );

	if( projectFileText.hasObject() )
	{
		return fm::writeTextFile( *Project->FileName, projectFileText );
	}
	else
	{
		fatal( L"Unable to save project with error %s", *saveError );
		return false;
	}
}


//
// Save project with ask.
//
Bool CEditor::SaveAsProject()
{
	if( !Project )
		return false;

	// Ask file name.
	String FileName;
	if	(	!ExecuteSaveFileDialog
			( 
				hWnd,
				FileName, 
				Project->FileName ? Project->FileName :	fm::getCurrentDirectory(), 
				L"Fluorine Project (*.fluproj)\0*.fluproj\0" 
			) 
		)
			return false;

	// Append extension, if it missing.
	if( String::pos( L".fluproj", String::lowerCase(FileName) ) == -1 )
		FileName += L".fluproj";

	// Whether override?
	if( fm::fileExists( *FileName ) )
	{
		if( FileName != Project->FileName )
		{
			int	S	= MessageBox
			(
				hWnd,
				*String::format( L"%s already exists.\nOverride?", *FileName ), 
				L"Saving", 
				MB_YESNO | MB_ICONWARNING | MB_TASKMODAL 
			);

			if( S == IDNO )
				return false;
		}
	}

	// Set as project file name and save it.
	Project->FileName		= FileName;
	Project->ProjName		= fm::getFileName( *FileName );

	// Save to file.
	if( !SaveProject() )
		return false;

	// Update caption.
	SetCaption(String::format(L"%s - Fluorine Engine", *Project->ProjName));

	return true;
}


/*-----------------------------------------------------------------------------
    Project opening helpers.
-----------------------------------------------------------------------------*/

//
// An information about just loaded 
// property.
//
struct TLoadProperty
{
public:
	// Used static arrays instead dynamic string, since
	// it's fast and used very often at load time.
	Char		Name[64];
	Char		Value[64];

	//
	// Parse a value from the 'Value' string,
	// anyway return something even in case of
	// failure. Make sure this function's doesn't 
	// crash the app.
	//

	// Parse byte value. 
	UInt8 ToByte()
	{
		return _wtoi(Value);
	}

	// Parse int value.
	Int32 ToInteger()
	{
		return _wtoi(Value);
	}

	// Parse float value.
	Float ToFloat()
	{
		return _wtof(Value);
	}

	// Parse bool value.
	Bool ToBool()
	{
		return wcsstr( Value, L"true" ) != nullptr;
	}

	// Parse angle value.
	math::Angle ToAngle()
	{
		return _wtoi(Value);
	}

	// Parse string value.
	String ToString()
	{
		if( Value[0] != '"' ) 
			return L"";

		Char	Buffer[64] = {}, *Walk = Buffer, 
				*ValWalk = &Value[1], *End = &Value[arraySize(Value)-1];

		while( *ValWalk != '"' && ValWalk != End )
		{
			*Walk = *ValWalk;
			Walk++;
			ValWalk++;
		}
		return Buffer;
	}

	// Parse vector value.
	math::Vector ToVector()
	{
		Float X=0.f, Y=0.f;
		Char *Walk=Value, *End = &Value[arraySize(Value)-1];
		Walk++;
		X = _wtof(Walk);
		while( *Walk != ';' )
		{
			Walk++;
			if( Walk > End ) 
				return math::Vector( X, Y );
		}
		Walk++;
		Y = _wtof(Walk);
		return math::Vector( X, Y );
	}

	// Parse color value.
	math::Color ToColor()
	{
		UInt8 R, G, B, A;
		R = cstr::fromHex(Value[1])*16 + cstr::fromHex(Value[2]);
		G = cstr::fromHex(Value[3])*16 + cstr::fromHex(Value[4]);
		B = cstr::fromHex(Value[5])*16 + cstr::fromHex(Value[6]);
		A = cstr::fromHex(Value[7])*16 + cstr::fromHex(Value[8]);
		return math::Color( R, G, B, A );
	}

	// Parse object value.
	FObject* ToObject()
	{
		FObject* Result = nullptr;
		Char *Walk=Value, *End = &Value[arraySize(Value)-1];
		Char ClassName[32]={}, ObjName[32]={};
		CClass* ReqClass = nullptr;

		for( Char* C=ClassName; *Walk!=':' && C!=&ClassName[31]; C++, Walk++ )
			*C = *Walk;

		Walk += 2;
		ReqClass = CClassDatabase::StaticFindClass( ClassName );
		if( !ReqClass ) 
			return nullptr;

		while( true )
		{
			mem::zero( ObjName, sizeof(ObjName) );
			for( Char* C=ObjName; C!=&ObjName[31] && *Walk>32 && *Walk!='.'; C++, Walk++ )
				*C = *Walk;
			Walk++;
			Result = GObjectDatabase->FindObject( ObjName, FObject::MetaClass, Result );
			if( !Result ) return nullptr;
			if( *Walk <= 32 )
			{
				if( Result->IsA(ReqClass) )
					return Result;
			}
		}

		return Result;
	}

	// Parse rect value.
	math::Rect ToAABB()
	{
		math::Rect Rect;
		Char *Walk=Value, *End = &Value[arraySize(Value)-1];
		Walk++;
		Rect.min.x = _wtof(Walk);
		while( *Walk != ';' )
		{
			Walk++;
			if( Walk > End ) return Rect;
		}
		Walk++;
		Rect.min.y = _wtof(Walk);
		while( *Walk != ';' )
		{
			Walk++;
			if( Walk > End ) return Rect;
		}
		Walk++;
		Rect.max.x = _wtof(Walk);
		while( *Walk != ';' )
		{
			Walk++;
			if( Walk > End ) return Rect;
		}
		Walk++;
		Rect.max.y = _wtof(Walk);
		return Rect;
	}
};


//
// Parse word from the line, start from iFirst character.
//
String ParseWord( String Source, Int32& iFirst )
{
	while( Source[iFirst]==' ' && iFirst<Source.len() )
		++iFirst;

	Char Buffer[64] = {};
	Char* Walk = Buffer;
	while( iFirst<Source.len() && Source[iFirst]!=' ' )
	{
		*Walk = Source[iFirst];
		Walk++;
		iFirst++;
	}
	return Buffer;
}


//
// A loaded object type.
//
enum ELoadObject
{
	LOB_Invalid,		// Bad object.
	LOB_Resource,		// Resource object, its a root of the file.
	LOB_Entity,			// Entity object, in level.
	LOB_Component,		// Component object.
	LOB_Instance,		// CInstanceBuffer.
	LOB_Modifier,		// Resource modifier.
};


//
// A just loaded object.
//
struct TLoadObject
{
	// Variables.
	ELoadObject				Type;
	CClass*					Class;		// Except 'LOB_Instance'.
	String					Name;		// Except 'LOB_Instance'.
	FScript*				Script;		// 'LOB_Entity' only.

	union 
	{
		FObject*			Object;
		CInstanceBuffer*	Instance;
	};

	// Tables.
	Array<TLoadProperty>	Props;
	Array<TLoadObject*>		Nodes;

	// Load object constructor.
	TLoadObject()
		:	Type( LOB_Invalid ),
			Class( FObject::MetaClass ),
			Name( L"" ),
			Script( nullptr ),
			Object( nullptr ),
			Props(),
			Nodes()
	{}

	// Load object destructor.
	~TLoadObject()
	{
		for( Int32 i=0; i<Nodes.size(); i++ )
			delete Nodes[i];
		Props.empty();
		Nodes.empty();
	}

	// Find property by it name.
	TLoadProperty* FindProperty( String PropName, Bool bMandatory=false )
	{
		for( Int32 i=0; i<Props.size(); i++ )
			if( PropName == Props[i].Name )
				return &Props[i];

		if( bMandatory )	
			throw String::format( L"Property '%s' not found", *PropName );

		return nullptr;
	}

	// Parse property from the line and add it property
	// to list.
	void ParseProp( const String& Line )
	{
		TLoadProperty Prop;
		mem::zero( &Prop, sizeof(TLoadProperty) );
		Int32 iPos = 0;
		Char *NameWalk = Prop.Name, *ValueWalk = Prop.Value;

		while( Line[iPos]==' ' && iPos<Line.len() )
			++iPos;

		while( Line[iPos]!=' ' && iPos<Line.len() )
		{
			*NameWalk = Line[iPos];
			NameWalk++;
			iPos++;
		}

		while( Line[iPos]==' ' && iPos<Line.len() )
			++iPos;

		if( Line[iPos++] != '=' )
			throw String(L"Missing assignment");

		while( Line[iPos]==' ' && iPos<Line.len() )
			++iPos;

		while( iPos<Line.len() )
		{
			*ValueWalk = Line[iPos];
			ValueWalk++;
			iPos++;
		}

		Props.push( Prop );
#if 0
		// Dbg.
		log( L"Got property '%s' with value '%s'", Prop.Name, Prop.Value );
#endif
	}
};


/*-----------------------------------------------------------------------------
    Project opening.
-----------------------------------------------------------------------------*/

//
// Load an object info from the single file.
//
TLoadObject* LoadResource( String FileName, String Directory )
{
	TLoadObject* Resource = new TLoadObject();
	CTextReader Reader( FileName );	
	
	//
	// Load resource header.
	//
	{
		String Header = Reader.ReadLine();
		String ObjName, ObjClass;
		Int32 iPos = 0;

		if( ParseWord( Header, iPos ) != L"BEGIN_RESOURCE" )
			throw String::format( L"Bad resource file: '%s'", *FileName );

		ObjClass = ParseWord( Header, iPos );
		ObjName	= ParseWord( Header, iPos );

		// Preinitialize Resource.
		Resource->Type		= LOB_Resource;
		Resource->Class		= CClassDatabase::StaticFindClass( *ObjClass );
		Resource->Name		= ObjName;
		if( !Resource->Class )
			throw String::format( L"Class '%s' not found", *ObjClass );
	}

	// Read resource data.
	if( Resource->Class->IsA(FBitmap::MetaClass) )
	{
		//
		// Bitmap.
		//
		for( ; ; )
		{
			String Line = Reader.ReadLine();

			if( String::pos( L"END_RESOURCE", Line ) != -1 )
			{
				// End of text.
				break;
			}
			else
			{
				// Read property.
				Resource->ParseProp(Line);
			}
		}

		// Allocate bitmap.
#if DEMO_EFFECTS_ENABLED
		if( Resource->Class->IsA(FDemoBitmap::MetaClass) )
		{
			// Allocate new one.
			FBitmap* Bitmap;
			Int32 UBits, VBits;
			UBits	= Resource->FindProperty( L"UBits", true )->ToInteger();
			VBits	= Resource->FindProperty( L"VBits", true )->ToInteger();

			Resource->Object	= Bitmap = NewObject<FBitmap>( Resource->Class, Resource->Name, nullptr );
			Bitmap->Init( 1 << UBits, 1 << VBits );
		}
		else
#endif
		{
			// Import bitmap from the file.
			String BitFile = Resource->FindProperty( L"image", true )->ToString();
			FBitmap* bitmap;
			Resource->Object = bitmap = NewObject<FBitmap>( Resource->Name, nullptr );
			bitmap->m_image = res::ResourceManager::get<img::Image>( BitFile, res::EFailPolicy::FATAL );
		}
	}
	else if( Resource->Class->IsA(FSound::MetaClass) )
	{
		//
		// Sound.
		//
		for( ; ; )
		{
			String Line = Reader.ReadLine();

			if( String::pos( L"END_RESOURCE", Line ) != -1 )
			{
				// End of text.
				break;
			}
			else
			{
				// Read property.
				Resource->ParseProp(Line);
			}
		}		

		{
			// Import sound from file.
			String SndFile, FullFn;
			SndFile = Resource->FindProperty( L"FileName", true )->ToString();
			FullFn	= String::format( L"%s\\Sounds\\%s", *Directory, *SndFile );
			if( !fm::fileExists( *FullFn ) ) 
				throw String::format( L"File '%s' not found", *FullFn );
			Resource->Object	= GEditor->ImportResource( FullFn );
		}
	}
	else if( Resource->Class->IsA(FScript::MetaClass) )
	{
		//
		// Script.
		//
		for( ; ; )
		{
			String Line = Reader.ReadLine();

			if( String::pos( L"END_RESOURCE", Line ) != -1 )
			{
				// End of text.
				break;
			}
			else if( String::pos( L"BEGIN_COMPONENT", Line ) != -1 )
			{
				// Read component.
				TLoadObject* Component = new TLoadObject();
				Resource->Nodes.push(Component);

				// Load resource header.
				{
					String Header = Line;
					String ObjName, ObjClass;
					Int32 iPos = 0;

					if( ParseWord( Header, iPos ) != L"BEGIN_COMPONENT" )
						throw String::format( L"Bad resource file: '%s'", *FileName );

					ObjClass = ParseWord( Header, iPos );
					ObjName	= ParseWord( Header, iPos );

					// Preinitialize Component.
					Component->Type		= LOB_Component;
					Component->Class	= CClassDatabase::StaticFindClass( *ObjClass );
					Component->Name		= ObjName;
					if( !Component->Class )
						throw String::format( L"Class '%s' not found", *ObjClass );
				}
				for( ; ; )
				{
					String Line = Reader.ReadLine();

					if( String::pos( L"END_COMPONENT", Line ) != -1 )
					{
						// End of text.
						break;
					}
					else
					{
						// Read component property.
						Component->ParseProp(Line);
					}
				}
			}
			else if( String::pos( L"BEGIN_INSTANCE", Line ) != -1 )
			{
				// Instance buffer.
				TLoadObject* Instance = new TLoadObject();
				Resource->Nodes.push( Instance );
				Instance->Type	= LOB_Instance;
				if( !(Resource->FindProperty( L"ScriptFlags" )? Resource->FindProperty( L"ScriptFlags" )->ToInteger() & SCRIPT_Scriptable : false) )
					throw String::format( L"Script '%s' has no instance buffer", *Resource->Name );

				for( ; ; )
				{
					String Line = Reader.ReadLine();

					if( String::pos( L"END_INSTANCE", Line ) != -1 )
					{
						// End of text.
						break;
					}
					else
					{
						// Read instance property.
						Instance->ParseProp(Line);
					}
				}
			}
			else
			{
				// Read property.
				Resource->ParseProp(Line);
			}
		}

		// Allocate script.
		FScript* Script			= NewObject<FScript>( Resource->Name );
		Resource->Object		= Script;
		Script->ScriptFlags		= Resource->FindProperty( L"ScriptFlags" ) ? Resource->FindProperty( L"ScriptFlags" )->ToInteger() : SCRIPT_None;
		Script->FileName		= Resource->FindProperty( L"FileName" ) ? Resource->FindProperty( L"FileName" )->ToString() : L"";
		Script->InstanceBuffer	= Script->IsScriptable() && !Script->IsStatic() ? new CInstanceBuffer(Script->Properties) : nullptr;

		// Load script text.
		if( Script->IsScriptable() )
		{
			String FullFileName = String::format( L"%s\\Scripts\\%s", *Directory, *Script->FileName );
			if( !fm::fileExists( *FullFileName ) )
				throw String::format( L"File '%s' not found", *FullFileName );
			CTextReader TextReader(FullFileName);
			while( !TextReader.IsEOF() )
				Script->Text.push( TextReader.ReadLine() );

			// Eliminate empty lines at the end of file.
			while( Script->Text.size() && !Script->Text.last() )
				Script->Text.removeFast(Script->Text.size()-1);
		}

		// Create components.
		for( Int32 iNode=0; iNode<Resource->Nodes.size(); iNode++ )
		{
			TLoadObject* Node = Resource->Nodes[iNode];

			if( Node->Type == LOB_Component )
			{
				FComponent*	Com = NewObject<FComponent>( Node->Class, Node->Name, Script );
				Com->InitForScript( Script );
				Node->Object	= Com;
			}
			else if( Node->Type == LOB_Instance )
			{
				Node->Instance	= Script->InstanceBuffer;
			}
		}
	}
#if MATERIAL_ENABLED
	else if( Resource->Class->IsA(FMaterial::MetaClass) )
	{
		//
		// Material.
		//
		for( ; ; )
		{
			String Line = Reader.ReadLine();

			if( String::pos( L"END_RESOURCE", Line ) != -1 )
			{
				// End of text.
				break;
			}
			else if( String::pos( L"BEGIN_LAYER", Line ) != -1 )
			{
				// Material layer.
				TLoadObject* Layer = new TLoadObject();
				Resource->Nodes.push( Layer );
				Layer->Type	= LOB_Modifier;

				// Load layer header.
				{
					String Header = Line;
					String ObjName, ObjClass;
					Int32 iPos = 0;

					if( ParseWord( Header, iPos ) != L"BEGIN_LAYER" )
						throw String::format( L"Bad resource file: '%s'", *FileName );

					ObjClass = ParseWord( Header, iPos );
					ObjName	= ParseWord( Header, iPos );

					// Preinitialize Layer.
					Layer->Class	= CClassDatabase::StaticFindClass( *ObjClass );
					Layer->Name		= ObjName;
					if( !Layer->Class )
						throw String::format( L"Class '%s' not found", *ObjClass );

				}

				for( ; ; )
				{
					String Line = Reader.ReadLine();

					if( String::pos( L"END_LAYER", Line ) != -1 )
					{
						// End of text.
						break;
					}
					else
					{
						// Read instance property.
						Layer->ParseProp(Line);
					}
				}
			}
			else
			{
				// Read property.
				Resource->ParseProp(Line);
			}
		}

		// Allocate material.
		FMaterial* Material		= NewObject<FMaterial>(Resource->Name);
		Resource->Object		= Material;

		// Create layers.
		for( Int32 iNode=0; iNode<Resource->Nodes.size(); iNode++ )
		{
			TLoadObject* Node = Resource->Nodes[iNode];

			assert(Node->Type == LOB_Modifier);
			FMaterialLayer* Layer = NewObject<FMaterialLayer>(Node->Class, Node->Name, Material);
			Material->Layers.push(Layer);
			Node->Object = Layer;
		}
	}
#endif
	else if( Resource->Class->IsA(FLevel::MetaClass) )
	{
		//
		// Level.
		//
		for( ; ; )
		{
			String Line = Reader.ReadLine();

			if( String::pos( L"END_RESOURCE", Line ) != -1 )
			{
				// End of text.
				break;
			}
			else if( String::pos( L"BEGIN_ENTITY", Line ) != -1 )
			{
				// Read entity.
				TLoadObject* Entity = new TLoadObject();
				Resource->Nodes.push( Entity );

				// Load entity header.
				{
					String Header = Line;
					String ObjName, ObjScript;
					Int32 iPos = 0;

					if( ParseWord( Header, iPos ) != L"BEGIN_ENTITY" )
						throw String::format( L"Bad resource file: '%s'", *FileName );

					ObjScript	= ParseWord( Header, iPos );
					ObjName		= ParseWord( Header, iPos );

					// Preinitialize Component.
					Entity->Type		= LOB_Entity;
					Entity->Class		= FEntity::MetaClass;
					Entity->Name		= ObjName;
					Entity->Script		= (FScript*)GObjectDatabase->FindObject( ObjScript, FScript::MetaClass, nullptr );
					if( !Entity->Script )
						throw String::format( L"Script '%s' not found", *ObjScript );
				}
				for( ; ; )
				{
					String Line = Reader.ReadLine();

					if( String::pos( L"END_ENTITY", Line ) != -1 )
					{
						// End of text.
						break;
					}
					else if( String::pos( L"BEGIN_COMPONENT", Line ) != -1 )
					{
						// Read component.
						TLoadObject* Component = new TLoadObject();
						Entity->Nodes.push( Component );

						// Load resource header.
						{
							String Header = Line;
							String ObjName, ObjClass;
							Int32 iPos = 0;

							if( ParseWord( Header, iPos ) != L"BEGIN_COMPONENT" )
								throw String::format( L"Bad resource file: '%s'", *FileName );

							ObjClass = ParseWord( Header, iPos );
							ObjName	= ParseWord( Header, iPos );

							// Preinitialize Component.
							Component->Type		= LOB_Component;
							Component->Class	= CClassDatabase::StaticFindClass( *ObjClass );
							Component->Name		= ObjName;
							if( !Component->Class )
								throw String::format( L"Class '%s' not found", *ObjClass );
						}
						for( ; ; )
						{
							String Line = Reader.ReadLine();

							if( String::pos( L"END_COMPONENT", Line ) != -1 )
							{
								// End of text.
								break;
							}
							else
							{
								// Read component property.
								Component->ParseProp(Line);
							}
						}
					}
					else if( String::pos( L"BEGIN_INSTANCE", Line ) != -1 )
					{
						// Instance buffer.
						TLoadObject* Instance = new TLoadObject();
						Entity->Nodes.push( Instance );
						Instance->Type	= LOB_Instance;
						if( !Entity->Script->IsScriptable() || Entity->Script->IsStatic() )
							throw String::format( L"Script '%s' has no instance buffer", *Resource->Name );

						for( ; ; )
						{
							String Line = Reader.ReadLine();

							if( String::pos( L"END_INSTANCE", Line ) != -1 )
							{
								// End of text.
								break;
							}
							else
							{
								// Read instance property.
								Instance->ParseProp(Line);
							}
						}
					}
				}
			}
			else
			{
				// Read property.
				Resource->ParseProp(Line);
			}
		}

		// Allocate level.
		FLevel* Level = NewObject<FLevel>( Resource->Name );
		Resource->Object	= Level;

		// Create entities.
		for( Int32 iEntity=0; iEntity<Resource->Nodes.size(); iEntity++ )
		{
			TLoadObject* EntNode = Resource->Nodes[iEntity];
			FEntity* Entity = NewObject<FEntity>( EntNode->Name, Level );
			Entity->Level	= Level;
			Entity->Script	= EntNode->Script;
			Entity->InstanceBuffer	= Entity->Script->IsScriptable() ? new CInstanceBuffer(Entity->Script->Properties) : nullptr;
			EntNode->Object	= Entity;

			// Create components.
			for( Int32 iNode=0; iNode<EntNode->Nodes.size(); iNode++ )
			{
				TLoadObject* Node = EntNode->Nodes[iNode];	

				if( Node->Type == LOB_Component )
				{
					FComponent*	Com = NewObject<FComponent>( Node->Class, Node->Name, Entity );
					Com->InitForEntity( Entity );
					Node->Object	= Com;
				}
				else if( Node->Type == LOB_Instance )
				{
					Node->Instance	= Entity->InstanceBuffer;
				}
			}

			Level->Entities.push( Entity );
		}
	}
	else
	{
		//
		// Regular resource.
		//
		for( ; ; )
		{
			String Line = Reader.ReadLine();

			if( String::pos( L"END_RESOURCE", Line ) != -1 )
			{
				// End of text.
				break;
			}
			else
			{
				// Read property.
				Resource->ParseProp(Line);
			}
		}

		// Allocate resource.
		Resource->Object = NewObject<FObject>( Resource->Class, Resource->Name, nullptr );
	}

#if 0
	// Dbg.
	log( L"Load '%s'", *FileName );
#endif
	return Resource;
}



//
// Text importer.
//
class CImporter: public CImporterBase
{
public:
	// Variables.
	TLoadObject*	Object;

	// Importer constructor.
	CImporter()
		:	Object( nullptr )
	{}

	// Importer destructor.
	~CImporter()
	{}

	// Set object to import properties from.
	void SetObject( TLoadObject* InObject )
	{
		assert(InObject);
		Object	= InObject;
	}

	// Byte import.
	UInt8 ImportByte( const Char* FieldName )
	{
		TLoadProperty* Prop = Object->FindProperty( FieldName, false );
		return Prop ? Prop->ToByte() : 0x00;
	}

	// Integer import.
	Int32	ImportInteger( const Char* FieldName )
	{
		TLoadProperty* Prop = Object->FindProperty( FieldName, false );
		return Prop ? Prop->ToInteger() : 0;
	}

	// Float import.
	Float ImportFloat( const Char* FieldName )
	{
		TLoadProperty* Prop = Object->FindProperty( FieldName, false );
		return Prop ? Prop->ToFloat() : 0.f;
	}

	// String import.
	String ImportString( const Char* FieldName )
	{
		TLoadProperty* Prop = Object->FindProperty( FieldName, false );
		return Prop ? Prop->ToString() : L"";
	}

	// Bool import.
	Bool ImportBool( const Char* FieldName )
	{
		TLoadProperty* Prop = Object->FindProperty( FieldName, false );
		return Prop ? Prop->ToBool() : false;
	}

	// Color import.
	math::Color ImportColor( const Char* FieldName )
	{
		TLoadProperty* Prop = Object->FindProperty( FieldName, false );
		return Prop ? Prop->ToColor() : math::colors::BLACK;
	}

	// Vector import.
	math::Vector ImportVector( const Char* FieldName )
	{
		TLoadProperty* Prop = Object->FindProperty( FieldName, false );
		return Prop ? Prop->ToVector() : math::Vector(0,0);
	}

	// Rect import.
	math::Rect ImportAABB( const Char* FieldName )
	{
		TLoadProperty* Prop = Object->FindProperty( FieldName, false );
		return Prop ? Prop->ToAABB() : math::Rect(math::Vector(0.f,0.f), 0.f, 0.f);
	}

	// Angle import.
	math::Angle ImportAngle( const Char* FieldName )
	{
		TLoadProperty* Prop = Object->FindProperty( FieldName, false );
		return Prop ? Prop->ToAngle() : math::Angle(0.f);
	}

	// Object import.
	FObject* ImportObject( const Char* FieldName )
	{
		TLoadProperty* Prop = Object->FindProperty( FieldName, false );
		return Prop ? Prop->ToObject() : nullptr;
	}
};


//
// Import object's fields. Warning
// this routine are recursive to handle
// objects nodes graph.
//
void ImportFields( CImporter& Importer, TLoadObject* Object )
{
	// Load own fields.
	if( Object->Type == LOB_Instance )
	{
		// Load instance buffer.
		assert(Object->Instance);
		Importer.SetObject( Object );
		Object->Instance->ImportValues( Importer );
	}
	else
	{
		// Load 'F' object.
		assert(Object->Object);
		Importer.SetObject( Object );
		Object->Object->Import( Importer );
	}

	// Load sub-objects.
	for( Int32 i=0; i<Object->Nodes.size(); i++ )
		ImportFields( Importer, Object->Nodes[i] );
}


//
// Open project from the file.
//
Bool CEditor::OpenProjectFrom( String FileName )
{
	// No opened project before.
	assert(!GProject);

	// Allocate the project.
	String	Directory	= fm::getFilePath( *FileName );
	String	ProjName	= fm::getFileName( *FileName );
	Project				= new CProject();
	Project->BlockMan	= new CBlockManager();
	Project->FileName	= FileName;
	Project->ProjName	= ProjName;

	// Load project file.
	Bool Result		= true;
	Array<TLoadObject*>	ResObjs;

	String projectFileError;
	Text::Ptr projectFileText = fm::readTextFile( *FileName );
	JSon::Ptr projectFile = JSon::loadFromText( projectFileText, &projectFileError );

	// Use loaging dialog, for coolness.
	IProgressIndicator::THolder Ind( TaskDialog, L"Project Loading" );

	// Let loading wrap into try-catch.
	try
	{
		// Load all resources into temporal TLoadObject structs.
		// And allocate all objects. All fields will be loaded
		// later.
		if( !projectFile.hasObject() )
			throw String( L"Bad project file" );

		Ind.UpdateDetails(L"Loading Objects");
		Ind.SetProgress( 0, 100 );

		JSon::Ptr includesList = projectFile->getField( L"includes", JSon::EMissingPolicy::USE_STUB );

		for( Int32 i = 0; i < includesList->arraySize(); ++i )
		{
			String ObjName = includesList->getElement( i )->asString();

			if( ObjName )
			{
				TLoadObject* Obj = LoadResource( String::format( L"%s\\Objects\\%s", *Directory, *ObjName ), Directory );
				ResObjs.push( Obj );
			}
		}

		// Find project info.
		for( Int32 iRes=0; iRes<ResObjs.size(); iRes++ )
			if( ResObjs[iRes]->Object->IsA(FProjectInfo::MetaClass) )
			{
				GProject->Info	= As<FProjectInfo>(ResObjs[iRes]->Object);
				break;
			}
		if( !GProject->Info )
			throw String(L"Project info not found");

		// Compile scripts.
		Ind.UpdateDetails(L"Script Compiling");
		Ind.SetProgress( 20, 100 );
		if( !CompileAllScripts(nullptr) )
			throw String(L"Failed compile script at start time");

		// Setup all fields.
		Ind.UpdateDetails(L"Properties Import");
		Ind.SetProgress( 50, 100 );
		CImporter Importer;
		for( Int32 iRes=0; iRes<ResObjs.size(); iRes++ )
			ImportFields( Importer, ResObjs[iRes] );

		Result	= true;
	}
	catch( String Message )
	{
		// Failure with message.
		warn( L"Failed load project '%s' with message: '%s'", *ProjName, *Message );

		MessageBox( 0, *String::format( L"Failed load project '%s' with message: '%s'", *ProjName, *Message ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		CloseProject( false );
		Result	= false;
	}
	catch( ... )
	{
		// Unhandled failure.
		warn( L"Failed load project '%s'.", *ProjName );

		MessageBox( 0, *String::format( L"Failed load project '%s'", *ProjName ), 
			L"Editor", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );

		CloseProject( false );
		Result	= false;
	}

	// Destroy temporal objects.
	for( Int32 i=0; i<ResObjs.size(); i++ )
		delete ResObjs[i];
	ResObjs.empty();

	// Notify all objects about loading.
	Ind.UpdateDetails(L"Notification");
	Ind.SetProgress( 75, 100 );
	for( Int32 iObj=0; GObjectDatabase && iObj<GObjectDatabase->GObjects.size(); iObj++ )
		if( GObjectDatabase->GObjects[iObj] )
			GObjectDatabase->GObjects[iObj]->PostLoad();

	// Update all editor panels.
	Array<FObject*> EmptyArr;
	Inspector->SetEditObjects( EmptyArr );
	Browser->Refresh();

	// Restore pages.
	Ind.UpdateDetails(L"Pages Restoring");
	Ind.SetProgress( 90, 100 );

	if( projectFile.hasObject() )
	{
		JSon::Ptr pagesList = projectFile->getField( L"editorPages" );

		for( Int32 i = 0; i < pagesList->arraySize(); ++i )
		{
			String PageName = pagesList->getElement( i )->asString();
	
			FResource* Res = nullptr;
			if( PageName && (Res = (FResource*)Project->FindObject( PageName, FResource::MetaClass )) )
			{
				this->OpenPageWith( Res );
			}
		}	
	}

	// Update list of recent projects.
	{
		Int32 i;
		for( i=0; i<5; i++ )
			if(	String::upperCase(Config->ReadString( L"Editor", L"Recent", *String::format(L"Recent[%i]", i), L"" )) == 
				String::upperCase(FileName) )
			{
				// Yes, it found in list.
				break;
			}

		if( i == 5 )
		{
			// Project not found in list of recent.
			// So shift list and add new one.
			for( Int32 j=4; j>0; j-- )
			{
				String Prev	= Config->ReadString( L"Editor", L"Recent", *String::format(L"Recent[%i]", j-1), L"" );
				Config->WriteString( L"Editor", L"Recent", *String::format(L"Recent[%i]", j), Prev );
			}

			// Add new.
			Config->WriteString( L"Editor", L"Recent", L"Recent[0]", FileName );
		}
	}

	// Update caption.
	if( Project )
		SetCaption(String::format(L"%s - Fluorine Engine", *Project->ProjName));

	return Result;
}


//
// Open entire project.
//
Bool CEditor::OpenProject()
{
	// Unload last project.
	if( Project && !CloseProject() )
		return false;

	// Ask file name.
	String FileName;
	if	( 
			!ExecuteOpenFileDialog
			( 
				hWnd,
				FileName, 
				fm::getCurrentDirectory(), 
				L"Fluorine Project (*.fluproj)\0*.fluproj\0" 
			) 
		)
			return false;

	// Open it.
	return OpenProjectFrom( FileName );
}


//
// Load external resource from the file.
//
FResource* CEditor::PreloadResource( String Name )
{
	// Don't load if no project.
	if( !Project )
		return nullptr;

	FResource*	Res			= nullptr;
	String		FileName	= fm::getCurrentDirectory() + Name;
	String		Directory	= fm::getFilePath( *FileName );
	assert( fm::fileExists( *FileName ) );

	// Let loading wrap into try-catch.
	try
	{
		// Load object.
		TLoadObject* Obj = LoadResource( FileName, Directory );
		
		// If it is script, compile script, to load instance buffer
		// properly.
		Res					= As<FResource>(Obj->Object);
		FScript*	Script	= As<FScript>(Res);
		if( Script && Script->IsScriptable() )
			if( !CompileAllScripts(nullptr) )
				fatal( L"Unable to load script '%s' with errors", *Name );

		// Import all fields.
		CImporter Importer;
		ImportFields( Importer, Obj );

		// Destroy temporal objects.
		delete Obj;
	}
	catch( ... )
	{
		fatal( L"Unable to load resource '%s'", *Name );
	}

	Res->PostLoad();
	return Res;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/