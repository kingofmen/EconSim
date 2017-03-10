# Makefile for industry library.

CXX           = g++
LINKER        = g++
DEFINES       = -DUNICODE
BASE          = "C:/Users/Rolf/codebase"
PROTOBUF_INCLUDES = "c:/Users/Rolf/protobuf/src-3.2.0/src/"
CXXFLAGS      = -pipe -fno-keep-inline-dllexport -std=c++11 -frtti -Wall -Wextra -fexceptions -mthreads $(DEFINES) -isystem $(PROTOBUF_INCLUDES)
INCPATH       = -I. -I$(BASE) -I$(PROTOBUF_INCLUDES)


default: libindustry.a

libindustry.a:	$(patsubst %.cc,%.o,$(wildcard *.cc))
		ar rcs $@ $^

%.o:	%.cc %.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	rm *.o
	rm libindustry.a
