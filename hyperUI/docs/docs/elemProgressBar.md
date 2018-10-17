Represents a horizontal progress bar, with optional text.

| | |
|-|-|
Tag | &lt;progress&gt;
elemType | progress
C++ Class | UIProgressElement

## Example
```
<progress>
	id = exampleProgressBar
	text = Completion:
	textOffsetX = -75up
	textOffsetY = -0.5up
	textHorAlign = Right
</progress>
```

Property | Description
-------- | -----------
image | Specifies the empty progress bar image.
secondaryImage | Optionally specifies the full progress bar image.
direction | Optionally, specifies the direction from which the progress bar begins at zero progress. The default is `FromLeft`, but it can be one of the following values: <br>- `FromLeft` - begins drawing the progress bar from the left<br>- `FromRight` - begins drawing the progress bar from the right<br>- `FromTop` - begins drawing the progress bar from the top<br>- `FromLeft` - begins drawing the progress bar from the bottom<br><br>Note that for vertical values, corresponding vertical images are expected