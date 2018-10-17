Represents an non-editable, multi-line text element that can contain other arbitrary elements in the text flow. The user can optionally interact with these contained elements if they support it (such as clicking a button, for example).

| | |
|-|-|
Tag | &lt;richText&gt;
elemType | richText
C++ Class | UIRichTextElement

## Example
```
<button>
	id = richTextButton
	width = 150up
</button>

<richText>
	id = exampleRichText
	fillXLess = 100up
	fillYLess = 40up
	borderColor = { 255, 255, 255, 0.8 }
	borderThickness = 0.25up
	<elem>
		id = defContent
		text = ___[[[This is an example of plain-text content, with a button with a customized text and action below.^
    {richTextButton rtPaddingLeft=10up text=___(((Custom Button)))___ action=showMessage actionValue=___(((Test message)))___}]]]___
	</elem>
</richText>
```

##Inner Document Format
Note that multi-line text can be specified using the `___[[[ ...multi-line text here... ]]]___` notation. Also note that the `^` symbol forces a line break.

The format of elements within the text is as follows:
```
{elementId rtPaddingLeft=10up property1=___(((stringValue1)))___ property2=___(((stringValue2)))___ ... }
```

| | |
|-|-|
elementId | An id of the element defined at the top level (i.e. not a child element of any other element) to use in this location.
rtPaddingLeft | An optional left-side padding for the element in universal pixels.
property1, property2, ... | These are optional properties to override on the element defined by `elementId`, and are useful in case the same element needs trigger different actions or display a different piece of text. For example, you can reuse the same button element, but specify `text=___(((Custom Button Text)))___`