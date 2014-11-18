#ifndef _UTIL_TOSTRING_EXT_HPP_
#define _UTIL_TOSTRING_EXT_HPP_

#include <vector>
#include <sstream>

namespace util {

template <class T> 
class ToStringExt {

public:

  static std::string ToString(T value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
  }

  static std::string ToString(const std::vector<T>& array) {
    std::stringstream ss;
    ss << "[";
    for (uint32_t i = 0; i < array.size(); ++i) {
      if (i)
        ss << ", ";
      ss << ToString(array[i]);
    }
    ss << "]";
    return ss.str();
  }

  static std::string ToString(const std::vector<std::vector<T> >& array) {
    std::stringstream ss;
    ss << "[";
    for (uint32_t i = 0; i < array.size(); ++i) {
      if (i)
        ss << ", ";
      ss << ToString(array[i]);
    }
    ss << "]";
    return ss.str();
  }

};

}

#endif
