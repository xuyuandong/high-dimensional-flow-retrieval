#ifndef _UTIL_RANDOM_EXT_HPP_
#define _UTIL_RANDOM_EXT_HPP_

#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>

namespace util {

class RandomExt {
public:

  static double Random() {
    return static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
  }

  template <class type>
  static type Random(type min, type max) {
    return static_cast<type>(Random() * static_cast<double>(max - min) + min);
  }

  static void RandomBitmap(uint32_t row, uint32_t col, double prob,
                           std::vector<std::vector<uint32_t> >& datas) {
    datas.clear();
    std::vector<uint32_t> line;
    for (uint32_t i = 0; i < row; ++i) {
      for (uint32_t j = 0; j < col; ++j) {
        if (Random() < prob) {
          line.push_back(j);
        }
      }
      datas.push_back(line);
      line.clear();
    }
  }

  template <class type>
  static void RandomArray(uint32_t size, type min, type max,
                          std::vector<type>& array) {
    array.clear();
    for (uint32_t i = 0; i < size; ++i) {
      array.push_back(Random<type>(min, max));
    }
  }

};

}

#endif
