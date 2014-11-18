#include <gtest/gtest.h>

#include <bitmap.hpp>

using namespace common;
using namespace std;

TEST (BitmapTest, nullTest) {
  Bitmap map;

  EXPECT_STREQ("", map.ToString().c_str());
}
