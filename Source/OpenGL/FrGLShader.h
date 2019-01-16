/*=============================================================================
	FrGLShader.h: OpenGL shader.
	Created by Vlad Gordienko, Oct. 2016.
	Redesigned by Vlad Gordienko, Mar. 2018.
=============================================================================*/

//
// Shader constants.
//
#define SHADER_DIR			L"\\OpenGL\\"
#define VERT_SHADER_EXT		L".vsh"
#define FRAG_SHADER_EXT		L".fsh"

/*-----------------------------------------------------------------------------
	CGLShaderBase.
-----------------------------------------------------------------------------*/

//
// An abstract OpenGL shader.
//
class CGLShaderBase
{
public:
	// CGLShaderBase interface.
	CGLShaderBase();
	virtual ~CGLShaderBase();
	virtual bool Init( String ShaderName );

	// Setters.
	inline void SetValue1f( Int32 iUniform, Float Value )
	{
		SetValue( iUniform, 1, &Value );
	}
	inline void SetValue2f( Int32 iUniform, const Float* Value )
	{
		SetValue( iUniform, 2, Value );
	}
	inline void SetValue3f( Int32 iUniform, const Float* Value )
	{
		SetValue( iUniform, 3, Value );
	}
	inline void SetValue4f( Int32 iUniform, const Float* Value )
	{
		SetValue( iUniform, 4, Value );
	}
	inline void SetValue2f( Int32 iUniform, const TVector& Value )
	{
		SetValue( iUniform, 2, &Value.X );
	}
	inline void SetValue1i( Int32 iUniform, Int32 Value )
	{
		SetValue( iUniform, 0, &Value );
	}

	// Accessors.
	inline Bool IsEnabled() const
	{
		return bEnabled;
	}

protected:
	// An uniform variable.
	struct TUniform
	{
	public:
		UInt32		Dimension = 1;
		GLint		iUniform = -1;
		Int32		iCommitNext = -1;
		Bool		bDirty = false;

		union
		{
			Float	FloatValue[4];
			Int32	IntValue[4];
		};
	};

	// Internal.
	GLuint				iglProgram;
	GLuint				iglVertShader;
	GLuint				iglFragShader;

	String				Name;
	Array<TUniform>		Uniforms;
	Int32				iCommitFirst;
	Bool				bEnabled;

	// CGLShaderBase interface.
	Int32 RegisterUniform( AnsiChar* Name );
	void CommitValues();
	void SetValue( Int32 iUniform, UInt32 Dimension, const void* Value );

	// Friends.
	friend class COpenGLCanvas;
};


/*-----------------------------------------------------------------------------
	CGLFluShader.
-----------------------------------------------------------------------------*/

//
// GOD fluorine shader.
//
class CGLFluShader: public CGLShaderBase
{
public:
	// Maximum light sources per lightmap.
	enum { MAX_LIGHTS	= 16 };

	// CGLFluShader interface.
	CGLFluShader();
	~CGLFluShader();

	// CGLShaderBase interface.
	bool Init( String ShaderName ) override;

	// CGLFluShader setters.
	void SetAmbientLight( const TColor& InAmbient );
	Bool AddLight( FLightComponent* Light, const TVector& Location, TAngle Rotation );
	void ResetLights();

	// Shader modes.
	void SetModeUnlit();
	void SetModeLightmap();
	void SetModeComplex();

private:
	// In-shader structures.
	struct TLightSource
	{
		Int32	Effect;
		Int32	Color;
		Int32	Brightness;
		Int32 Radius;
		Int32 Location;
		Int32 Rotation;
	};

	// Uniform variables.
	Int32 idUnlit;
	Int32 idRenderLightmap;
	Int32 idBitmap;
	Int32 idSaturation;
	Int32 idANum;
	Int32 idMNum;
	Int32 idGameTime;
	Int32 idAmbientLight;

	TLightSource idALights[MAX_LIGHTS];
	TLightSource idMLights[MAX_LIGHTS];

	Int32	ANum, MNum;

	// Friends.
	friend class COpenGLCanvas;
	friend class COpenGLRender;
};


/*-----------------------------------------------------------------------------
	CGLFinalShader.
-----------------------------------------------------------------------------*/

//
// Post-Processing shader.
//
class CGLFinalShader: public CGLShaderBase
{
public:
	// CGLFinalShader interface.
	CGLFinalShader();
	~CGLFinalShader();

	// CGLShaderBase interface.
	bool Init( String ShaderName ) override;

	void SetPostEffect( const TPostEffect& InEffect )
	{
		SetValue3f( idHighlights, InEffect.Highlights );
		SetValue3f( idMidTones, InEffect.MidTones );
		SetValue3f( idShadows, InEffect.Shadows );
		SetValue1f( idBWScale, InEffect.BWScale );
	}

private:
	// Uniform variables.
	Int32 idTexture;

	Int32 idHighlights;
	Int32 idMidTones;
	Int32 idShadows;
	Int32 idBWScale;
};


// Not implemented properly.
#if 0

/*-----------------------------------------------------------------------------
	CGLHorizBlurShader.
-----------------------------------------------------------------------------*/

//
// Horizontal blur shader.
//
class CGLHorizBlurShader: public CGLShaderBase
{
public:
	// CGLHorizBlurShader interface.
	CGLHorizBlurShader();
	~CGLHorizBlurShader();

	// CGLShaderBase interface.
	bool Init( String ShaderName ) override;

	// CGLHorizBlurShader interface.
	void SetTargetWidth( Float NewWidth )
	{
		SetValue1f( idTargetWidth, NewWidth );
	}

private:
	// Uniform variables.
	Int32 idTexture;
	Int32 idTargetWidth;
};


/*-----------------------------------------------------------------------------
	CGLVertBlurShader.
-----------------------------------------------------------------------------*/

//
// Vertical blur shader.
//
class CGLVertBlurShader: public CGLShaderBase
{
public:
	// CGLVertBlurShader interface.
	CGLVertBlurShader();
	~CGLVertBlurShader();

	// CGLShaderBase interface.
	bool Init( String ShaderName ) override;

	// CGLVertBlurShader interface.
	void SetTargetHeight( Float NewHeight )
	{
		SetValue1f( idTargetHeight, NewHeight );
	}

private:
	// Uniform variables.
	Int32 idTexture;
	Int32 idTargetHeight;
};
#endif

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/