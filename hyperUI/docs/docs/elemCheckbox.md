Represents a checkbox with a custom piece of text.

| | |
|-|-|
Tag | &lt;checkbox&gt;
elemType | checkbox
C++ Class | UICheckboxElement

## Example
```
<checkbox>
	id = exampleCheckbox
	text = Checkbox Label
	width = 200up
	fillXLess = -1
	action = showMessage
	actionValue = The button has been pressed.
</checkbox>
```

## Properties

Property | Description
-------- | -----------
action | Specifies the action string to process when an element is clicked. This string is passed up to every parent in the change through the `handleAction` method which can be overriden by a custom element or handled in the window class. The unprocessed or global actions are processed automatically or ignored if the action is custom.
actionValue, actionValue2, actionValue3 | Action values passed along with the action type itself. These can be anything and are specific to each particular action type.
