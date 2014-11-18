#include <fstream>
#include <map>

#include <glog/logging.h> 

#include <string_ext.hpp>
#include <tostring_ext.hpp>
#include <param.h>

namespace ea {

const char LINE_SEPARATOR = '=';
const size_t LINE_MIN_FIELD_NUM = 2;
const size_t LINE_NAME_INDEX = 0;
const size_t LINE_VALUE_INDEX = 1;
const char MAP_SEPARATOR_FIRST = '|';
const char MAP_SEPARATOR_SECOND = ',';
const size_t MAP_MIN_FIELD_NUM = 3;
const size_t MAP_FIRST_KEY_INDEX = 0;
const size_t MAP_SECOND_KEY_INDEX = 1;
const size_t MAP_VALUE_INDEX = 2;
const size_t FUNCTION_SEPARATOR = '|';


const std::string PV_USE_DELTA = "pv_use_delta";
const std::string UV_USE_DELTA = "uv_use_delta";
const std::string AVG_BID_USE_POS = "avg_bid_use_pos";
const std::string AVG_CTR_USE_POS = "avg_ctr_use_pos";
const std::string WIN_RATE_USE_POS = "win_rate_use_pos";
const std::string WIN_RATE_USE_DELTA = "win_rate_use_delta";

const std::string UV_PV_MIN_RATE = "uv_pv_min_rate";
const std::string UV_PV_MAX_RATE = "uv_pv_max_rate";
const std::string MAX_WIN_RATE = "max_win_rate";

const std::string PV_STEP_FUNCTION = "pv_step_function";
const std::string UV_STEP_FUNCTION = "uv_step_function";
const std::string WIN_RATE_STEP_FUNCTION = "win_rate_step_function";

const std::string PLATFORM_MIN_BIDS = "platform_min_bids";
const std::string WIN_RATE_SCALES = "win_rate_scales";
const std::string WIN_RATE_THRESHOLDS = "win_rate_thresholds";

std::string Param::ToString(const std::unordered_map<int, std::unordered_map<int, double> >& hashmap) const {
  std::stringstream ss;
  ss << "{";
  for (std::unordered_map<int, std::unordered_map<int, double> >::const_iterator it1 = hashmap.begin(); it1 != hashmap.end(); ++it1) {
    if (it1 != hashmap.begin())
      ss << ", ";
    ss << it1->first << " -> {";
    for (std::unordered_map<int, double>::const_iterator it2 = it1->second.begin(); it2 !=it1->second.end(); ++it2) {
      if (it2 != it1->second.begin())
        ss << ", ";
      ss << it2->first << " -> " << it2->second;
    }
    ss << "}";
  }
  ss << "}";
  return ss.str();
}

int Param::Load(const char* path) {
  Clear();
  std::ifstream in;
  in.open(path);
  if (!in) {
    DLOG(WARNING) << "file not exist in " << path << ".";
    return -1;
  }
  std::string line;
  std::vector<std::string> items;
  std::string str;
  std::map<std::string, std::string> buffer;
  std::string name;
  std::string value;
  while (!in.eof()) {
    getline(in, line);
    str = util::StringExt::Strip(line);
    if (str.empty())
      continue;
    if (*(str.begin()) == '#')
      continue;
    DLOG(INFO) << "input string: " << str << ".";
    util::StringExt::Split(str, '=', items);
    DLOG(INFO) << "split result: " << util::ToStringExt<std::string>::ToString(items) << ".";
    if (items.size() < LINE_MIN_FIELD_NUM)
      continue;
    name = util::StringExt::ToLower(util::StringExt::Strip(items[LINE_NAME_INDEX]));
    value = util::StringExt::Strip(items[LINE_VALUE_INDEX]);
    if (!name.empty() && !value.empty()) {
      buffer.insert(std::pair<std::string, std::string>(name, value));
      DLOG(INFO) << "insert: " << name << " -> " << value << ".";
    }
  }
  { // Load Bool
    if (buffer.find(PV_USE_DELTA) != buffer.end())
      pv_use_delta_ = ToBool(buffer[PV_USE_DELTA]);
    if (buffer.find(UV_USE_DELTA) != buffer.end())
      uv_use_delta_ = ToBool(buffer[UV_USE_DELTA]);
    if (buffer.find(AVG_CTR_USE_POS) != buffer.end())
      avg_ctr_use_pos_ = ToBool(buffer[AVG_CTR_USE_POS]);
    if (buffer.find(AVG_BID_USE_POS) != buffer.end())
      avg_bid_use_pos_ = ToBool(buffer[AVG_BID_USE_POS]);
    if (buffer.find(WIN_RATE_USE_POS) != buffer.end())
      win_rate_use_pos_ = ToBool(buffer[WIN_RATE_USE_POS]);
    if (buffer.find(WIN_RATE_USE_DELTA) != buffer.end())
      win_rate_use_delta_ = ToBool(buffer[WIN_RATE_USE_DELTA]);
  }
  { // Load Double
    if (buffer.find(UV_PV_MIN_RATE) != buffer.end())
      uv_pv_min_rate_ = ToDouble(buffer[UV_PV_MIN_RATE]);
    if (buffer.find(UV_PV_MAX_RATE) != buffer.end())
      uv_pv_max_rate_ = ToDouble(buffer[UV_PV_MAX_RATE]);
    if (buffer.find(MAX_WIN_RATE) != buffer.end())
      max_win_rate_ = ToDouble(buffer[MAX_WIN_RATE]);
  }
  { // Load Map
    if (buffer.find(PLATFORM_MIN_BIDS) != buffer.end())
      Deserialize(buffer[PLATFORM_MIN_BIDS], platform_min_bids_);
    if (buffer.find(WIN_RATE_SCALES) != buffer.end())
      Deserialize(buffer[WIN_RATE_SCALES], win_rate_scales_);
    if (buffer.find(WIN_RATE_THRESHOLDS) != buffer.end())
      Deserialize(buffer[WIN_RATE_THRESHOLDS], win_rate_thresholds_);
  }
  { // Load Step Function

    int return_code;
    if (buffer.find(PV_STEP_FUNCTION) != buffer.end()) {
      return_code = Deserialize(buffer[PV_STEP_FUNCTION], pv_step_function_);
      if (return_code < 0)
        return return_code;
    }
    if (buffer.find(UV_STEP_FUNCTION) != buffer.end()) {
      return_code = Deserialize(buffer[UV_STEP_FUNCTION], uv_step_function_);
      if (return_code < 0)
        return return_code;
    }
    if (buffer.find(WIN_RATE_STEP_FUNCTION) != buffer.end()) {
      return_code = Deserialize(buffer[WIN_RATE_STEP_FUNCTION], win_rate_step_function_);
      if (return_code < 0)
        return return_code;
    }
  }
  return 0;
}

int Param::Load(const std::string& path) {
  return Load(path.c_str());
}

bool Param::ToBool(const std::string& str) const {
  if (util::StringExt::ToLower(util::StringExt::Strip(str)) == "true")
    return true;
  return false;
}

double Param::ToDouble(const std::string& str) const {
  return strtod(str.c_str(), NULL);
}

void Param::Deserialize(const std::string& str, std::unordered_map<int, std::unordered_map<int, double> >& map) {
  map.clear();
  std::vector<std::vector<std::string> > items;
  util::StringExt::Split(str, MAP_SEPARATOR_FIRST, MAP_SEPARATOR_SECOND, items);
  int first_key, second_key;
  double value;
  for (size_t i = 0; i < items.size(); ++i) {
    if (items[i].size() < MAP_MIN_FIELD_NUM) {
      DLOG(WARNING) << util::ToStringExt<std::string>::ToString(items[i]) << " fields num too small.";
      continue;
    }
    first_key = static_cast<int>(strtol(util::StringExt::Strip(items[i][MAP_FIRST_KEY_INDEX]).c_str(), NULL, 10));
    second_key = static_cast<int>(strtol(util::StringExt::Strip(items[i][MAP_SECOND_KEY_INDEX]).c_str(), NULL, 10));
    value = strtod(util::StringExt::Strip(items[i][MAP_VALUE_INDEX]).c_str(), NULL);
    map[first_key][second_key] = value;
    DLOG(INFO) << first_key << ", " << second_key << " -> " << value << ".";
  }
}

int Param::Deserialize(const std::string& str, common::StepFunction& function) {
  function.Clear();
  std::vector<std::string> items;
  std::vector<double> array;
  std::string num;
  util::StringExt::Split(str, FUNCTION_SEPARATOR, items);
  for (size_t i = 0; i < items.size(); ++i) {
    num = util::StringExt::Strip(items[i]);
    if (!num.empty()) {
      array.push_back(strtod(num.c_str(), NULL));
    }
  }
  std::vector<double> splits;
  std::vector<double> values;
  for (size_t i = 0; i < array.size(); ++i) {
    if (i & 1)
      splits.push_back(array[i]);
    else
      values.push_back(array[i]);
  }
  DLOG(INFO) << "splits: " << util::ToStringExt<double>::ToString(splits) << ".";
  DLOG(INFO) << "values: " << util::ToStringExt<double>::ToString(values) << ".";
  if (values.size() != (splits.size() + 1))
    return -1;
  function.SetValues(values);
  function.SetSplits(splits);
  return 0;
}

}
