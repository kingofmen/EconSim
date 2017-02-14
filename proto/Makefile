# Makefile for market protobufs.

MAKEFILE      = Makefile
CXX           = g++
LINKER        = g++
DEFINES       = -DUNICODE
PROTOBUF_INCLUDES = "c:/Users/Rolf/protobuf/src-3.2.0/src/"
PROTOBUF_LIBS = "c:/Users/Rolf/protobuf/src-3.2.0/cmake/build/release/ -lprotobuf"
PROTOC        = "C:/Program Files (x86)/protoc/bin/protoc.exe"
CXXFLAGS      = -pipe -fno-keep-inline-dllexport -Wno-unused-local-typedefs -std=c++11 -g -frtti -Wall -Wextra -fexceptions -mthreads $(DEFINES)
INCPATH       = -I. -I$(PROTOBUF_INCLUDES)
LIBS          = -L$(PROTOBUF_LIBS)

default: market_proto.lib

market_proto.lib:	goods.pb.o
			ar rcs $@ $< 

%.pb.o:	%.pb.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

%.pb.cc:	%.proto
		$(PROTOC) $<


