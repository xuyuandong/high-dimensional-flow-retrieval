#include <pv_bid_bitmap.h>

namespace ea {

const uint32_t HOURS_PER_DAY = 24;
const double ESCAPE_ZERO = 1E-9;

bool PvBidBitmap::Generate(const std::string& psid,
                           uint64_t request,
                           uint64_t pv,
                           const std::vector<uint32_t>& hourly_request,
                           const std::vector<uint32_t>& hourly_pv,
                           const std::vector<float>& hourly_ctr,
                           const std::vector<std::vector<uint32_t> >& cols,
                           const std::vector<uint16_t>& bids,
                           uint32_t height) {
  this->Clear();
  
  if (IsValidHourlyArray<uint32_t>(hourly_request) && IsValidHourlyArray<uint32_t>(hourly_pv) && 
      IsValidHourlyArray<float>(hourly_ctr)) {
    if (!BidBitmap::Generate(cols, bids, height))
      return false;
    psid_ = psid;
    request_ = request;
    pv_ = pv;
    hourly_pv_.insert(hourly_pv_.begin(), hourly_pv.begin(), hourly_pv.end());
    hourly_request_.insert(hourly_request_.begin(), hourly_request.begin(), hourly_request.end());
    hourly_ctr_.insert(hourly_ctr_.begin(), hourly_ctr.begin(), hourly_ctr.end());
    return true;
  }
  return false;
}

double PvBidBitmap::GetRate() const {
  return static_cast<double>(pv_) / (static_cast<double>(request_) + ESCAPE_ZERO);
}

template <class type>
bool PvBidBitmap::IsValidHourlyArray(const std::vector<type>& array) const {
  return (array.size() == HOURS_PER_DAY) ? true : false;
}

bool PvBidBitmap::IsValid() const {
  return IsValidHourlyArray<uint32_t>(hourly_request_) && IsValidHourlyArray<uint32_t>(hourly_pv_) &&
         IsValidHourlyArray<float>(hourly_ctr_);
}

int64_t PvBidBitmap::Dump(FILE* out) const {
  if (out == NULL)
    return -1;
  int64_t file_size = BidBitmap::Dump(out);
  if (file_size < 0)
    return file_size;
  if (IsValid()) {
    uint32_t size = psid_.length();
    fwrite(&size, sizeof(uint32_t), 1, out);
    fwrite(psid_.c_str(), sizeof(char), size, out);
    file_size += (sizeof(char) * size + sizeof(uint32_t));
    fwrite(&request_, sizeof(uint64_t), 1, out);
    fwrite(&pv_, sizeof(uint64_t), 1, out);
    file_size += sizeof(uint64_t) * 2;
    for (uint32_t i = 0; i < hourly_request_.size(); ++i)
      fwrite(&hourly_request_[i], sizeof(uint32_t), 1, out);
    file_size += (sizeof(uint32_t) * hourly_request_.size());
    for (uint32_t i = 0; i < hourly_pv_.size(); ++i)
      fwrite(&hourly_pv_[i], sizeof(uint32_t), 1, out);
    file_size += (sizeof(uint32_t) * hourly_pv_.size());
    for (uint32_t i = 0; i < hourly_ctr_.size(); ++i)
      fwrite(&hourly_ctr_[i], sizeof(float), 1, out);
    file_size += (sizeof(float) * hourly_ctr_.size());
    return file_size;
  }
  return -1;
}

int PvBidBitmap::Load(FILE* in) {
  this->Clear();
  if (in == NULL)
    return -1;
  int return_code = BidBitmap::Load(in);
  if (return_code != 0)
    return return_code;
  uint32_t size;
  if (fread(&size, sizeof(uint32_t), 1, in) == 0)
    return 1;
  std::vector<char> psid_buf;
  char ch;
  for (uint32_t i = 0; i < size; ++i) {
    if (fread(&ch, sizeof(char), 1, in) == 0)
      return 1;
    psid_buf.push_back(ch);
  }
  psid_ = std::string(psid_buf.begin(), psid_buf.end());
  if (fread(&request_, sizeof(uint64_t), 1, in) == 0)
    return 1;
  if (fread(&pv_, sizeof(uint64_t), 1, in) == 0)
    return 1;
  uint32_t integer;
  float val;
  for (uint32_t i = 0; i < HOURS_PER_DAY; ++i) {
    if (fread(&integer, sizeof(uint32_t), 1, in) == 0)
      return 1;
    hourly_request_.push_back(integer);
  }
  for (uint32_t i = 0; i < HOURS_PER_DAY; ++i) {
    if (fread(&integer, sizeof(uint32_t), 1, in) == 0)
      return 1;
    hourly_pv_.push_back(integer);
  }
  for (uint32_t i = 0; i < HOURS_PER_DAY; ++i) {
    if (fread(&val, sizeof(float), 1, in) == 0)
      return 1;
    hourly_ctr_.push_back(val);
  }
  return 0;
}

double PvBidBitmap::GetHourlyCtr(const std::vector<uint32_t>& hours) const {
  uint32_t hourly_pv_sum = 0;
  double hourly_clk_sum = 0.0;
  for (uint32_t i = 0; i < hours.size(); ++i) {
    if (hours[i] < HOURS_PER_DAY) {
      hourly_pv_sum += hourly_pv_[hours[i]];
      hourly_clk_sum += static_cast<double>(hourly_pv_[hours[i]] * hourly_ctr_[hours[i]]);
    }
  }
  return hourly_clk_sum / (static_cast<double>(hourly_pv_sum) + ESCAPE_ZERO);
}

}
