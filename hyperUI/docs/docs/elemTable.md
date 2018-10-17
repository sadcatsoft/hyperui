Represents a vertically-scrolling, single- or multi-column table that supports per-row or per-cell selection (or none at all), with custom elements that can be optionally configured per cell.

| | |
|-|-|
Tag | &lt;table&gt;
elemType | table (and tableCell for each cell element)
C++ Class | UITableElement (see also UITableCellElement and ITableCellInfoProvider)

## Example
```
<selectableCell>
	id = sampleCell
	borderColor = { 255, 255, 255, 1.0 }
</selectableCell>

<table>
	id = exampleVertTable
	fillXLess = 40up
	fillYLess = 40up
	cellTypes = { sampleCell }
</table>
```

Note that when table is being generated, it first searches all of its parents (and itself) for any class that implements the `ITableCellInfoProvider` interface. If it finds one, it queries all the values necessary for table construction from that class. If it doesn't find one, or if the value returned is invalid, it attempts to query the properties listed below on the table instead. 

In practice, this means you can make a parent element of the window responsible for dynamically determining table behaviour instead of relying on the properties below.

Property | Description
-------- | -----------
perCellSelection | A boolean property that specifies whether the table should do per-cell or per-row selection. The value of `True` means per-cell selection. Defaults to `False`.
allowSelection | A boolean value that specifies whether the table supports selection at all or not. Defaults to `False`.
allowMultipleSelection | A boolean value that specifies whether multiple cells or rows (depending on the value of `perCellSelection` above) can be selected at once.
numColumns | Specifies the number of columns in this table. Defaults to 1.
cellTypes | An array of cell types whose length must match the value of `numColumns` above. Specifies the ids of elements to use for cells in each column.
rmbAction | Optional element that specifies the action to trigger when the right-mouse button is pressed over the table.
cellSpacing | Optionally, specifies the spacing (padding) for all cells, both on the x and y axis.
cellSelectionElem | Optionally, specifies the id of an element to show when the cell or row is selected. This is done automatically for convenience. 