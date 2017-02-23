# Makefile for market protobufs.

MAKEFILE      = Makefile
CXX           = g++
LINKER        = g++
DEFINES       = -DUNICODE
PROTOBUF_INCLUDES = "c:/Users/Rolf/protobuf/src-3.2.0/src/"
PROTOC        = "C:/Program Files (x86)/protoc/bin/protoc.exe"
CXXFLAGS      = -pipe -fno-keep-inline-dllexport -std=c++11 -frtti -Wall -Wextra -fexceptions -mthreads -isystem $(PROTOBUF_INCLUDES) $(DEFINES)
INCPATH       = -I. -I$(PROTOBUF_INCLUDES)

default: libmarket_proto.a

libmarket_proto.a:	goods.pb.o
			ar rcs $@ $< 

%.pb.o:	%.pb.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

%.pb.cc:	%.proto
		$(PROTOC) $< --cpp_out=./

clean:
	rm *.pb.o
	rm libmarket_proto.a
