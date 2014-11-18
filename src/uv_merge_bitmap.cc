#include <uv_merge_bitmap.h>

namespace ea {

const uint32_t PSID_CAPACITY = 32;

bool UvMergeBitmap::Generate (const std::vector<std::vector<std::string> >& psids) {
  Clear();
  std::vector<std::vector<uint32_t> > dict_result;
  std::unordered_map<std::string, int>::const_iterator it;
  std::vector<uint32_t> column;
  int pos;
  for (uint32_t i = 0; i < psids.size(); ++i) {
    column.clear();
    for (uint32_t j = 0; j < psids[i].size(); ++j) {
      it = psid_dict_.find(psids[i][j]);
      if (it == psid_dict_.end()) {
        pos = static_cast<int>(psid_dict_.size());
        psid_dict_.insert(std::pair<std::string, int>(psids[i][j], pos));
      }
      else {
        pos = it->second;
      }
      column.push_back(pos);
    }
    dict_result.push_back(column);
  }
  if (!common::Bitmap::Generate(dict_result, psid_dict_.size()))
    return false;
  for (uint32_t i = 0; i < GetHeight(); ++i) {
    sample_uv_.push_back(Count(i));
  }
  return true;
}

std::string UvMergeBitmap::ToString() const {
  std::stringstream buffer;
  buffer << common::Bitmap::ToString();
  buffer << "[";
  for (std::unordered_map<std::string, int>::const_iterator it = psid_dict_.begin(); it != psid_dict_.end(); ++it) {
    if (it != psid_dict_.begin())
      buffer << ", ";
    buffer << it->first << " -> " << it->second;
  }
  buffer << "]" << std::endl;
  buffer << util::ToStringExt<int>::ToString(sample_uv_);
  return buffer.str();
}

int64_t UvMergeBitmap::Dump(FILE* out) const {
  int64_t file_size = common::Bitmap::Dump(out);
  if (file_size < 0)
    return file_size;
  for (uint32_t i = 0; i < sample_uv_.size(); ++i) {
    fwrite(&sample_uv_[i], sizeof(int), 1, out);
  }
  file_size += (sizeof(int) * sample_uv_.size());
  for (std::unordered_map<std::string, int>::const_iterator it = psid_dict_.begin(); it != psid_dict_.end(); ++it) {
    uint32_t len = it->first.length();
    fwrite(&len, sizeof(uint32_t), 1, out);
    fwrite(it->first.c_str(), sizeof(char), len, out);
    fwrite(&(it->second), sizeof(int), 1, out);
    file_size += (sizeof(uint32_t) + sizeof(int) + len);
  }
  return file_size;
}

int UvMergeBitmap::Load(FILE* in) {
  Clear();
  if (in == NULL)
    return -1;
  int return_code = common::Bitmap::Load(in);
  if (return_code != 0)
    return return_code;
  uint32_t size = GetHeight();
  int buf_integer;
  uint32_t len;
  for (uint32_t i = 0; i < size; ++i) {
    if (fread(&buf_integer, sizeof(int), 1, in) == 0)
      return 1;
    sample_uv_.push_back(buf_integer);
  }
  std::vector<char> buf_string;
  buf_string.reserve(PSID_CAPACITY);
  char buf_ch;
  for (uint32_t i = 0; i < size; ++i) {
    if (fread(&len, sizeof(uint32_t), 1, in) == 0)
      return 1;
    buf_string.clear();
    for (uint32_t j = 0; j < len; ++j) {
      if (fread(&buf_ch, sizeof(char), 1, in) == 0)
        return 1;
      buf_string.push_back(buf_ch);
    }
    if (fread(&buf_integer, sizeof(int), 1, in) == 0)
      return 1;
    psid_dict_.insert(std::pair<std::string, int>(std::string(buf_string.begin(), buf_string.end()), buf_integer));
  }
  return 0;
}


}
