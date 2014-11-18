#include <gtest/gtest.h>
#include <random_ext.hpp>
#include <tostring_ext.hpp>

using namespace util;
using namespace std;

TEST(RandomExtTest, RandomTest) {
  srand(time(NULL));

  vector<uint16_t> array;
  vector<vector<uint32_t> > datas;

  RandomExt::RandomArray<uint16_t>(100, 20, 500, array);
  RandomExt::RandomBitmap(100, 10, 0.3, datas);

  EXPECT_STREQ("", ToStringExt<uint16_t>::ToString(array).c_str());
  EXPECT_STREQ("", ToStringExt<uint32_t>::ToString(datas).c_str());
}


