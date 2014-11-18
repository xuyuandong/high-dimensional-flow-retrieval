#ifndef _QUERY_PARSER_H_
#define _QUERY_PARSER_H_

#include <stdint.h>

#include <vector>
#include <string>

namespace Json {
  class Value;
}

namespace ea {

struct QueryData {

  public:

    QueryData() : price(0), bid_type(0), advert_type(0), platform(0) { }

    int price;                                             // 出价 (分)
    int bid_type;                                          // 出价类型 (0-CPM,1-CPC)
    int advert_type;                                       // 资源类型 (0-IMG,1-TXT)
    int platform;                                          // 平台 (PC, WAP, MOBUNION)
    std::vector<uint32_t> hours;                           // 时间定向
    std::vector<std::vector<std::string> > targetings;     // 定向条件(包含时间定向条件)
    std::vector<std::string> psids;                        // psid列表

    std::string ToString() const;

    bool Empty() const {
      return (price == 0) && 
             (bid_type == 0) &&
             (advert_type == 0) &&
             (platform == 0) &&
             hours.empty() && 
             targetings.empty() &&
             psids.empty();
    }

    void Clear() {
      price = bid_type = advert_type = platform = 0;
      hours.clear();
      targetings.clear();
      psids.clear();
    }

};

class QueryParser {

  public:

    // just overload , ignorn in this project
    int Parse(const std::string&, QueryData&) const;
    int Parse(const char* json_str, QueryData&) const;

    static const char* GetErrMsg(int err_code);

  private:

    int ParseString(const std::string&, Json::Value&) const; // string -> json_value
    int ParseString(const char*, Json::Value&) const; // cstring -> json_value
    int Deserialize(const Json::Value&, QueryData&) const; // json_value -> query_data

};

}

#endif
