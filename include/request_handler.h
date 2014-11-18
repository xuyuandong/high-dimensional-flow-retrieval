#ifndef _REQUEST_HANDLER_H_
#define _REQUEST_HANDLER_H_

#include <stdint.h>
#include <string>
#include <vector>

#include <pv_bid_bitmap.h>
#include <query_parser.h>

namespace ea {

class SharedMemory;
class BidBitmap;
class UvBidBitmap;
class UvMergeBitmap;
class Param;
class Result;

class RequestHandler {
  public:

    RequestHandler(); 
    ~RequestHandler();
    
    void Process(const SharedMemory&, const QueryData&);

    const std::string& GetJsonResult() const {
      return json_str_;
    }

    char* GetRequestBuffer() {
      return request_buf_;
    }
    const char* GetRequestBuffer() const {
      return request_buf_;
    }
    static uint32_t GetRequestBufferSize() {
      return request_buffer_size;
    }

    void Clear() {
      memset(request_buf_, 0, request_buffer_size);
      targetings_.clear();
      json_str_.clear();
      uv_sum_ = pv_sum_ = uv_merge_rate_ = \
                avg_ctr_weights_sum_ = avg_ctr_weighted_sum_ = \
                avg_bid_weights_sum_ = avg_bid_weighted_sum_ = \
                win_rate_weights_sum_ = win_rate_weighted_sum_ = \
                uv_ = pv_ = avg_ctr_ = avg_bid_ = win_rate_ = 0;
    }

  private:

    double GetHourlyCtr(const PvBidBitmap& bitmap, const QueryData& qd) const {
      return bitmap.GetHourlyCtr(qd.hours);
    }
    void EncodeTargeting(const SharedMemory&, const QueryData&);
    void GetTarget(const BidBitmap&);
    void GetUvMergeRate(const UvMergeBitmap&, const QueryData&);
    void ToJson();
   
    void Accumulate(const PvBidBitmap&,
                    const Result&,
                    const Param&,
                    const QueryData&,
                    double);

    void Accumulate(const UvBidBitmap&,
                    const Result&, 
                    const Param&);

    void Calculate(const Param&, const QueryData&);

    static const uint32_t request_buffer_size;          // request buffer存放json
    static const uint32_t compute_buffer_size;          // compute buffer存放临时Bitmap
    char* request_buf_;                                 // 存放 post json串缓存
    uint8_t* compute_buf_;                              // Bitmap 与 或操作缓存
    std::vector<std::vector<int> > targetings_;         // 定向编码，组内或，组间与
    std::string json_str_;                              // json 结果

    double uv_sum_;                                     // UV 和
    double pv_sum_;                                     // PV 和
    double uv_merge_rate_;                              // UV 合并率
    double avg_ctr_weights_sum_;                        // 平均ctr的权重和
    double avg_ctr_weighted_sum_;                       // 平均ctr的带权和
    double avg_bid_weights_sum_;                        // 平均出价的权重和
    double avg_bid_weighted_sum_;                       // 平均出价的带权和
    double win_rate_weights_sum_;                       // win rate的权重和
    double win_rate_weighted_sum_;                      // win rate的带权和

    double uv_;                                         // 返回UV
    double pv_;                                         // 返回PV
    double avg_ctr_;                                    // 返回avg ctr
    double avg_bid_;                                    // 返回avg bid
    double win_rate_;                                   // 返回win rate
};

}
#endif
