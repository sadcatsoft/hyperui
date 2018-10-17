#pragma once

#define  _USE_MATH_DEFINES

#include <math.h>
#include <float.h>
#include <string.h>
#include <limits.h>

// STL
#include <assert.h>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <list>
#include <algorithm>
#include <tuple>

#if defined(WINDOWS) || defined(LINUX)
	#include <time.h>
#elif defined(MAC_BUILD) || defined(IOS_BUILD)
	#import <mach/mach_time.h>
#endif

using namespace std;
/*****************************************************************************/
#define FLOAT_TYPE		float
#define FLOAT_TYPE_MIN	FLT_MIN
#define FLOAT_TYPE_MAX	FLT_MAX
#define FLOAT_EPSILON	1e-6
#define DOUBLE_EPSILON	1e-14
#define DOUBLE_TYPE_MAX	DBL_MAX
#define HUGE_SIZE_TYPE	unsigned long long
typedef long long UNIQUEID_TYPE;

#define SECONDS_PER_DAY		(24.0*60.0*60.0)

#define FIXED_TIMEZONE		"EST"

#define TRUE_VALUE							"True"
#define FALSE_VALUE							"False"

#ifdef WINDOWS
	typedef __int64 int64;
	typedef unsigned __int64 uint64;
#else
	typedef int64_t int64;
	typedef uint64_t uint64;
#endif

#define SIZE_T_MAX ((size_t) -1)

// Note: if we ever change this, ResourceItem::getAsLong()
// may need looking at (and setAsLong(), too).
typedef long long GTIME;
typedef double UTC_TYPE;

typedef int PropertyType;
typedef int ResourceType;
/*****************************************************************************/
#ifdef USE_UNICODE_STRINGS
	typedef wstring STRING_TYPE;
	typedef wchar_t CHAR_TYPE;
	typedef wchar_t UNSIGNED_CHAR_TYPE;

	#define STR_LIT(x)		TEXT(x)
	#define EMPTY_STRING	TEXT("")

	#define STRLEN		wcslen
	#define STRCMP		wcscmp
	#define STRSTR		wcsstr
	#define STRCHR		wcschr
	#define STRRCHR		wcsrchr
	#define STRTOL		wcstol
	#define STRICMP		_wcsicmp
	#define STRNCMP		wcsncmp
	#define ISDIGIT		iswdigit
	#define ISPRINT		iswprint
	#define ISALPHA		iswalpha
	#define ATOF		_wtof
	#define ATOL		_wtol
	#define ATOI		_wtoi
	#define TOLOWER		::towlower
	#define TOUPPER		::towupper
	#define SPRINTF		swprintf_s
	#define F_FOPEN		_wfopen
	#define FPRINTF		fwprintf
#else
	typedef string STRING_TYPE;
	typedef char CHAR_TYPE;
	typedef unsigned char UNSIGNED_CHAR_TYPE;

	#define STR_LIT(x)		x
	#define EMPTY_STRING	""

	#define STRLEN		strlen
	#define STRCMP		strcmp
	#define STRSTR		strstr
	#define STRCHR		strchr
	#define STRRCHR		strrchr
	#define STRTOL		strtol
	#define STRICMP		_stricmp
	#define STRNCMP		strncmp
	#define ISDIGIT		isdigit
	#define ISPRINT		isprint
	#define ISALPHA		isalpha
	#define ATOF		atof
	#define ATOL		atol
	#define ATOI		atoi
	#define TOLOWER		::tolower
	#define TOUPPER		::toupper
	#define F_FOPEN		fopen
	#define FPRINTF		fprintf

#if !defined(WINDOWS)
	#define SPRINTF		snprintf
#else
	#define SPRINTF		sprintf_s
#endif

#endif

#if defined(WINDOWS)
	#define EMPLACE_BACK	emplace_back
#else
	#define EMPLACE_BACK	push_back
#endif

#ifndef WINDOWS
#ifdef _DEBUG
	void _ASSERT(bool bArg);
#else
	#define _ASSERT(x)
#endif
#endif

#if defined(_DEBUG) || defined(_DEBUG)
	#define IS_OF_TYPE(x)				((STRCMP(this->getStringProp(PropertyId), (x)) == 0) || (this->doesPropertyExist(PropertyOldId) && STRCMP(this->getStringProp(PropertyOldId), (x)) == 0))
	#define IS_OBJ_OF_TYPE(obj, x)		((STRCMP((obj)->getStringProp(PropertyId), (x)) == 0) || ((obj)->doesPropertyExist(PropertyOldId) && STRCMP((obj)->getStringProp(PropertyOldId), (x)) == 0))
#else
	#define IS_OF_TYPE(x)
	#define IS_OBJ_OF_TYPE(obj, x)
#endif

#define ELSE_ASSERT					else { _ASSERT(0); }
#define ELSE_ASSERT_RETURN			else { _ASSERT(0); return; }
#define ELSE_ASSERT_RETURN_NULL		else { _ASSERT(0); return NULL; }
#define ASSERT_CONTINUE				{ _ASSERT(0); continue; }
#define ASSERT_BREAK				{ _ASSERT(0); break; }
#define ASSERT_RETURN				{ _ASSERT(0); return; }
#define ASSERT_RETURN_NULL			{ _ASSERT(0); return NULL; }
#define ASSERT_RETURN_FALSE			{ _ASSERT(0); return false; }
#define ASSERT_RETURN_TRUE			{ _ASSERT(0); return true; }

#ifdef _DEBUG
	#define STATIC_ASSERT(condition) ((void)sizeof(char[1 - 2*!(condition)]))
#else
	#define STATIC_ASSERT(condition) 
#endif

#define NEWLINE_SEPARATORS "\r\n"

#define SAVE_LONG_AS_STRING_SYMBOL		'L'
#define SAVE_DOUBLE_AS_STRING_SYMBOL	'D'

#ifdef WINDOWS
	#define FOLDER_SEP			"\\"
	#define FOLDER_SEP_CHAR		'\\'
#else
	#define FOLDER_SEP			"/"
	#define FOLDER_SEP_CHAR		'/'
#endif

#define IS_VALID_STRING(x)					((x != NULL) && (STRLEN(x) > 0))
#define IS_VALID_STRING_AND_NOT_NONE(x)		((x != NULL) && (STRLEN(x) > 0) && (STRCMP((x), PROPERTY_NONE) != 0))
#define IS_STRING_EQUAL(x, y)				(STRCMP((x), (y)) == 0)

#define GET_ARRAY_ELEM(x, y, fArray, width) ((fArray)[(((y)*(width)) + (x))])
#define SET_ARRAY_ELEM(x, y, fArray, width, fValue) (((fArray)[(((y)*(width)) + (x))]) = (fValue))

#define FORWARD_DECLARE_TEMPOLATE(templateName, templateArgument, finalTypeName)			template < class TYPE > class templateName;\
																							typedef templateName<templateArgument> finalTypeName;

#if defined(WINDOWS) || defined(LINUX)
	#define RANDOM_INT_BETWEEN_INCLUSIVE(nMin, nMax)	((int)(((FLOAT_TYPE)rand()/(FLOAT_TYPE)RAND_MAX)*(FLOAT_TYPE)((nMax) - (nMin)) + 0.5 + (nMin)))
	#define RANDOM_ULONG_BETWEEN_INCLUSIVE(nMin, nMax)	((unsigned long)(((double)rand()/(double)RAND_MAX)*(double)((nMax) - (nMin)) + 0.5 + (nMin)))
	#define RANDOM_FLOAT_BETWEEN_INCLUSIVE(nMin, nMax)	((FLOAT_TYPE)(((FLOAT_TYPE)rand()/(FLOAT_TYPE)RAND_MAX)*(FLOAT_TYPE)((nMax) - (nMin)) + (nMin)))
	#define RANDOM_TIME_BETWEEN_INCLUSIVE(nMin, nMax)	((GTIME) (((FLOAT_TYPE)rand()/(FLOAT_TYPE)RAND_MAX)*(FLOAT_TYPE)((nMax) - (nMin)) + 0.5 + (nMin)))
	#define RAND_CALL()									rand()
#else
	#define INT32_MAX        2147483647
	#define RANDOM_INT_BETWEEN_INCLUSIVE(nMin, nMax)	((int)(((FLOAT_TYPE)random()/(FLOAT_TYPE)INT32_MAX)*(FLOAT_TYPE)(nMax - nMin) + 0.5 + (nMin)))
	#define RANDOM_ULONG_BETWEEN_INCLUSIVE(nMin, nMax)	((unsigned long)(((double)random()/(double)INT32_MAX)*(double)(nMax - nMin) + 0.5 + (nMin)))
	#define RANDOM_FLOAT_BETWEEN_INCLUSIVE(nMin, nMax)	((FLOAT_TYPE)(((FLOAT_TYPE)random()/(FLOAT_TYPE)INT32_MAX)*(FLOAT_TYPE)(nMax - nMin) + (nMin)))
	#define RANDOM_TIME_BETWEEN_INCLUSIVE(nMin, nMax)	((GTIME)(((FLOAT_TYPE)random()/(FLOAT_TYPE)INT32_MAX)*(FLOAT_TYPE)(nMax - nMin) + 0.5 + (nMin)))
	#define RAND_CALL()									random()
#endif

#define GET_PROBABILITY() (RANDOM_FLOAT_BETWEEN_INCLUSIVE(0, 1))
/*****************************************************************************/
// Debugging and profiling
#ifdef WIN32
	#define START_TIMING()			double dStart, dFinish, dDur; dStart = clock();
	#define END_TIMING(text)		dFinish = clock(); dDur = (double)(dFinish - dStart)/(double)CLOCKS_PER_SEC; char pcsBuff[1024]; const char* pcsTemp = text; sprintf(pcsBuff, "%s %.2fs\n", pcsTemp, dDur); OutputDebugStringA(pcsBuff);
#else
	#define START_TIMING()
	#define END_TIMING(text)
#endif

#define START_TIMING_NO_OUTPUT()		double dStart, dFinish, dDur; dStart = clock();
#define END_TIMING_NO_OUTPUT()			dFinish = clock(); dDur = (double)(dFinish - dStart)/(double)CLOCKS_PER_SEC;

#define START_TIMING_TO_FILE()			double dStart, dFinish, dDur; dStart = clock();
#define END_TIMING_TO_FILE(filename, formatString)	{ dFinish = clock(); dDur = (double)(dFinish - dStart)/(double)CLOCKS_PER_SEC;  FILE* fDbTimingOut = fopen(filename, "a"); fprintf(fDbTimingOut, formatString, dDur); fclose(fDbTimingOut); }

#define OUTPUT_TO_FILE_RELEASE(pcsDbFileName, ...) \
	\
{ \
	FILE *outDb; \
	outDb = fopen(pcsDbFileName, "a"); \
	if(outDb) \
	{ fprintf(outDb, __VA_ARGS__); \
	fclose(outDb); }\
}
/*****************************************************************************/
// Generic properties
#define	PROPERTY_NONE						"__none__"
// The difference here is the __none__ is a special string to indicate
// everywhere that the value is none. The empty property can be used as 
// a flag instead to say that the property is empty. It carries no special
// meaning.
#define	PROPERTY_EMPTY						"__empty__"
/*****************************************************************************/
namespace HyperCore
{
/*****************************************************************************/
// Note that actual dynamic casts are way, way too slow, so instead, we use the regular cast
// where we know that the result is always going to be of certain type.
template < class TYPE > 
inline TYPE FAST_CAST(void* pObject)
{ 
	return (TYPE)pObject; 
}

template < class TYPE > 
inline const TYPE CONST_FAST_CAST(const void* pObject)
{ 
	return (const TYPE)pObject; 
}

class HYPERCORE_API IBaseObject;
template < class TYPE > inline TYPE* as(IBaseObject* pObj) { return dynamic_cast<TYPE*>(pObj); }
template < class TYPE > inline const TYPE* as(const IBaseObject* pObj) { return dynamic_cast<const TYPE*>(pObj); }
/*****************************************************************************/
typedef tuple<string, string> TStringStringTuple;

typedef list < string > TStringList;

typedef vector < int > TIntVector;
typedef vector < string > TStringVector;
typedef vector < const char* > TCharPtrVector;
typedef vector < FLOAT_TYPE> TFloatVector;
typedef vector < string* > TStringPtrVector;
typedef vector < bool > TBoolVector;
typedef vector < TStringStringTuple > TStringStringTupleVector;

typedef set < int > TIntSet;
typedef set < long long > TLongLongSet;
typedef set < string > TStringSet;

typedef map < string, string > TStringStringMap;
typedef map < string, int > TStringIntMap;
typedef map < string, bool > TStringBoolMap;
typedef map < string, TStringVector > TStringStringVectorMap;
/*****************************************************************************/
};
