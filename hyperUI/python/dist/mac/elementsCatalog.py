import hyperui
import sys, os

element_types = [
    ['Color Palette', 'exampleColorPalette'],
    ['Auto Dialog', 'exampleAutoDialog'],
    ["Horizontal Layout", "exampleHorizontalLayout"],
    ["Vertical Layout", "exampleVerticalLayout"],
    ["Grid Layout", "exampleGrid"],
    ["Rich Text Element", "exampleRichText"],
    ["Horizontal Table", "exampleHorTable"],
    ["Vertical Table", "exampleVertTable"],
    ["Single Line Input", "exampleStringInput"],
    ["Splitter", "exampleSplitter"],
    ["Menu", "exampleMenu"],
    ["Color", "exampleColor"],
    ["Multline Input", "exampleMultilineInput"],
    ["Dropdown", "exampleDropdown"],
    ["Numeric Slider", "exampleNumericSlider"],
    ["Knob", "exampleKnob"],
    ["Radio Button", "exampleRadioButton"],
    ["Checkbox", "exampleCheckbox"],
    ["Progress Bar", "exampleProgressBar"],
    ["Button", "exampleButton"],
    ["Static Element", "exampleStatic"],
    ];

class TopSplitterElement(hyperui.UISplitterElement, hyperui.TableCellInfoProvider, hyperui.GridCellInfoProvider):
    def __init__(self):
        hyperui.UISplitterElement.__init__(self)
        hyperui.TableCellInfoProvider.__init__(self)
        hyperui.GridCellInfoProvider.__init__(self)

    def handle_action(self, source_element, action, data):

        did_handle = False
       
        if action == 'uiaTableRowSelected':
            selected_row = data['row']
            self.window.set_is_visible_for_all_with_tag('exampleElements', False)
            elem = self.window.get_element_by_id(element_types[selected_row][1])
            elem.set_is_visible(True)
            did_handle = True

        return did_handle

    def on_update(self):
        print("CustomUIElement::on_update for " + self.id)

    def get_num_rows(self, source_element):
        if source_element.id == 'leftElemTypesTable':
            return len(element_types)
        elif source_element.id == 'exampleHorTable':
            return 1
        else:
            return 32

    def get_num_columns(self, source_element):
        if source_element.id == 'exampleHorTable':
            return 32
        else:
            return 1

    def get_num_total_cells(self, source_element):
        return 32

    def update_cell(self, cell):
        target_child = cell.get_child_by_id('defCellContent')
        if cell.parent.id == 'leftElemTypesTable':
            target_child.text = element_types[cell.row][0]
        else:
            target_child.text = "Cell at (%d, %d)" % (cell.column, cell.row)

    def on_post_create_rows(self, soruce_table):
        if soruce_table.id == 'leftElemTypesTable':
            self.window.get_element_by_id('leftElemTypesTable').select_row(0)
            self.window.get_element_by_id('exampleProgressBar').set_progress(0.25)

    def on_timer_tick(self, time):
        progress_elem = self.window.get_element_by_id('exampleProgressBar')
        if progress_elem.get_is_visible():
            new_progress = progress_elem.get_progress() + 0.005
            if new_progress > 1.0:
                new_progress = 0.0
            progress_elem.set_progress(new_progress)

module_path = os.path.dirname(os.path.abspath(sys.argv[0]))

hyperui.register(TopSplitterElement)
hyperui.add_collection(os.path.join(module_path, 'elementsCatalogUI', 'resGraphics.txt'), hyperui.CollectionRole.Graphics, os.path.join(module_path, 'customGraphics'))

hyperui.enable_debugging()

wnd = hyperui.Window(os.path.join(module_path, 'elementsCatalogUI', 'resUI.txt'), 'UI Elements Catalog', -1, -1, 'firstLayer')
hyperui.run(wnd)
