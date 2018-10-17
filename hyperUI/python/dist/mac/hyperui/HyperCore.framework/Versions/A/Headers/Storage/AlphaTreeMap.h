#pragma once

#include <vector>

using namespace std;

#define ATM_START_SYMBOL	('-')
#define ATM_END_SYMBOL		('z')
#define NUM_SYMBOLS			( (ATM_END_SYMBOL) - (ATM_START_SYMBOL) + 1 )

#define ATM_WILDCARD	'*'

template < class TYPE > class AlphaTreeMap;

// The tree is now ALWAYS case-sensitive.
/*****************************************************************************/
template < class TYPE >
class AlphaTreeNode
{
public:
	AlphaTreeNode();
	~AlphaTreeNode();

	// If allowing finding prefix, we will return a non-NULL
	// value if a prefix of a keyword is in the map. However,
	// the value itself will be meaningless.
	TYPE* findPrefix(const char* pcsKey, bool& bIsCompleteKey, bool& bIsPrefix);
	const TYPE* findComplex(const char* pcsKey) const;
	const TYPE* findSimple(const char* pcsKey) const;
	void findNPossibleKeys(const char* pcsPrefix, int iMaxResults, string& strKeySoFar, vector < string >& vecResOut );
	void insert(const char* pcsKey, const TYPE& rValue);

	void remove(const char* pcsKey, bool bDeleteAllChildrenUnderneathRecursive);

	inline bool haveChildren() const { return myChildren != NULL; }

	void clear();

	inline void setParent(AlphaTreeMap<TYPE> *pParent) { myParent = pParent; }

private:

	inline TYPE* getNonNullValue() const { return (TYPE*)1; }
	inline int getOffsetForChar(char cChar) const { _ASSERT((cChar - ATM_START_SYMBOL) >= 0);  _ASSERT((cChar - ATM_START_SYMBOL) < (NUM_SYMBOLS)); return cChar - ATM_START_SYMBOL;  }
	void createChildren();

private:

	AlphaTreeNode* myChildren;
	TYPE myContent;
	bool myHaveContent;

	AlphaTreeMap<TYPE> *myParent;
	long myBlockMemId;
};
/*****************************************************************************/
template < class TYPE >
class AlphaTreeMap
{
public:
	AlphaTreeMap(MemoryCacher< TArrayHolder<AlphaTreeNode<TYPE>, NUM_SYMBOLS > > *pOptNodeMemoryCacher);
	~AlphaTreeMap();

	TYPE* findPrefix(const char* pcsKey, bool& bIsCompleteKey, bool& bIsPrefix);
	// Finds a key with possible wildcarts
	inline const TYPE* findComplex(const char* pcsKey) const { return myRoot.findComplex(pcsKey); }
	// Straight key, no wildcards
	inline const TYPE* findSimple(const char* pcsKey) const { return myRoot.findSimple(pcsKey); }
	inline TYPE* findSimple(const char* pcsKey) { return const_cast<TYPE*>(const_cast<const AlphaTreeNode<TYPE>&>(myRoot).findSimple(pcsKey)); }
	void findNPossibleKeys(const char* pcsPrefix, int iMaxResults, vector < string >& vecResOut );
	void insertRef(const char* pcsKey, TYPE& rValue);
	void insert(const char* pcsKey, const TYPE rValue);

	void remove(const char* pcsKey);

	inline bool isEmpty() const { return !myRoot.haveChildren(); }

	void clear();

	inline MemoryCacher< TArrayHolder<AlphaTreeNode<TYPE>, NUM_SYMBOLS > >* getMemoryCacher() { return myMemoryCacher; }

private:

	AlphaTreeNode<TYPE> myRoot;

	MemoryCacher< TArrayHolder<AlphaTreeNode<TYPE>, NUM_SYMBOLS > >* myMemoryCacher;
};
/*****************************************************************************/
#include "AlphaTreeMap.hpp"

#define DEFINE_ALPHA_TREE_MEMCACHER(TYPE) typedef MemoryCacher< TArrayHolder<AlphaTreeNode<TYPE>, NUM_SYMBOLS > > 
