/*=============================================================================
	FrGLFbo.h: OpenGL FBO class.
	Created by Vlad Gordienko, Mar. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CGLFbo.
-----------------------------------------------------------------------------*/

//
// A frame-buffer object.
//
class CGLFbo
{
public:
	// CGLFbo interface.
	CGLFbo();
	~CGLFbo();
	void Bind( Int32 NewX, Int32 NewY );
	void Unbind();

	// Accessors.
	inline GLuint GetTextureId() const
	{
		return TextureId;
	}

private:
	// Constants.
	enum{ DEFAULT_FBO_WIDTH = 256 };
	enum{ DEFAULT_FBO_HEIGHT = 256 };

	// Variables.
	Int32 XSize, YSize;
	GLuint FrameBuffer;
	GLuint TextureId;
};


/*-----------------------------------------------------------------------------
	CGLFbo implementation.
-----------------------------------------------------------------------------*/

//
// GL FBO constructor.
//
CGLFbo::CGLFbo()
	:	XSize( DEFAULT_FBO_WIDTH ),
		YSize( DEFAULT_FBO_HEIGHT )
{
	// Allocate new frame-buffer.
	glGenFramebuffers( 1, &FrameBuffer );
	glBindFramebuffer( GL_FRAMEBUFFER_EXT, FrameBuffer );
	glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT );

	// Allocate Render Target.
	glGenTextures( 1, &TextureId );
	glBindTexture( GL_TEXTURE_2D, TextureId );

	// Setup render texture.
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, XSize, YSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Link texture to FBO.
	glFramebufferTexture2D( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, TextureId, 0 );

	Unbind();
}


//
// FBO destructor.
//
CGLFbo::~CGLFbo()
{
	glDeleteFramebuffers( 1, &FrameBuffer );
	glDeleteTextures( 1, &TextureId );
}


//
// Turn off the FBO.
//
void CGLFbo::Unbind()
{
	glBindFramebuffer( GL_FRAMEBUFFER_EXT, 0 );
}


//
// Turn on the FBO.
//
void CGLFbo::Bind( Int32 NewX, Int32 NewY )
{
	// Resize if required.
	if( NewX != XSize || NewY != YSize )
	{
		XSize = NewX;
		YSize = NewY;

		glBindTexture( GL_TEXTURE_2D, TextureId );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, XSize, YSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	}

	// Enable.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, FrameBuffer);
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/