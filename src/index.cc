#include <index.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <pv_bid_bitmap.h>
#include <uv_bid_bitmap.h>
#include <uv_merge_bitmap.h>


namespace ea {

DEFINE_int32(bitmap_parallel_threads, 4, "the threads num of bitmap operation");

const uint32_t PV_OFFSET_MAP_SIZE = 64 * 1024;
const uint32_t UV_OFFSET_MAP_SIZE = 64 * 1024;
const uint32_t PSID_CAPACITY = 32;

int Index::AddPvBidBitmap(const std::string& psid,
                          uint64_t request,
                          uint64_t pv,
                          const std::vector<uint32_t>& hourly_request,
                          const std::vector<uint32_t>& hourly_pv,
                          const std::vector<float>& hourly_ctr,
                          const std::vector<std::vector<uint32_t> >& datas,
                          const std::vector<uint16_t>& bids,
                          uint32_t size) {
  if (pv_map_[psid] != NULL) {
    DLOG(WARNING) << "pv bid bitmap of " << psid << " has already exists.";
    return 1;
  }
  PvBidBitmap *ptr = new (std::nothrow) PvBidBitmap(FLAGS_bitmap_parallel_threads);
  if (ptr == NULL) {
    DLOG(ERROR) << "no memory for pv bid bitmap of " << psid << ".";
    return -1;
  }
  if (!ptr->Generate(psid, request, pv, hourly_request, hourly_pv, hourly_ctr, datas, bids, size)) {
    delete ptr;
    DLOG(ERROR) << "fail to generate pv bid bitmap of " << psid << ".";
    return -2;
  }
  pv_map_[psid] = ptr;
  DLOG(INFO) << "add pv bid bitmap of " << psid << ".";
  return 0;
}

int Index::AddUvBidBitmap(const std::string& psid,
                          uint64_t request,
                          uint64_t pv,
                          const std::vector<std::vector<uint32_t> >& datas,
                          const std::vector<uint16_t>& bids,
                          uint32_t size) {
  if (uv_map_[psid] != NULL) {
    DLOG(WARNING) << "uv bid bitmap of " << psid << " has already exists.";
    return 1;
  }
  UvBidBitmap *ptr = new (std::nothrow) UvBidBitmap(FLAGS_bitmap_parallel_threads);
  if (ptr == NULL) {
    DLOG(ERROR) << "no memory for uv bid bitmap of " << psid << ".";
    return -1;
  }
  if (!ptr->Generate(psid, request, pv, datas, bids, size)) {
    delete ptr;
    DLOG(ERROR) << "fail to generate uv bid bitmap of " << psid << ".";
    return -2;
  }
  uv_map_[psid] = ptr;
  DLOG(INFO) << "add uv bid bitmap of " << psid << ".";
  return 0;
}

int Index::AddUvMergeBitmap(const std::vector<std::vector<std::string> >& psids) {
  if (uv_merge_map_ != NULL) {
    DLOG(WARNING) << "uv merge bitmap has already exists.";
    return 1;
  }
  UvMergeBitmap *ptr = new (std::nothrow) UvMergeBitmap(FLAGS_bitmap_parallel_threads);
  if (ptr == NULL) {
    DLOG(ERROR) << "no memory for uv merge bitmap.";
    return -1;
  }
  if (!ptr->Generate(psids)) {
    delete ptr;
    DLOG(ERROR) << "fail to generate uv merge bitmap.";
    return -2;
  }
  uv_merge_map_ = ptr;
  DLOG(INFO) << "add uv merge bitmap.";
  return 0;
}

void Index::Clear() {
  for (std::unordered_map<std::string, PvBidBitmap*>::iterator it = pv_map_.begin(); it != pv_map_.end(); ++it) {
    delete it->second;
    it->second = NULL;
  }
  for (std::unordered_map<std::string, UvBidBitmap*>::iterator it = uv_map_.begin(); it != uv_map_.end(); ++it) {
    delete it->second;
    it->second = NULL;
  }
  delete uv_merge_map_;
  uv_merge_map_ = NULL;
  pv_map_.clear();
  uv_map_.clear();
  DLOG(INFO) << "index clear.";
}

int64_t Index::DumpOffsetMap(const std::map<std::string, uint64_t>& offset_map,
                             FILE* out) const {
  if (out == NULL) {
    DLOG(ERROR) << "no destination to dump offset map.";
    return -1;
  }
  int64_t file_size = 0;
  uint32_t size = offset_map.size();
  fwrite(&size, sizeof(uint32_t), 1, out);
  file_size += sizeof(uint32_t);
  for (std::map<std::string, uint64_t>::const_iterator it = offset_map.begin(); it != offset_map.end(); ++it) {
    size = it->first.length();
    fwrite(&size, sizeof(uint32_t), 1, out);
    fwrite(it->first.c_str(), sizeof(char), size, out);
    fwrite(&(it->second), sizeof(uint64_t), 1, out);
    file_size += (sizeof(uint32_t) + size + sizeof(uint64_t));
    DLOG(INFO) << "dump " << it->first << " -> " << it->second << " in offset map.";
  }
  DLOG(INFO) << "total size of offset map: " << file_size;
  return file_size;
}

int Index::LoadOffsetMap(FILE* in,
                         std::map<std::string, uint64_t>& offset_map) const {
  offset_map.clear();
  if (in == NULL) {
    DLOG(ERROR) << "no source to load offset map.";
    return -1;
  }
  uint32_t size;
  std::vector<char> buf_string;
  char buf_char;
  uint64_t buf_offset;
  uint32_t buf_integer;
  buf_string.reserve(PSID_CAPACITY);
  if (fread(&size, sizeof(uint32_t), 1, in) == 0) {
    DLOG(ERROR) << "offset map size load error.";
    return 1;
  }
  for (uint32_t i = 0; i < size; ++i) {
    buf_string.clear();
    if (fread(&buf_integer, sizeof(uint32_t), 1, in) == 0) {
      DLOG(ERROR) << "No." << (i + 1) << " psid size in offset map load error.";
      return 1;
    }
    for (uint32_t j = 0; j < buf_integer; ++j) {
      if (fread(&buf_char, sizeof(char), 1, in) == 0) {
        DLOG(ERROR) << "No." << (j + 1) << " char of No." << (i + 1)
                    << " psid in offset map load error.";
        return 1;
      }
      buf_string.push_back(buf_char);
    }
    if (fread(&buf_offset, sizeof(uint64_t), 1, in) == 0) {
      DLOG(ERROR) << "offset of " << std::string(buf_string.begin(), buf_string.end())
                  << " in offset map load error.";
      return 1;
    }
    offset_map.insert(std::pair<std::string, uint64_t>(std::string(buf_string.begin(), buf_string.end()), buf_offset));
  }
  DLOG(INFO) << "offset map load.";
  return 0;
}

int64_t Index::Dump(const char* path) const {
  if (uv_merge_map_ == NULL) {
    DLOG(ERROR) << "uv merge bitmap doesn't exist.";
    return -3;
  }
  {
    FILE* out = fopen(path, "wb");
    if (out == NULL) {
      DLOG(ERROR) << "fail to open file \"" << path << "\".";
      return -1;
    }
    fclose(out);
  } // clear file

  FILE* out = fopen(path, "wb+");
  if (out == NULL) {
    DLOG(ERROR) << "fail to write file \"" << path << "\".";
    return -2;
  }

  std::map<std::string, uint64_t> pv_offset_map;
  std::map<std::string, uint64_t> uv_offset_map;
  uint64_t offset = PV_OFFSET_MAP_SIZE + UV_OFFSET_MAP_SIZE + sizeof(uint64_t);

  uint8_t *place_holder = new uint8_t[offset];
  memset(place_holder, 0, offset);
  fseek(out, 0, SEEK_SET);
  fwrite(place_holder, sizeof(uint8_t), offset, out);
  delete [] place_holder;

  uint64_t size;

  for (std::unordered_map<std::string, PvBidBitmap*>::const_iterator it = pv_map_.begin(); it != pv_map_.end(); ++it) {
    if (it->second == NULL) {
      DLOG(WARNING) << "pv bid bitmap of " << it->first << " is null.";
      continue;
    }
    size = it->second->Dump(out);
    if (size < 0) {
      fseek(out, offset, SEEK_SET);
      DLOG(WARNING) << "pv bid bitmap of " << it->first << " dump error.";
      continue;
    }
    pv_offset_map[it->first] = offset;
    DLOG(INFO) << "write pv bid bitmap of " << it->first << " to index file, offset: "
               << offset << " , size: " << size << " .";
    offset += size;
  }

  for (std::unordered_map<std::string, UvBidBitmap*>::const_iterator it = uv_map_.begin(); it != uv_map_.end(); ++it) {
    if (it->second == NULL) {
      DLOG(WARNING) << "uv bid bitmap of " << it->first << " is null.";
      continue;
    }
    size = it->second->Dump(out);
    if (size < 0) {
      fseek(out, offset, SEEK_SET);
      DLOG(WARNING) << "uv bid bitmap of " << it->first << " dump error.";
      continue;
    }
    uv_offset_map[it->first] = offset;
    DLOG(INFO) << "write uv bid bitmap of " << it->first << " to index file, offset: "
               << offset << " , size: " << size << " .";
    offset += size;
  }

  size = uv_merge_map_->Dump(out);
  if (size < 0) {
    DLOG(ERROR) << "uv merge bitmap dump error.";
    return -4;
  }
  fseek(out, 0, SEEK_SET);
  if (DumpOffsetMap(pv_offset_map, out) < 0) {
    DLOG(ERROR) << "pv offset map dump error.";
    return -5;
  }
  fseek(out, PV_OFFSET_MAP_SIZE, SEEK_SET);
  if (DumpOffsetMap(uv_offset_map, out) < 0) {
    DLOG(ERROR) << "uv offset map dump error.";
    return -6;
  }
  fseek(out, PV_OFFSET_MAP_SIZE + UV_OFFSET_MAP_SIZE, SEEK_SET);
  fwrite(&offset, sizeof(uint64_t), 1, out);
  DLOG(INFO) << "index file dump over.";
  fclose(out);
  return offset + size;
}

int64_t Index::Dump(const std::string& path) const {
  return Dump(path.c_str());
}


int Index::Load(const char* path) {
  Clear();
  FILE* in = fopen(path, "rb");
  if (in == NULL) {
    DLOG(ERROR) << "can't open index file \"" << path << "\".";
    return -1;
  }
  std::map<std::string, uint64_t> pv_offset_map;
  std::map<std::string, uint64_t> uv_offset_map;
  uint64_t uv_merge_map_offset;
  if (LoadOffsetMap(in, pv_offset_map) != 0) {
    DLOG(ERROR) << "pv offset map load error.";
    return -2;
  }
  if (fseek(in, PV_OFFSET_MAP_SIZE, SEEK_SET) != 0) {
    DLOG(ERROR) << "fseek error.";
    return -3;
  }
  if (LoadOffsetMap(in, uv_offset_map) != 0) {
    DLOG(ERROR) << "uv offset map load error.";
    return -2;
  }
  if (fseek(in, PV_OFFSET_MAP_SIZE + UV_OFFSET_MAP_SIZE, SEEK_SET) != 0) {
    DLOG(ERROR) << "fseek error.";
    return -3;
  }
  if (fread(&uv_merge_map_offset, sizeof(uint64_t), 1, in) == 0) {
    DLOG(ERROR) << "fread uv merge map offset error.";
    return -4;
  }
  for (std::map<std::string, uint64_t>::const_iterator it = pv_offset_map.begin(); it != pv_offset_map.end(); ++it) {
    if (fseek(in, it->second, SEEK_SET) != 0) {
      DLOG(WARNING) << "fseek error of psid " << it->first << " , offset: " << it->second << ".";
      continue;
    }
    PvBidBitmap* ptr = new (std::nothrow) PvBidBitmap(FLAGS_bitmap_parallel_threads);
    if (ptr == NULL) {
      DLOG(ERROR) << "no memory for pv bid bitmap of " << it->first << ".";
      return -5;
    }

    if (ptr->Load(in) != 0) {
      DLOG(WARNING) << "pv bid bitmap of psid " << it->first << " load error.";
      delete ptr;
      continue;
    }
    pv_map_.insert(std::pair<std::string, PvBidBitmap*>(it->first, ptr));
    DLOG(INFO) << "load pv bid bitmap of psid " << it->first << ".";
  }

  for (std::map<std::string, uint64_t>::const_iterator it = uv_offset_map.begin(); it != uv_offset_map.end(); ++it) {
    if (fseek(in, it->second, SEEK_SET) != 0) {
      DLOG(WARNING) << "fseek error of psid " << it->first << " , offset: " << it->second << ".";
      continue;
    }
    UvBidBitmap* ptr = new (std::nothrow) UvBidBitmap(FLAGS_bitmap_parallel_threads);
    if (ptr == NULL) {
      DLOG(ERROR) << "no memory for uv bid bitmap of " << it->first << " ";
      return -5;
    }

    if (ptr->Load(in) != 0) {
      DLOG(WARNING) << "uv bid bitmap of psid " << it->first << " load error.";
      delete ptr;
      continue;
    }
    uv_map_.insert(std::pair<std::string, UvBidBitmap*>(it->first, ptr));
    DLOG(INFO) << "load uv bid bitmap of psid " << it->first << ".";
  }
  if (fseek(in, uv_merge_map_offset, SEEK_SET) != 0) {
    DLOG(ERROR) << "fseek error.";
    return -3;
  }
  UvMergeBitmap *ptr = new (std::nothrow) UvMergeBitmap(FLAGS_bitmap_parallel_threads);
  if (ptr == NULL) {
    DLOG(ERROR) << "no memory for uv merge bitmap.";
    return -5;
  }
  if (ptr->Load(in) != 0) {
    DLOG(ERROR) << "uv merge bitmap load error.";
    delete ptr;
    return -6;
  }
  uv_merge_map_ = ptr;
  DLOG(INFO) << "index file load over.";
  return 0;
}

int Index::Load(const std::string& path) {
  return Load(path.c_str());
}

std::string Index::ToString() const {
  std::stringstream ss;
  ss << "{";
  for (std::unordered_map<std::string, PvBidBitmap*>::const_iterator it = pv_map_.begin(); it != pv_map_.end(); ++it) {
    if (it != pv_map_.begin())
      ss << ", ";
    ss << it->first << " -> ";
    if (it->second) {
      ss << it->second->ToString();
    }
    else {
      ss << "null";
    }
  }
  ss << "}" << std::endl;
  ss << "{";
  for (std::unordered_map<std::string, UvBidBitmap*>::const_iterator it = uv_map_.begin(); it != uv_map_.end(); ++it) {
    if (it != uv_map_.begin())
      ss << ", ";
    ss << it->first << " -> ";
    if (it->second) {
      ss << it->second->ToString();
    }
    else {
      ss << "null";
    }
  }
  ss << "}" << std::endl;
  if (uv_merge_map_) {
    ss << uv_merge_map_->ToString();
  }
  return ss.str();
}

}
