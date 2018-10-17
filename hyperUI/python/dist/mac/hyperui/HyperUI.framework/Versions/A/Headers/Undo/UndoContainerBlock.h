#pragma once

/*****************************************************************************/
class HYPERUI_API UndoContainerBlock : public UndoBlock
{
public:

	UndoContainerBlock(UndoBlock* pParent, UndoManager* pParentManager, const char* pcsBlockTitle, const char* pcsContainerId);
	virtual ~UndoContainerBlock();

	virtual const char* getType();

private:

	string myContainerId;
};
/*****************************************************************************/