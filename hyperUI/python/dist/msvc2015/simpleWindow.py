import hyperui
import sys, os

class CustomUIElement(hyperui.UIElement):
    def __init__(self):
        hyperui.UIElement.__init__(self)

    def handle_action(self, source_element, action, data):
        
        print("CustomUIElement::handle_action " + action)
        did_handle = False
        
        if action == 'changeButtonText':
            self.text = 'The ' + source_element.id  +' button has been pressed last'
            did_handle = True

        return did_handle

    def on_update(self):
        print("CustomUIElement::on_update for " + self.id)

class PyWindow(hyperui.Window):
    myCounter = 0
    def __init__(self, file_path, title = '', w = -1, h = -1, first_layer = ''):
        hyperui.Window.__init__(self, file_path, title, w, h, first_layer)

    def handle_action(self, source_element, action):

        did_handle = False

        if action == 'incrementGlobalCounter':
            self.myCounter = self.myCounter + 1
            source_element.text = "PRESSED #" + str(self.myCounter)
            source_element.width = "200up"
            print(source_element.parent.id)
            source_element.parent.update()
            self.elements['exampleElement'].text = 'The ' + source_element.id  +' button has been pressed last'
            did_handle = True

        return did_handle

    def on_initialized(self):
        self.show_layer('firstLayer')

    def on_key_down(self, key, bControl, bAlt, bShift, bMacActualControl, isRepeat):
        print("Key down: " + str(key) + " is repeat: " + str(isRepeat))
        return False

    def on_key_up(self, key, bControl, bAlt, bShift, bMacActualControl):
        print("Key up: " + str(key))
        return False

module_path = os.path.dirname(os.path.abspath(sys.argv[0]))

hyperui.register(CustomUIElement)
hyperui.add_collection(os.path.join(module_path, "customUI", "resGraphics.txt"), hyperui.CollectionRole.Graphics, os.path.join(module_path, "customGraphics"))

wnd = PyWindow(os.path.join(module_path, "customUI", "resUI.txt"), "Simple App")
hyperui.run(wnd)
