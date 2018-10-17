Represents a single-line text or number input field. Comes with its own label.

| | |
|-|-|
Tag | &lt;stringInput&gt;
elemType | textField
C++ Class | UITextFieldElement

## Example
```
<stringInput>
	id = exampleStringInput
	fillXLess = 40up
	<elem>
		id = defLabel
		text = My Label
	</elem>
	<elem>
		id = defTextBox
		text = Initial text
	</elem>
</stringInput>
```

Property | Description
-------- | -----------
textFormat | Specifies the format, in C notation, to display the value in, such as `%d`. Note that specifying numeric formats (`%d, %g, %f`) will convert this field to a numeric field where only numbers will be allowed. This property is ignored for non-numeric fields.
min | For numeric input fields, the minimum value that can be displayed.
max | For numeric input fields, the maximum value that can be displayed.
text | Specifies the default text value.
textIsProtected | If `True`, the characters are replaced with asterisks. Defaults to `False`.
lostFocusAction | A string value that represents the action to trigger when the input field loses focus.
textChangedAction | A string value that represents the action to trigger when the text in the input field changes.
isReadOnly | If `True`, existing text cannot be edited by the user.
