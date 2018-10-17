#pragma once

/*****************************************************************************/
typedef	IExpression* (*ExpressionConstructorType)();
typedef	IExpressionContext* (*ExpressionContextConstructorType)();

struct ExpressionAllocatorEntry
{
	ExpressionConstructorType myConstructor;
	STRING_TYPE myType;
};
typedef map < STRING_TYPE, ExpressionAllocatorEntry > TStringExpressionAllocatorMap;
/*****************************************************************************/
struct ExpressionContextAllocatorEntry
{
	ExpressionContextConstructorType myConstructor;
	STRING_TYPE myType;
};
typedef map < STRING_TYPE, ExpressionContextAllocatorEntry > TStringExpressionContextAllocatorMap;
/*****************************************************************************/
class ExpressionTypeManager
{
public:
	HYPERCORE_API static ExpressionTypeManager* getInstance();

	HYPERCORE_API void registerType(const CHAR_TYPE* pcsType, ExpressionConstructorType pExpressionConstructor, const CHAR_TYPE* pcsContextType, ExpressionContextConstructorType pContextConstructor);

	// Note that the string may not be just the type itself, but is likely to be
	// a string *starting* with the type. Or not.
	HYPERCORE_API IExpressionContext* createContextFrom(const CHAR_TYPE* pcsString);
	HYPERCORE_API IExpression* createExpressionFrom(const CHAR_TYPE* pcsString);

	HYPERCORE_API const CHAR_TYPE* getExpressionType(const CHAR_TYPE* pcsString);

private:
	ExpressionTypeManager();

	ExpressionAllocatorEntry* findExpressionTypeEntry(const CHAR_TYPE* pcsString);

private:
	static ExpressionTypeManager* theInstance;

	TStringExpressionAllocatorMap myExpressionTypes;
	TStringExpressionContextAllocatorMap myContextTypes;

	int myMaxExpressionTypeStringLength;
};
/*****************************************************************************/