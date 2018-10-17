Represents a button with a custom piece of text. Images are also supported by adding a new child element to the button.

| | |
|-|-|
Tag | &lt;button&gt;
elemType | button
C++ Class | UIButtonElement


## Example
```
<button>
	id = testButton
	text = Press Me
	action = showMessage
	actionValue = The button has been pressed.
	# Optional image if you need it
	<elem>
		id = buttonIcon
		image = customIcon
		# Offset it horizontally to the left
		x = -30up
	</elem>
</button>
```

## Properties

Property | Description
-------- | -----------
action | Specifies the action string to process when an element is clicked. This string is passed up to every parent in the change through the `handleAction` method which can be overriden by a custom element or handled in the window class. The unprocessed or global actions are processed automatically or ignored if the action is custom.
actionValue, actionValue2, actionValue3 | Action values passed along with the action type itself. These can be anything and are specific to each particular action type.
image | Specifies the image to draw when the button is not pressed. The value of this property is the value of the `id` property for the object being referenced in the *config/res_prelim_animations.txt* or your custom image collection.
image | Specifies the image to draw when the button is not pressed. The value of this property is the value of the `id` property for the object being referenced in the *config/res_prelim_animations.txt* or your custom image collection. Defaults to `buttonUpAnim`
secondaryImage | Specifies the image to draw when the button is pressed. Defaults to `buttonDownAnim`