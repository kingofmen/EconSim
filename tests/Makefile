# Makefile for geography tests.

CXX           = g++
LINKER        = g++
# WINVER for OpenThread.
DEFINES       = -DUNICODE -DWINVER=0x0500

# Variables for gTest.
GTEST_DIR     = C:/Users/Rolf/googletest/googletest
GTEST_HEADERS = $(wildcard $(GTEST_DIR)/include/gtest/*.h) $(wildcard $(GTEST_DIR)/include/gtest/internal/*.h)
GTEST_SRCS_   = $(wildcard $(GTEST_DIR)/src/*.cc) $(wildcard $(GTEST_DIR)/src/*.h) $(GTEST_HEADERS)

PROTOBUF_INC        = "c:/Users/Rolf/protobuf/src-3.2.0/src/"
CXXFLAGS            = -pipe -fno-keep-inline-dllexport -Wno-unused-local-typedefs -std=gnu++0x -frtti -Wall -Wextra -fexceptions -mthreads $(DEFINES)
BASE                = "C:/Users/Rolf/codebase"
INCPATH             = -I. -I$(BASE) -I$(GTEST_DIR)/include -I$(PROTOBUF_INC)
PROTOBUF_LIB        = -L"C:/Users/Rolf/protobuf/src-3.2.0/cmake/build/release/" -lprotobuf
GEOGRAPHY_LIB        = -L$(BASE)/geography/ -lgeography
GEOGRAPHY_PROTO_LIB  = -L$(BASE)/geography/proto/ -lgeography_proto
MARKET_LIB          = -L$(BASE)/market/ -lmarket
MARKET_PROTO_LIB    = -L$(BASE)/market/proto/ -lmarket_proto
INDUSTRY_LIB        = -L$(BASE)/industry/ -lindustry
INDUSTRY_PROTO_LIB  = -L$(BASE)/industry/proto/ -lindustry_proto
LIBS                = -static -static-libgcc $(GEOGRAPHY_PROTO_LIB) $(MARKET_PROTO_LIB) $(PROTOBUF_LIB) $(GEOGRAPHY_LIB) $(MARKET_LIB) $(INDUSTRY_LIB) $(INDUSTRY_PROTO_LIB)
CXXFLAGS         += -isystem $(GTEST_DIR)/include
CXXFLAGS         += -isystem $(PROTOBUF_INC)

default: geography_tests

geography_tests:	$(patsubst %test.cc,%test.o,$(wildcard *test.cc)) gtest_main.a
		$(LINKER) $^ C:/Users/Rolf/protobuf/src-3.2.0/cmake/build/release/libprotobuf.a -o $@ $(LIBS)

gtest-all.o:	$(GTEST_SRCS_)
		$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc

%test.o:	%test.cc
		$(CXX) $(CXXFLAGS) $(INCPATH) -c -o $@ $<

gtest_main.a:	gtest-all.o gtest_main.o
		ar rcs $@ $^

gtest_main.o:	$(GTEST_SRCS_)
		$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest_main.cc

clean:
		rm -f geography_tests.exe
		rm *.o



