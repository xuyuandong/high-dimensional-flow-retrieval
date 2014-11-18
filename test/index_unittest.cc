#include <gtest/gtest.h>
#include <index.h>
#include <string_ext.hpp>
#include <tostring_ext.hpp>
#include <random_ext.hpp>

using namespace ea;
using namespace util;
using namespace std;

TEST(IndexTest, IndexAddTest) {
  srand(time(NULL));
  Index index;
  
  EXPECT_STREQ("", index.ToString().c_str());
  
  vector<vector<uint32_t> > datas;
  vector<uint16_t> bids;

  vector<uint32_t> hour_pv;
  vector<uint32_t> hour_request;
  vector<float> hour_ctr;
  RandomExt::RandomArray<uint32_t>(24, 2000, 5000, hour_pv);
  RandomExt::RandomArray<uint32_t>(24, 10000, 20000, hour_request);
  RandomExt::RandomArray<float>(24, 0.0001, 0.001, hour_ctr);
  
  uint64_t request = RandomExt::Random<uint64_t>(1000000, 2500000);
  uint64_t pv = RandomExt::Random<uint64_t>(100000, 1200000);

  RandomExt::RandomBitmap(10, 3, 0.3, datas);
  RandomExt::RandomArray<uint16_t>(10, 20, 400, bids);
  EXPECT_STREQ("", ToStringExt<uint32_t>::ToString(datas).c_str());
  EXPECT_STREQ("", ToStringExt<uint16_t>::ToString(bids).c_str());

  EXPECT_EQ(0, index.AddPvBidBitmap(string("PDPS00000001"), request, pv, hour_request, hour_pv, hour_ctr, datas, bids, 3));

  EXPECT_EQ(0, index.AddUvBidBitmap(string("PDPS00000001"), request, pv, datas, bids, 3));

  string tosplit = "a,b,c,d,e,f,g|h,i,j,k,l,m,n|o,p,q,r,s,t|u,v,w,x,y,z";
  vector<vector<string> > psids;
  StringExt::Split(tosplit, '|', ',', psids);
  
  EXPECT_EQ(0, index.AddUvMergeBitmap(psids));
  
  EXPECT_STREQ("", index.ToString().c_str());

  EXPECT_EQ(0, index.Dump("index.test"));
  EXPECT_EQ(0, index.Load("index.test"));
  EXPECT_STREQ("", index.ToString().c_str());
}

