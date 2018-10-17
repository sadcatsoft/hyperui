###############################################################################
# Generic windows and elements
###############################################################################
# Generic, small element
<elem definition>
	transitionTime = 0.10
	zIndex = 2
	textHorAlign = Center
	textVertAlign = Center
	font = ftMainFontMedSmall
	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	elemType = static
	image = __none__
</elem>

<window definition>
	image = windowBack
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0

	textHorAlign = Center
	textVertAlign = Center
	font = ftMainFontMedSmall
	
	backMode = fill

	zIndex = 2
	
	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up
</window>

<mainMenu definition extends window>
	elemType = menu
	zIndex = 6
	height = 20up
	fillXLess = 0.0
	image = menuBack

	align = { __none__, Top }
	
	textOffsetY = 0up
	textColor = { 0.922, 0.922, 0.922, 1.0 }		
	selectedTextColor = { 1.0, 1.0, 1.0, 1.0 }		

	menuInitOffsets = { 0up, 10up }
	tags = toplevelmenu
</mainMenu>

<progress definition extends elem>
	elemType = progress
	image = progressFull
	secondaryImage = progressEmpty
</progress>

<separator definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = separatorHorAnim

	align = { __none__, Top }
	fillXLess = 20up
	height = 1up

	margin = 4up

	backMode = fill
</separator>

<button definition>
	transitionTime = 0.10
	zIndex = 2
	textHorAlign = Center
	textVertAlign = Center
	font = ftMainFontMedSmall
	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	elemType = button
	image = buttonUpAnim
	secondaryImage = buttonDownAnim
	text = Button
	layoutWidth = 57up
	width = 57up
	height = 18up
	backMode = fill
</button>

<iconButton definition extends button>
	text = ___[[[]]]___
	<elem>
		id = defIcon
		align = { Left, __none__ }
	</elem>
</iconButton>

<vertScrollbar definition>
	transitionTime = 0.10
	zIndex = 2
	textHorAlign = Center
	textVertAlign = Center
	font = ftMainFontMedSmall
	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	elemType = slider
	image = vScrollKnob
	secondaryImage = vScrollBody
	x = -5.5up
	width = 11up

	y = 0up
	fillYLess = 0up

	sliderParms = { scrollElemsAnim:2, scrollElemsAnim:4 }
	sliderButtonSizes = { 10up, 25.5up, 10up, 10up } 
	isVertical = True
	backMode = fill
</vertScrollbar>

<horScrollbar definition>
	transitionTime = 0.10
	zIndex = 2
	textHorAlign = Center
	textVertAlign = Center
	font = ftMainFontMedSmall
	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	elemType = slider
	image = hScrollKnob
	secondaryImage = hScrollBody
	y = -5.5up
	height = 11up

	x = 0up
	fillXLess = 0up

	sliderParms = { scrollElemsAnim:6, scrollElemsAnim:8 }
	sliderButtonSizes = { 25.5up, 10up, 10up, 10up } 
	isVertical = False
	backMode = fill
</horScrollbar>

<table definition extends window>
	image = __none__
	elemType = table
	numColumns = 1
	perCellSelection = False
	allowSelection = True
	fillXLess = 0up
	fillYLess = 0up

	<vertScrollbar>
		id = defScrollbar
		linkTo = ../.
		x = 0.5up
		y = 0up
		fillYLess = 0up	
		align = { Right, __none__ }
	</vertScrollbar>
</table>

<grid definition extends window>
	image = __none__
	elemType = grid
	numColumns = 1
	perCellSelection = True
	allowSelection = True
	fillXLess = 0up
	fillYLess = 0up
</grid>

<horTable definition extends window>
	image = __none__
	elemType = horizontalTable
	numColumns = 1
	perCellSelection = True
	allowSelection = True
	fillXLess = 0up
	height = 100up

	<horScrollbar>
		id = defScrollbar
		linkTo = ../.
		x = 0up
		fillXLess = 0up
		y = 0up
		align = { __none__, Bottom }
	</horScrollbar>
</horTable>

<selectableCell definition extends elem>
	elemType = tableCell
	image = __none__
	layer = __none__
	fillXLess = 5.5up
	x = -5up
	height = 32up
	backColor = { 1, 1, 1, 0.15 }
	borderThickness = 0.25up
	cellSelectionElem = ./$0

	<elem>
		id = defCellSelection
		image = menuHiAnim
		fillXLess = 0up
		fillYLess = 0up
		backMode = fill
		isHidden = True
		isPassthrough = True
	</elem>

	<elem>
		id = defCellContent
		textColor = { 0.9, 0.9, 0.9, 1.0 }	
		textShadowColor = __none__
	</elem>
</selectableCell>

<fixedGridSelectableCell definition extends elem>
	elemType = tableCell
	image = __none__
	layer = __none__
	x = -5up
	backColor = { 1, 1, 1, 0.15 }
	borderThickness = 0.25up
	cellSelectionElem = ./$0

	<elem>
		id = defCellSelection
		image = menuHiAnim
		fillXLess = 0up
		fillYLess = 0up
		backMode = fill
		isHidden = True
		isPassthrough = True
	</elem>

	<elem>
		id = defCellContent
		textColor = { 0.9, 0.9, 0.9, 1.0 }	
		textShadowColor = __none__
	</elem>
</fixedGridSelectableCell>

<submenu definition>
	image = submenuBack
	elemType = menu
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0

	textHorAlign = Center
	textVertAlign = Center
	font = ftMainFontMedSmall

	isVertical = True
	
	backMode = fill

	width = 240up
	height = 200up

	zIndex = 5
	
	textColor = { 0.922, 0.922, 0.922, 1.0 }		
	selectedTextColor = { 1.0, 1.0, 1.0, 1.0 }		
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	menuCheckmarkImage = smallIcons:137

	tags = submenu
</submenu>

<popupHostWindow definition>
	image = submenuBack
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0

	textHorAlign = Center
	textVertAlign = Center
	font = ftMainFontMedSmall
	
	backMode = fill

	width = 200up
	height = 150up

	zIndex = 5
	
	textColor = { 0.922, 0.922, 0.922, 1.0 }		
	selectedTextColor = { 1.0, 1.0, 1.0, 1.0 }		
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	tags = submenu
	menuCheckmarkImage = smallIcons:137
</popupHostWindow>


<defaultButton definition>
	transitionTime = 0.10
	zIndex = 2
	textHorAlign = Center
	textVertAlign = Center
	font = ftMainFontMedSmall
	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	isDefaultAcceptButton = True

	elemType = button
	image = buttonDefaultAnim
	secondaryImage = buttonDownAnim
	text = Button
	layoutWidth = 57up
	width = 57up
	height = 18up
</defaultButton>

###############################################################################
# Auto-sizing dialog
###############################################################################
<autoDialog definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0

	textHorAlign = Center
	textVertAlign = Center
	font = ftMainFontMedSmall

	fillXLess = 0.0
	fillYLess = 0.0

	backMode = fill
	
	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	zIndex = 7
	image = __none__

	haveOwnUndoManager = True
	backColor = { 0, 0, 0, 0.6 }

	<window>
		id = defDialogWindow
		width = 200up
		windowBarImage = windowTitleBar
		title = Dialog Title
		layout = column
		fitHeightToChildren = True

		<elem>
			id = defDialogTopPadding
			width = 10up
			height = 10up
		</elem>

		<elem>
			id = defDialogInner
			layout = column
			fitHeightToChildren = True
			fillXLess = 0up
		</elem>

		<elem>
			id = defDialogBottomPadding
			width = 10up
			height = 10up
		</elem>

		<elem>
			id = defBottomButtonContainer
			fillXLess = 0up
			height = 18up

			<defaultButton>
				id = defOkButton
				text = OK
				x = -32up
			</defaultButton>

			<button>
				id = defCancelButton
				text = Cancel
				x = 32up
				action = hideTopParent
				isDefaultCancelButton = True
			</button>
		</elem>

		<elem>
			id = defDialogBottommostPadding
			width = 10up
			height = 12up
		</elem>
	</window>
</dialog>


###############################################################################
# Controls
###############################################################################
# Dropdown
<dropdown definition>
	elemType = dropdown
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__
	secondaryImage = menuHiAnim

	fillXLess = 0up
	height = 20up		

	<elem>
		id = defTextPart
		elemType = dropdownTextPart
		image = dropDownTextAreaBack
		fillXLess = 0up
		height = 17up
		backMode = fill

		textOffsetX = 5up
		textOffsetY = 0.5up
		text = BFG
		textHorAlign = Left
		textPercOffsetX = -0.5
		font = ftMainFontSmall

		<elem>
			id = defTextPartSel
			image = menuHiAnim
			fillXLess = 3.5up
			fillYLess = 3.5up
			backMode = fill
			isHidden = True
		</elem>
	</elem>

	<window>
		id = __dropdownTemplate__
		layer = __uilDropDown__
		elemType = dropdownMenu
		zIndex = 5
		image = dropDownExpAreaBack
		fillXLess = 0up

		# Remove this if you want more spacing for menu drop down items
		menuLeftNoIconPadding = 0up

		isVertical = True

		width = 240up
		height = 200up

		animateChild = True
		tags = submenu
		isHidden = True
		menuCheckmarkImage = smallIcons:137

		textOffsetX = 1.5up
		font = ftMainFontSmall
		postponeRendering = True

		supportsAutoScrollingY = True

		selectedTextColor = { 1.0, 1.0, 1.0, 1.0 }

	</window>

	<elem>
		id = __dropdownTemplateScrollbar__
		elemType = slider
		image = vScrollKnob
		secondaryImage = vScrollBody
		x = -5.5up
		y = 9.5up
		width = 11up
		height = 40up
	

		sliderParms = { scrollElemsAnim:2, scrollElemsAnim:4 }
		sliderButtonSizes = { 10up, 25.5up, 10up, 10up } 
		isVertical = True
		backMode = fill
		layoutWidth = 1up
		linkTo = ../__dropdownTemplate__
		postponeRendering = True
		isHidden = True
		dontHideSubmenusOnClick = True
	</elem>

	<elem>
		id = __defDDText__
		text = Test
		postponeRendering = True

		fillXLess = 0up
		height = 17up
		textOffsetX = 5up
		textOffsetY = 0.5up
		text = Test
		textHorAlign = Left
		textPercOffsetX = -0.5
		font = ftMainFontSmall

		isHidden = True
	</elem>

	<elem>
		id = defButton
		elemType = checkbox
		image = ddButtonUp
		secondaryImage = ddButtonPressed
		x = -3up
		y = 0.0up
		width = 10up		
		height = 10up
		align = { Right, __none__ }
		action = uiaToggleDropdown
		# Set dynamically
		postponeRendering = False
	</elem>
</dropdown>

<topDropdown definition extends dropdown>
	align = { __none__, Top }
	fillXLess = 10up
</topDropdown>

# File path: Template for a file path control
<file definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	align = { __none__, Top }
	fillXLess = 10up
	height = 20up		

	layout = row
	isLogicalElemsContainer = True

	<elem>
		id = defLabel
		text = Label
		textHorAlign = Left
		layoutWidth = 55up
		textOffsetX = -27.5up
		<elem>
			id = defLabelParmName
			textOffsetX = -27.5up
			textOffsetY = 0up
			text = Test1
			textHorAlign = Left
			x = 1up
			y = 8up
			font = ftMainFontTiny
			textColor = #7a7a7a
			isHidden = True
		</elem>
	</elem>

	<elem>
		id = defTextBox
		elemType = textField
		image = textBox

		backMode = fill
		width = 172up
		height = 18up
		textOffsetX = 3up
		textOffsetY = 0.5up
		textPercOffsetX = -0.5
		layoutWidth = 1.0

		textHorAlign = Left	
		text = Test

		isParmReceptor = True
	</elem>

	<elem>
		id = deftextButton
		elemType = button
		image = buttonUpAnim
		secondaryImage = buttonDownAnim
		text = Browse...
		layoutWidth = 57up
		width = 57up
		height = 18up
		action = uiaSetLinkedElemValueToFile
		linkTo = ../defTextBox
		marginLeft = 10up
	</elem>

</file>

# Labelled single line text
<stringInput definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	fillXLess = 0up
	height = 20up		

	layout = row
	isLogicalElemsContainer = True

	<elem>
		id = defLabel
		text = Label
		textHorAlign = Left
		layoutWidth = 50up
		textOffsetX = -25up
		<elem>
			id = defLabelParmName
			textOffsetX = -25up
			textOffsetY = 0up
			text = Test1
			textHorAlign = Left
			x = 1up
			y = 8up
			font = ftMainFontTiny
			textColor = #7a7a7a
			isHidden = True
		</elem>
	</elem>

	<elem>
		id = defPadding
		layoutWidth = 5up
	</elem>

	<elem>
		id = defTextBox
		elemType = textField
		image = textBox

		backMode = fill
		width = 172up
		height = 18up
		textOffsetX = 3up
		textOffsetY = 0.5up
		textPercOffsetX = -0.5
		layoutWidth = 1.0

		textHorAlign = Left	
		text = Test

		isParmReceptor = True
	</elem>
</stringInput>

<topStringInput definition extends stringInput>
	align = { __none__, Top }
	fillXLess = 10up
</topStringInput>

# Labelled multiline text box
<multilineInput definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	fillXLess = 0up
	height = 60up	

	layout = row
	isLogicalElemsContainer = True

	<elem>
		id = defLabel
		text = Label
		textHorAlign = Left
		layoutWidth = 50up
		textOffsetX = -25up
		y = -25up
		width = 50up
		height = 30up
		<elem>
			id = defErrorIcon
			image = tinyElems:34
			width = 8up
			height = 8up
			x = 15up
			y = 24up
			isHidden = True
		</elem>
		<elem>
			id = defLabelParmName
			textOffsetX = -25up
			textOffsetY = 0up
			text = Test1
			textHorAlign = Left
			x = 1up
			y = 8up
			font = ftMainFontTiny
			textColor = #7a7a7a
			isHidden = True
		</elem>
	</elem>

	<elem>
		id = defTextBox
		elemType = multilineTextField
		image = textBox

		backMode = fill
		width = 172up
		height = 60up
		textOffsetX = 3up
		textOffsetY = 0.5up
		textPercOffsetX = -0.5
		textPercOffsetY = -0.5
		layoutWidth = 1.0

		textHorAlign = Left	
		textVertAlign = Top
		text = Test
		y = 0up
		isParmReceptor = True
	</elem>
</multilineInput>

<topMultilineInput definition extends multilineInput>
	align = { __none__, Top }
	fillXLess = 10up
</topMultilineInput>

# Checkbox
<checkbox definition>
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	fillXLess = 0up
	height = 18up

	textHorAlign = Left
	textVertAlign = Center
	font = ftMainFontMedSmall

	elemType = checkbox
	image = smallElemsAnim:3
	secondaryImage = smallElemsAnim:4
	text = Checkbox

	isParmReceptor = True
	isLogicalElemsContainer = True
	
	textOffsetX = 11up
	textOffsetY = 0.5up

	imagePercOffsetX = -0.5
	imageOffsetX = 10up

	<elem>
		id = defLabelParmName
		fillXLess = 0up
		height = 2up
		textPercOffsetX = -0.5
		textOffsetX = 21up
		textOffsetY = 0up
		text = Test1
		textHorAlign = Left
		x = 0up
		y = 8up
		font = ftMainFontTiny
		textColor = #7a7a7a
		isHidden = True
	</elem>
</checkbox>

<customCheckbox definition extends checkbox>
	align = { __none__, Top }
	fillXLess = 35up
	x = 25up
</customCheckbox>

# Radio button
<radioButton definition>
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	fillXLess = 0up
	height = 20up

	textHorAlign = Left
	textVertAlign = Center
	font = ftMainFontMedSmall

	elemType = button
	image = smallElemsAnim:1
	secondaryImage = smallElemsAnim:2
	text = Radio Button

	isParmReceptor = True
	isLogicalElemsContainer = True
	
	textOffsetX = 11up
	textOffsetY = 0.5up

	imagePercOffsetX = -0.5
	imageOffsetX = 10up
	action = __none__
</radioButton>

<customRadioButton definition extends radioButton>
	align = { __none__, Top }
	fillXLess = 35up
	x = 25up
</customRadioButton>


# Three radio buttons. It happens more often than you think
<offsetContainer definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	align = { __none__, Top }
	fillXLess = 10up
	x = 5up
	height = 20up	

	layout = row
	isLogicalElemsContainer = True

</offsetContainer>


<angle definition>
	elemType = roundSlider
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = roundSliderFull
	secondaryImage = roundSliderEmpty
	capImage = roundSliderKnob

	minSlidingDelta = 0.5

	width = 50up
	height = 50up
</angle>

<label definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	offOutOpacity = 0.0
	zIndex = 2
	image = __none__

	height = 20up		

	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	align = { __none__, Top }
	fillXLess = 10up
	textPercOffsetX = -0.5

	textHorAlign = Left
	textVertAlign = Center
	font = ftMainFontMedSmall
	text = Label
</label>

# Template for a numeric control - no slider, just up/down
<uiNumeric definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	offOutOpacity = 0.0
	zIndex = 2
	image = __none__

	width = 67up
	height = 18up		

	<elem>
		id = defErrorIcon
		image = tinyElems:34
		width = 8up
		height = 8up
		x = -28up
		isHidden = True
	</elem>
	
	<elem>
		id = defNumBox
		elemType = textField
		width = 42up
		height = 17up		
		image = textBox	
		textHorAlign = Left
		textOffsetX = -16up	
		text = 0
		textFormat = %d

		minSlidingDelta = 1
		isParmReceptor = True
		font = ftMainFontSmall

#ifdef PRO_VERSION
		elemType = expressionTextField
		dontHidePopupOnClick = True
		rmbAction = uiaToggleContextMenuAtMouseLocation
		rmbActionValue = expressionTextEditRMB
		rmbActionValue2 = Right
#endif
	</elem>
	
	<elem>
		id = defNumBoxUpButton
		elemType = button
		width = 12up
		height = 8up
		image = buttonUp
		secondaryImage = buttonUpPressed
		x = 26up
		y = -4up
		action = uiaChangeLinkedElemValueBy
		actionValue = 1
		linkTo = ../defNumBox
	</elem>
	
	<elem>
		id = defNumBoxDownButton
		elemType = button
		width = 12up
		height = 8up
		image = buttonDown
		secondaryImage = buttonDownPressed
		x = 26up
		y = 4up
		action = uiaChangeLinkedElemValueBy
		actionValue = -1
		linkTo = ../defNumBox
	</elem>
	
</uiNumeric>

# Template for a SMALL numeric control - no slider, just up/down
<uiSmallNumeric definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	offOutOpacity = 0.0
	zIndex = 2
	image = __none__

	width = 40up
	height = 15up		
	
	<elem>
		id = defNumBox
		elemType = textField
		width = 30up
		height = 16up		
		image = textBox	
		textHorAlign = Left
		textOffsetX = -11up	
		text = 0
		textFormat = %d
		x = -9up
		font = ftMainFontSmall

		backMode = fill 
		isParmReceptor = True
		minSlidingDelta = 1
	</elem>
	
	<elem>
		id = defNumBoxUpButton
		elemType = button
		width = 12up
		height = 8up
		image = buttonUp
		secondaryImage = buttonUpPressed
		x = 12up
		y = -4up
		action = uiaChangeLinkedElemValueBy
		actionValue = 1
		linkTo = ../defNumBox
	</elem>
	
	<elem>
		id = defNumBoxDownButton
		elemType = button
		width = 12up
		height = 8up
		image = buttonDown
		secondaryImage = buttonDownPressed
		x = 12up
		y = 4up
		action = uiaChangeLinkedElemValueBy
		actionValue = -1
		linkTo = ../defNumBox
	</elem>
	
</uiSmallNumeric>

# Numeric up/down with a label
<uiNumericLabeled definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	align = { __none__, Top }
	fillXLess = 10up
	height = 18up		

	layout = row
	isLogicalElemsContainer = True
	
	<elem>
		id = defLabel
		height = 18up		
		image = __none__
		layoutWidth = 1up
		textOffsetY = 0.5up
		text = Test1
		textHorAlign = Left
	</elem>

	<elem>
		id = defPadding
		layoutWidth = 1.0
	</elem>
		
	<uiNumeric>
		id = defLabeledNumUpdown
		layoutWidth = 67up
	</uiNumeric>

</uiNumericLabeled>

<uiRadioNumericLabeled definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	align = { __none__, Top }
	fillXLess = 10up
	height = 18up		

	layout = row
	isLogicalElemsContainer = True
	
	<customRadioButton>
		id = defLabel
		width = 33up
		layoutWidth = 33up
		height = 18up		
		text = Test
		isParmReceptor = False
		fillXLess = -1
		textOffsetX = 8up
		textOffsetY = 0up
	</customRadioButton>

	<elem>
		id = defPadding
		layoutWidth = 1.0
	</elem>
		
	<uiNumeric>
		id = defLabeledNumUpdown
		layoutWidth = 67up
	</uiNumeric>
</uiRadioNumericLabeled>

# A label and a text value. Changed programmatically.
<labeledText definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	align = { __none__, Top }
	fillXLess = 10up
	height = 18up		

	layout = row
	isLogicalElemsContainer = True

	<elem>
		id = defLabel
		text = Label
		textHorAlign = Left
		layoutWidth = 50up
		textOffsetX = -25up
	</elem>

	<elem>
		id = defPadding
		layoutWidth = 1.0
	</elem>

	<elem>
		id = defValue
		text = Value
		textHorAlign = Right
		textOffsetX = 45up
		width = 90up
		layoutWidth = 90up
	</elem>
</labeledText>

<labeledDropdown definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	fillXLess = 0up
	height = 18up		

	layout = row
	isLogicalElemsContainer = True

	<elem>
		id = defLabel
		text = Label
		textHorAlign = Left
		layoutWidth = 50up
		textOffsetX = -25up
		<elem>
			id = defLabelParmName
			textOffsetX = -25up
			textOffsetY = 0up
			text = Test1
			textHorAlign = Left
			x = 1up
			y = 8up
			font = ftMainFontTiny
			textColor = #7a7a7a
			isHidden = True
		</elem>
	</elem>

	<elem>
		id = defPadding
		#layoutWidth = 1.0
		layoutWidth = 40up
	</elem>

	<topDropdown>
		id = defDropdown
		align = { __none__, __none__ }
		#fillXLess = -1
		#width = 90up
		#layoutWidth = 90up
		x = 27.5up
		layoutWidth = 1.0
		isParmReceptor = True
		<elem>
			id = defButton
			#x = 37up
			#align = { __none__, __none__ }
		</elem>

		<elem>
			id = defTextPart
			textOffsetX = 3up	
			textOffsetY = -0.5up
			uioDisplayUnits = False
		</elem>
		<elem>
			id = __defDDText__
			textOffsetX = 3up	
			textOffsetY = -0.5up
		</elem>
	</topDropdown>

</labeledDropdown>

<topLabeledDropdown definition extends labeledDropdown>
	align = { __none__, Top }
	fillXLess = 10up
</topLabeledDropdown>

<slider definition>
	elemType = slider
	image = progressKnob
	secondaryImage = progressEmpty
	sliderParms = { __none__, __none__, progressFull, sliderPopAnim }
	sliderButtonSizes = { 20up, 20up }
		
	layoutWidth = 1.0
	height = 20up

	sliderMarkParms = { 2up, 10 }
	sliderMarkColor = { 0.230, 0.230, 0.230, 1.0 }
	sliderValParms = { -15up, 1.0 }
	textFormat = %.1f
	textOffsetX = -0.5up
	textOffsetY = -2.0up

	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	selectedTextColor = { 0.8, 0.8, 0.8, 1.0 }	
	font = ftMainFontTinySmall
	#startInMiddle = True
</slider>


# Slider with text and numeric updown template
<uiNumericSlider definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	align = { __none__, Top }
	fillXLess = 10up
	height = 20up		

	layout = row

	isLogicalElemsContainer = True

	<elem>
		id = defLabel
		height = 18up		
		image = __none__
		layoutWidth = 50up
		textOffsetX = -25up
		textOffsetY = 0.5up
		text = Test
		textHorAlign = Left
		<elem>
			id = defLabelParmName
			textOffsetX = -25up
			textOffsetY = 0up
			text = Test1
			textHorAlign = Left
			x = 1up
			y = 8up
			font = ftMainFontTiny
			textColor = #7a7a7a
			isHidden = True
		</elem>
	</elem>

	<slider>
		id = defNumSliderSlider
		linkTo = ../defNumSliderUpdown/defNumBox
	</slider>
	
	<uiNumeric>
		id = defNumSliderUpdown
		layoutWidth = 67up
		<elem>
			id = defNumBox
			linkTo = ../../defNumSliderSlider
		</elem>
	</uiNumeric>

</uiNumericSlider>

<numericSlider definition extends uiNumericSlider>
	align = { __none__, __none__ }
	fillXLess = 0up
</numericSlider>

# Slider with text and numeric updown template
<uiNumericRoundSlider definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	align = { __none__, Top }
	fillXLess = 10up
	height = 50up		

	layout = row
	isLogicalElemsContainer = True

	<elem>
		id = defLabel
		height = 18up		
		image = __none__
		layoutWidth = 50up
		textOffsetX = -25up
		textOffsetY = 0.5up
		text = Test1
		textHorAlign = Left
		<elem>
			id = defLabelParmName
			textOffsetX = -25up
			textOffsetY = 0up
			text = Test1
			textHorAlign = Left
			x = 1up
			y = 8up
			font = ftMainFontTiny
			textColor = #7a7a7a
			isHidden = True
		</elem>
	</elem>

	<angle>
		id = defNumSliderSlider
		layoutWidth = 1.0
		linkTo = ../defNumSliderUpdown/defNumBox
	</angle>
	
	<uiNumeric>
		id = defNumSliderUpdown
		layoutWidth = 67up
		<elem>
			id = defNumBox
			linkTo = ../../defNumSliderSlider
#ifdef PRO_VERSION
			elemType = expressionTextField
			dontHidePopupOnClick = True
			rmbAction = uiaToggleContextMenuAtMouseLocation
			rmbActionValue = expressionTextEditRMB
			rmbActionValue2 = Right
#endif
		</elem>
	</uiNumeric>

</uiNumericRoundSlider>

<numericRoundSlider definition extends uiNumericRoundSlider>
	align = { __none__, __none__ }
	fillXLess = 0up
</numericRoundSlider>

# Short Slider with text and numeric updown template
# Leaves more space for text
<uiShortNumericSlider definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	align = { __none__, Top }
	fillXLess = 10up
	height = 20up		

	layout = row
	isLogicalElemsContainer = True

	<elem>
		id = defLabel
		height = 18up		
		image = __none__
		layoutWidth = 80up
		textOffsetX = -40up
		textOffsetY = 0.5up
		text = Test1
		textHorAlign = Left
		<elem>
			id = defLabelParmName
			textOffsetX = -40up
			textOffsetY = 0up
			text = Test1
			textHorAlign = Left
			x = 1up
			y = 8up
			font = ftMainFontTiny
			textColor = #7a7a7a
			isHidden = True
		</elem>
	</elem>

	<elem>
		id = defNumSliderSlider
		elemType = slider
		image = progressKnob
		secondaryImage = progressEmpty
		sliderParms = { __none__, __none__, progressFull, sliderPopAnim }
		sliderButtonSizes = { 20up, 20up }
		
		layoutWidth = 1.0
		height = 20up

		sliderMarkParms = { 2up, 10 }
		sliderMarkColor = { 0.230, 0.230, 0.230, 1.0 }
		sliderValParms = { -15up, 1.0 }
		textFormat = %.1f
		textOffsetX = -0.5up
		textOffsetY = -2.0up

		textColor = { 0.8, 0.8, 0.8, 1.0 }	
		selectedTextColor = { 0.8, 0.8, 0.8, 1.0 }	
		font = ftMainFontTinySmall

		linkTo = ../defNumSliderUpdown/defNumBox

		#startInMiddle = True
	</elem>
	
	<uiNumeric>
		id = defNumSliderUpdown
		layoutWidth = 67up
		<elem>
			id = defNumBox
			linkTo = ../../defNumSliderSlider
		</elem>
	</uiNumeric>

</uiShortNumericSlider>

# Small slider with text and numeric updown template
<uiSmallNumericSlider definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	fillXLess = 10up
	height = 18up		

	layout = row
	isLogicalElemsContainer = True

	<elem>
		id = defLabel
		height = 18up		
		image = __none__
		layoutWidth = 36up
		textOffsetX = -18up
		textOffsetY = 0.5up
		text = Test1
		textHorAlign = Left
	</elem>

	<elem>
		id = defNumSliderSlider
		elemType = slider
		image = progressKnob
		secondaryImage = progressEmpty
		sliderParms = { __none__, __none__, progressFull, sliderPopAnim }
		sliderButtonSizes = { 20up, 20up }
		
		layoutWidth = 1.0
		height = 18up

		#sliderMarkParms = { 2up, 10 }
		#sliderMarkColor = { 0.230, 0.230, 0.230, 1.0 }
		sliderValParms = { -15up, 1.0 }
		textFormat = %.1f
		textOffsetX = -0.5up
		textOffsetY = -2.0up

		textColor = { 0.8, 0.8, 0.8, 1.0 }	
		selectedTextColor = { 0.8, 0.8, 0.8, 1.0 }
		font = ftMainFontTinySmall

		linkTo = ../defNumSmallSliderUpdown/defNumBox

		#startInMiddle = True
	</elem>

	
	<uiSmallNumeric>
		id = defNumSmallSliderUpdown
		layoutWidth = 40up
		marginLeft = 8up
		<elem>
			id = defNumBox
			linkTo = ../../defNumSliderSlider
		</elem>
	</uiSmallNumeric>

</uiSmallNumericSlider>


# Color, in a row
<uiColor definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0
	zIndex = 2
	image = __none__

	align = { __none__, Top }
	fillXLess = 10up
	height = 20up

	layout = row
	isLogicalElemsContainer = True

	# This isn't really linked, it's here so that when we hide/show this
	# we also hide/show the whole thing.
	linkTo = defColor

	<elem>
		id = defLabel
		height = 18up
		image = __none__
		layoutWidth = 50up
		textOffsetX = -25up
		textOffsetY = 0.5up
		text = Color
		textHorAlign = Left
		<elem>
			id = defLabelParmName
			textOffsetX = -25up
			textOffsetY = 0up
			text = Test1
			textHorAlign = Left
			x = 1up
			y = 8up
			font = ftMainFontTiny
			textColor = #7a7a7a
			isHidden = True
		</elem>
	</elem>

	<elem>
		id = defPadding
		layoutWidth = 1.0
	</elem>

	<elem>
		id = defColor
		elemType = colorSwatch
		layoutWidth = 20up
		width = 20up
		height = 13up
		image = largeSwatchDarkBackAnim
		marginRight = 2up
		isParmReceptor = True
	</elem>

	<uiNumeric>
		id = defRed
		layoutWidth = 40up
		width = 40up
		<elem>
			id = defNumBox
			tags = redchannel
			min = 0
			max = 255
			width = 27up
			textOffsetX = -9up	
			backMode = fill
			linkTo = ../../defColor
			x = -5up
			isParmReceptor = False
		</elem>
		<elem>
			id = defNumBoxUpButton
			x = 14up
		</elem>
		<elem>
			id = defNumBoxDownButton
			x = 14up
		</elem>
	</uiNumeric>

	<uiNumeric>
		id = defGreen
		layoutWidth = 40up
		width = 40up
		<elem>
			id = defNumBox
			min = 0
			max = 255
			width = 27up
			textOffsetX = -9up	
			backMode = fill
			linkTo = ../../defColor
			x = -5up
			tags = greenchannel
			isParmReceptor = False
		</elem>
		<elem>
			id = defNumBoxUpButton
			x = 14up
		</elem>
		<elem>
			id = defNumBoxDownButton
			x = 14up
		</elem>
	</uiNumeric>

	<uiNumeric>
		id = defBlue
		layoutWidth = 40up
		width = 40up
		<elem>
			id = defNumBox
			min = 0
			max = 255
			width = 27up
			textOffsetX = -9up	
			backMode = fill
			linkTo = ../../defColor
			x = -5up
			tags = bluechannel
			isParmReceptor = False
		</elem>
		<elem>
			id = defNumBoxUpButton
			x = 14up
		</elem>
		<elem>
			id = defNumBoxDownButton
			x = 14up
		</elem>

		marginRight = 2up
	</uiNumeric>

</uiColor>

<color definition extends uiColor>
	align = { __none__, __none__ }
	fillXLess = 0up
</color>

<uiHistogram definition>
	elemType = histogram
	transitionTime = 0.10
	opacity = 1.0
	zIndex = 2
	image = __none__

	align = { __none__, Top }
	fillXLess = 10up
	height = 80up
	isParmReceptor = True

	<customCheckbox>
		id = defHistR
		text = R
		align = { __none__, Top }
		fillXLess = -1
		width = 40up
		x = -60up
		y = 0up
		action = toggleHistogramChannel
		actionValue = Red
		isParmReceptor = False
	</customCheckbox>

	<customCheckbox>
		id = defHistG
		text = G
		align = { __none__, Top }
		fillXLess = -1
		width = 40up
		x = -20up
		y = 0up
		action = toggleHistogramChannel
		actionValue = Green
		isParmReceptor = False
	</customCheckbox>

	<customCheckbox>
		id = defHistB
		text = B
		align = { __none__, Top }
		fillXLess = -1
		width = 40up
		x = 20up
		y = 0up
		action = toggleHistogramChannel
		actionValue = Blue
		isParmReceptor = False
	</customCheckbox>


	<customCheckbox>
		id = defHistRGB
		text = RGB
		align = { __none__, Top }
		fillXLess = -1
		width = 40up
		x = 60up
		y = 0up
		checked = True
		action = toggleHistogramChannel
		actionValue = Intensity
		isParmReceptor = False
	</customCheckbox>

</uiHistogram>

<fixedDialog definition>
	elemType = static
	transitionTime = 0.10
	offOpacity = 0.0
	opacity = 1.0

	textHorAlign = Center
	textVertAlign = Center
	font = ftMainFontMedSmall

	fillXLess = 0.0
	fillYLess = 0.0

	
	backMode = fill
	
	textColor = { 0.8, 0.8, 0.8, 1.0 }	
	textShadowColor = { 0.0, 0.0, 0.0, 1 }
	textShadowOffsetX = 0
	textShadowOffsetY = -0.5up

	zIndex = 7
	image = __none__

	haveOwnUndoManager = True
	backColor = { 0, 0, 0, 0.6 }

	<window>
		id = defDialogInner
		width = 200up
		height = 95up

		windowBarImage = windowTitleBar
		title = Dialog Title
	</window>

</fixedDialog>

<vertSplitter definition>
	elemType = splitter
	isVertical = True
	splitterParms = { vertSepBar, vertBarHandle }
	splitterPos = 0.5
</vertSplitter>

<horSplitter definition>
		elemType = splitter
		isVertical = False
		splitterParms = { horSepBar, horBarHandle }
		splitterPos = 0.5				
</horSplitter>

<richText definition extends elem>
	fillXLess = 0up
	fillYLess = 0up
	<elem>
		id = defContent
		elemType = richText
		fillXLess = 5.5up
		fillYLess = 0up
		x = -5.5up
		textWidthLessPadding = 4up
		rtPaddingLeft = 2up
		text = ___[[[ Contents ]]]___
	</elem>

	<vertScrollbar>
		id = defScrollbar
		linkTo = ../$0
		x = 0up
		y = 0up		
		align = { Right, __none__ }
	</vertScrollbar>
</richText>

###############################################################################
# Message Box Dialog
###############################################################################
<autoDialog>
	id = messageBox
	layer = uilMessageBox
	zIndex = 8
	haveOwnUndoManager = False
	<window>
		id = defDialogWindow
		title = Message
		<elem>
			id = defDialogInner
			<elem>
				id = msgBoxDialogText
				textWidth = 170up
				height = 40up
			</elem>
		</elem>
		<elem>
			id = defBottomButtonContainer
			<defaultButton>
				id = defOkButton
				action = hideTopParent
				x = 0up
			</defaultButton>
			<button>
				id = defCancelButton
				isHidden = True
			</button>
		</elem>
	</window>
</autoDialog>


