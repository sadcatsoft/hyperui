#include "stdafx.h"
#include "PyUITableElement.h"

namespace PythonAPI
{
/*****************************************************************************/
UITableElement::UITableElement()
{

}
/*****************************************************************************/
UITableElement::UITableElement(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping)
	: UIElement(idWindow, pcsElemId, idMapping)
{

}
/*****************************************************************************/
UITableElement::~UITableElement()
{

}
/*****************************************************************************/
void UITableElement::selectRow(int iRow)
{
	HyperUI::UITableElement* pTableElem = dynamic_cast<HyperUI::UITableElement*>(this->getNativeElement());
	pTableElem->selectRow(iRow);
}
/*****************************************************************************/
};