# Makefile for status library.

CXX           = g++
LINKER        = g++
DEFINES       = -DUNICODE
BASE          = "C:/Users/Rolf/codebase"
CXXFLAGS      = -pipe -fno-keep-inline-dllexport -std=c++14 -frtti -Wall -Wextra -fexceptions -mthreads $(DEFINES)
INCPATH       = -I. -I$(BASE)


default: libstatus.a

libstatus.a:	$(patsubst %.cc,%.o,$(wildcard *.cc))
		ar rcs $@ $^

%.o:	%.cc %.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	rm *.o
	rm libstatus.a
