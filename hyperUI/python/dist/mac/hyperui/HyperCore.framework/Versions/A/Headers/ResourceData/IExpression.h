#pragma once

#define EXPRESSION_PREFIX_SEP			":"
#define EXPRESSION_PREFIX_SEP_CHAR		':'

/*****************************************************************************/
class IExpressionContext
{
public:
	IExpressionContext(const CHAR_TYPE* pcsType) { myType = pcsType; }
	virtual ~IExpressionContext() { }
	virtual IExpressionContext* cloneSelf() const = 0;
	virtual void createCode(std::string& strOut) const = 0;

	inline const CHAR_TYPE* getType() const { return myType.c_str(); }

	HYPERCORE_API void loadFrom(const char* pcsBase);
	HYPERCORE_API void saveAppendToString(string& strInOut) const;

protected:

	virtual void loadFromSubclass(const char* pcsBase) = 0;
	virtual void saveAppendToStringSubclass(string& strInOut) const = 0;

private:

	STRING_TYPE myType;
};
/*****************************************************************************/
class IExpression : public IBaseObject
{
public:
	IExpression(const CHAR_TYPE* pcsType) { myIsDirty = true; myContext = NULL; myType = pcsType; }
	HYPERCORE_API virtual ~IExpression();

	HYPERCORE_API void updateWithText(const char* pcsText, bool bUpdateContext);

	HYPERCORE_API static IExpression* createExpressionFrom(const char* pcsText);
	HYPERCORE_API static void stripExpressionPrefix(string& strInOut);

	inline const CHAR_TYPE* getType() const { return myType.c_str(); }

	HYPERCORE_API const char* getResultAsString();
	HYPERCORE_API double getResultAsDoubleNumber();
	HYPERCORE_API FLOAT_TYPE getResultAsNumber();
	HYPERCORE_API bool getResultAsBool();

	HYPERCORE_API void copyFrom(const IExpression* pOther);
	HYPERCORE_API IExpression* cloneSelf() const;

	inline const char* getLastError() const { return myLastError.c_str(); }
	inline bool getHaveError() const { return myLastError.length() > 0; }

	const char* getRawExpressionContentsOnly() const { return myRawExpression.c_str(); }
	HYPERCORE_API void getRawExpressionWithPrefix(string& strOut) const;
	HYPERCORE_API bool isEqualTo(const IExpression* pOther) const;

	inline void setContext(IExpressionContext* pOwnedContext) { clearContext(); myContext = pOwnedContext; myIsDirty = true; }
	inline const IExpressionContext* getContext() const { return myContext; }
	HYPERCORE_API void clearContext();

	HYPERCORE_API void saveToString(string& strOut) const;

protected:
	virtual bool evaluate(const char* pcsStrIn, string& strOut) = 0;

	void ensureEvaluated();

private:

	// Don't forget to update copyFrom() when adding new members! 
	bool myIsDirty;
	string myRawExpression;
	string myLastResult;
	string myLastError;

	STRING_TYPE myType;

	// Optional evaluation context. Used, for example, to create a this
	// node function when evaluating node parms.
	IExpressionContext *myContext;
};
/*****************************************************************************/