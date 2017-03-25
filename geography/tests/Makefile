# Makefile for geography tests.
include common.mk
include tests.mk
include libs.mk

LIBS += $(GEOGRAPHY_LIB)
LIBS += $(MARKET_LIB)
LIBS += $(PROTOBUF_LIB)
LIBS += $(INDUSTRY_LIB)
LIBS += $(STATUS_LIB)

default: geography_tests

geography_tests:	$(patsubst %test.cc,%test.o,$(wildcard *test.cc)) gtest_main.a
			$(LINKER) $^ -o $@ $(LIBS)

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



