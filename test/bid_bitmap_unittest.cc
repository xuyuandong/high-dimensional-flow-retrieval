#include <gtest/gtest.h>
#include <bid_bitmap.hpp>
#include <string_ext.hpp>
#include <tostring_ext.hpp>
#include <random_ext.hpp>

using namespace ea;
using namespace util;
using namespace common;
using namespace std;


TEST(BidBitmapTest, BidBitmapGenerateTest) {
  srand(time(NULL));

  BidBitmap bitmap;

  vector<vector<uint32_t> > datas;
  vector<uint16_t> bids;

  RandomExt::RandomBitmap(10, 3, 0.3, datas);
  RandomExt::RandomArray<uint16_t>(10, 20, 400, bids);
  EXPECT_STREQ("", ToStringExt<uint32_t>::ToString(datas).c_str());
  EXPECT_STREQ("", ToStringExt<uint16_t>::ToString(bids).c_str());

  EXPECT_TRUE(bitmap.Generate(datas, bids, 3));
  EXPECT_STREQ("", bitmap.ToString().c_str());

}

TEST(BidBitmapTest, BidBitmapAndOrTest) {
  srand(time(NULL));
  BidBitmap bitmap;
  
  vector<vector<uint32_t> > datas;
  vector<uint16_t> bids;

  RandomExt::RandomBitmap(20, 5, 0.4, datas);
  RandomExt::RandomArray<uint16_t>(20, 20, 400, bids);

  EXPECT_TRUE(bitmap.Generate(datas, bids, 5));
  EXPECT_STREQ("", bitmap.ToString().c_str());

  EXPECT_EQ(8, (int)bitmap.GetWidth());
  EXPECT_EQ(5, (int)bitmap.GetHeight());
  EXPECT_EQ(20, (int)bitmap.GetSize());


  uint8_t* result = new uint8_t[bitmap.GetWidth()];
    
  bitmap.Or(0, 1, result);
  for (uint32_t i = 0; i < bitmap.GetWidth(); ++i) {
    EXPECT_EQ((uint32_t)0, static_cast<uint32_t>(result[i])) << " of index " << i;
  }
}

TEST(BidBitmapTest, BidBitmapLoadSaveTest) {
  srand(time(NULL));
  BidBitmap bitmap;

  vector<vector<uint32_t> > datas;
  vector<uint16_t> bids;

  RandomExt::RandomBitmap(20, 5, 0.4, datas);
  RandomExt::RandomArray<uint16_t>(20, 0, 40, bids);

  EXPECT_TRUE(bitmap.Generate(datas, bids, 5));
  string before = bitmap.ToString();
  EXPECT_STREQ("", before.c_str());
  FILE* out = fopen("dump_bid_bitmap.dat", "wb");
  EXPECT_EQ(0, bitmap.Dump(out));
  fclose(out);
  FILE* in = fopen("dump_bid_bitmap.dat", "rb");
  EXPECT_EQ(0, bitmap.Load(in));
  fclose(in);
  string after = bitmap.ToString();
  EXPECT_STREQ("", after.c_str());
  EXPECT_STREQ(before.c_str(), after.c_str());

}

TEST (BidBitmapTest, BidBitmapCountTest) {
  srand(time(NULL));
  BidBitmap bitmap;
  vector<vector<uint32_t> > datas;
  vector<uint16_t> bids;

  RandomExt::RandomBitmap(20, 5, 0.4, datas);
  RandomExt::RandomArray<uint16_t>(20, 0, 40, bids);

  EXPECT_TRUE(bitmap.Generate(datas, bids, 5));

  uint8_t* result = new uint8_t[bitmap.GetWidth()];
  
  bitmap.And(0, 1, result);

  EXPECT_STREQ("", bitmap.ToString().c_str());

  Result count_result;

  bitmap.Count(result, 20, count_result);

  EXPECT_EQ(0, (int)count_result.bid_sum);
  EXPECT_EQ(0, (int)count_result.target_num);
  EXPECT_EQ(0, (int)count_result.win_num);
  EXPECT_EQ(0, (int)count_result.zero_num);
  EXPECT_EQ(0, (int)count_result.bid_num);
}
