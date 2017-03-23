#ifndef UTIL_STATUS_MACROS_H
#define UTIL_STATUS_MACROS_H

#include "gtest/gtest.h"

namespace testing {

#define EXPECT_OK(status) \
  EXPECT_TRUE(status.ok()) << "Expected OK, was " << status << "\n";

}

#endif
