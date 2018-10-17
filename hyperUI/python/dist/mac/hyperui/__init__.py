import os
import sys

module_path = os.path.dirname(os.path.abspath((__file__)))
sys.path.append(module_path)
from pyhyperui import *

initialize(module_path)

def register(py_class):
    register_uielement(py_class.__name__)

def add_collection(full_path, role=None, target_data_path=''):
    if role is None:
        role = CollectionRole.Generic
    add_collection_internal(full_path, role, target_data_path)

def run(window = None):
	return create_window(window)