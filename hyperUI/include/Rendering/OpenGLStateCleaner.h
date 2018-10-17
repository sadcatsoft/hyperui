#pragma once

/*****************************************************************************/
class HYPERUI_API OpenGLStateCleaner
{
public:

	inline static void doVertexPointerCall(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) 
	{
#if !defined(USE_OPENGL2) && !defined(DIRECTX_PIPELINE)
		if(theVPpointer == pointer && theVPsize == size && theVPstride == stride && theVPtype == type)
			return;

		glVertexPointer(size, type, stride, pointer);
		theVPpointer = pointer;
		theVPsize = size;
		theVPstride = stride;
		theVPtype = type;
#endif
	}

	inline static void doTexturePointerCall(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
	{
#if !defined(USE_OPENGL2) && !defined(DIRECTX_PIPELINE)
		if(theTXpointer == pointer && theTXsize == size && theTXstride == stride && theTXtype == type)
			return;

		glTexCoordPointer(size, type, stride, pointer);
		theTXpointer = pointer;
		theTXsize = size;
		theTXstride = stride;
		theTXtype = type;
#endif
	}

	inline static void doColorPointerCall(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
	{
#if !defined(USE_OPENGL2) && !defined(DIRECTX_PIPELINE)
		if(theCLpointer == pointer && theCLsize == size && theCLstride == stride && theCLtype == type)
			return;

		glColorPointer(size, type, stride, pointer);
		theCLpointer = pointer;
		theCLsize = size;
		theCLstride = stride;
		theCLtype = type;	
#endif
	}
	
	inline static void doLineWidth(GLfloat width)
	{
#if !defined(DIRECTX_PIPELINE)
		if(theLINEwidth == width)
			return;

		glLineWidth(width);
		theLINEwidth = width;
#endif
	}

	inline static void doScissorCall(GLint x, GLint y, GLsizei width, GLsizei height)
	{
#if !defined(DIRECTX_PIPELINE)
		if(theSCIx == x && theSCIy == y && theSCIwidth == width && theSCIheight == height)
			return;

		glScissor(x, y, width, height);
		theSCIx = x;
		theSCIy = y;
		theSCIwidth = width;
		theSCIheight = height;		
#endif
	}

private:

#if !defined(DIRECTX_PIPELINE)
	static GLint theVPsize;
	static GLenum theVPtype;
	static GLsizei theVPstride;
	static const GLvoid *theVPpointer;

	static GLint theTXsize;
	static GLenum theTXtype;
	static GLsizei theTXstride;
	static const GLvoid *theTXpointer;

	static GLint theCLsize;
	static GLenum theCLtype;
	static GLsizei theCLstride;
	static const GLvoid *theCLpointer;

	static GLint theSCIx;
	static GLint theSCIy;
	static GLsizei theSCIwidth;
	static GLsizei theSCIheight;

	static GLfloat theLINEwidth;
#endif
};
/*****************************************************************************/