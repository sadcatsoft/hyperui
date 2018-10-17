#include "stdafx.h"
#include "TopSplitterElement.h"

/*****************************************************************************/
TopSplitterElement::TopSplitterElement(UIPlane* pParentPlane)
	: UISplitterElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
TopSplitterElement::~TopSplitterElement()
{
	onDeallocated();
}
/*****************************************************************************/
void TopSplitterElement::onAllocated(IBaseObject* pData)
{
	UISplitterElement::onAllocated(pData);

	myElementTypeStrings.clear();

	myElementTypeStrings.push_back(std::make_tuple("Color Palette", "exampleColorPalette"));

	myElementTypeStrings.push_back(std::make_tuple("Auto Dialog", "exampleAutoDialog"));
	myElementTypeStrings.push_back(std::make_tuple("Horizontal Layout", "exampleHorizontalLayout"));
	myElementTypeStrings.push_back(std::make_tuple("Vertical Layout", "exampleVerticalLayout"));
	myElementTypeStrings.push_back(std::make_tuple("Grid Layout", "exampleGrid"));
	myElementTypeStrings.push_back(std::make_tuple("Fixed Grid", "exampleFixedGrid"));

	myElementTypeStrings.push_back(std::make_tuple("Rich Text Element", "exampleRichText"));
	myElementTypeStrings.push_back(std::make_tuple("Horizontal Table", "exampleHorTable"));
	myElementTypeStrings.push_back(std::make_tuple("Vertical Table", "exampleVertTable"));
	myElementTypeStrings.push_back(std::make_tuple("Single Line Input", "exampleStringInput"));
	myElementTypeStrings.push_back(std::make_tuple("Splitter", "exampleSplitter"));
	myElementTypeStrings.push_back(std::make_tuple("Menu", "exampleMenu"));
	myElementTypeStrings.push_back(std::make_tuple("Color", "exampleColor"));
	myElementTypeStrings.push_back(std::make_tuple("Multline Input", "exampleMultilineInput"));
	myElementTypeStrings.push_back(std::make_tuple("Dropdown", "exampleDropdown"));
	myElementTypeStrings.push_back(std::make_tuple("Numeric Slider", "exampleNumericSlider"));
	myElementTypeStrings.push_back(std::make_tuple("Knob", "exampleKnob"));
	myElementTypeStrings.push_back(std::make_tuple("Radio Button", "exampleRadioButton"));
	myElementTypeStrings.push_back(std::make_tuple("Checkbox", "exampleCheckbox"));
	myElementTypeStrings.push_back(std::make_tuple("Progress Bar", "exampleProgressBar"));
	myElementTypeStrings.push_back(std::make_tuple("Button", "exampleButton"));
	myElementTypeStrings.push_back(std::make_tuple("Static Element", "exampleStatic"));
}
/*****************************************************************************/
void TopSplitterElement::onDeallocated()
{
	UISplitterElement::onDeallocated();
}
/*****************************************************************************/
bool TopSplitterElement::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
	bool bDidHandle = false;
	if(strAction == UIA_TABLE_ROW_SELECTED)
	{
		const char* pcsTableId = pSourceElem->getId();
		if(IS_STRING_EQUAL(pcsTableId, "leftElemTypesTable"))
		{
			int iRowIndex = as<ResourceItem>(pData)->getNumProp(PropertyRow);
			TStringStringTuple pInfo = myElementTypeStrings[iRowIndex];

			this->getUIPlane()->setIsVisibleForAllWithTag("exampleElements", false);

			// Show element with ID:
			const char* pcsIdToShow = get<1>(pInfo).c_str();
			UIElement* pTargetElement = this->getUIPlane()->getElementById(pcsIdToShow);
			pTargetElement->setIsVisible(true);

			bDidHandle = true;
		}
	}

	return bDidHandle;
}
/*****************************************************************************/
int TopSplitterElement::getNumRows(UIElement* pCaller)
{
	if(IS_STRING_EQUAL(pCaller->getId(), "leftElemTypesTable"))
		return myElementTypeStrings.size();
	else if(IS_STRING_EQUAL(pCaller->getId(), "exampleHorTable"))
		return 1;
	else
		return 32;
}
/*****************************************************************************/
int TopSplitterElement::getNumColumns(UIElement* pCaller)
{
	if(IS_STRING_EQUAL(pCaller->getId(), "exampleHorTable"))
		return 32;
	else
		return 1;
}
/*****************************************************************************/
void TopSplitterElement::updateCell(UITableCellElement* pCell)
{
	if(IS_STRING_EQUAL(pCell->getParent()->getId(), "leftElemTypesTable"))
	{
		TStringStringTuple pInfo = myElementTypeStrings[pCell->getRow()];
		pCell->setTextForChild("defCellContent", get<0>(pInfo).c_str());
	}
	else
	{
		char pcsBuff[128];
		sprintf(pcsBuff, "Cell at (%d, %d)", pCell->getCol(), pCell->getRow());
		pCell->setTextForChild("defCellContent", pcsBuff);
	}
}
/*****************************************************************************/
void TopSplitterElement::onPostCreateRows(UITableElement* pCaller)
{
	if(IS_STRING_EQUAL(pCaller->getId(), "leftElemTypesTable"))
	{
		getUIPlane()->getElementById<UITableElement>("leftElemTypesTable")->selectRow(0);
		getUIPlane()->getElementById<UIProgressElement>("exampleProgressBar")->setProgress(0.25);
	}
}
/*****************************************************************************/
void TopSplitterElement::onTimerTick(GTIME lGlobalTime)
{
	UISplitterElement::onTimerTick(lGlobalTime);

	UIProgressElement* pProgress = getUIPlane()->getElementById<UIProgressElement>("exampleProgressBar");
	if(pProgress->getIsVisible())
	{
		FLOAT_TYPE fNewProgress = pProgress->getProgress() + 0.005;
		if(fNewProgress > 1.0)
			fNewProgress = 0.0;
		pProgress->setProgress(fNewProgress);
	}
}
/*****************************************************************************/
