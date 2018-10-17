Represents a horizontal or vertical splitter, each of which contains exactly two child elements whose size is automatically managed to split its parent element into to. Cna be resized or fixed.

| | |
|-|-|
Tag | &lt;vertSplitter&gt; for vertical splitter, &lt;horSplitter&gt; for a horizontal one.
elemType | splitter
C++ Class | UISplitterElement

## Example
```
<vertSplitter>
	id = exampleSplitterVert
	fillXLess = 40up
	height = 150up
	borderColor = { 255, 255, 255, 0.8 }
	borderThickness = 0.5up
	y = -90up
	<elem>
		id = exampleSplitterVertLeft
		backColor = #ff7800
	</elem>
	<elem>
		id = exampleSplitterVertRight
		backColor = #00baff
	</elem>
</vertSplitter>
```

Property | Description
-------- | -----------
isVertical | If set to `True`, the splitter is a vertical splitter; if set to `False`, it becomes a horizontal splitter.
splitterFixedChildIndex | Optionally, specifies the child index (0 or 1) that is kept constant size as the splitter is resized. If omitted, both child elements are reszied proportionately.
splitterDisableResizing | If set to `True`, the splitter is a fixed splitter and cannot be resized by the user.
splitterPos | Specifies the initial splitter position. If the value is greater than 1.0, the position is assumed to be a pixel position from the left for vertical splitters, and from the top for horizontal ones. If the value is betwee 0 and 1, this is assumed to be a ratio of the parent element's size.