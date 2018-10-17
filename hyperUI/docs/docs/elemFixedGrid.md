A fixed grid is very similar to a [Grid](elemGrid.md), but, when resized, instead of putting more cells into the same space, it resizes the cells to match the new size. Therefore, it always remains NxM cells.

| | |
|-|-|
Tag | &lt;fixedGrid&gt;
elemType | fixedGrid (and tableCell for each cell element)
C++ Class | UIFixedGrid (see also UITableCellElement and IFixedGridCellInfoProvider)

## Example
```
<fixedGridSelectableCell>
	id = fixedGridCell
	borderColor = { 255, 255, 255, 1.0 }
</fixedGridSelectableCell>

<fixedGrid>
	id = exampleFixedGrid
	fillXLess = 40up
	fillYLess = 40up
	cellTypes = fixedGridCell
	numCellsX = 5
	numCellsY = 3
</fixedGrid>
```

Similarly to regular [Tables](elemTable.md), a grid searches all of its parents (and itself) for any class that implements the `ITableCellInfoProvider` interface. If it finds one, it queries all the values necessary for table construction from that class.

Property | Description
-------- | -----------
cellTypes | A single string value for the uniform cell type for this table.
numCellsX | Specifies the number of cells per row.
numCellsY | Specifies the number of cells per column.