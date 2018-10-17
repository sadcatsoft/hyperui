rmdir /S /Q dist
rmdir /S /Q build
rmdir /S /Q hyperui.egg-info
REM c:\Python36\python setup.py --tag-build=development sdist bdist_wheel
c:\Python36\python setup.py bdist_wheel