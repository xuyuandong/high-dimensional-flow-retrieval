#ifndef _PARAM_H_
#define _PARAM_H_

#include <unordered_map>

#include <step_function.hpp>

namespace ea {

class Param {

  public:

    Param() : pv_use_delta_(false), uv_use_delta_(false), avg_ctr_use_pos_(false), 
              avg_bid_use_pos_(false), win_rate_use_pos_(false), win_rate_use_delta_(false),
              uv_pv_min_rate_(0), uv_pv_max_rate_(0), max_win_rate_(0) {}

    std::string ToString() const {
      std::stringstream ss;
      ss << "pv_use_delta: " << (pv_use_delta_ ? std::string("true") : std::string("false"));
      ss << ", uv_use_delta: " << (uv_use_delta_ ? std::string("true") : std::string("false"));
      ss << ", avg_ctr_use_pos: " << (avg_ctr_use_pos_ ? std::string("true") : std::string("false"));
      ss << ", avg_bid_use_pos: " << (avg_bid_use_pos_ ? std::string("true") : std::string("false"));
      ss << ", win_rate_use_pos: " << (win_rate_use_pos_ ? std::string("true") : std::string("false"));
      ss << ", win_rate_use_delta: " << (win_rate_use_delta_ ? std::string("true") : std::string("false"));
      ss << std::endl;
      ss << "uv_pv_min_rate: " << uv_pv_min_rate_ << ", uv_pv_max_rate: " << uv_pv_max_rate_ << std::endl;
      ss << pv_step_function_.ToString() << std::endl;
      ss << uv_step_function_.ToString() << std::endl;
      ss << win_rate_step_function_.ToString() << std::endl;
      ss << ToString(platform_min_bids_) << std::endl;
      ss << ToString(win_rate_scales_) << std::endl;
      ss << ToString(win_rate_thresholds_);
      return ss.str();
    }

    double GetMinBids(int platform, int adpos_type) const {
      std::unordered_map<int, std::unordered_map<int, double> >::const_iterator iter_first = platform_min_bids_.find(platform);
      if (iter_first == platform_min_bids_.end())
        return 0.0;
      std::unordered_map<int, double>::const_iterator iter = iter_first->second.find(adpos_type);
      if (iter == iter_first->second.end())
        return 0.0;
      return iter->second;
    }

    double GetWinRateScale(int platform, int adpos_type) const {
      std::unordered_map<int, std::unordered_map<int, double> >::const_iterator iter_first = win_rate_scales_.find(platform);
      if (iter_first == win_rate_scales_.end())
        return 0.0;
      std::unordered_map<int, double>::const_iterator iter = iter_first->second.find(adpos_type);
      if (iter == iter_first->second.end())
        return 0.0;
      return iter->second;
    }

    double GetWinRateThreshold(int platform, int adpos_type) const {
      std::unordered_map<int, std::unordered_map<int, double> >::const_iterator iter_first = win_rate_thresholds_.find(platform);
      if (iter_first == win_rate_thresholds_.end())
        return 0.0;
      std::unordered_map<int, double>::const_iterator iter = iter_first->second.find(adpos_type);
      if (iter == iter_first->second.end())
        return 0.0;
      return iter->second;
    }

    double GetPvDelta(double x) const {
      return pv_step_function_.GetValue(x);
    }
    double GetUvDelta(double x) const {
      return uv_step_function_.GetValue(x);
    }
    double GetWinRateDelta(double x) const {
      return win_rate_step_function_.GetValue(x);
    }

    bool GetPvUseDelta() const {
      return pv_use_delta_;
    }
    bool GetUvUseDelta() const {
      return uv_use_delta_;
    }
    bool GetAvgCtrUsePos() const {
      return avg_ctr_use_pos_;
    }
    bool GetAvgBidUsePos() const {
      return avg_bid_use_pos_;
    }
    bool GetWinRateUsePos() const {
      return win_rate_use_pos_;
    }
    bool GetWinRateUseDelta() const {
      return win_rate_use_delta_;
    }
    double GetUvPvMinRate() const {
      return uv_pv_min_rate_;
    }
    double GetUvPvMaxRate() const {
      return uv_pv_max_rate_;
    }
    double GetMaxWinRate() const {
      return max_win_rate_;
    }

    int Load(const char* path);
    int Load(const std::string& path);

    void Clear() {
      pv_use_delta_ = uv_use_delta_ = avg_ctr_use_pos_ = \
      avg_bid_use_pos_ = win_rate_use_pos_ = win_rate_use_delta_ = false;
      uv_pv_min_rate_ = uv_pv_max_rate_ = max_win_rate_ = 0;
      platform_min_bids_.clear();
      win_rate_scales_.clear();
      win_rate_thresholds_.clear();
      pv_step_function_.Clear();
      uv_step_function_.Clear();
      win_rate_step_function_.Clear();
    }

  private:

    std::string ToString(const std::unordered_map<int, std::unordered_map<int, double> >& ) const;

    bool ToBool(const std::string&) const;
    double ToDouble(const std::string&) const;
    void Deserialize(const std::string&, std::unordered_map<int, std::unordered_map<int, double> >&);
    int Deserialize(const std::string&, common::StepFunction&);

    bool pv_use_delta_;
    bool uv_use_delta_;
    bool avg_ctr_use_pos_;
    bool avg_bid_use_pos_;
    bool win_rate_use_pos_;
    bool win_rate_use_delta_;

    double uv_pv_min_rate_;
    double uv_pv_max_rate_;
    double max_win_rate_;

    common::StepFunction pv_step_function_;
    common::StepFunction uv_step_function_;
    common::StepFunction win_rate_step_function_;

    std::unordered_map<int, std::unordered_map<int, double> > platform_min_bids_;
    std::unordered_map<int, std::unordered_map<int, double> > win_rate_scales_;
    std::unordered_map<int, std::unordered_map<int, double> > win_rate_thresholds_;
};

}

#endif
