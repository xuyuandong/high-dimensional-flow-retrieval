#include <gtest/gtest.h>
#include <pv_info.h>
#include <tostring_ext.hpp>
#include <random_ext.hpp>
using namespace ea;
using namespace util;
using namespace std;

TEST (PvInfoTest, PvLoadDumpTest) {
  srand(time(NULL));
  vector<uint32_t> hour_pv;
  vector<uint32_t> hour_request;
  vector<float> hour_ctr;
  RandomExt::RandomArray<uint32_t>(24, 2000, 5000, hour_pv);
  RandomExt::RandomArray<uint32_t>(24, 10000, 20000, hour_request);
  RandomExt::RandomArray<float>(24, 0.0001, 0.001, hour_ctr);

  string psid = "PDPS000000TEST";
  uint64_t request = RandomExt::Random<uint64_t>(1000000, 2500000);
  uint64_t pv = RandomExt::Random<uint64_t>(100000, 1200000);
  
  vector<uint32_t> hours;
  RandomExt::RandomArray<uint32_t>(11, 0, 23, hours);

  PvInfo info;
  EXPECT_TRUE(info.Generate(psid, request, pv, hour_request, hour_pv, hour_ctr));
  EXPECT_TRUE(info.IsValid());

  EXPECT_EQ(0.0, info.GetRate());
  EXPECT_EQ(pv, info.GetPv());
  EXPECT_EQ(request, info.GetRequest());

  EXPECT_EQ(0.0, info.GetHourlyCtr(hours));

  EXPECT_STREQ("", info.ToString().c_str());

  FILE *out = fopen("pvinfo_output", "wb");
  EXPECT_EQ(0, info.Dump(out));
  fclose(out);
  FILE *in = fopen("pvinfo_output", "rb");
  EXPECT_EQ(0, info.Load(in));
  fclose(in);
  EXPECT_STREQ("", info.ToString().c_str());
}
