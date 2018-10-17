------------

## Specifying UI
The UI for each window is specified in a single UI specification file, such as resUI.txt. Each of these files is called a resource collection, and can be accessed through `HyperUI::ResourceManager`. As a note, you can also use this mechanism to load any other kind of data you need, not just UI elements.

At this point, you might wonder why is there a single file instead of individual files for each UI element. This was a deliberate decision to avoid making things more complicated than they need to be (in terms of both structure and the necessary build process changes that would have been required), as well as for performance reasons, since a single file is faster to load and parse. However, you can use `#include` directives to break up that file into multiple files and include them into the project.

------------

## UI Element Types

Each UI element has a type, which determines its behaviour. These are described in the "UI Elements" section. The most basic element type is "static", and it can specify either a label (text) or an image - or both. Both can be controlled and offset individually. For example:
```
<elem>
	id = elem1
	text = This is a sample label
	image = windowBackground
</elem>
```

Here, both the text and the image will be drawn at the center of the element, but can be offset:
```
<elem>
	id = elem1
	text = This is a sample label
	image = windowBackground
	textOffsetX = 20up
	textOffsetY = 10up
	imageOffsetX = -50up
	imageOffsetY = -25up
</elem>
```

Now, the text will be shifted 20 universal pixels to the right and 10 universal pixels down, while the image will be shifted 50 universal pixels up and 25 universal pixels to the left. For more options on what else can be done with static elements, please see the [Static Element](elemStatic.md) documentation.

Note that the `image` value of `windowBackground` refers to an entry in the graphics resource collection, not the file directly.

------------

## Layers

Most UI systems show and hide UI elements directly and individually - that is, they expect you to say "show (or hide) an element with id X". In our system, we use **layers** of UI elements instead; each top-level UI element must have a `layer` property, which is set to any simple custom string, i.e.: `layer = myCustomLayerName`. A layer can then be shown using a `showUI("myCustomLayerName")` function call.

This method has several advantages:

- There are fewer hard-coded strings in the code, since there are fewer layer names in the code than individual element ids
- It is much easier to show several UI elements at the same time, especially if each of them has its own animation style, than individually showing each one using ids

------------

## Universal Pixels

All distances and sizes in HyperUI are all specified in "universal pixels" - abbreviated as "up". The reasoning behind this is that HyperUI runs on many devices with different screen densities, and we want to specify the UI only once. So, if you specify an element width of "100up", it will be automatically converted to 100 pixels on a non-Retina laptop display, but to 200 pixels on a Retina one. This ensures that the button remains the same size on the physical screen of the device.

------------

## Element Positioning

Our elements are positioned using the XY system with the origin in the top-left corner of the screen, similar to many other UI systems. Similarly, like other systems, each UI element has an (x,y) position and a size. However, unlike most other systems where the element position usually denotes where its top-left corner is positioned, in our system, it denotes where the **center** of the element is. This approach makes it much easier to nest and transform elements.

By default, an element is poitioned at (0, 0), which means:

- If the element is a top-level element and has no parent, it is positioned in the center of the screen
- If the element has a parent, it is positioned at the center of the parent element

You can change an element's position by modifying it's `x` and `y` properties:
```
<elem>
	id = elem1
	x = 200up
	y = 100up
</elem>
```

It will now be offset by 200 universal pixels horizontall and 100 universal pixels vertically from its (0,0) position. In general, you can think of `x` and `y` properties as offsets from where the element would have otherwise been given all other flags on it (such as alignment).

------------

## Aligning Elements

To make UI elements more adaptible and scalable, elements can be automatically aligned to its parent's element area (or the entire screen if the element is a top-level element). To do this, you can add an `align` property to the element. This property is an array of two elements, where the first one specifies horizontal alignment, and the second specifies  vertical alignment:

```
<elem>
	id = elem1
	align = { Right, Bottom }
</elem>
```

Since this is a top-level element with no parent, it will be aligned to the right-bottom corner of the screen. Note that if the element has a size, then the element's right-bottom corner will be aligned to the right-bottom corner of the screen; however, if the element has a zero size, then its center will be aligned to that screen corner.

What if you want to align an element only on one axis? You can always specify `__none___` as the other array element to avoid performing any alignment at all: `align = { Right, __none__}`

What if you want to align the same element to the bottom-right corner of the screen, but also offset it a little? Well, it's easy and logical - you just specify its new position

The following values are accepted for the first (horizontal) align flag:

| Value | Meaning |
| ----- | ------- |
| `Left`  | Aligns the element to the left boundary of the parent element or screen |
| `Center` | Aligns the element to the center of the parent element or screen, proportionately scaling the offset specified by the element's `x` value when the window size changes |
| `Right`  | Aligns the element to the right boundary of the parent element or screen |
| `__none__` | Does not align the element horizontally |

The following values are accepted for the second (vertical) align flag:

| Value | Meaning |
| ----- | ------- |
| `Top`  | Aligns the element to the top boundary of the parent element or screen |
| `Center` | Aligns the element to the center of the parent element or screen, proportionately scaling the offset specified by the element's `y` value when the window size changes |
| `Bottom`  | Aligns the element to the bottom boundary of the parent element or screen |
| `__none__` | Does not align the element vertically |

------------

## Specifying Size

By default, an element has no size. This means it cannot be clicked, for example. It also means its image cannot be stretched or tiled to fill it, and all alignment will be done on its center, instead of on its actual area.

Two of the most common ways of specifying the size are by setting a static size or by setting the size as a portion of its parent's size. To specify a static size, you can simply set the element's `width` and `height` property:
```
<button>
	id = elem1
	width = 200up
	height = 100up
</button>
```
This button is now 200 universal pixels wide and 100 universal pixels tall. This, however, is an approach that works well only for a specific subset of cases. It is often the case that an element needs to resizable. In this case, we can use the following properties:
```
<elem>
	id = elem1
	fillXLess = 20up
	fillYLess = 10up
</elem>
```
This element will be now be the same size as its parent less a padding of 20 universal pixels on each side horizontally, and 10 universal pixels on each top and bottom. This approach is often useful when dividing the screen or an element into sections that need to be resizable. These can also be mixed - that is, an element's width can be specified using the `fillXLess` property, but it's height can be specified using the `height` property - or vice versa.

------------

## Layout Types

There are three main methods of laying out the user interface elements in our platform. 

###Explicit Positioning
In this method, children of an element are explicitly positioned using `x` and `y` properties of the element. To make sure the UI remains coherent for screens of various sizes, normally these would be used together with the *align* property to automatically adjust elements when the screen size changes.

###Automatic Horizontal Layout
In this layout type, the `layout` property and setting to the `column` value: `layout = column`, which forces all of its children to be automatically arranged horizontally in columns. The size of each child specified by its `width` property. You can also override it by providing a `layoutWidth` property, which, if set to a value greater than 1, specifies an explicit size; if set to a value less than one, it is treated as a percent of the available space (i.e. 0.25 would mean the element will be allocated 25% of the available space). Setting `layoutWidth` to exactly 1.0 will ensure this element fills any space remaining after computing widths for all other elements in this row.

###Automatic Vertical Layout
With this layout, the `layout` property is set to `row`: `layout = row`, which forces all of the element's children to be laid out vertically using their computed height. This is useful for laying out forms, for example, where a series of controls each have their own row. You also use the `marginTop` property on each child to specify top margins for elements.

Note that these three methods can be combined with nesting, to produce essentially any layout possible. One could have a top level element with two horizontally-spaced panes, each of which contains a series of vertically spaced items (such as a lsit of emails on the left side and a form with details on the right which reflects the information about the currently selected email).

Also note that there is another option to layout elements - you could create a [UIGrid element](elemGrid.md), which creates a regular grid, and place UI elements in cells to produce a regular grid layout.

------------

## Debug Mode Development

When running a project in the debug mode (Visual Studio/XCode), you will see two additional items on the screen. In the top-left corner, you will see a piece of text that changes between "Active" and "Inactive". When this label says "Inactive", it means HyperUI isn't redrawing the screen - an optimization to avoid using too much power and drain laptop and mobile device batteries. When something that requires a screen redraw happens, the screen is redrawn and the label temporarily reverts to say "Active".

In the top-right corner, you will see a button that says "Reload". Pressing it will perform a hot reload of your UI and image files, and will refresh the interface with anything you've changed.

The general workflow when working with UI, then, is to add the desired element in your UI configuration file (with initial guess at its position, size, and other properties), run the app, examine the result visually, edit the properties, reload the application as per above, and repeat the process until the element has a desired appearance. Please note that some changes (such as adding a brand-new element or removing a property) may not get picked up by the hot reload. In this case, please re-run the application and continue the process.

When running in Python, you can enable the debugging mode as following:
``` python
hyperui.enable_debugging()
```
