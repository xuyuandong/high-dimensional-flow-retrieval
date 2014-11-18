#include <gtest/gtest.h>
#include <uv_info.h>
#include <tostring_ext.hpp>
#include <string_ext.hpp>
#include <random_ext.hpp>

using namespace ea;
using namespace util;
using namespace std;

TEST (UvInfoTest, UvInfoTest) {
  UvInfo info;
  uint64_t request = RandomExt::Random<uint64_t>(2000, 50000);
  uint64_t uv = RandomExt::Random<uint64_t>(300, 4000);
  string psid = "PDPS009899850";
  EXPECT_TRUE(info.Generate(psid, request, uv));
  EXPECT_STREQ("", info.ToString().c_str());
  FILE* out = fopen("uvinfo_output", "wb");
  EXPECT_EQ(0, info.Dump(out));
  EXPECT_EQ(0, info.Dump(out));

  fclose(out);

  FILE* in = fopen("uvinfo_output", "rb");
  EXPECT_EQ(0, info.Load(in));
  EXPECT_STREQ("", info.ToString().c_str());
  EXPECT_EQ(0, info.Load(in));
  EXPECT_STREQ("", info.ToString().c_str());

}
