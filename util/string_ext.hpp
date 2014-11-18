#ifndef _UTIL_STRING_EXT_HPP_
#define _UTIL_STRING_EXT_HPP_

#include <string>
#include <vector>

namespace util {

class StringExt {
public:
  
  static uint32_t Split(const char* str, char separator, std::vector<std::string>& result) {
    result.clear();
    uint32_t begin = 0;
    uint32_t i;
    for(i = 0; str[i] != '\0'; ++i) {
      if(str[i] == separator) {
        result.push_back(std::string(&str[begin], i - begin));
        begin = i + 1;
      }
    }
    result.push_back(std::string(&str[begin], i - begin));
    return result.size();
  }

  static uint32_t Split(const std::string& str, char separator, std::vector<std::string>& result) {
    result.clear();
    uint32_t begin = 0;
    for(uint32_t i = 0; i < str.size(); ++i) {
      if(str[i] == separator) {
        result.push_back(str.substr(begin, i - begin));
        begin = i + 1;
      }
    }
    result.push_back(str.substr(begin, str.size() - begin));
    return result.size();
  }

  static uint32_t Split(const char* str, char sep_first, char sep_second, std::vector<std::vector<std::string> >& result) {
    result.clear();
    std::vector<std::string> buffer;
    uint32_t begin = 0;
    uint32_t i;
    for (i = 0; str[i] != '\0'; ++i) {
      if (str[i] == sep_second) {
        buffer.push_back(std::string(&str[begin], i - begin));
        begin = i + 1;
      }
      else if (str[i] == sep_first) {
        buffer.push_back(std::string(&str[begin], i - begin));
        begin = i + 1;
        result.push_back(buffer);
        buffer.clear();
      }
    }
    buffer.push_back(std::string(&str[begin], i - begin));
    result.push_back(buffer);
    return result.size();
  }

  static uint32_t Split(const std::string& str, char sep_first, char sep_second, std::vector<std::vector<std::string> >& result) {
    result.clear();
    std::vector<std::string> buffer;
    uint32_t begin = 0;
    for (uint32_t i = 0; i < str.length(); ++i) {
      if (str[i] == sep_second) {
        buffer.push_back(str.substr(begin, i - begin));
        begin = i + 1;
      }
      else if (str[i] == sep_first) {
        buffer.push_back(str.substr(begin, i - begin));
        begin = i + 1;
        result.push_back(buffer);
        buffer.clear();
      }
    }
    buffer.push_back(str.substr(begin, str.length() - begin));
    result.push_back(buffer);
    return result.size();
  }

  
  static std::string ToLower(const std::string& str) {
    std::string result(str);
    for (uint32_t i = 0; i < result.length(); ++i) {
      if(result[i] <= 'Z' && result[i] >= 'A')
        result[i] += 32;
    }
    return result;
  }

  static std::string ToUpper(const std::string& str) {
    std::string result(str);
    for (uint32_t i = 0; i < result.length(); ++i) {
      if(result[i] <= 'z' && result[i] >= 'a')
        result[i] -= 32;
    }
    return result;
  }

  static std::string Strip(const std::string& str) {
    int len = static_cast<int>(str.length());
    int i, j;
    for (i = 0; i < len && str[static_cast<size_t>(i)] == ' '; ++i);
    for (j = len - 1; j >= i && str[static_cast<size_t>(j)] == ' '; --j);
    if (j >= i)
      return str.substr(i, j + 1 - i);
    else
      return std::string();
  }

};

}

#endif
