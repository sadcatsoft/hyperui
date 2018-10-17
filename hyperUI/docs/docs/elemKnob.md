Represents a rotary knob, also known as a round slider, that can rotate and change a value between given minimum and maximum values. Comes with a label and an updateable numeric input control.

| | |
|-|-|
Tag | &lt;numericRoundSlider&gt; (or &lt;angle&gt; for the pure knob itself with no additional elements)
elemType | roundSlider
C++ Class | UIRoundSliderElement

## Example
```
<numericRoundSlider>
	id = exampleKnob
	y = 25up
	fillXLess = 40up
	<elem>
		id = defLabel
		text = My Label
	</elem>
</numericRoundSlider>
```

Property | Description
-------- | -----------
image | Specifies the empty progress bar image.
secondaryImage | Optionally specifies the full progress bar image.
capImage | Optionally, provide a cap image that is rotated by an appropriate amount and rendered on top of the previous two images.