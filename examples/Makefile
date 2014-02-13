###############################################################################
#
#                          Makefile for VATA examples
#
###############################################################################

CFLAGS=-std=c++0x \
  -pedantic-errors \
  -Wextra \
  -Wall \
  -Wfloat-equal \
  -Wctor-dtor-privacy \
  -Weffc++ \
  -Woverloaded-virtual \
  -fdiagnostics-show-option \
	-g


INCLUDE=-I../include

LIBS_ADD=-L../build/src

LIBS=-lvata


###############################################################################

.PHONY: all clean

all: $(patsubst %.cc,%,$(wildcard *.cc))

%: %.cc
	g++ $(CFLAGS) $(INCLUDE) $(LIBS_ADD) $< $(LIBS) -o $@

clean:
	rm -rf $(patsubst %.cc,%,$(wildcard *.cc))
