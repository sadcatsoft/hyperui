A grid contains a series of cells that are arranged horizontally-first, wrapping at the right edge and continuing onto the next row. It's a convenient layout for displayn an arbitrary, dynamically-determined list of items in the table form.

| | |
|-|-|
Tag | &lt;grid&gt;
elemType | grid (and tableCell for each cell element)
C++ Class | UIGrid (see also UITableCellElement and ITableCellInfoProvider)

## Example
```
<selectableCell>
	id = gridCell
	borderColor = { 255, 255, 255, 1.0 }
	fillXLess = -1
	fillYLess = -1
	width = 64up
	height = 32up
</selectableCell>

<grid>
	id = exampleGrid
	fillXLess = 40up
	fillYLess = 40up
	cellTypes = { gridCell }
	cellWidth = 64up
	cellHeight = 32up
</grid>
```

Similarly to regular [Tables](elemTable.md), a grid searches all of its parents (and itself) for any class that implements the `ITableCellInfoProvider` interface. If it finds one, it queries all the values necessary for table construction from that class.

Property | Description
-------- | -----------
cellTypes | An array with a single value for the uniform cell type for this table.
cellWidth | Specifies the width of the table cell in the grid.
cellHeight | Specifies the height of the table cell in the grid.