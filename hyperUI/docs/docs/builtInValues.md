------------

## Common Properties

###General Properties

Property | Description
-------- | -----------
elemType | Specifies the type of the element. Built-in types can be found below. Alternatively, this can be custom user class (such as `UserMyElement`), in which case the property value is set to the name of the C++ or Python class that you implement. See [Custom UI Elements and Action Handling](advanced.md) for exact details on how to do that.
transitionTime | Speicifies the time, in seconds, of the animation length while a layer of elements is being show or hidden.
zIndex | This is the property that specifies the top-level element's z-order. Its value can be betwee 1 and 14, inclusive. Layers with higher values appear above layers with lower ones.
opacity | Specifies the opacity, in the range of [0, 1] of an element. Note that unlike `imageOpacity`, this applies to both the text and the image on the element, as well as all of its children.
isHidden | A boolean property which defaults to *False* which, if set to *True* hides the element (and makes it not clickable). This is used to have some elements hidden and control their visiblity programmatically from C++ or Python upon certain events completing.
layer | On top-level elements, specifies the name of the layer this element belongs to. Has no effect on child elements.
backColor | If specified, fills the element with the solid color provided.
borderColor | If specified, draws a border around the element using the color provided. Note that it must be paired with `borderThickness`.
borderThickness | If specified, draws a border around the element of the specified thickness. Note that it must be paired with `borderColor`.

###Sizing and Positioning Properties

Property | Description
-------- | -----------
x, y | Specify the position, in ups, of the element. Note that for top-level element, the position is in the range of [0, screenWidth] for X axis and [0, screenHeight] for Y axis. However, for child elements, this is the position relative to their parent's center point, meaning that a child positioned at (0, 0) is located exactly at its parent's center.
width, height | Specifies the size, in ups, of the element. Note that these are mutually exclusive with their corresponding *uioFillParentXLessPadding/uioFillParentYLessPadding* properties. Not specifying the element's size using either of these types of properties makes it not clickable.
boxOffsetX, boxOffsetY | Specifies the offset, if any, in ups, of the element's clickable area.
align | This property aligns the element to its parent or the screen. It is set to an array of two values, one for horizontal alignment type, one for vertical. Currently, first value can be: <br><br>- Left - left-align the element's left edge, if it has a valid size, or its center, if it doesn't, to the parent container's left edge. <br> - Center - align the element's center to the parent container's center. Scale the center distance proportionally as the parent grows or shrinks. <br> - Right - right-align the element's right edge, if it has a valid size, or its center, if it doesn't, to the parent container's right edge.<br>- \_\_none\_\_ - do not perform any horizontal alignment.<br><br>The second value can be one of:<br><br>- Top - top-align the element's top edge, if it has a valid size, or its center, if it doesn't, to the parent container's top edge. <br> - Center - align the element's center to the parent container's center. Scale the center distance proportionally as the parent grows or shrinks. <br> - Bottom - bottom-align the element's bottom edge, if it has a valid size, or its center, if it doesn't, to the parent container's bottom edge.<br>- \_\_none\_\_ - do not perform any vertical alignment.<br><br>Note that the "parent's container" is defined as the screen area for top-level elements and as their immediate parent's area for children elements.
fillXLess, fillYLess | These specify that the element is to fill its parent's rectangle area less a certain padding. Each of these is mutually exclusive with a corresponding `width/height` property. These are used to create elements that automatically resize when their parents do. As above, the parent's area is defined as the screen area for top-level elements and as their immediate parent's area for children elements. The value specified for each of these properties, in ups, is subtract from each side of the parent's contianer. For example, if the parent element is of size 400up x 200up, specifying a child with these two properties set to 40up and 20up, respectively, means that the child element will be of the width (400 - 40\*2) = 320up and of the height (200 - 20\*2) = 160up.

###Text Properties

Property | Description
-------- | -----------
text | Specifies the text to show for this element.
font | Specifies the name of the font object, defined in *config/res_fonts.txt* and *config/res_fonts.txt* files.
textColor | Specifies the color to use when rendering the text of this element, if any. Values can be an array of numbers in the range [0, 1] (e.g. { 0.99, 0.85, 0.01, 0.95 }, which is a yellowish color with 95% opacity) or as hex values (e.g. #ff0000 which would be flat red color). You can also use 255-range values: { 255, 200, 200, 0.1 }
textShadowColor | Specifies the text drop shadow color. Note that the shadow offset also needs to be non-zero for this property to have any effect.
textShadowOffsetX, textShadowOffsetY | Offsets, in ups, for the text drop shadow. If not specified or both are 0, no shadow is drawn.
textHorAlign | Specifies the horizontal alignment of the text relative to the center of the element. Can be one of *Left, Center, Right* and if not specified, defaults to *Center*. If the value of *Left* is specified, the left edge of the text starts at the center of the element and extends further to the right; if the value of *Right* is set, the text's right edge (i.e. the end of the text) coincides with the element's center X position, and the rest of the text is positioned to the left.
textVertAlign | Specifies the vertical alignment of the text relative to the center of the element. Can be one of *Top, Center, Bottom* and if not specified, defaults to *Center*. If the value of *Top* is specified, the top edge of the text starts at the center of the element and extends further to the right; if the value of *Bottom* is set, the text's bottom edge (i.e. the end of the text) coincides with the element's center Y position, and the rest of the text is positioned upwards of the element's center.
textOffsetX, textOffsetY | Specifies the offset, in ups, of the text relative to its normal position (which includes text alignment).
textWidth | The value, specified in ups, tells the engine to start automatically wrapping the text if it extends beyond the length of this value. This property is mutually exclusive with *textWidthLessPadding*.
textWidthLessPadding | Similar to `fillXLess`, this property, if specified, forces the text to automatically wrap if it reaches beyond the given size. The size is computed as the size of this element (note: not it's parent) less specified padding from each side. Thus, it the elements width is 400ups, and this value is set to 20ups, the text will start wrapping after it exceeds (400 - 20*2) = 360up in length.
textPercOffsetX, textPercOffsetY | Similar to `textOffsetX/textOffsetY`, these propertyes control offset of the text relative to element's center. However, they are specified in the range [-1, 1] and are treated as a percentage of the element's size. For example, specifying the value of -0.5 will offset the text point to the left edge of the element (since it moves it to the left by 50% of the element's total width, and the default starting point is at the center). These properties are useful for controling the text alignment on resizable elements.
textIsProtected | If set to True, the text is protected and its contents is replaced with asterisks. Useful for password fields.


###Image Properties

Property | Description
-------- | -----------
image | Specifies the image to draw at the center of the element. The value of this property is the value of the `id` property for the object being referenced in the *config/res_prelim_animations.txt* or your custom image collection.
secondaryImage | Used in some more complex UI elements to specify a secondary image. For example, in for a scrollbar, this property specifies the image to use as a background, while `image` specifies the image to use for the slider knob.
backMode | If set to `fill`, the image is intelligently tiled to fill the entire size of the element. If set to `scale`, the element's image is scaled to fill its size. Otherwise, the image is used untransformed, as it appears in the file. Note that for fill mode, the image definition must include *realContentWidth, realContentHeight, cornerW, cornerH* properties.
overlayColor | Specifies the color to overlap on top of the element image. Note that this is usually done with white bitmaps, since overlaying color on multi-color images results in nothing visually pleasing.
imageOpacity | An opacity value in the range [0, 1] that controls the opacity of this element's image. The default is assumed to be 1.0.
imageScale | A scale value, in the range of [0, 1] that controls the scale of this element's image. The default is 1.0.

###Layout Properties

Property | Description
-------- | -----------
layout | If set to `column`, forces all of its children to be automatically arranged horizontally in columns. The size of each child specified by its `width` property. You can also override it by providing a `layoutWidth` property, which, if set to a value greater than 1, specifies an explicit size; if set to a value less than one, it is treated as a percent of the available space (i.e. 0.25 would mean the element will be allocated 25% of the available space). Setting `layoutWidth` to exactly 1.0 will ensure this element fills any space remaining after computing widths for all other elements in this row.<br><br>If set to `column`, forces all of the element's children to be laid out vertically using their computed height. This is useful for laying out forms, for example, where a series of controls each have their own row. You also use the `marginTop` property on each child to specify top margins for elements.
layoutWidth | A numeric property that is used to layout the children horizontally when `layout` is set to `column`. This value can be either an up pixel width value (e.g. 200up) or, if it is <= 1.0, it is treated as a percentage of the space available after all sibling elements with pixel widths have been laid out. Most commonly used with a value 1.0 on a single child while the rest of the children have specific pixel width to fill up the remaining space.
marginTop | Used in conjuction with *uioChildrenAutoLayoutStyle* to specify, if needed, the padding at the top of the element in the layout in ups.

###Action and Interaction Properties

Property | Description
-------- | -----------
action | Specifies the action string to process when an element is clicked. This string is passed up to every parent in the change through the `handleAction` method which can be overriden by a custom element or handled in the window class. The unprocessed or global actions are processed automatically or ignored if the action is custom.
actionValue, actionValue2, actionValue3 | Action values passed along with the action type itself. These can be anything and are specific to each particular action type.
isPassthrough | If this is set to True, the element, even if it has a valid size set, becomes not clickable, and passes all of the clicks going through to its parent or children, depending on the layout. This is used if one wants to render an overlay over the entire screen but still pass clicks to the elements underneath, for example.

------------

## Built-In Actions
Action | Expected Action Values | Description
-------- | ----------- | -----------
showLayer | actionValue = layerNameToShow | Shows all elements with the specified layer
hideLayer | actionValue = layerNameToHide | Hides all elements with the specified layer
hideElement | actionValue = elementId | Hides a single element with the specified id
hideTopParent | none | Find the top parent of the element which has this action attribute and hides all elements that belong to that layer
hideAllLayers | none | Hides all currently shown layers
hideLayerShowLayer | actionValue = layerNameToHide<br>actionValue2 = layerNameToShow | Hides all elements that belong to the layer specified in `actionValue` and then shows all elements that belong to the layer specified in `actionValue2`
hideLastShowLayer | actionValue = layerNameToShow | Hides the layer that was shown last and then shows all elements with the specified layer
showLayerExclusive | actionValue = layerNameToShow | Hides all layers and then shows the layer specified
toggleLayer | actionValue = layerName | If the specified layer is shown, hides it; if it was hidden, shows it
showMessage | actionValue = messageContents<br>(optional) actionValue2 = messageTitle | Shows a text message with the contents specified in `actionValue`. If `actionValue2` is present, uses its value as a title
quitApp | none | Shuts down the application
openFile | none | Propmpts the application to open a file. Note that you must handle this yourself since the specific requirements vary so much, but can use the provided `Application::showOpenFileDialog()` function
openFolder |  none | Propmpts the application to open a folder. Note that you must handle this yourself since the specific requirements vary so much, but can use the provided `Application::showOpenFolderDialog()` function
acceptNewColorFromPicker | none | When used in the color picker window, closes it and calls `Window::onAcceptNewColorFromPicker()`
cancelColorPicker | none | When used in the color picker window, closes it and cancels the color change


------------

## Built-In Element Types
Element Type | Description
-------- | -----------
static | A static element that contains a piece of text, an image, or both
button | A button that can be clicked
table | A vertically-scrolling table with any number of rows and columns. Can contain individually-selectable custom cells.
horizontalTable | A horizontally-scrolling table.
tableCell | An element that represents a table or a grid cell.
progress | A progress bar element.
slider | A slider with a knob and a sliding range. Can be vertical or horizontal. Scrollbars are also built on this element.
zoomWindow | A window that shows zoomable and scrollable content (in both direction). Supports pinch-zoom and flicking to pan.
textField | A single-line text input field.
multilineTextField | A multiline text input field.
checkbox | A checkbox that can be toggled on and off.
menu | A horizontal or vertical menu where each entry can be a direct action item or a submenu.
splitter | A resizable horizontal or vertical splitter that splits its space into two panes.
tabWindow | A window that lays out all of its children as tabs.
grid | A grid that supports laying out items in horizntal-first order.
colorSwatch | A rectangle which holds a color and supports changing it by using the built-in color picker when clicked.
dropdown | A dropdown/combobox element.
gradientEdit | A gradient editing element with preview.
roundSlider | A knob that can be turned left or right between minimum and maximum values.
roundProgress | A round progress bar.
richText | A multi-line text window that supports not only text, but any of the other elements above.
