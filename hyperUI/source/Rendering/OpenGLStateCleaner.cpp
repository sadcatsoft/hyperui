#include "stdafx.h"

namespace HyperUI
{
#if !defined(DIRECTX_PIPELINE)
GLint OpenGLStateCleaner::theVPsize = 0;
GLenum OpenGLStateCleaner::theVPtype = 0;
GLsizei OpenGLStateCleaner::theVPstride = 0;
const GLvoid * OpenGLStateCleaner::theVPpointer = 0;

GLint OpenGLStateCleaner::theTXsize = 0;
GLenum OpenGLStateCleaner::theTXtype = 0;
GLsizei OpenGLStateCleaner::theTXstride = 0;
const GLvoid * OpenGLStateCleaner::theTXpointer = 0;

GLint OpenGLStateCleaner::theCLsize = 0;
GLenum OpenGLStateCleaner::theCLtype = 0;
GLsizei OpenGLStateCleaner::theCLstride = 0;
const GLvoid * OpenGLStateCleaner::theCLpointer = 0;

GLint OpenGLStateCleaner::theSCIx = -1;
GLint OpenGLStateCleaner::theSCIy = -1;
GLsizei OpenGLStateCleaner::theSCIwidth = 0;
GLsizei OpenGLStateCleaner::theSCIheight = 0;

GLfloat OpenGLStateCleaner::theLINEwidth = -1.0;
#endif
};