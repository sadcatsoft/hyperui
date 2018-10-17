#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UndoContainerBlock::UndoContainerBlock(UndoBlock* pParent, UndoManager* pParentManager, const char* pcsBlockTitle, const char* pcsContainerId)
	: UndoBlock(pParent, pParentManager, pcsBlockTitle)
{
	myContainerId = pcsContainerId;
}
/*****************************************************************************/
UndoContainerBlock::~UndoContainerBlock()
{

}
/*****************************************************************************/
const char* UndoContainerBlock::getType()
{
	return myContainerId.c_str();
}
/*****************************************************************************/
};