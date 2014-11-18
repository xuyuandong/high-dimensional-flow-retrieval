#include <uv_merge_bitmap.h>
#include <string_ext.hpp>

#include <gtest/gtest.h>
using namespace ea;
using namespace util;
using namespace std;

TEST (UvMergeBitmapTest, totalTest) {

  UvMergeBitmap bitmap;
  string tosplit = "a,b,c,d,e,f,g|h,i,j,k,l,m,n|o,p,q,r,s,t|u,v,w,x,y,z";

  vector<vector<string> > datas;

  StringExt::Split(tosplit, '|', ',', datas);

  EXPECT_TRUE(bitmap.Generate(datas));
  EXPECT_STREQ("", bitmap.ToString().c_str());


}
