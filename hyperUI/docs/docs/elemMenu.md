Represents any kind of meny in the program. Can be a horizontal menu bar or a vertical submenu with other child items. Can also be used for context (right-mouse button) menus.

| | |
|-|-|
Tag | &lt;mainMenu&gt; for main menus, &lt;submenu&gt; for submenus
elemType | menu
C++ Class | UIMenuElement

## Example
```
<mainMenu>
	id = exampleMenu
	menuIds = {	idFile,	idEdit }
	menuLabels = { File, Edit }
	menuSubitems = { exampleFileSubmenu, exampleEditSubmenu }
	menuActions = {	__none__, __none__ }
	menuActionValues = { __none__, __none__ }
</mainMenu>

<submenu>
	id = exampleFileSubmenu
	layer = uilFileSubmenu
	menuIds = { idFileOption1, __sep__, idFileOption2 }
	menuLabels = { Show Test Message, __none__, Another Menu Item  }
	menuActions = { showMessage, __none__, __none__ }
	menuActionValues = { File Option 1 has been clicked., __none__, __none__ }
</submenu>

<submenu>
	id = exampleEditSubmenu
	layer = uilEditSubmenu
	menuIds = { idEditOption1, __sep__, idEditOption2 }
	menuLabels = { Edit Option 1, __none__, Edit Option 2  }
	menuActions = { __none__, __none__, __none__ }
</submenu>
```

Note that for all properties below, each is an array, and all arrays that are provided must be the same length. 

Property | Description
-------- | -----------
menuIds | Specifies ids of the items listed. These are invisibile to the user and are intended for programmatic use. To specify a separator, use `__sep__`
menuLabels | An array that specifies specify user-visible dropdown labels.
menuActions | An array that speicifies each item's action to trigger.
menuActionValues, menuActionValues2, menuActionValues3 | An array that speicifies each item's action values, up to three per item.
menuIcons | Optionally, specify icons for each menu item.
menuSubitems | If a menu item contains a submenu, this is the id of each submenu. Otherwise, should be set to `__none__`.
menuIsCheckmark | If set to `True`, the item is checkbox that can be toggled on and off; if set to `False`, each item is a regular menu item.
menuCheckmarkImage | A string (not an array) property that specifies the image to use for the checkmark.