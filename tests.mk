# Variables for gTest.
GTEST_DIR     = C:/Users/Rolf/googletest/googletest
GTEST_HEADERS = $(wildcard $(GTEST_DIR)/include/gtest/*.h) $(wildcard $(GTEST_DIR)/include/gtest/internal/*.h)
GTEST_SRCS_   = $(wildcard $(GTEST_DIR)/src/*.cc) $(wildcard $(GTEST_DIR)/src/*.h) $(GTEST_HEADERS)

INCPATH            += -I$(GTEST_DIR)/include
CXXFLAGS         += -isystem $(GTEST_DIR)/include

