#include <gtest/gtest.h>
#include <pv_bid_bitmap.h>
#include <tostring_ext.hpp>
#include <random_ext.hpp>
using namespace ea;
using namespace util;
using namespace std;

TEST (PvBidBitmapTest, PvLoadDumpTest) {
  srand(time(NULL));
  vector<uint32_t> hour_pv;
  vector<uint32_t> hour_request;
  vector<float> hour_ctr;
  vector<uint16_t> bids;
  vector<vector<uint32_t> > datas;

  RandomExt::RandomBitmap(10, 3, 0.3, datas);
  RandomExt::RandomArray<uint16_t>(10, 20, 400, bids);
  RandomExt::RandomArray<uint32_t>(24, 2000, 5000, hour_pv);
  RandomExt::RandomArray<uint32_t>(24, 10000, 20000, hour_request);
  RandomExt::RandomArray<float>(24, 0.0001, 0.001, hour_ctr);

  string psid = "PDPS000000TEST";
  uint64_t request = RandomExt::Random<uint64_t>(1000000, 2500000);
  uint64_t pv = RandomExt::Random<uint64_t>(100000, 1200000);
  
  vector<uint32_t> hours;
  RandomExt::RandomArray<uint32_t>(11, 0, 23, hours);

  PvBidBitmap map;
  EXPECT_TRUE(map.Generate(psid, request, pv, hour_request, hour_pv, hour_ctr, datas, bids, 3));

  EXPECT_EQ(0.0, map.GetRate());
  EXPECT_EQ(pv, map.GetPv());
  EXPECT_EQ(request, map.GetRequest());

  EXPECT_EQ(0.0, map.GetHourlyCtr(hours));

  EXPECT_STREQ("", map.ToString().c_str());

  FILE *out = fopen("pvmap_output", "wb");
  EXPECT_EQ(0, map.Dump(out));
  fclose(out);
  FILE *in = fopen("pvmap_output", "rb");
  EXPECT_EQ(0, map.Load(in));
  fclose(in);
  EXPECT_STREQ("", map.ToString().c_str());
}
