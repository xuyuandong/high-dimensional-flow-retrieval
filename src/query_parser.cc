#include <string.h>
#include <stdlib.h>

#include <sstream>
#include <iomanip>
#include <iostream>

#include <json/json.h>
#include <glog/logging.h>

#include <tostring_ext.hpp>
#include <string_ext.hpp>

#include <query_parser.h>

namespace ea {

const uint32_t HOURS_PER_DAY = 24;
const char QUERY_PSIDS_SEP = ',';
const char QUERY_TARGETING_OUTER_SEP = '|';
const char QUERY_TARGETING_FILED_SEP = ':';
const char QUERY_TARGETING_INNER_SEP = ',';
const std::string PC_NAME = "PC";
const std::string WAP_NAME = "WAP";
const std::string MOB_UNION_NAME = "MOBUNION";


const static char* ErrMsg[] = {
  "OK",
  "json parse error",
  "object type error",
  "bidPrice error",
  "bidType error",
  "schemaType error",
  "launchTime error",
  "launchTime length error",
  "launchTime value error",
  "adposIdList error",
  "orientation error",
  "bidPrice value error",
  "advertType error",
};



std::string QueryData::ToString() const {
  std::stringstream ss;
  ss << "price: " << price << ", ";
  ss << "bid type: " << bid_type << ", ";
  ss << "advert type: " << advert_type << ", ";
  ss << "platform: " << platform << std::endl;
  ss << "psids: " << util::ToStringExt<std::string>::ToString(psids) << std::endl;
  ss << "hours: " << util::ToStringExt<uint32_t>::ToString(hours) << std::endl;
  ss << "targeting: [";
  for (uint32_t i = 0; i < targetings.size(); ++i) {
    if (i != 0)
      ss << ", ";
    ss << util::ToStringExt<std::string>::ToString(targetings[i]);
  }
  ss << "]";
  return ss.str();
}

int QueryParser::ParseString(const std::string& json_str, Json::Value& value) const {
  Json::Reader reader;
  if (!reader.parse(json_str, value))
    return 1;
  return 0; 
}

int QueryParser::ParseString(const char* json_str, Json::Value& value) const {
  Json::Reader reader;
  if (!reader.parse(json_str, json_str + strlen(json_str), value))
    return 1;
  return 0;
}

int QueryParser::Deserialize(const Json::Value& value, QueryData& qd) const {
  qd.Clear();
  Json::Value iter;
  std::string buffer;
  std::vector<std::string> array;
  std::vector<std::vector<std::string> > multi_array;

  if (!value.isObject())
    return 2; // json value type error

  // bidPrice 
  iter = value["bidPrice"];
  if (!iter.isDouble())
    return 3; // bidPrice error in json
  qd.price = static_cast<int>(iter.asDouble());
  if (qd.price < 0)
    return 11; // bidPrice value error in json

  // bidType
  iter = value["bidType"];
  if (!iter.isInt())
    return 4; // bidType error in json
  qd.bid_type = iter.asInt();

  iter = value["advertType"];
  if (!iter.isInt())
    return 12; // advertType error in json
  qd.advert_type = iter.asInt();

  // schemaType
  iter = value["schemaType"];
  if (!iter.isString())
    return 5; // schemaType error in json
  std::string platform = util::StringExt::ToUpper(iter.asString());
  if (platform == MOB_UNION_NAME)
    qd.platform = 2;
  else if (platform == WAP_NAME)
    qd.platform = 1;
  else 
    qd.platform = 0;

  // launchTime
  iter = value["launchTime"];
  if (!iter.isString())
    return 6; // launchTime error in json
  buffer = iter.asString();
  if (buffer.length() == HOURS_PER_DAY) {
    for(uint32_t i = 0; i < buffer.length(); ++i) {
      if (buffer[i] == '1')
        qd.hours.push_back(i);
      else if (buffer[i] != '0')
        return 8; // launchTime TAG error in json
    }
  }
  else {
    return 7; // launchTime LENGTH error in json
  }

  // adposIdList
  iter = value["adposIdList"];
  if (!iter.isString())
    return 9; // adposIdList error in json
  util::StringExt::Split(iter.asString(), QUERY_PSIDS_SEP, array);
  for (uint32_t i = 0; i < array.size(); ++i) {
    if (!array[i].empty()) {
      qd.psids.push_back(array[i]);
    }
  }
  
  // orientation
  iter = value["orientation"];
  if (!iter.isString())
    return 10; // orientation error in json
  util::StringExt::Split(iter.asString(), QUERY_TARGETING_OUTER_SEP, 
                         QUERY_TARGETING_FILED_SEP, multi_array);

  for (uint32_t i = 0; i < multi_array.size(); ++i) {
    if (multi_array[i].size() >= 2 && 
        util::StringExt::Split(multi_array[i][1], QUERY_TARGETING_INNER_SEP, array)) {
      qd.targetings.push_back(array);
    }
    else if (!multi_array[i].empty() &&
             util::StringExt::Split(multi_array[i][0], QUERY_TARGETING_INNER_SEP, array)) {
      qd.targetings.push_back(array);
    }
  }

  // add hours into targetings 

  array.clear();         // clean to store hour format result
  std::stringstream time_fmt;   // use to format hour

  for (uint32_t i = 0; i < qd.hours.size(); ++i) {
    time_fmt.str("");
    time_fmt << std::setw(2) << std::setfill('0') << qd.hours[i];
    array.push_back(time_fmt.str());
  }
  if (!array.empty()) { // ignore null group
    qd.targetings.push_back(array);
  }
  return 0;
}

int QueryParser::Parse(const std::string& json_str, QueryData& qd) const {
  Json::Value value;
  int err_code = 0;
  err_code = ParseString(json_str, value);
  if (err_code)
    return err_code;
  return Deserialize(value, qd);
}

int QueryParser::Parse(const char* json_str, QueryData& qd) const {
  Json::Value value;
  int err_code = 0;
  err_code = ParseString(json_str, value);
  if (err_code)
    return err_code;
  return Deserialize(value, qd);
}


const char* QueryParser::GetErrMsg(int err_code) {
  return ErrMsg[err_code]; 
}

}
