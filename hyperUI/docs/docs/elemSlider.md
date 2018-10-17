Represents a horizontal slider for numeric values with a label and an auto-updating text field.

| | |
|-|-|
Tag | &lt;numericSlider&gt; (or &lt;slider&gt; for the slider itself with no other elements attached)
elemType | slider
C++ Class | UISliderElement

## Example
```
<numericSlider>
	id = exampleNumericSlider
	fillXLess = 40up
	<elem>
		id = defLabel
		text = My Label
	</elem>
	<elem>
		id = defNumSliderSlider
		max = 100
	</elem>
</numericSlider>
```

## Properties

Property | Description
-------- | -----------
image | Specifies the empty slider image.
secondaryImage | Optionally specifies the full slider image.
startInMiddle | If `True`, the zero knob position vill be in the middle of the slider; if `False`, it will be on the far left. The default is `False`.
max | Optionally, specify a maximum value for the slider. The default is 1.0.
min | Optionally, specify a minimum value for the slider. If the slider starts in the middle, the default value is -1; otherwise, it's 0.
textFormat | If the image for a pop up while dragging the slider is provided, this value specifies how to display the current slider value in C notation. The defeault value is `%d`
sliderButtonSizes | Optionally, specifies the increment and decrement button clickable areas as a list of two numbers. The first number is the width of the button, the second is the height. Requires button images to be provided via `sliderParms` option. For example: `sliderButtonSizes = { 20up, 20up }
sliderParms | A list of up to four values that specify images to use for the slider. The first value is the image to use of the decrement button, or `__none__` if none; the second is the image for the increment button, or `__none__` for no button visible. The third value is an image to use to fill in the full (left) part of the slider. Finally, the last image is an option image for the pop up display of value as the slider is being dragged. For example: `sliderParms = { __none__, __none__, progressFull, sliderPopAnim }`
sliderMarkParms | Optionally, specifies the parameters for marks (vertical dashes) along the slider. This is an array of two numbers, the first of which denotes the mark spacing, in universal pixels, and the second denotes how many small marks are drawn before the large one is. For example: `sliderMarkParms = { 2up, 10 }`
sliderMarkColor | For cases where marks are specified, provides the color to use for those marks. For example: `sliderMarkColor = { 0.230, 0.230, 0.230, 1.0 }`
sliderValParms | If the pop up image is specified, this optional array can specify an array of two numbers, the first of which is the vertical offset of the value text in the popup, and the second is an optional multiplier to apply to the actual slider value before it is displayed. For example: `sliderValParms = { -15up, 1.0 }`
