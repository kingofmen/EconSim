# Makefile for geography library.
include common.mk

default: libgeography.a

libgeography.a:	$(patsubst %.cc,%.o,$(wildcard *.cc))
		ar rcs $@ $^

%.o:	%.cc %.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	rm *.o
	rm libgeography.a
