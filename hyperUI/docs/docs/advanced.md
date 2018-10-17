------------

## Custom UI Elements and Action Handling

In general, custom elements are used in two cases: when there is a custom UI widget that needs to be developed (such, for example, an odometer gauge with custom indicator animation and graphics) or when a screen with many subcontrols needs to be implemented and needs to set the data from the game onto the controls and then modify that data based on the player's actions on that screen. We will focus on the second such case here. Let's say we have an element that spans the entire screen with a single button in the center:

```
<uiElem>
    id = helloWorldElement
    layer = uilMyLayerName
    elemType = etStatic
    fillXLess = 0up
    fillYLess = 0up
    
    <uiButton>
        id = myButtonTest
        elemType = etButton
        width = 200up
        height = 100up
        text = Custom Button
        action = uiaMyCustomAction
        actionValue = Good Bye!
        actionValue2 = 3.141592654
    </uiButton>
</uiElem>
```

Now, let's add a top-level custom class that can handle the action when the button is pressed. To do so, create and add the two files called UserHelloWorldParent.h and UserHelloWorldParent.cpp to your XCode or Visual Studio solution. If you're using Python, you can just add the corresponding Python snippet to your main script file:

``` c++ tab="C++"
// In UserHelloWorldParent.h:
#pragma once
class UserHelloWorldParent : public UIElement
{
public:
    DECLARE_UIELEMENT(UserHelloWorldParent);
    virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData);
};

// In UserHelloWorldParent.cpp:
#include "stdafx.h"
#include "UserHelloWorldParent.h"

UserHelloWorldParent::UserHelloWorldParent(UIPlane* pParentPlane)
    : UIElement(pParentPlane)
{
    onAllocated(pParentPlane);
}

UserHelloWorldParent::~UserHelloWorldParent()
{
    onDeallocated();
}

void UserHelloWorldParent::onAllocated(IBaseObject* pData)
{
    UIElement::onAllocated(pData);
}

void UserHelloWorldParent::onDeallocated()
{
    UIElement::onDeallocated();
}

bool UserHelloWorldParent::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
    bool bDidHandle = false;
    return bDidHandle;
}

```

``` python tab="Python"

class UserHelloWorldParent(hyperui.UIElement):
    def __init__(self):
        hyperui.UIElement.__init__(self)

    def handle_action(self, source_element, action, data):
        did_handle = False
        return did_handle
```

We also need to register our custom element type with HyperUI so it knows it's available. In C++-based projects, you can add the following line after initializing HyperUI; in Python-based projects, you can make the registration call anywhere before you create the main window:

``` c++ tab="C++"
REGISTER_CUSTOM_UI_ELEMENT(UserHelloWorldParent);
```

``` python tab="Python"
hyperui.register(UserHelloWorldParent)
```

We now need to tell the top-level parent that we want it to be represented by the custom class we've just created. To do that, change the line:
```
elemType = etStatic
```
to:
```
elemType = UserHelloWorldParent
```
This will create one instance of your custom class for every one instance of this UI element.

You can now compile the files, since they are fully ready to go. However, they still do nothing. Let's add a handler for our custom button action. In `UserHelloWorldParent.cpp`, a method called `handleAction()` exists (in Python, the same method exists in the snippet above). When a button is pressed, the application calls this method first on an element that was clicked, then on its parent, then on the parent of that parent, and so on, until it either reaches the topmost element or until one of these calls returns `true`, meaning that it handled the action.

Let's modify our method as follows:

``` c++ tab="C++"
bool UserHelloWorldParent::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
    bool bDidHandle = false;

    if(strAction == "uiaMyCustomAction")
    {
        // pSourceElem is the button we clicked
        // The call below will ask for its action value, which 
        // will return "Good Bye!"
        const char* pcsActionValue = pSourceElem->getStringProp(PropertyActionValue);
        
        // Also, just for kicks, let's ask for the second value.
        // This will return 3.141592654
        FLOAT_TYPE fNumValue2 = pSourceElem->getNumProp(PropertyActionValue2);
    
        // Now, let's change the text on the button to the value of its action:
        this->setUiTextForChild("myButtonTest", pcsActionValue);
        bDidHandle = true;
    }
    
    return bDidHandle;
}
```

``` python tab="Python"

    def handle_action(self, source_element, action, data):

        did_handle = False
        
        if action == 'uiaMyCustomAction':
            # pSourceElem is the button we clicked
            # The call below will ask for its action value, which 
            # will return "Good Bye!"
            action_value = source_element.actionValue
            
            # Also, just for kicks, let's ask for the second value.
            # This will return 3.141592654
            num_value = source_element.actionValue2

            # Now, let's change the text on the button to the value of its action:
            target_child = self.get_child_by_id('myButtonTest')
            target_child.text = action_value
            
            did_handle = True

        return did_handle


```

You can now compile and run the application. Clicking the button will now change its text to the value of its first action value.

------------

## Updating UI Elements

HyperUI uses an explicit update mechanism. When an event that requires changes to other elements happens, you are expected to trigger the update call as following:

``` c++ tab="C++"
// Let's say we're handling an action in a UIElement-based class.
// The call below triggers an update for helloWorldElement element and all 
// of its children.
this->getUIPlane()->updateElement("helloWorldElement");
```

``` python tab="Python"
# Let's say we're handling an action in a UIElement-based class.
# The call below triggers an update for helloWorldElement element and all 
# of its children.
self.window.update_element('helloWorldElement')
```

This triggers the helloWorldElement and all of its child elements to receive the following call:

``` c++ tab="C++"
void UserHelloWorldParent::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	// Here, you can update anything you want
}
```

``` python tab="Python"
def on_update(self):
	# Here, you can update anything you want
	pass
```

While this seems like a very basic update mechanism, in practice it turns out sufficient for all but the most complex applications. For the latter, there is a built-in, multi-threaded event-based system, which still needs to be documented.

------------

## Preprocessor Directives in Resource Files

Our resource files support several basic preprocessor defines. In particular, you can include other resource files as following:
```
#include "../hyperUI/config/resUICommon.h"
```
Note that the include path is relative to the file it is being included from, and may appear anywhere in the file, including inside existing elements.

In C++-based projects, we also support `#ifdef` and `#if defined` macros:
```
<elem>
	id = testElementId
#ifdef MY_CUSTOM_VARIABLE
	text = Condition 1
#else
	text = Condition 2
#endif
</elem>
```

In order to evaluate whether the condition is true or false, it is necessary to provide a custom callback before HyperUI is initialized:
``` c++
class CustomVarCheckCallback: public IVariableDefinedCheckCallback
{
public:
	CustomVarCheckCallback() { }
	virtual ~CustomVarCheckCallback() { }
	virtual bool getIsVariableDefined(const char* pcsVar)
	{
		// Evaluate whether the variable is defined or not according
		// to your logic and return true or false:
		return true;
	}
}

// Register it with the ResourceManager:
ResourceManager::getInstance()->setCallback(new CustomVarCheckCallback());
```

------------

## Custom Graphics

It is also possible to use custom graphics for your application. For this, we need to provide the graphics in the correct location, add a resource file that defines properties on our custom graphics files, and then change our elements to use them.

First, in the root folder of your project, create two folders named `customGraphics_x1` and `customGraphics_x2`. These will contain the two versions of your images - one for regular-density displays, and another for high-density displays. Place your two versions of the png image file into each folder, making sure file dimensions are a power of two (i.e. 32x32). Let's say the file is named `sample_icon.png`.

Next, we need to create a custom resource collection to tell HyperUI that the files is there. In the same folder where your custom UI elements are specified (`customUI`) create another file called `resGraphics.txt`, and copy-paste the following content there:

```
<animation>
	id = customIcon
	file = sample_icon
</animation>
```

Above, we basically bind the file name (`sample_icon`) to the id (`customIcon`). In general, this is also the place where we can provide other attributes of the file, such as corner sizes for tiling it, optional compressed texture formats, and other values.

Now we need to tell HyperUI to actually load this collection, and use it as an additional graphics collection:

``` c++ tab="C++"
// This adds the collection to resource manager using a relative path,
// and marks it as a graphics collection.
ResourceManager::getInstance()->addCollection("customUI" FOLDER_SEP "resGraphics.txt", true, CollectionRoleGraphics, "customGraphics");
```

``` python tab="Python"
module_path = os.path.dirname(os.path.abspath(sys.argv[0]))
hyperui.add_collection(os.path.join(module_path, "customUI", "resGraphics.txt"), hyperui.CollectionRole.Graphics, os.path.join(module_path, "customGraphics"))
```

Finally, we need to tell the element to actually use this image. We can do that simply by specifying the image property:
```
image = customIcon
```