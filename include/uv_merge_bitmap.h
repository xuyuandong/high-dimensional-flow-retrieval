#ifndef _UV_MERGE_BITMAP_H_
#define _UV_MERGE_BITMAP_H_

#include <tostring_ext.hpp>
#include <bitmap.hpp>

#include <unordered_map>

namespace ea {

class UvMergeBitmap : public common::Bitmap {

  public:

    UvMergeBitmap() : common::Bitmap() { }
    UvMergeBitmap(uint32_t threads) : common::Bitmap(threads) { }

    std::string ToString() const;

    void Clear() {
      common::Bitmap::Clear();
      sample_uv_.clear();
      psid_dict_.clear();
    }

    // 由抽样数据生成uv merge bit map
    bool Generate(const std::vector<std::vector<std::string> >& psids);

    int64_t Dump(FILE* out) const;

    int Load(FILE* in);

    // 计算一组广告位的总UV
    void Union(const std::vector<int>& row_ids, uint8_t* result) const {
      if (row_ids.size() == 0) {
        memset(result, 0, GetWidth());
        return ;
      }
      if (row_ids.size() == 1) {
        Or(row_ids[0], row_ids[0], result);
        return ;
      }
      Or(row_ids[0], row_ids[1], result);
      for (uint32_t i = 2; i < row_ids.size(); ++i) {
        Or(row_ids[i], result, result);
      }
    }

    int GetRowId(const std::string& psid) const {
      std::unordered_map<std::string, int>::const_iterator it = psid_dict_.find(psid);
      if (it == psid_dict_.end()) {
        return -1;
      }
      return it->second;
    }
    int GetRowUv(uint32_t row_id) const {
      if (row_id < sample_uv_.size())
        return sample_uv_[row_id];
      return -1;
    }

  private:

    std::vector<int> sample_uv_;                         // i -> 样本集中第i个psid的uv
    std::unordered_map<std::string, int> psid_dict_;     // psid -> 该psid的编号

};

}

#endif
