#include "stdafx.h"
#include "PyUIProgressElement.h"

namespace PythonAPI
{
/*****************************************************************************/
UIProgressElement::UIProgressElement()
{

}
/*****************************************************************************/
UIProgressElement::UIProgressElement(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping)
	: UIElement(idWindow, pcsElemId, idMapping)
{

}
/*****************************************************************************/
UIProgressElement::~UIProgressElement()
{

}
/*****************************************************************************/
void UIProgressElement::setMinProgress(FLOAT_TYPE fValue)
{
	HyperUI::UIProgressElement* pElem = dynamic_cast<HyperUI::UIProgressElement*>(this->getNativeElement());
	pElem->setMinProgress(fValue);
}
/*****************************************************************************/
void UIProgressElement::setMaxProgress(FLOAT_TYPE fValue)
{
	HyperUI::UIProgressElement* pElem = dynamic_cast<HyperUI::UIProgressElement*>(this->getNativeElement());
	pElem->setMaxProgress(fValue);
}
/*****************************************************************************/
void UIProgressElement::setProgress(FLOAT_TYPE fValue)
{
	HyperUI::UIProgressElement* pElem = dynamic_cast<HyperUI::UIProgressElement*>(this->getNativeElement());
	pElem->setProgress(fValue);
}
/*****************************************************************************/
FLOAT_TYPE UIProgressElement::getProgress() const
{
	HyperUI::UIProgressElement* pElem = dynamic_cast<HyperUI::UIProgressElement*>(this->getNativeElement());
	return pElem->getProgress();
}
/*****************************************************************************/
};