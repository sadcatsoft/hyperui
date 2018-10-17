#include "stdafx.h"
#include "PyUITableCellElement.h"

namespace PythonAPI
{
/*****************************************************************************/
UITableCellElement::UITableCellElement()
{

}
/*****************************************************************************/
UITableCellElement::UITableCellElement(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping)
	: UIElement(idWindow, pcsElemId, idMapping)
{

}
/*****************************************************************************/
UITableCellElement::~UITableCellElement()
{

}
/*****************************************************************************/
int UITableCellElement::getCol()
{
	HyperUI::UITableCellElement* pCellElem = dynamic_cast<HyperUI::UITableCellElement*>(this->getNativeElement());
	return pCellElem->getCol();
}
/*****************************************************************************/
int UITableCellElement::getRow()
{
	HyperUI::UITableCellElement* pCellElem = dynamic_cast<HyperUI::UITableCellElement*>(this->getNativeElement());
	return pCellElem->getRow();
}
/*****************************************************************************/
};