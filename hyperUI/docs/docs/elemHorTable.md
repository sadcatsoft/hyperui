Represents a horizontally-scrolling table where cells are arranged horizontally.

| | |
|-|-|
Tag | &lt;horTable&gt;
elemType | horizontalTable (and tableCell for each cell element)
C++ Class | UIHorizontalTableElement (see also UITableCellElement and ITableCellInfoProvider)

## Example
```
<selectableCell>
	id = sampleHorCell
	borderColor = { 255, 255, 255, 1.0 }
	fillXLess = -1
	fillYLess = 5.5up
	y = -5up
	width = 64up
</selectableCell>

<horTable>
	id = exampleHorTable
	fillXLess = 40up
	height = 100up
	cellTypes = { sampleHorCell }
</horTable>
```

Similarly to regular [Tables](elemTable.md), horizontal table searches all of its parents (and itself) for any class that implements the `ITableCellInfoProvider` interface. If it finds one, it queries all the values necessary for table construction from that class.