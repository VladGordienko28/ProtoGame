/*=============================================================================
    FrGLExt.h: FluRender required Extensions.
    Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

//
// Function variables.
//
PFNGLGETUNIFORMLOCATIONPROC		glGetUniformLocation;
PFNGLUNIFORM1FVPROC				glUniform1fv;
PFNGLUNIFORM2FVPROC				glUniform2fv;
PFNGLUNIFORM3FVPROC				glUniform3fv;
PFNGLUNIFORM4FVPROC				glUniform4fv;
PFNGLUNIFORM1IPROC				glUniform1i;
PFNGLUNIFORM1FPROC				glUniform1f;
PFNGLCREATESHADERPROC			glCreateShader;
PFNGLSHADERSOURCEPROC			glShaderSource;
PFNGLCOMPILESHADERPROC			glCompileShader;
PFNGLGETSHADERIVPROC			glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC		glGetShaderInfoLog;
PFNGLATTACHSHADERPROC			glAttachShader;
PFNGLLINKPROGRAMPROC			glLinkProgram;
PFNGLUSEPROGRAMPROC				glUseProgram;
PFNGLACTIVETEXTUREPROC			glActiveTexture;
PFNGLCREATEPROGRAMPROC			glCreateProgram;
PFNGLDELETESHADERPROC			glDeleteShader;
PFNGLDELETEPROGRAMPROC			glDeleteProgram;
PFNGLDELETEFRAMEBUFFERSEXTPROC	glDeleteFramebuffers;
PFNGLDELETERENDERBUFFERSEXTPROC	glDeleteRenderbuffers;
PFNGLBINDFRAMEBUFFEREXTPROC		glBindFramebuffer;
PFNGLGENFRAMEBUFFERSEXTPROC		glGenFramebuffers;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2D;
//...


//
// Initialize OpenGL extensions.
//
void InitOpenGLext()
{
	*(void**)&glGetUniformLocation		= wglGetProcAddress("glGetUniformLocation");
	*(void**)&glUniform1fv				= wglGetProcAddress("glUniform1fv");
	*(void**)&glUniform2fv				= wglGetProcAddress("glUniform2fv");
	*(void**)&glUniform3fv				= wglGetProcAddress("glUniform3fv");
	*(void**)&glUniform4fv				= wglGetProcAddress("glUniform4fv");
	*(void**)&glUniform1i				= wglGetProcAddress("glUniform1i");
	*(void**)&glUniform1f				= wglGetProcAddress("glUniform1f");
	*(void**)&glCreateShader			= wglGetProcAddress("glCreateShader");
	*(void**)&glShaderSource			= wglGetProcAddress("glShaderSource");
	*(void**)&glCompileShader			= wglGetProcAddress("glCompileShader");
	*(void**)&glGetShaderiv				= wglGetProcAddress("glGetShaderiv");
	*(void**)&glGetShaderInfoLog		= wglGetProcAddress("glGetShaderInfoLog");
	*(void**)&glAttachShader			= wglGetProcAddress("glAttachShader");
	*(void**)&glLinkProgram				= wglGetProcAddress("glLinkProgram");
	*(void**)&glUseProgram				= wglGetProcAddress("glUseProgram");
	*(void**)&glActiveTexture			= wglGetProcAddress("glActiveTexture");
	*(void**)&glCreateProgram			= wglGetProcAddress("glCreateProgram");
	*(void**)&glDeleteShader			= wglGetProcAddress("glDeleteShader");
	*(void**)&glDeleteProgram			= wglGetProcAddress("glDeleteProgram");
	*(void**)&glDeleteFramebuffers		= wglGetProcAddress("glDeleteFramebuffers");
	*(void**)&glDeleteRenderbuffers		= wglGetProcAddress("glDeleteRenderbuffers");
	*(void**)&glBindFramebuffer			= wglGetProcAddress("glBindFramebuffer");
	*(void**)&glGenFramebuffers			= wglGetProcAddress("glGenFramebuffers");
	*(void**)&glFramebufferTexture2D	= wglGetProcAddress("glFramebufferTexture2D");
	//*(void**)&		= wglGetProcAddress("");
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/