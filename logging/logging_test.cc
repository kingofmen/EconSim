#include "logging/logging.h"

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Log {

std::vector<std::string> messages;
Priority silenceLevel = P_UNKNOWN;

void TestListener(const std::string& m, Priority p) {
  if (p == silenceLevel) {
    return;
  }
  messages.push_back(m);
}

class LogTest : public testing::Test {
 protected:
  void SetUp() override {
    messages.clear();
    silenceLevel = P_UNKNOWN;
    Log::UnRegister(TestListener);
    Log::Register(TestListener);
  }
};

TEST_F(LogTest, NullListener) {
  Log::UnRegister(TestListener);
  Log::Register(NULL);
  Log::Trace("Trace");
  EXPECT_TRUE(messages.empty());
}

TEST_F(LogTest, UnRegister) {
  Log::Register(TestListener);
  Log::UnRegister(TestListener);
  Log::Trace("Check");
  EXPECT_TRUE(messages.empty());
}

TEST_F(LogTest, Message) {
  Log::Trace("Trace");
  Log::Debug("Debug");
  Log::Info("Info");
  Log::Warn("Warn");
  Log::Error("Error");
  EXPECT_THAT(messages,
              testing::ElementsAre("Trace", "Debug", "Info", "Warn", "Error"));
}

TEST_F(LogTest, Format) {
  Log::Tracef("%s %d %.2f", "Trace", 42, 1.119);
  Log::Infof("%s %d %.2f", "Info", 43, 2.203);
  EXPECT_THAT(messages, testing::ElementsAre("Trace 42 1.12", "Info 43 2.20"));
}

TEST_F(LogTest, Levels) {
  Log::UnRegister(TestListener);
  Log::Register(TestListener, P_INFO);
  silenceLevel = P_WARN;
  Log::Trace("Trace");
  Log::Debug("Debug");
  Log::Info("Info");
  Log::Warn("Warn");
  Log::Error("Error");
  EXPECT_THAT(messages, testing::ElementsAre("Info", "Error"));
}

} // namespace Log
