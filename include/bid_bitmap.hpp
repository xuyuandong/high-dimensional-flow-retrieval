#ifndef _BID_BITMAP_HPP_
#define _BID_BITMAP_HPP_

#include <tostring_ext.hpp>
#include <bitmap.hpp>

namespace ea {
// 结果 数据结构
struct Result {
  Result () : bid_sum(0), target_num(0), win_num(0), zero_num(0) {}
  uint64_t bid_sum;     // sum(bid) 单位分
  uint32_t target_num;  // card({target})
  uint32_t win_num;     // card({target} ∩  {win})
  uint32_t zero_num;    // card({target} ∩  {bid=0})
  uint32_t bid_num;     // card({target} ∩  {bid>0})
};

class BidBitmap : public common::Bitmap {

  public:

    BidBitmap() : common::Bitmap() { }

    BidBitmap(uint32_t threads) : common::Bitmap(threads) { }

    std::string ToString() const {
      std::stringstream buffer;
      buffer << common::Bitmap::ToString();
      buffer << util::ToStringExt<uint16_t>::ToString(bids_);
      return buffer.str();
    }

    void Clear() {
      common::Bitmap::Clear();
      bids_.clear();
    }
    
    // 由内存数据生成 bid bitmap
    bool Generate(const std::vector<std::vector<uint32_t> >& cols,
                  const std::vector<uint16_t>& bids, 
                  uint32_t height) {
      this->Clear();
      if (!common::Bitmap::Generate(cols, height))
        return false;
      if (bids.size() != this->GetSize())
        return false;
      bids_.insert(bids_.begin(), bids.begin(), bids.end());
      return true;
    }

    // 从二进制文件中 加载 bid bitmap 
    int Load(FILE* in) {
      this->Clear();
      int return_code = common::Bitmap::Load(in);
      if (return_code != 0)
        return return_code;
      uint16_t bid;
      for (uint32_t i = 0; i < this->GetSize(); ++i) {
        if (fread(&bid, sizeof(uint16_t), 1, in) == 0)
          return 1;
        bids_.push_back(bid);
      }
      return 0;
    }

    // 将bid bitmap 写入二进制文件
    int64_t Dump(FILE* out) const {
      int64_t file_size = common::Bitmap::Dump(out);
      if (file_size < 0)
        return file_size;
      for (uint32_t i = 0; i < bids_.size(); ++i) {
        fwrite(&bids_[i], sizeof(uint16_t), 1, out);
      }
      file_size += (sizeof(uint16_t) * bids_.size());
      return file_size;
    }


    void Count(const uint8_t* mask, uint32_t bid, Result& result) const {
      uint64_t bid_sum = 0;
      uint32_t target_num = 0;
      uint32_t win_num = 0;
      uint32_t zero_num = 0;
      uint32_t bid_num = 0;
#pragma omp parallel for num_threads(threads_) reduction(+: bid_sum, target_num, win_num, zero_num, bid_num)
      for (int i = 0; i < static_cast<int>(this->GetWidth()); ++i) {
        uint8_t s = mask[i];
        for (int j = this->Pop(s); j >= 0; j = this->Pop(s)) {
          ++ target_num;
          uint16_t _bid = bids_[(i << 3) + 7 - j];
          if (_bid > 0) {
            bid_sum += _bid;
            ++ bid_num;
          }
          else {
            ++ zero_num;
          }
          if (bid >= _bid) {
            ++ win_num;
          }
        }
      }
      result.bid_sum = bid_sum;
      result.target_num = target_num;
      result.win_num = win_num;
      result.zero_num = zero_num;
      result.bid_num = bid_num;
    }


  private:

    std::vector<uint16_t> bids_;

};

}

#endif
