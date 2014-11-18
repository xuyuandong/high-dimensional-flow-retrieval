#ifndef _COMMON_STEP_FUNCTION_HPP_
#define _COMMON_STEP_FUNCTION_HPP_

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

namespace common {

class StepFunction {

  public:

    std::string ToString() const {
      if (!IsValid()) {
        return std::string("invalid");
      }
      std::stringstream ss;
      for (uint32_t i = 0; i < splits_.size(); ++i) {
        ss << values_[i] << " -> " << splits_[i] << " <- ";
      }
      ss << values_[splits_.size()];
      return ss.str();
    }

    void SetValues(const std::vector<double>& values) {
      values_.clear();
      values_.insert(values_.begin(), values.begin(), values.end());
    }

    void SetSplits(const std::vector<double>& splits) {
      splits_.clear();
      splits_.insert(splits_.begin(), splits.begin(), splits.end());
      std::sort(splits_.begin(), splits_.end());
    }

    double GetValue(double x) const {
      if (!IsValid()) {
        return 0.0;
      }
      uint32_t left = 0;
      uint32_t right = splits_.size();
      while (left < right) {
        uint32_t mid = (left + right) >> 1;
        if (x > splits_[mid])
          left = mid + 1;
        else
          right = mid;
      }
      if (left < values_.size())
        return values_[left];
      return 0.0;
    }

    void Clear() {
      values_.clear();
      splits_.clear();
    }

  private:

    bool IsValid() const {
      if (values_.size() == (splits_.size() + 1))
        return true;
      return false;
    }

    std::vector<double> values_;

    std::vector<double> splits_;

};

}

#endif
