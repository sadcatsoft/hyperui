A static element provides the ability to show text, image, or both. It is usually not given a size, unless you want it to be outlines or filled with a background color. All elements in HyperUI inherit from this type. 

| | |
|-|-|
Tag | &lt;elem&gt;
elemType | static
C++ Class | UIElement


## Example
```
<elem>
	id = exampleElement
	layer = main
	text = This is an example of static text element.
	image = customIcon
	x = -10up
	y = 20up
	width = 200up
	height = 100up
</elem>
```

## Properties

The static UI element supports all of the standard properties described in [Properties and Element Types](builtInValues.md).
