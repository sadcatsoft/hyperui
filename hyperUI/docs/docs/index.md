## Overview
Each HyperUI application consists of three basic sets of files:

- Binary data (images, sounds, any custom files, etc.)
- Text based resource files that specify the entire structure of the user interface, as well as any other text-based data you may need
- The executable itself, which handles all logic, custom actions, custom rendering, etc. using the above two categories

------------

## Installation
Getting started on your first project is very simple. Please note that for C++ based projects, you need Visual Studio 2015 or newer on Windows, and the latest version of XCode on Mac OS.

Hyper UI source and sample projects can be downloaded here: [https://github.com/sadcatsoft/hyperui](https://github.com/sadcatsoft/hyperui)

``` c++ tab="C++"
/* 
- Download and unzip HyperUI to any location of your choice
- On Windows, open the HyperUI/examples/blankAppTemplate/msvc2015 solution in Visual Studio
- On Mac OS:
     * In the terminal, run buildLibs to compile the two dependencies, freetype and libpng
     * Open the HyperUI/examples/blankAppTemplate/macOS project in XCode
*/
```

``` python tab="Python"
pip install hyperui
```

------------

## Your First Application
To use HyperUI, it first needs to be initialized. In C++, you need to explicitly do so, while in Python it's done for you when you import the hyperui module:

``` c++ tab="C++"
HyperUI::initialize("Blank HyperUI App");

// We need to tell our application where the resources from the library
// are. In this case, we store them in the subfolder "hyperUI" relative
// to our main application folder.
ResourceManager::setStandardDataPathPrefix("hyperUI");
```

``` python tab="Python"
import hyperui
```

Now, we can create our main window and run the application:

``` c++ tab="C++"
// Run it!
HyperUI::run();
```

``` python tab="Python"
hyperui.run()
```

When you compile and run the application (C++) or run the script (Python), you should see a blank window appear. Hooray!

------------

## Adding a Button
Well, a window is nice, but pretty useless. How do we make it do something and be actually useful? For that, we need to add some custom UI elements:

- In the root folder where your Python script or C++ source is located, create a subfolder called "customUI". This will server as a folder that contains our custom UI.
- In that subfolder, crate an empty file named resUI.txt - this will the main file which specifies our user interface elements.
- Open resUI.txt, and add the following text to it:
```
#include "../hyperUI/config/resUICommon.h"
<window>
	id = mainWindow
	layer = main
	fillXLess = 0up
	fillYLess = 0up
	<button>
		id = testButton
		text = PRESS ME
		action = showMessage
		actionValue = The button has been pressed. Yay!
		width = 100up
	</button>
</window>
```

Now that we have created some UI specifications, we need to tell our app to actually use it:

``` c++ tab="C++"
// This struct holds optional parameters we can use to customize our window
HyperUI::SWindowParms rParms;
// Here, we tell HyperUI that the UI collection we want to use for our window is resUI.txt 
// and provide a relative path to it
rParms.myInitCollectionType = ResourceManager::getInstance()->addCollection("customUI" FOLDER_SEP "resUI.txt", true);

// Modify the run() call to take these parameters
HyperUI::run(rParms);
```

``` python tab="Python"
# Create a custom Window class that inherits from our base HyperUI window
class PyWindow(hyperui.Window):
    def __init__(self, file_path, title = '', w = -1, h = -1, first_layer = ''):
        hyperui.Window.__init__(self, file_path, title, w, h, first_layer)
		
# Get the path to where we're running
module_path = os.path.dirname(os.path.abspath(sys.argv[0]))

# Create a window and provide the full path to our UI specifications file
wnd = hyperui.Window(os.path.join(module_path, 'elementsCatalogUI', 'resUI.txt'))

# Tell HyperUI that we want to use our own window class:
hyperui.run(wnd)
```


Now re-run the application. You should see that the background of the window has changed to a dark texture, and that there is a button in the middle which, when pressed, shows a text box with the confirmation message.

We've introduced a lot of new stuff, so let's go line-by-line and explain what's going on here. First, you'll notice that all of our UI and other resource files come in an XML-like (but better :) format, designed first and foremost for readability. Here are some notes on it:

- The indentation does not matter.
- Elements are opened using `<tag>` tags, and closed using the corresponding `</tag>`. These can be nested.
- Data is specified using format: `propertyName = property value` The type of the property is determined automatically.
- The property must fit on one line. Special syntax exists for specifying longer text segments: `text = ___[[[ This is a long piece of text that may be broken up over any number of lines. ]]]___`
- Properties currently cannot be arbitrary and must be previously declared using `DECLARE_PROPERTY()` macro in C++ - or be one of existing properties. This is done for performance reasons.
- We support `#ifdef/#endif` and `#include` directives, as well as single-class inheritance. See [Advanced section](advanced.md) for specifics on these.

------------

## Analysis

Now, what do all those lines mean? Let's start with the first one:
```
#include "../hyperUI/config/resUICommon.h"
```
HyperUI is built on the idea that there should be no magic, black-box behaviour - everything should be traceable and logical. The above statement includes the main UI configuration file that specifies default values for elements with tags `<window>` and `<button>`, and is located using the standard path relative to the location of the file it's included from. You can open it up and see for yourself.

```
id = mainWindow
layer = main
```
Each element must have a globally-unique id - which is what the `id` property specifies in this case. Also, each top-level UI element must have a `layer` property to be useful - in HyperUI, we show user interface elements not based on telling the program to show a specific window or element, but on showing *layers* of elements. This means several disjoint elements can be shown all at once with a single command. Here, we specify that the window element in question belongs to the layer named "main". Generally, layer names are just custom strings - you can use anything. The "main" layer is the exception - by default, these elements will be shown on start up. You can override this behaviour by specifying your own layer name to show when the application launches.

```
fillXLess = 0up
fillYLess = 0up
```
What do these mean? And what is this "up" thing? "up" stands for "universal pixel", and is used as a unit of measurement on the screen which scales with the screen density on the specific device the app is running on. This means you can use a single UI configuration for all devices (but don't worry, you can also override it on specific screen densities).

The `fillXLess` property tells HyperUI to stretch this element to fill the main window or its parent element if it has one - less the padding on each side specified by the value of this property. In this case, we want the element to completely fill the window, so the values are set to 0. However, if you set them to 20up and re-run the app, you'll notice that on regular-density screens, there will now be a padding on each side of 20 pixels.

```
action = showMessage
actionValue = The button has been pressed. Yay!
```
Clickable elements such as buttons, menus, checkboxes, etc. all can have an action associated with them, as well as parameters that action needs. Some actions are built-in for convenience ("showMessage", "showLayer", "hideLayer", etc.), but you can also add any custom action you want. In this case, we specify that when the button is clicked, we want to show a message and provide its text in the `actionValue` property.

------------

## Inheritance

HyperUI resource files support an equavalent to classes and single-class inheritance. The items you have seen in the examples above can all be thought as instances of previously-declared classes.

A class, or definition, of an element, is defined as follwoing:

```
<customElem definition>
	text = Base text
	x = 20up
	<anotherElem>
		id = defChildElem
		text = Child text
	</anotherElem>
</customElem>

<customElem>
	id = elemInstance
	text = Overriding text
	<anotherElem>
		id = defChildElem
		text = Overriding child text
	</anotherElem>
</customElem>
```

Here, the first item is the **definition** of the `customElem` type. All items that use the same tag after that point in the file will inherit all of the properties of the defintion, as well as all of its children. Also note that in the instantiation of `customElem` we override both text properties of the definition, and they will have new values if queried. 

We also support inheritance. Suppose you wanted to extend `customElem` type above. You can do this as following:
```
<anotherElem definition extends customElem>
	x = 30up
</anotherElem>
```
Here, any element that instantiates `anotherElem` will have property x equal to 30up, but will preserve all other properties and child elements of `customElem`.

Note that many of the UI elements provided for your convenience are definted using the above mechanism in resUICommon.h