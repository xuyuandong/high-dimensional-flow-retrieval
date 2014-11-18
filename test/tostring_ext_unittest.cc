#include <string>

#include <gtest/gtest.h>

#include <tostring_ext.hpp>

using namespace std;
using namespace util;

TEST (VectorTest, TestToString) {
  vector<string> strs;
  strs.push_back("test1");
  strs.push_back("test2");
  EXPECT_STREQ("[test1, test2]", ToStringExt<string>::ToString(strs).c_str());
  vector<int> ints;
  ints.push_back(3);
  ints.push_back(0);
  ints.push_back(-1);
  ints.push_back(3);
  EXPECT_STREQ("[3, 0, -1, 3]", ToStringExt<int>::ToString(ints).c_str());
  vector<vector<int> > datas;
  datas.push_back(ints);
  ints.push_back(2);
  datas.push_back(ints);
  datas.push_back(ints);
  EXPECT_STREQ("", ToStringExt<int>::ToString(datas).c_str());
}
