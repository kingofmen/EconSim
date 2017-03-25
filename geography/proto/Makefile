# Makefile for geography protobufs.
include common.mk

default: libgeography_proto.a

libgeography_proto.a:	$(patsubst %.proto,%.pb.o,$(wildcard *.proto))
			ar rcs $@ $^

%.pb.o:	%.pb.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

# Absolute paths because protoc isn't very bright about finding files.
%.pb.cc:	$(abspath $(wildcard *.proto))
		$(PROTOC) $(PROTO_PATH) --cpp_out=$(BASE) $^ 

clean:
	rm *.pb.*
	rm libgeography_proto.a





