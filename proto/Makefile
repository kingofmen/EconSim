# Makefile for market protobufs.

MAKEFILE      = Makefile
CXX           = g++
LINKER        = g++
DEFINES       = -DUNICODE
BASE          = "C:/Users/Rolf/"
PROTOBUF_INCLUDES = "c:/Users/Rolf/protobuf/src-3.2.0/src/"
PROTOC        = "C:/Program Files (x86)/protoc/bin/protoc.exe"
CXXFLAGS      = -pipe -fno-keep-inline-dllexport -std=c++11 -frtti -Wall -Wextra -fexceptions -mthreads -isystem $(PROTOBUF_INCLUDES) $(DEFINES)
INCPATH       = -I. -I$(BASE) -I$(PROTOBUF_INCLUDES)
PROTO_PATH    = --proto_path $(BASE) 

default: libmarket_proto.a

libmarket_proto.a:	$(patsubst %.proto,%.pb.o,$(wildcard *.proto))
			ar rcs $@ $^

market.pb.o:	goods.pb.o

%.pb.o:	%.pb.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

# Absolute paths because protoc isn't very bright about finding files.
# Note that if protoc isn't run on both files simultaneously, it will make
# very silly guesses about the namespaces in goods.proto when it imports it
# into market.proto.
%.pb.cc:	$(abspath $(wildcard *.proto))
		$(PROTOC) $(PROTO_PATH) --cpp_out=$(BASE) $^ 

clean:
	rm *.pb.o
	rm libmarket_proto.a
	rm *.pb.cc
	rm *.pb.h




