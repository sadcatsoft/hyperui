Represents a dropdown element that allows the user to pick one of the listed items.

| | |
|-|-|
Tag | &lt;labeledDropdown&gt; (or &lt;dropdown&gt; for the pure dropdown itself with no additional elements)
elemType | dropdown
C++ Class | UIDropdown (see also UIDropdownMenu, UIDropdownTextPart)

## Example
```
<dropdown>
	id = exampleDropdown1
	fillXLess = 40up
	menuIds = { __none__, idOption1, idOption2, idOption3 }
	menuLabels = {  __none__, Option 1, Option 2, Option 3 }
	y = -20up
</dropdown>
```

Property | Description
-------- | -----------
menuIds | Specifies ids of the items listed. These are invisibile to the user and are intended for programmatic use.
menuLabels | These specify user-visible dropdown labels.

Please note that the first entry in both `menuIds` and `menuLabels` **must** be `__none__`, since it corresponds to the currently selected item.