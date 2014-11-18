#include <string_ext.hpp>
#include <tostring_ext.hpp>

#include <gtest/gtest.h>

using namespace std;
using namespace util;

TEST (StringExtTest, SplitTest) {
  string a_str = "";
  char a_cstr[] = "";
  vector<string> res_str;
  vector<string> res_cstr;
  EXPECT_EQ(static_cast<uint32_t>(9), StringExt::Split(a_str, ',', res_str));
  EXPECT_EQ(static_cast<uint32_t>(9), StringExt::Split(a_cstr, ',', res_cstr));
  EXPECT_STREQ("", ToStringExt<string>::ToString(res_str).c_str());
  EXPECT_STREQ("", ToStringExt<string>::ToString(res_cstr).c_str());
  EXPECT_EQ(ToStringExt<string>::ToString(res_str), ToStringExt<string>::ToString(res_cstr));

  string long1 = "1|a,c,v|i,j,k||,,|,|";
  char long2[] = "1|a,c,v|i,j,l||,,|,|";
  vector<vector<string> > fs1;
  vector<vector<string> > fs2;
  EXPECT_EQ(static_cast<uint32_t>(7), StringExt::Split(long1, '|', ',', fs1));
  EXPECT_EQ(static_cast<uint32_t>(7), StringExt::Split(long2, '|', ',', fs2));
  EXPECT_EQ(ToStringExt<string>::ToString(fs1), ToStringExt<string>::ToString(fs2));
  EXPECT_STREQ("", ToStringExt<string>::ToString(fs1).c_str());
  EXPECT_STREQ("", ToStringExt<string>::ToString(fs2).c_str());

  string origin = "abcDEFGhijkLMnOpQRsT";
  EXPECT_STREQ("abcdefghijklmnopqrst", StringExt::ToLower(origin).c_str());
  EXPECT_STREQ("ABCDEFGHIJKLMNOPQRST", StringExt::ToUpper(origin).c_str());

  string str1 = "            ";
  EXPECT_STREQ("", StringExt::Strip(str1).c_str());
  string str2 = "       acbd  bj   ";
  EXPECT_STREQ("acbd  bj", StringExt::Strip(str2).c_str());
}
