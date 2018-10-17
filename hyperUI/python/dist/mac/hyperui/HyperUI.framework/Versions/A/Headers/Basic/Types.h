#pragma once

#if defined(WINDOWS) || defined(DIRECTX_PIPELINE)
	#if defined(DIRECTX_PIPELINE)
		#include <wrl.h>
		#include <d3d11_1.h>
		#include <dxgi1_2.h>
	#else
		#include <windows.h>
		#ifdef USE_OPENGL2
				#include <EGL/egl.h>
				#include <GLES2/gl2.h>
		#else
				#include <GL/gl.h>
				#include <GL/glu.h>
		#endif
	#endif
#elif defined(LINUX)
        #include <GL/gl.h>
        #include <GL/glut.h>
        #include <GL/freeglut.h>
#else
	#ifdef MAC_BUILD
 		#ifdef USE_OPENGL2
 			#import <OpenGL/gl3.h>
// 			#import <OpenGLES/ES2/glext.h>
 		#else
			#import <OpenGL/gl.h>
			#import <OpenGL/glext.h>
		#endif
 	#else
		#ifdef USE_OPENGL2
			#import <OpenGLES/ES2/gl.h>
			#import <OpenGLES/ES2/glext.h>
		#else
			#import <OpenGLES/ES1/gl.h>
			#import <OpenGLES/ES1/glext.h>
		#endif
	#endif
#endif


#ifdef DIRECTX_PIPELINE
	typedef float GLfloat;
	typedef unsigned int GLuint;
	typedef int GLint;
	typedef short GLshort;
	typedef unsigned int     GLenum;
	typedef int GLsizei;
	typedef void GLvoid;
	#define GL_TRIANGLES                      0x0004
#endif

#ifdef USE_OPENGL2
	#define GL_COLOR_TYPE		float
#else
	#define GL_COLOR_TYPE		unsigned char
#endif

// Return type for the function manager
#ifdef DIRECTX_PIPELINE
	#define TX_MAN_RETURN_TYPE		ID3D11ShaderResourceView**
	#define INDEX_BUFFER_FORMAT		DXGI_FORMAT_R16_UINT
	#define INDEX_BUFFER_TYPE		unsigned short

#else
	#define TX_MAN_RETURN_TYPE		GLuint
#endif

#ifdef USE_OPENGL2
	const int g_iNumVertexComponents2D = 4;
	const int g_iNumVertexComponents3D = 4;
#else
	const int g_iNumVertexComponents2D = 2;
	const int g_iNumVertexComponents3D = 3;
#endif

/*****************************************************************************/
enum BezierPointType
{
	BezierPointStart = 0,
	BezierPointControl1 = 1,
	BezierPointControl2 = 2,
	BezierPointEnd = 3,

	BezierPointLastPlaceholder
};

const char* const g_pcsBezierPointTypeStringsShort[BezierPointLastPlaceholder + 1] = 
{
	"s",
	"c1",
	"c2",
	"e",
	""
};
/*****************************************************************************/
enum ClockType
{
	ClockUniversal = 0,
	ClockMainGame,
	ClockUiPrimary,
	ClockUiSecondary,

	ClockLastPlaceholder
};
/*****************************************************************************/
enum CurveType
{
	CurveLinear1D = 0,
	CurveLinear2D,
	CurveBezier2D,

	CurveLastPlaceholder
};

const char* const g_pcsCurveTypes[CurveLastPlaceholder] = 
{
	"curveLinear1D",
	"curveLinear2D",
	"curveBezier2D",
};
/*****************************************************************************/
enum BlendModeType
{
	BlendModeNormal = 0,
	BlendModeAdditive,
	BlendModeText,
	BlendModeDiscolour,

	BlendModeLastPlaceholder
};

const char* const g_pcsBlendModes[BlendModeLastPlaceholder + 1] =
{
	"bmNormal",
	"bmAdditive",
	"bmText",
	"bmDiscolour",

	0
};
/*****************************************************************************/
enum HorAlignType
{
	HorAlignLeft = 0,
	HorAlignCenter,
	HorAlignRight
};

const char* const g_pcsHorAlignStrings[4] =
{
	"Left",
	"Center",
	"Right",

	0
};
/*****************************************************************************/
enum VertAlignType
{
	VertAlignTop = 0,
	VertAlignCenter,
	VertAlignBottom
};

const char* const g_pcsVertAlignStrings[4] =
{
	"Top",
	"Center",
	"Bottom",

	0
};
/*****************************************************************************/
enum TextureSamplingType
{
	TextureSamplingNearestNeighbour = 0,
	TextureSamplingLinear,

	TextureSamplingLastPlaceholder
};
/*****************************************************************************/
enum ChannelType
{
	ChannelRed			= 0x0001,
	ChannelGreen		= 0x0002,
	ChannelBlue			= 0x0004,
	ChannelIntensity	= 0x0008,
	ChannelAlpha		= 0x0010,
	ChannelCustom		= 0x0020,

	ChannelLastPlaceholder
};

#define ChannelsRGB			(ChannelRed | ChannelGreen | ChannelBlue)
#define ChannelsRGBA		(ChannelRed | ChannelGreen | ChannelBlue | ChannelAlpha)

const char* const g_pcsChannelStrings[6] =
{
	"Red",
	"Green",
	"Blue",
	"Intensity",
	"Alpha",
	"Custom",
};

const char* const g_pcsChannelIdStrings[6] =
{
	"idRed",
	"idGreen",
	"idBlue",
	"idRGB",
	"idAlpha",
	"idCustom",
};
/*****************************************************************************/
enum ShaderSetType
{
	ShaderSetFlatColor = 0,
	ShaderSetFlatTexture,
	ShaderSetLighted,		// Note that everything since this config on assumes it uses lights...
	ShaderSetLightedNormalMapping,

	ShaderSetLastPlaceholder
};
/*****************************************************************************/
enum MouseButtonType
{
	MouseButtonNone = 0,
	MouseButtonLeft,
	MouseButtonRight
};
/*****************************************************************************/
enum DirectionType
{
	DirectionHorizontal = 0,
	DirectionVertical
};
/*****************************************************************************/
enum AcceptType
{
	AcceptNone = 0,
	AcceptContain,
	AcceptBefore,
	AcceptAfter,
	AcceptLastGlobal,
};
/*****************************************************************************/
enum UiElemType
{
	UiElemStatic = 0,
	UiElemButton,
	UiElemTable,
	UiElemHorizontalTable,
	UiElemTableCell,
	UiElemProgress,
	UiElemSlider,
	UiElemZoomWindow,
	UiElemTextField,
	UiElemExpressionTextField,
	UiElemExpressionEditTextField,
	UiElemMultilineTextField,
	UiElemSolidColor,
	UiElemCheckbox,
	UiElemMenu,
	UiElemSplitter,
	UiElemTabWindow,
	UiElemGrid,
	UiElemFixedGrid,
	UiElemColorSwatch,
	UiElemImageDropdown,
	UiElemDropdown,
	UiElemDropdownTextPart,
	UiElemDropdownMenu,
	UiElemUnitedNumeric,
	UiElemGradientEdit,
	UiElemRoundSlider,
	UiElemRoundProgress,
	UiElemPopup,
	UiElemThumbnailGrid,
	UiElemHistogram,
	UiElemCurveEditor,
	UiElemSlidingStopsEdit,
	UiElemColorList,
	UiElemRichText,
	UiElemColorPalette,
	UiElemRecentColorsPalette,
	UiElemColorPicker,
	UiElemUser,

	UiElemLastPlaceholder
};


const char* const g_pcsUiElemTypes[UiElemLastPlaceholder] =
{
	"static",
	"button",
	"table",
	"horizontalTable",
	"tableCell",
	"progress",
	"slider",
	"zoomWindow",
	"textField",
	"expressionTextField",
	"expressionEditTextField",
	"multilineTextField",
	"solidColor",
	"checkbox",
	"menu",
	"splitter",
	"tabWindow",
	"grid",
	"fixedGrid",
	"colorSwatch",
	"imageDropdown",
	"dropdown",
	"dropdownTextPart",
	"dropdownMenu",
	"unitedNumeric",
	"gradientEdit",
	"roundSlider",
	"roundProgress",
	"popup",
	"thumbnailGrid",
	"histogram",
	"curveEditor",
	"slidingStopsEdit",
	"colorList",
	"richText",
	"colorPalette",
	"recentColorsPalette",
	"colorPicker",
	"user"		// really n/a, but just so we don't step over the bounds.
};
/*****************************************************************************/
enum CursorType
{
	CursorArrow = 0,
	CursorWait,
	CursorHorSize,
	CursorVertSize,
	CursorForbidden,
	CursorCrosshair,
	CursorIBeam,
	CursorEyeDropper,
	CursorPan,
	CursorFill,
	CursorTarget,
	CursorMagicWand,
	CursorZoom,

	CursorSizeTopLeftCorner,
	CursorSizeBottomRightCorner,
	CursorSizeTopRightCorner,
	CursorSizeBottomLeftCorner,

	CursorLastPlaceholder,
	CursorInvalidUnset
};

const char* const g_pcsCursorTypes[CursorLastPlaceholder + 1] =
{
	"arrow",
	"wait",
	"horsizing",
	"vertsizing",
	"forbidden",
	"crosshair",
	"ibeam",
	"eyedropper",
	"pan",
	"fill",
	"target",
	"magicwand",
	"zoom",
	"sizeTopLeft",
	"sizeBottomRight",
	"sizeTopRight",
	"sizeBottomLeft",
	0
};
/*****************************************************************************/
enum AlignType
{
	AlignFar = 0,
	AlignCenter,
	AlignNear
};

const char* const g_pcsGenericAlignStrings[4] =
{
	"Far",
	"Center",
	"Near",

	0
};
/*****************************************************************************/
enum BackgroundModeType
{
	BackgroundModeNone = 0,
	BackgroundModeFill,
	BackgroundModeScale,
};

const char* const g_pcsBackgroundModeTypeStrings[] =
{
	PROPERTY_NONE,
	"fill",
	"scale",

	0
};
/*****************************************************************************/
enum AutolayoutType
{
	AutolayoutNone = 0,
	AutolayoutRow,
	AutolayoutColumn,
};

const char* const g_pcsAutolayoutTypeStrings[] =
{
	PROPERTY_NONE,
	"row",
	"column",

	0
};
/*****************************************************************************/
enum TabletCursorType
{
	TabletCursorDraw = 0,
	TabletCursorErase
};
/*****************************************************************************/
enum FontStyleType
{
	FontStyleNormal = 0,
	FontStyleBold,
	FontStyleItalic,
	FontStyleBoldItalic,

	FontStyleLastPlaceholder,
};
/*****************************************************************************/
enum ResultCodeType
{
	ResultCodeCancel = 0,
	ResultCodeOk,
	ResultCodeError
};
/********************************************************************************************/
enum KeyMeaningType
{
	KeyMeaningNone = 0,
	KeyMeaningRight,
	KeyMeaningLeft,
	KeyMeaningUp,
	KeyMeaningDown,
	KeyMeaningTab,
	KeyMeaningEnter,
	KeyMeaningDelete,
	KeyMeaningEscape,
	KeyMeaningUiAction,

	/*
	// More specific keys
	KeyMeaningPulseJump,
	KeyMeaningBomb,
	KeyMeaningHyperjump,
	KeyMeaningFormationChange,
	KeyMeaningPause,
	KeyMeaningShoot,
	*/

	KeyMeaningSpace,


	KeyMeaningLastPlaceholder
};
/*****************************************************************************/
// TODO: Move this
class HYPERUI_API IWindowBase
{
public:
	virtual ~IWindowBase() { }
	virtual BlendModeType setBlendMode(BlendModeType eNewMode) = 0;
	virtual BlendModeType getBlendMode() const = 0;
};
/*****************************************************************************/
#ifndef DIRECTX_PIPELINE
// Some bindings for easier migration...
inline void gClearDepth(float depth) 
{ 
#ifdef USE_OPENGL2
	glClearDepthf(depth); 
#else
#if defined(MAC_BUILD) || defined(WIN32) || defined(LINUX)
	glClearDepth(depth); 
#else
	glClearDepthf(depth); 
#endif
#endif
}
#endif
/*****************************************************************************/
#define MAX_DEEP_RENDER_STAGES		11

// If changing these, change them in OpenGL 2 shader, too.
#define NUM_FIXED_LIGHTS		1
#define MAX_CUSTOM_LIGHTS		2
#define NUM_TOTAL_LIGHTS (NUM_FIXED_LIGHTS + MAX_CUSTOM_LIGHTS)

// We store units in pixels because that way they don't depend on DPI
// and it non-procedurally changing...
#define DEFAULT_STORAGE_UNITS		UnitPixels
/********************************************************************************************/
// For glOrtho call
#ifdef ALLOW_3D
	#define ORTHO_DEPTH_LIMIT		500.0
#else
	#define ORTHO_DEPTH_LIMIT		10.0
#endif
