#ifndef _UV_BID_BITMAP_H_
#define _UV_BID_BITMAP_H_

#include <bid_bitmap.hpp>
#include <string>
#include <sstream>

namespace ea {

class UvBidBitmap : public BidBitmap {

  public:

    UvBidBitmap() : request_(0), uv_(0) { }

    UvBidBitmap(uint32_t threads) : BidBitmap(threads), request_(0), uv_(0) { }
    
    ~UvBidBitmap() { }

    std::string ToString() const {
      std::stringstream buffer;
      buffer << BidBitmap::ToString() << std::endl;
      buffer << psid_ << ';' << request_ << ';' << uv_;
      return buffer.str();
    }

    uint64_t GetRequest() const {
      return request_;
    }

    uint64_t GetUv() const {
      return uv_;
    }

    double GetRate() const;

    bool Generate(const std::string& psid, 
                  uint64_t request,
                  uint64_t uv,
                  const std::vector<std::vector<uint32_t> >& cols,
                  const std::vector<uint16_t>& bids, 
                  uint32_t height);

    int64_t Dump(FILE* out) const;

    int Load(FILE* in);

    void Clear() {
      BidBitmap::Clear();
      psid_.clear();
      request_ = 0;
      uv_ = 0;
    }

  private:

    std::string psid_;      // psid

    uint64_t request_;      // 请求日志中的uv数
    uint64_t uv_;           // 曝光日志中的uv数

};

}

#endif
