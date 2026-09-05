CXX ?= g++
PYTHON ?= python3
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -g

.PHONY: all demo test autograd sanitize list verify package

all: demo

build/demo: main.cpp shared_ptr.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) main.cpp -o build/demo

demo: build/demo

test autograd:
	$(PYTHON) autograder/autograder.py --cxx "$(CXX)"

sanitize:
	$(PYTHON) autograder/autograder.py --cxx "$(CXX)" --sanitize

list:
	$(PYTHON) autograder/autograder.py --list

verify:
	$(PYTHON) instructor/verify.py --cxx "$(CXX)"

package:
	$(PYTHON) instructor/package.py --output dist/shared-pointer-lab.zip
