#ifndef _PV_BID_BITMAP_H_
#define _PV_BID_BITMAP_H_

#include <bid_bitmap.hpp>
#include <tostring_ext.hpp>

namespace ea {
class PvBidBitmap : public BidBitmap {

  public:

    PvBidBitmap() : BidBitmap(), request_(0), pv_(0) { }

    PvBidBitmap(uint32_t threads) : BidBitmap(threads), request_(0), pv_(0) { }
    
    std::string ToString() const {
      std::stringstream buffer;
      buffer << BidBitmap::ToString();
      buffer << std::endl;
      buffer << psid_ << ';' << request_ << ';' << pv_ << std::endl;
      buffer << util::ToStringExt<uint32_t>::ToString(hourly_request_) << std::endl;
      buffer << util::ToStringExt<uint32_t>::ToString(hourly_pv_) << std::endl;
      buffer << util::ToStringExt<float>::ToString(hourly_ctr_);
      return buffer.str();
    }

    std::string GetPsid() const {
      return psid_;
    }

    uint64_t GetRequest() const {
      return request_;
    }

    uint64_t GetPv() const {
      return pv_;
    }

    double GetHourlyCtr(const std::vector<uint32_t>& hours) const;

    // 获取曝光及请求数的比例
    double GetRate() const;

    void Clear() {
      BidBitmap::Clear();
      request_ = 0;
      pv_ = 0;
      psid_.clear();
      hourly_request_.clear();
      hourly_pv_.clear();
      hourly_ctr_.clear();
    }


    // 生成 pv bid bitmap
    bool Generate(const std::string& psid,
                  uint64_t request,
                  uint64_t pv,
                  const std::vector<uint32_t>& hourly_request,
                  const std::vector<uint32_t>& hourly_pv,
                  const std::vector<float>& hourly_ctr,
                  const std::vector<std::vector<uint32_t> >& cols,
                  const std::vector<uint16_t>& bids,
                  uint32_t height);


    // pv bid bitmap 写入二进制文件
    int64_t Dump(FILE* out) const;

    // 二进制文件读入 pv bid bitmap
    int Load(FILE* in);

  private:

    // 判断小时级数据是否合法
    bool IsValid() const; 
    
    template <class type>
    bool IsValidHourlyArray (const std::vector<type>&) const;


    std::string psid_;
    
    uint64_t request_;
    uint64_t pv_;

    std::vector<uint32_t> hourly_request_;
    std::vector<uint32_t> hourly_pv_;

    std::vector<float> hourly_ctr_;

  };
}


#endif
