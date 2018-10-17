Represents a multi-line text input field that comes with its own label.

| | |
|-|-|
Tag | &lt;multilineInput&gt;
elemType | multilineTextField
C++ Class | UIMultilineTextFieldElement

## Example
```
<multilineInput>
	id = exampleMultilineInput
	fillXLess = 40up
	<elem>
		id = defLabel
		text = My Label
	</elem>
</multilineInput>
```

Property | Description
-------- | -----------
text | Specifies the default text value.
lostFocusAction | A string value that represents the action to trigger when the input field loses focus.
textChangedAction | A string value that represents the action to trigger when the text in the input field changes.
isReadOnly | If `True`, existing text cannot be edited by the user.
