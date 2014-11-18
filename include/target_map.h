#ifndef _TARGET_MAP_H_
#define _TARGET_MAP_H_

#include <unordered_map>

namespace ea {

class TargetMap {
  
  public:

    TargetMap() {}

    int Load(const char* path);
    int Load(const std::string& path);

    int QueryOnline(const std::string& target) const;
    int QueryOffline(const std::string& target) const;

    uint32_t GetSize() const {
      return target_offline_dict_.size();
    }

  private:

    std::unordered_map<std::string, int> target_offline_dict_;
    std::unordered_map<std::string, int> target_online_dict_;

};

}

#endif
