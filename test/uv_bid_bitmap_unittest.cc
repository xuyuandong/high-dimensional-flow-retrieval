#include <gtest/gtest.h>
#include <uv_bid_bitmap.h>
#include <tostring_ext.hpp>
#include <string_ext.hpp>
#include <random_ext.hpp>

using namespace ea;
using namespace util;
using namespace std;

TEST (UvBidBitmapTest, UvBidBitmapTest) {
  UvBidBitmap map;
  vector<vector<uint32_t> > datas;
  vector<uint16_t> bids;

  RandomExt::RandomBitmap(10, 3, 0.3, datas);
  RandomExt::RandomArray<uint16_t>(10, 20, 400, bids);
  uint64_t request = RandomExt::Random<uint64_t>(2000, 50000);
  uint64_t uv = RandomExt::Random<uint64_t>(300, 4000);
  string psid = "PDPS009899850";
  EXPECT_TRUE(map.Generate(psid, request, uv, datas, bids, 3));
  EXPECT_STREQ("", map.ToString().c_str());
  FILE* out = fopen("uvmap_output", "wb");
  EXPECT_EQ(0, map.Dump(out));
  EXPECT_EQ(0, map.Dump(out));

  fclose(out);

  FILE* in = fopen("uvmap_output", "rb");
  EXPECT_EQ(0, map.Load(in));
  EXPECT_STREQ("", map.ToString().c_str());
  EXPECT_EQ(0, map.Load(in));
  EXPECT_STREQ("", map.ToString().c_str());

}
