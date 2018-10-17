/*****************************************************************************/
// AlphaTreeNode
/*****************************************************************************/
template < class TYPE > 
AlphaTreeNode<TYPE>::AlphaTreeNode()
{
	myBlockMemId = -1;
	myParent = NULL;
	myChildren = NULL;
	myHaveContent = false;
}
/*****************************************************************************/
template < class TYPE > 
AlphaTreeNode<TYPE>::~AlphaTreeNode()
{
	clear();
	myParent = NULL;
}
/*****************************************************************************/
template < class TYPE > 
const TYPE* AlphaTreeNode<TYPE>::findSimple(const char* pcsKey) const
{
	// Avoid recursion for it slows us down...
	_ASSERT(pcsKey);
	int iOffset;
	const AlphaTreeNode<TYPE>* pCurrNode = this;
	do 
	{
		if(pcsKey[0] == 0)
		{
			// We've found the end of the key.
			if(pCurrNode->myHaveContent)
				return &pCurrNode->myContent;
			else
				return NULL;
		}
		
		if(!pCurrNode->myChildren)
			return NULL;
		else
		{
			iOffset = pCurrNode->getOffsetForChar(pcsKey[0]);
			pCurrNode = &pCurrNode->myChildren[iOffset];
			pcsKey++;
		}

	} while (1);
	
}
/*****************************************************************************/
template < class TYPE > 
const TYPE* AlphaTreeNode<TYPE>::findComplex(const char* pcsKey) const
{
	if(!pcsKey || pcsKey[0] == 0)
	{
		// We've found the end of the key.
		if(myHaveContent)
			return &myContent;
		else
			return NULL;
	}

	if(!myChildren)
		return NULL;

	if(pcsKey[0] == ATM_WILDCARD)
	{
		// Wildcard - any will do.
		TYPE* pLocRes;
		int iCurrChild, iNumChildren = NUM_SYMBOLS;
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			pLocRes = myChildren[iCurrChild].findComplex(pcsKey + 1);
			if(pLocRes)
				return pLocRes;
		}

		return NULL;
	}
	else
	{
		// Find the child with the right letter
		int iOffset = getOffsetForChar(pcsKey[0]);
		return myChildren[iOffset].findComplex(pcsKey + 1);
	}
}
/*****************************************************************************/
template < class TYPE >
int findInVector(vector < TYPE >& vec_in, TYPE& strToFind )
{
	int iCurr, iNum = vec_in.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(vec_in[iCurr] == strToFind)
			return iCurr;
	}

	return -1;
}

template < class TYPE > 
void AlphaTreeNode<TYPE>::findNPossibleKeys(const char* pcsPrefix, int iMaxResults, string& strKeySoFar, vector < string >& vecResOut )
{
	if((int)vecResOut.size() >= iMaxResults)
		return;

	bool bForceAllChildrenTraversal = false;
	if(!pcsPrefix || pcsPrefix[0] == 0)
	{
		// We've found the end of the key.
		// However, since we need 
		if(myHaveContent && !pcsPrefix)
		{
			// NO! Push the current complete path string on.
			// This means we have to accumulate it...
			// Note: If we ever need both keys and values for all
			// possibilities, we could just use a map as a result
			// container instead of the vector.
			vecResOut.push_back(strKeySoFar);
			
			// vecResOut.push_back(&myContent);
		}

		// However, because we're doing an or,
		// we essentially treat this as a wildcard.
		if((int)vecResOut.size() < iMaxResults)
			bForceAllChildrenTraversal = true;
	}

	if(!myChildren)
		return;

	char pcsTempBuff[2] = { 0, 0 };
	string strNextKey;
	if(bForceAllChildrenTraversal || (pcsPrefix && pcsPrefix[0] == ATM_WILDCARD))
	{
		// Wildcard - any will do.
		int iCurrChild, iNumChildren = NUM_SYMBOLS;
		const char* pcsKeyToPass = NULL;
		if(pcsPrefix && pcsPrefix[0])
			pcsKeyToPass = pcsPrefix + 1;
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			pcsTempBuff[0] = iCurrChild + 'A';
			strNextKey = strKeySoFar + pcsTempBuff;

			myChildren[iCurrChild].findNPossibleKeys(pcsKeyToPass, iMaxResults, strNextKey, vecResOut);
			if((int)vecResOut.size() >= iMaxResults)
				return;
		}
	}
	else if(pcsPrefix && pcsPrefix[0])
	{
		// Find the child with the right letter
		int iOffset = getOffsetForChar(pcsPrefix[0]);
		pcsTempBuff[0] = pcsPrefix[0];
		strNextKey = strKeySoFar + pcsTempBuff;
		return myChildren[iOffset].findNPossibleKeys(pcsPrefix + 1, iMaxResults, strNextKey, vecResOut);
	}
}
/*****************************************************************************/
template < class TYPE > 
TYPE* AlphaTreeNode<TYPE>::findPrefix(const char* pcsKey, bool& bIsCompleteKey, bool& bIsPrefix)
{
	if(!pcsKey || pcsKey[0] == 0)
	{
		// We've found the end of the key.
		if(myHaveContent)
		{
			bIsCompleteKey = true;
			if(myChildren)
				bIsPrefix = true;
			return &myContent;
		}
		else if(myChildren)
		{
			bIsPrefix = true;
			return getNonNullValue();
		}
		else
			return NULL;
	}

	if(!myChildren)
		return NULL;

	// Find the child with the right letter
	if(pcsKey[0] == ATM_WILDCARD)
	{
		// Wildcard - any will do.
		//bIsCompleteKey = false;
		TYPE* pCompleteRes = NULL, *pPrefixRes = NULL;
		TYPE* pLocRes;
		bool bLocIsComplete, bLocIsPrefix;
		int iCurrChild, iNumChildren = NUM_SYMBOLS;
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			bLocIsComplete = false;
			bLocIsPrefix = false;
			pLocRes = myChildren[iCurrChild].findPrefix(pcsKey + 1, bLocIsComplete, bLocIsPrefix);
			_ASSERT(!pLocRes || (bLocIsComplete || bLocIsPrefix));
			if(pLocRes)
			{
				if(bLocIsComplete)
				{
					if(!pCompleteRes)
						pCompleteRes = pLocRes;
					bIsCompleteKey = true;
				}
				else if(bLocIsPrefix)
				{
					if(!pPrefixRes)
						pPrefixRes = pLocRes;
					bIsPrefix = true;
				}
			}

			if(pPrefixRes && pCompleteRes)
				break;
		}

		if(pCompleteRes)
			return pCompleteRes;
		else
			return pPrefixRes;
	}
	else
	{
		int iOffset = getOffsetForChar(pcsKey[0]);
		return myChildren[iOffset].findPrefix(pcsKey + 1, bIsCompleteKey, bIsPrefix);
	}
}
/*****************************************************************************/
template < class TYPE > 
void AlphaTreeNode<TYPE>::remove(const char* pcsKey, bool bDeleteAllChildrenUnderneathRecursive)
{
	if(!pcsKey || pcsKey[0] == 0)
	{
		// We've found the end of the key.
		myHaveContent = false;
		// Delete all children below us - this is in
		// case we're delete a wildcard key.
		// NOTE!!! We cannot just blindly delete everythign
		// below - for if, with no wildcards, we're deleting
		// "test" and we have elements such as "test1", "test2",
		// etc. that are supposed to remain there, we will
		// delete them with "test", as well, which is wrong!
		if(bDeleteAllChildrenUnderneathRecursive)
			clear();

		return;
	}

	if(!myChildren)
		return;

	if(pcsKey[0] == ATM_WILDCARD)
	{
		// Wildcard - delete all keys below.
		int iCurrChild, iNumChildren = NUM_SYMBOLS;
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
			myChildren[iCurrChild].remove(pcsKey + 1, true);
	}
	else
	{
		// Find the child with the right letter
		int iOffset = getOffsetForChar(pcsKey[0]);
		myChildren[iOffset].remove(pcsKey + 1, false);
	}
}
/*****************************************************************************/
template < class TYPE > 
void AlphaTreeNode<TYPE>::insert(const char* pcsKey, const TYPE& rValue)
{
	if(!pcsKey || pcsKey[0] == 0)
	{
		// We've found the leaf of the tree to put this into.
		myContent = rValue;
		myHaveContent = true;
		return;
	}
	
	if(!myChildren)
		createChildren();

	// Find the child with the right letter
	int iOffset = getOffsetForChar(pcsKey[0]);
	return myChildren[iOffset].insert(pcsKey + 1, rValue);
}
/*****************************************************************************/
template < class TYPE > 
void AlphaTreeNode<TYPE>::createChildren()
{
	_ASSERT(!myChildren);

	MemoryCacher< TArrayHolder<AlphaTreeNode<TYPE>, NUM_SYMBOLS > >* pCacher = myParent->getMemoryCacher();
	if(pCacher)
	{
		TArrayHolder<AlphaTreeNode<TYPE>, NUM_SYMBOLS >* pBlock = pCacher->getNewObject();
		myChildren = pBlock->myArray;
		myBlockMemId = pBlock->myMemId;
	}
	else
		myChildren = new AlphaTreeNode[NUM_SYMBOLS];

	int iCurrChild;
	for(iCurrChild = 0; iCurrChild < NUM_SYMBOLS; iCurrChild++)
		myChildren[iCurrChild].setParent(myParent);
}
/*****************************************************************************/
template < class TYPE > 
void AlphaTreeNode<TYPE>::clear()
{
	if(myChildren)
	{
		MemoryCacher< TArrayHolder<AlphaTreeNode<TYPE>, NUM_SYMBOLS > >* pCacher = myParent->getMemoryCacher();
		if(pCacher)
		{
			// In this case, we also need to recursively clear all the children first
			int iCurr;
			for(iCurr = 0; iCurr < NUM_SYMBOLS; iCurr++)
				myChildren[iCurr].clear();

			_ASSERT(myBlockMemId >= 0);
			pCacher->deleteObjectByMemoryIdDontCallOnDeallocate(myBlockMemId);
		}
		else
		{
			delete[] myChildren;
		}
		myChildren = NULL;
	}
	myBlockMemId = -1;
	myHaveContent = false;
}
/*****************************************************************************/
// AlphaTreeMap
/*****************************************************************************/
template < class TYPE > 
AlphaTreeMap<TYPE>::AlphaTreeMap(MemoryCacher< TArrayHolder<AlphaTreeNode<TYPE>, NUM_SYMBOLS > > *pOptNodeMemoryCacher)
{
	myMemoryCacher = pOptNodeMemoryCacher;
	myRoot.setParent(this);
}
/*****************************************************************************/
template < class TYPE > 
AlphaTreeMap<TYPE>::~AlphaTreeMap()
{

}
/*****************************************************************************/
template < class TYPE > 
TYPE* AlphaTreeMap<TYPE>::findPrefix(const char* pcsKey, bool& bIsCompleteKey, bool& bIsPrefix)
{
	bIsCompleteKey = false;
	bIsPrefix = false;
	return myRoot.findPrefix(pcsKey, bIsCompleteKey, bIsPrefix);
}
/*****************************************************************************/
template < class TYPE > 
void AlphaTreeMap<TYPE>::insertRef(const char* pcsKey, TYPE& rValue)
{
	if(!pcsKey || strlen(pcsKey) == 0)
		return;
	return myRoot.insert(pcsKey, rValue);
}
/*****************************************************************************/
template < class TYPE > 
void AlphaTreeMap<TYPE>::remove(const char* pcsKey)
{
	if(!pcsKey || strlen(pcsKey) == 0)
		return;
	return myRoot.remove(pcsKey, false);
}
/*****************************************************************************/
template < class TYPE > 
void AlphaTreeMap<TYPE>::insert(const char* pcsKey, const TYPE rValue)
{
	if(!pcsKey || strlen(pcsKey) == 0)
		return;
	return myRoot.insert(pcsKey, rValue);
}
/*****************************************************************************/
template < class TYPE > 
void AlphaTreeMap<TYPE>::findNPossibleKeys(const char* pcsPrefix, int iMaxResults, vector < string >& vecResOut )
{
	string strKeySoFar("");
	vecResOut.clear();
	myRoot.findNPossibleKeys(pcsPrefix, iMaxResults, strKeySoFar, vecResOut);
}
/*****************************************************************************/
template < class TYPE > 
void AlphaTreeMap<TYPE>::clear()
{
	myRoot.clear();
}
/*****************************************************************************/