#include <gtest/gtest.h>

#include <param.h>

using namespace ea;
using namespace common;
using namespace std;

TEST(ParamTest, ParamLoadTest) {
  Param param;
  EXPECT_EQ(param.Load(string("param.conf")), 0);
  EXPECT_STREQ("", param.ToString().c_str());
}
