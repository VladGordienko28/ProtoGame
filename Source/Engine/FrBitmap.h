/*=============================================================================
    FrBitmap.h: An image class.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
	FTexture.
-----------------------------------------------------------------------------*/

//
// An abstract image instance that imposed to objects.
//
class FTexture: public FResource
{
REGISTER_CLASS_H(FTexture);
public:
	// FTexture interface.
	FTexture();
	~FTexture();

	virtual UInt8 getUBits() const { return 0; };
	virtual UInt8 getVBits() const { return 0; };
	virtual UInt32 getUSize() const { return 0; };
	virtual UInt32 getVSize() const { return 0; };
};


/*-----------------------------------------------------------------------------
    FBitmap.
-----------------------------------------------------------------------------*/

//
// A Bitmap.
// 
class FBitmap: public FTexture
{
REGISTER_CLASS_H(FBitmap)
public:
	// Simple effects.
	Float				PanUSpeed = 0.f;
	Float				PanVSpeed = 0.f;
	Float				Saturation = 1.f;
	Float				AnimSpeed = 60.f;

	// Internal.
	Bool				bDynamic = false;
	Bool				bRedrawn = false;
	Double				LastRenderTime = 0.0;

	img::Image::Ptr		m_image;

	// Static.
	static img::Image::Ptr NullBitmap();

	// FBitmap interface.
	FBitmap();
	~FBitmap();
	void Tick();
	virtual void Erase();
	virtual void MouseClick( Int32 Button, Int32 X, Int32 Y );
	virtual void MouseMove( Int32 Button, Int32 X, Int32 Y );
	virtual void Redraw();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void EditChange();
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

	UInt8 getUBits() const override { return m_image ? m_image->getUBits() : 0; };
	UInt8 getVBits() const override { return m_image ? m_image->getVBits() : 0; };
	UInt32 getUSize() const override { return m_image ? m_image->getUSize() : 0; };
	UInt32 getVSize() const override { return m_image ? m_image->getVSize() : 0; };
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/