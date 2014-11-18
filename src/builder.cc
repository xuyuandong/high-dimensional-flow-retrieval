#include <signal.h>

#include <fstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <string>

#include <index.h>
#include <target_map.h>

#include <string_ext.hpp>
#include <tostring_ext.hpp>

using namespace std;
using namespace ea;
using namespace util;

const size_t REGION_LENGTH = 6;
const char REGION_SEPARATOR = ',';
const char SEPARATOR = '\t';
const char VSEPARATOR = ',';

struct PvInfo {
  PvInfo() : request(0), pv(0) { }
  string psid;
  uint64_t request;
  uint64_t pv;
  vector<uint32_t> hourly_request;
  vector<uint32_t> hourly_pv;
  vector<float> hourly_ctr;
};

struct UvInfo {
  UvInfo() : request(0), uv(0) { }
  string psid;
  uint64_t request;
  uint64_t uv;
};


DEFINE_string(targeting_conf, "", "The targeting dict config file");
DEFINE_string(pv_sample_filename, "", "The pv sample file sort by psid");
DEFINE_string(pv_info_filename, "", "The pv info file sort by psid");
DEFINE_string(uv_sample_filename, "", "The uv sample file sort by psid");
DEFINE_string(uv_info_filename, "", "The uv info file sort by psid");
DEFINE_string(uv_merge_filename, "", "The uv merge file");
DEFINE_string(index_filename, "", "The index filename.");

size_t ProcessRegion(const string& region, vector<string>& regions) {
  regions.clear();
  vector<string> items;
  StringExt::Split(region, REGION_SEPARATOR, items);
  for (size_t i = 0; i < items.size(); ++i) {
    size_t begin = 0;
    while(begin < items[i].length()) {
      regions.push_back(items[i].substr(begin, REGION_LENGTH));
      begin += REGION_LENGTH;
    }
  }
  return regions.size();
}

bool ParseLine(const string& line, PvInfo& info) {
  vector<string> items;
  StringExt::Split(line, SEPARATOR, items);
  if (items.size() < 7) {
    DLOG(WARNING) << "pv info error line: " << line;
    return false;
  }
  info.psid = items[0];
  info.request = static_cast<uint64_t>(strtol(items[1].c_str(), NULL, 10));
  info.pv = static_cast<uint64_t>(strtol(items[2].c_str(), NULL, 10));
  vector<string> nums;
  StringExt::Split(items[4], VSEPARATOR, nums);
  info.hourly_request.clear();
  for (size_t i = 0; i < nums.size(); ++i)
    info.hourly_request.push_back(static_cast<uint32_t>(strtol(nums[i].c_str(), NULL, 10)));
  StringExt::Split(items[5], VSEPARATOR, nums);
  info.hourly_pv.clear();
  for (size_t i = 0; i < nums.size(); ++i)
    info.hourly_pv.push_back(static_cast<uint32_t>(strtol(nums[i].c_str(), NULL, 10)));
  info.hourly_ctr.clear();
  StringExt::Split(items[6], VSEPARATOR, nums);
  for (size_t i = 0; i < nums.size(); ++i)
    info.hourly_ctr.push_back(static_cast<float>(strtod(nums[i].c_str(), NULL)));
  return true;
}

bool ParseLine(const string& line, UvInfo& info) {
  vector<string> items;
  StringExt::Split(line, SEPARATOR, items);
  if (items.size() < 3) {
    DLOG(WARNING) << "uv info error line: " << line;
    return false;
  }
  info.psid = items[0];
  info.request = static_cast<uint64_t>(strtol(items[1].c_str(), NULL, 10));
  info.uv = static_cast<uint64_t>(strtol(items[2].c_str(), NULL, 10));
  return true;
}

bool ParseLine(const string& line, const TargetMap& map, string& psid, vector<uint32_t>& targets, uint16_t& bid) {
  targets.clear();
  vector<string> items;
  StringExt::Split(line, SEPARATOR, items);
  if (items.size() < 9) {
    DLOG(WARNING) << "sample error line: " << line;
    return false;
  }
  psid = items[2];
  vector<string> names;
  for (size_t i = 1; i < 6; ++i) {
    if (i == 2)
      continue;
    StringExt::Split(items[i], VSEPARATOR, names);
    for (size_t j = 0; j < names.size(); ++j) {
      if (map.QueryOffline(names[j]) >= 0)
        targets.push_back(static_cast<uint32_t>(map.QueryOffline(names[j])));
    }
  }
  vector<string> regions;
  ProcessRegion(items[6], regions);
  for (size_t i = 0; i < regions.size(); ++i) {
    if (map.QueryOffline(regions[i]) >= 0)
      targets.push_back(static_cast<uint32_t>(map.QueryOffline(regions[i])));
  }
  int64_t num = strtol(items[7].c_str(), NULL, 10);
  if (num > 0xFFFF)
    bid = 0xFFFF;
  else 
    bid = static_cast<uint16_t>(num);
  return true;
}

int ProcessPvMap(const char* sample_path, const char* info_path, const TargetMap& map, Index& index) {
  ifstream sample, info;
  sample.open(sample_path);
  info.open(info_path);
  if (!sample) {
    LOG(ERROR) << "pv sample path error in " << sample_path << ".";
    return -1;
  }
  if (!info) {
    LOG(ERROR) << "pv info path error in " << info_path << ".";
    return -1;
  }
  PvInfo pv_info;
  string line;
  string psid;
  unordered_map<string, PvInfo> infos;
  while (!info.eof()) {
    getline(info, line);
    if (!ParseLine(line, pv_info))
      continue;
    infos.insert(pair<string, PvInfo>(pv_info.psid, pv_info));
  }
  info.close();
  vector<uint32_t> target;
  uint16_t bid;
  vector<uint16_t> bids;
  vector<vector<uint32_t> > targets;
  string last_psid = string();
  int return_code;
  while (!sample.eof()) {
    getline(sample, line);
    if (!ParseLine(line, map, psid, target, bid))
      continue;
    if (psid != last_psid) {
      if (targets.empty() || infos.find(last_psid) == infos.end()) {
        DLOG(WARNING) << "ignore pv bitmap of " << last_psid << ".";
      }
      else {
        return_code = index.AddPvBidBitmap(last_psid, infos[last_psid].request, \
                      infos[last_psid].pv, infos[last_psid].hourly_request, \
                      infos[last_psid].hourly_pv, infos[last_psid].hourly_ctr, \
                      targets, bids, map.GetSize());
        DLOG(INFO) << "insert pv bitmap of " << last_psid << ", return code: " << return_code;
      }
      targets.clear();
      bids.clear();
      last_psid = psid;
    }
    targets.push_back(target);
    bids.push_back(bid);
  }
  sample.close();
  if (targets.empty() || infos.find(last_psid) == infos.end()) {
    DLOG(WARNING) << "ignore pv bitmap of " << last_psid << ".";
  }
  else {
    return_code = index.AddPvBidBitmap(last_psid, infos[last_psid].request, \
                  infos[last_psid].pv, infos[last_psid].hourly_request, \
                  infos[last_psid].hourly_pv, infos[last_psid].hourly_ctr, \
                  targets, bids, map.GetSize());
    DLOG(INFO) << "insert pv bitmap of " << last_psid << ", return code: " << return_code;
  }
  return 0;
}

int ProcessUvMap(const char* sample_path, const char* info_path, const TargetMap& map, Index& index) {
  ifstream sample, info;
  sample.open(sample_path);
  info.open(info_path);
  if (!sample) {
    LOG(ERROR) << "uv sample path error in " << sample_path << ".";
    return -1;
  }
  if (!info) {
    LOG(ERROR) << "uv info path error in " << info_path << ".";
    return -1;
  }
  UvInfo uv_info;
  string line;
  string psid;
  unordered_map<string, UvInfo> infos;
  while (!info.eof()) {
    getline(info, line);
    if (!ParseLine(line, uv_info))
      continue;
    infos.insert(pair<string, UvInfo>(uv_info.psid, uv_info));
  }
  info.close();
  vector<uint32_t> target;
  uint16_t bid;
  vector<uint16_t> bids;
  vector<vector<uint32_t> > targets;
  string last_psid = string();
  int return_code;
  while (!sample.eof()) {
    getline(sample, line);
    if (!ParseLine(line, map, psid, target, bid))
      continue;
    if (psid != last_psid) {
      if (targets.empty() || infos.find(last_psid) == infos.end()) {
        DLOG(WARNING) << "ignore uv bitmap of " << last_psid << ".";
      }
      else {
        return_code = index.AddUvBidBitmap(last_psid, infos[last_psid].request, \
                      infos[last_psid].uv, targets, bids, map.GetSize());
        DLOG(INFO) << "insert uv bitmap of " << last_psid << ", return code: " << return_code;
      }
      targets.clear();
      bids.clear();
      last_psid = psid;
    }
    targets.push_back(target);
    bids.push_back(bid);
  }
  sample.close();
  if (targets.empty() || infos.find(last_psid) == infos.end()) {
    DLOG(WARNING) << "ignore uv bitmap of " << last_psid << ".";
  }
  else {
    return_code = index.AddUvBidBitmap(last_psid, infos[last_psid].request, \
                  infos[last_psid].uv, targets, bids, map.GetSize());
    DLOG(INFO) << "insert uv bitmap of " << last_psid << ", return code: " << return_code;
  }
  return 0;
}

int ProcessUvMergeMap(const char* path, Index& index) {
  ifstream infile;
  infile.open(path);
  if (!infile) {
    LOG(ERROR) << "uv merge file not exist in " << path;
    return -1;
  }
  string line;
  vector<string> items;
  vector<string> psid;
  vector<vector<string> > psids;
  while(!infile.eof()) {
    getline(infile, line);
    StringExt::Split(line, SEPARATOR, items);
    if (items.size() < 2) {
      DLOG(WARNING) << "error line in uv merge file, line: " << line;
      continue;
    }
    StringExt::Split(items[1], VSEPARATOR, psid);
    psids.push_back(psid);
  }
  infile.close();
  int return_code = index.AddUvMergeBitmap(psids);
  DLOG(INFO) << "insert uv merge success.";
  return return_code;
}

int main(int argc, char* argv[]) {
  ::google::InitGoogleLogging(argv[0]);
  ::google::ParseCommandLineFlags(&argc, &argv, true);

  // Setup signal handler: quit on Ctrl-C 
  FLAGS_logbufsecs  = 0; 

  time_t begin_time = time(NULL);
  
  Index index;
  TargetMap map;

  if (map.Load(FLAGS_targeting_conf) != 0) {
    LOG(ERROR) << "target conf not exist in " << FLAGS_targeting_conf << ".";
    exit(1);
  }
  LOG(INFO) << "target map load over.";


  ProcessPvMap(FLAGS_pv_sample_filename.c_str(), FLAGS_pv_info_filename.c_str(), map, index);
  ProcessUvMap(FLAGS_uv_sample_filename.c_str(), FLAGS_uv_info_filename.c_str(), map, index);
  ProcessUvMergeMap(FLAGS_uv_merge_filename.c_str(), index);

  int64_t return_code = index.Dump(FLAGS_index_filename);
  DLOG(INFO) << "index dump, return code: " << return_code << ".";

  time_t end_time = time(NULL);

  LOG(INFO) << "process in " << (end_time - begin_time) << " s.";

  return 0;
}

