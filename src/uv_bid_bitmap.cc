#include <vector>

#include <uv_bid_bitmap.h>

namespace ea {

const double ESCAPE_ZERO = 1E-9;

double UvBidBitmap::GetRate() const {
  return static_cast<double>(uv_) / (static_cast<double>(request_) + ESCAPE_ZERO);
}

bool UvBidBitmap::Generate(const std::string& psid,
                           uint64_t request,
                           uint64_t uv,
                           const std::vector<std::vector<uint32_t> >& cols,
                           const std::vector<uint16_t>& bids,
                           uint32_t height) {
  this->Clear();
  if (!BidBitmap::Generate(cols, bids, height))
    return false;
  psid_ = psid;
  request_ = request;
  uv_ = uv;
  return true;
}

int64_t UvBidBitmap::Dump(FILE* out) const {
  if (out == NULL)
    return -1;
  int64_t file_size = BidBitmap::Dump(out);
  if (file_size < 0)
    return file_size;

  uint32_t size = psid_.length();
  fwrite(&size, sizeof(uint32_t), 1, out);
  file_size += sizeof(uint32_t);
  fwrite(psid_.c_str(), sizeof(char), size, out);
  file_size += (sizeof(char) * size);
  fwrite(&request_, sizeof(uint64_t), 1, out);
  fwrite(&uv_, sizeof(uint64_t), 1, out);
  file_size += (sizeof(uint64_t) * 2);
  return file_size;
}

int UvBidBitmap::Load(FILE* in) {
  this->Clear();
  if (in == NULL)
    return -1;
  int return_code = BidBitmap::Load(in);
  if (return_code != 0) {
    return return_code;
  }
  uint32_t size;
  if (fread(&size, sizeof(uint32_t), 1, in) == 0)
    return 1;
  std::vector<char> psid_buf;
  psid_buf.reserve(size);
  char ch;
  for (uint32_t i = 0; i < size; ++i) {
    if (fread(&ch, sizeof(char), 1, in) == 0)
      return 1;
    psid_buf.push_back(ch);
  }
  psid_ = std::string(psid_buf.begin(), psid_buf.end());
  if (fread(&request_, sizeof(uint64_t), 1, in) == 0)
    return 1;
  if (fread(&uv_, sizeof(uint64_t), 1, in) == 0)
    return 1;
  return 0;
}

}
