# MIT License
#
# Copyright (c) 2022 Ignacio Vizzo, Tiziano Guadagnino, Benedikt Mersch, Cyrill
# Stachniss.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Simply ignore the warnings when using scikit-build as the build system
if(${SKBUILD})
  set(ignore ${PYTHON_NumPy_INCLUDE_DIRS})
  set(ignore ${Python3_EXECUTABLE})
  set(ignore ${Python3_INCLUDE_DIR})
  set(ignore ${Python3_LIBRARY})
  set(ignore ${Python3_NumPy_INCLUDE_DIRS})
  set(ignore ${Python_EXECUTABLE})
  set(ignore ${Python_INCLUDE_DIR})
  set(ignore ${Python_LIBRARY})
  set(ignore ${Python_NumPy_INCLUDE_DIRS})
  set(ignore ${Python3_FIND_REGISTRY})
  set(ignore ${Python3_ROOT_DIR})
  set(ignore ${Python_FIND_REGISTRY})
  set(ignore ${Python_ROOT_DIR})
endif()
