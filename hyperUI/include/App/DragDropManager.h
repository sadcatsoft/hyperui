/*****************************************************************************

Disclaimer: This software is supplied to you by Sad Cat Software
("Sad Cat") in consideration of your agreement to the following terms, and 
your use, installation, modification or redistribution of this Sad Cat software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Sad Cat software.

This software is provided "as is". Sad Cat Software makes no warranties, 
express or implied, including without limitation the implied warranties
of non-infringement, merchantability and fitness for a particular
purpose, regarding Sad Cat's software or its use and operation alone
or in combination with other hardware or software products.

In no event shall Sad Cat Software be liable for any special, indirect,
incidental, or consequential damages (including, but not limited to, 
procurement of substitute goods or services; loss of use, data, or profits;
or business interruption) arising in any way out of the use, reproduction,
modification and/or distribution of Sad Cat's software however caused and
whether under theory of contract, tort (including negligence), strict
liability or otherwise, even if Sad Cat Software has been advised of the
possibility of such damage.

Copyright (C) 2012, Sad Cat Software. All Rights Reserved.

*****************************************************************************/
#pragma once

class HYPERUI_API CachedSequence;
class HYPERUI_API Window;

/*****************************************************************************/
class HYPERUI_API IDragDropManagerCallback
{
public:
	virtual ~IDragDropManagerCallback() { }

	virtual void onTouchMoveSubclass(Window* pWindow, TTouchVector& vecTouches, UIElement* pDraggedElem) { }
	virtual bool onTouchUpSubclass(Window* pWindow, TTouchVector& vecTouches, UIElement* pDraggedElem, UIElement* pTargetElem) { return false; }
};
/*****************************************************************************/
class HYPERUI_API DragDropManager
{
public:
	DragDropManager(Window* pWindow);
	virtual ~DragDropManager();

	void setCallback(IDragDropManagerCallback* pCallback) { myCallback = pCallback; }

	void beginDrag(UIElement* pElem);
	void cancelDragFor(UIElement* pElem);

	bool isDragging();

	void render();

	bool onTouchUp(TTouchVector& vecTouches);
	void onTouchMove(TTouchVector& vecTouches);

	void registerRendererByTypename(const char* pcsDraggedElemTypeName, ICustomDragRenderer* pRenderer);
	void registerRendererByElemType(const char* pcsDraggedElemClass, ICustomDragRenderer* pRenderer);
	void unregisterRenderer(ICustomDragRenderer* pRenderer);

	void setIsDragBeginAllowed(bool bIsEnabled) { myIsDragBeginAllowed = bIsEnabled; }

	inline Window* getParentWindow() { return myParentWindow; }

private:

	void onDragEnd(UIElement* pElem);

private:

	UIElement* myDraggedElem;

	int myAtlasFrame;
	CachedSequence* myOverrideRenderSeq;

	SVector2D myLastPos;

	FLOAT_TYPE myInitElemOpacity;
	
	TStringCustomRendererMap myCustomRenderersByTypename;
	TStringCustomRendererMap myCustomRenderersByElemType;
	bool myIsDragBeginAllowed;
	Window* myParentWindow;

	IDragDropManagerCallback* myCallback;
};
/*****************************************************************************/