# Variables for gTest.
GTEST_DIR     = C:/Users/Rolf/googletest/googletest
GTEST_HEADERS = $(wildcard $(GTEST_DIR)/include/gtest/*.h) $(wildcard $(GTEST_DIR)/include/gtest/internal/*.h)
GTEST_SRCS_   = $(wildcard $(GTEST_DIR)/src/*.cc) $(wildcard $(GTEST_DIR)/src/*.h) $(GTEST_HEADERS)

INCPATH            += -I$(GTEST_DIR)/include
CXXFLAGS         += -isystem $(GTEST_DIR)/include

default: $(TARGET_NAME)_tests

$(TARGET_NAME)_tests:	$(patsubst %test.cc,%test.o,$(wildcard *test.cc)) gtest_main.a
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
		rm -f $(TARGET_NAME)_tests.exe
		rm *.o

