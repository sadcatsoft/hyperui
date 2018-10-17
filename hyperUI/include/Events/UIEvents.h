#pragma once

// Last used id: 12
#define UI_EVENTS_START_VALUE			1024

/*****************************************************************************/
DECLARE_EVENT_EXPLICIT(EventSocketMessage, UI_EVENTS_START_VALUE + 0);
DECLARE_EVENT_EXPLICIT(EventExecuteAction, UI_EVENTS_START_VALUE + 1);
DECLARE_EVENT_EXPLICIT(EventUIParmChangeBegin, UI_EVENTS_START_VALUE + 2);
DECLARE_EVENT_EXPLICIT(EventUIParmChanging, UI_EVENTS_START_VALUE + 3);
DECLARE_EVENT_EXPLICIT(EventUIParmChangeEnd, UI_EVENTS_START_VALUE + 4);
DECLARE_EVENT_EXPLICIT(EventTabChanged, UI_EVENTS_START_VALUE + 5);
DECLARE_EVENT_EXPLICIT(EventTabAboutToBeChanged, UI_EVENTS_START_VALUE + 9);
DECLARE_EVENT_EXPLICIT(EventTabClosed, UI_EVENTS_START_VALUE + 10);
// Used for generic UI element value changes.
DECLARE_EVENT_EXPLICIT(EventValueChanged, UI_EVENTS_START_VALUE + 11);
DECLARE_EVENT_EXPLICIT(EventClipboardChanged, UI_EVENTS_START_VALUE + 12);

// Undo events
DECLARE_EVENT_EXPLICIT(EventUndoBlockAdded, UI_EVENTS_START_VALUE + 6);
DECLARE_EVENT_EXPLICIT(EventUndoPerformed, UI_EVENTS_START_VALUE + 7);
DECLARE_EVENT_EXPLICIT(EventRedoPerformed, UI_EVENTS_START_VALUE + 8);
/*****************************************************************************/

