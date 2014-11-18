#ifndef _INDEX_H_
#define _INDEX_H_

#include <stdint.h>

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

namespace ea {

class PvBidBitmap;
class UvBidBitmap;
class UvMergeBitmap;

class Index {

  public:

    Index() : uv_merge_map_(NULL) { }

    int AddPvBidBitmap(const std::string& psid,                       // psid
                       uint64_t,                                      // 请求数据的pv数
                       uint64_t,                                      // 曝光数据的pv数
                       const std::vector<uint32_t>&,                  // 小时级请求数
                       const std::vector<uint32_t>&,                  // 小时级曝光数
                       const std::vector<float>&,                     // 小时级点击率
                       const std::vector<std::vector<uint32_t> >&,    // 定向数据
                       const std::vector<uint16_t>&,                  // 出价数据
                       uint32_t);                                     // 定向数目

    int AddUvBidBitmap(const std::string& psid,                       // psid
                       uint64_t,                                      // 请求数据的uv数
                       uint64_t,                                      // 曝光数据的uv数
                       const std::vector<std::vector<uint32_t> >&,    // 定向数据
                       const std::vector<uint16_t>&,                  // 出价数据
                       uint32_t);                                     // 定向数目

    int AddUvMergeBitmap(const std::vector<std::vector<std::string> >& psids);


    int Load(const std::string& path);
    int Load(const char* path);
    
    int64_t Dump(const std::string& path) const;
    int64_t Dump(const char* path) const;

    void Clear();

    std::string ToString() const;

    const PvBidBitmap* GetPvBidBitmap(const std::string& psid) const {
      std::unordered_map<std::string, PvBidBitmap*>::const_iterator it = pv_map_.find(psid);
      if (it == pv_map_.end())
        return NULL;
      return it->second;

    }
    
    const UvBidBitmap* GetUvBidBitmap(const std::string& psid) const {
      std::unordered_map<std::string, UvBidBitmap*>::const_iterator it = uv_map_.find(psid);
      if (it == uv_map_.end())
        return NULL;
      return it->second;
    }

    const UvMergeBitmap* GetUvMergeBitmap() const {
      return uv_merge_map_;
    }

  private:
    
    int64_t DumpOffsetMap(const std::map<std::string, uint64_t>&,
                          FILE*) const;
    int LoadOffsetMap(FILE*,
                      std::map<std::string, uint64_t>&) const;

    std::unordered_map<std::string, PvBidBitmap*> pv_map_;
    std::unordered_map<std::string, UvBidBitmap*> uv_map_;

    UvMergeBitmap* uv_merge_map_;
};

}

#endif
