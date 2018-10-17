import setuptools
import os
from distutils.core import setup, Extension
from setuptools.command.build_ext import build_ext

class DummyExtension(Extension):
	def __init__(self, name, sourcedir=''):
		Extension.__init__(self, name, sources=[])
		self.sourcedir = os.path.abspath(sourcedir)

class DummyBuild(build_ext):
	def run(self):
		pass
		
	def build_extension(self, ext):
		pass

with open("DESCRIPTION.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="hyperui",
    version="1.0.0",
    author="Sad Cat Software",
    author_email="sadcatsoft@gmail.com",
    description="A cross-platform application and UI framework",
    long_description=long_description,
    long_description_content_type="text/markdown",
	license='MIT License',
    url="https://github.com/sadcatsoft/hyperui",
    packages=setuptools.find_packages(),
	include_package_data=True,
	keywords="ui framework opengl app",
	python_requires='>=3.3',
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
		"Topic :: Software Development :: Libraries :: Application Frameworks",
    ],
    ext_modules=[DummyExtension('cmake_example')],
	cmdclass=dict(build_ext=DummyBuild)
)