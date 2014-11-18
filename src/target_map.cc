#include <fstream>

#include <glog/logging.h>

#include <string_ext.hpp>

#include <target_map.h>

namespace ea {

const char CONFIG_SEPARATOR = '\t';
const int CONFIG_FIELDS_NUM = 4;
const int CONFIG_OFFLINE_INDEX = 1;
const int CONFIG_ONLINE_INDEX = 2;
const int CONFIG_BITMAP_INDEX = 3;

int TargetMap::QueryOnline(const std::string& target) const {
  std::unordered_map<std::string, int>::const_iterator it = target_online_dict_.find(target);
  if (it == target_online_dict_.end())
    return -1;
  return it->second;
}

int TargetMap::QueryOffline(const std::string& target) const {
  std::unordered_map<std::string, int>::const_iterator it = target_offline_dict_.find(target);
  if (it == target_offline_dict_.end())
    return -1;
  return it->second;
}

int TargetMap::Load(const char* path) {
  target_offline_dict_.clear();
  target_online_dict_.clear();
  std::ifstream infile;
  std::string line;
  std::vector<std::string> fields;
  int index;
  infile.open(path);
  if (!infile) {
    DLOG(WARNING) << " can't open file in \"" << path << "\".";
    return -1;
  }
  while(!infile.eof()) {
    getline(infile, line);
    util::StringExt::Split(line, CONFIG_SEPARATOR, fields);
    if (fields.size() < CONFIG_FIELDS_NUM) 
      continue;
    index = atoi(fields[CONFIG_BITMAP_INDEX].c_str());
    target_offline_dict_[fields[CONFIG_OFFLINE_INDEX]] = index;
    target_online_dict_[fields[CONFIG_ONLINE_INDEX]] = index;
  }
  infile.close();
  return 0;
}

int TargetMap::Load(const std::string& path) {
  return Load(path.c_str());
}

}
