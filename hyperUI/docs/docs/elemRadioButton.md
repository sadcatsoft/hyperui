Represents a radio button with a custom piece of text. Note that it uses exactly the same implementation as a checkbox, but with different images and a `radioGroup` property added.

| | |
|-|-|
Tag | &lt;radioButton&gt;
elemType | checkbox
C++ Class | UICheckboxElement

## Example
```
<radioButton>
	id = exampleRadioButton1
	text = Option 1
	width = 200up
	fillXLess = -1
	x = 100up
	y = -20up
	radioGroup = ___myExampleGroup___
</radioButton>

<radioButton>
	id = exampleRadioButton2
	text = Option 2
	width = 200up
	fillXLess = -1
	x = 100up
	radioGroup = ___myExampleGroup___
</radioButton>

<radioButton>
	id = exampleRadioButton3
	text = Option 3
	width = 200up
	fillXLess = -1
	x = 100up
	y = 20up
	radioGroup = ___myExampleGroup___
</radioButton>
```

## Properties

Property | Description
-------- | -----------
action | Specifies the action string to process when an element is clicked. This string is passed up to every parent in the change through the `handleAction` method which can be overriden by a custom element or handled in the window class. The unprocessed or global actions are processed automatically or ignored if the action is custom.
actionValue, actionValue2, actionValue3 | Action values passed along with the action type itself. These can be anything and are specific to each particular action type.
radioGroup | Specifies a string label for a group of radio boxes. This label is arbitrary, but all radio boxes with the same label will be mutually exclusive - i.e the user will only be able to select one of them at a time.