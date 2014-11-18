#include <cstring>
#include <cmath>
#include <new>

#include <glog/logging.h>
#include <json/json.h>
#include <timer.h>

#include <tostring_ext.hpp>

#include <param.h>
#include <uv_bid_bitmap.h>
#include <uv_merge_bitmap.h>
#include <request_handler.h>
#include <shared_memory.hpp>
#include <query_parser.h>

namespace ea {

const int DURATION = 7;
const double MIN_DISPLAY_PV = 0.99;
const double ESCAPE_ZERO = 1E-9;
const uint32_t HOURS_PER_DAY = 24;
const uint32_t RequestHandler::request_buffer_size = 32 * 1024;    // 32 KB
const uint32_t RequestHandler::compute_buffer_size = 10000000;     // 10 MB


RequestHandler::RequestHandler() {
  compute_buf_ = new (std::nothrow) uint8_t[compute_buffer_size * 2];
  request_buf_ = new (std::nothrow) char[request_buffer_size];
  if (compute_buf_ == NULL || request_buf_ == NULL) {
    LOG(ERROR) << "Request Handler construct failed!";
    exit(1);
  }
}

RequestHandler::~RequestHandler() {
  delete[] compute_buf_;
  delete[] request_buf_;
  compute_buf_ = NULL;
  request_buf_ = NULL;
}

void RequestHandler::EncodeTargeting(const SharedMemory& memory, const QueryData& qd) {
  std::vector<int> array;
  for (uint32_t i = 0; i < qd.targetings.size(); ++i) {
    array.clear();
    for (uint32_t j = 0; j < qd.targetings[i].size(); ++j) {
      int idx = memory.GetTargetId(qd.targetings[i][j]);
      DLOG(INFO) << "GetTargetId result: " << qd.targetings[i][j] << " -> " << idx;
      if (idx >= 0)
        array.push_back(idx);
    }
    if (!array.empty()) // ignore null group
      targetings_.push_back(array);
  }
  DLOG(INFO) << "encode targeting over!";
}

void RequestHandler::GetTarget(const BidBitmap& bitmap) {
  uint32_t width = bitmap.GetWidth();
  // 定向条件为空 认定所有样本都满足
  if (targetings_.empty()) {
    DLOG(WARNING) << "empty targeting request.";
    memset(compute_buf_, static_cast<char>(0xff), width);
    return ;
  }
  // 分配指针
  uint8_t* ptr1 = compute_buf_;
  uint8_t* ptr2 = compute_buf_ + compute_buffer_size;
  // 处理第一组定向条件，结果放入ptr1
  if (targetings_[0].size() < 2) {
    bitmap.Or(targetings_[0][0], targetings_[0][0], ptr1);
  }
  else {
    bitmap.Or(targetings_[0][0], targetings_[0][1], ptr1);
  }
  for (uint32_t i = 2; i < targetings_[0].size(); ++i) {
    bitmap.Or(targetings_[0][i], ptr1, ptr1);
  }

  // 其他组内定向计算结果放入ptr2, ptr1保存 之前 组定向的 and 结果
  for (uint32_t i = 1; i < targetings_.size(); ++i) {
    if (targetings_[i].size() < 2) {
      bitmap.And(targetings_[i][0], ptr1, ptr1);
    }
    else {
      bitmap.Or(targetings_[i][0], targetings_[i][1], ptr2);
      for (uint32_t j = 2; j < targetings_[i].size(); ++j) {
        bitmap.Or(targetings_[i][j], ptr2, ptr2);
      }
      bitmap.And(ptr1, ptr2, ptr1);
    }
  }
  // ptr1 结果存放位置 
}

void RequestHandler::GetUvMergeRate(const UvMergeBitmap& uv_merge_map, const QueryData& qd) {
  std::vector<int> row_ids;
  int row_id;
  int row_uv;
  double sum = 0;
  for(uint32_t i = 0; i < qd.psids.size(); ++i) {
    row_id = uv_merge_map.GetRowId(qd.psids[i]);
    if (row_id < 0) {
      DLOG(WARNING) << qd.psids[i] << " not in uv merge bitmap.";
      continue;
    }
    row_ids.push_back(row_id);
    row_uv = uv_merge_map.GetRowUv(row_id);
    DLOG(INFO) << "uv merge bitmap: psid " << qd.psids[i] \
               << " -> row id " << row_id \
               << " -> row uv " << row_uv << ".";
    if (row_uv <0) {
      DLOG(WARNING) << qd.psids[i] << " not in uv merge bitmap.";
      continue;
    }
    sum += static_cast<double>(row_uv);
  }
  uv_merge_map.Union(row_ids, compute_buf_);
  uv_merge_rate_ = (static_cast<double>(uv_merge_map.Count(compute_buf_)) + ESCAPE_ZERO) / (sum + ESCAPE_ZERO);
  DLOG(INFO) << "merge uv: " << uv_merge_map.Count(compute_buf_) \
             << ", sum uv: " << static_cast<int>(sum) \
             << ", uv merge rate: " << uv_merge_rate_ << ".";
}

void RequestHandler::Accumulate(const PvBidBitmap& bitmap,
                                    const Result& result,
                                    const Param& param,
                                    const QueryData& qd,
                                    double ctr) {
  if (param.GetPvUseDelta()) {
    pv_sum_ += (result.win_num * 1.0 / bitmap.GetSize() * param.GetPvDelta(bitmap.GetRate()) * bitmap.GetRequest());
    DLOG(INFO) << "pv use delta function: " << bitmap.GetRate() << " -> " << param.GetPvDelta(bitmap.GetRate());
  }
  else {
    pv_sum_ += (result.win_num * 1.0 / bitmap.GetSize() * bitmap.GetPv());
    DLOG(INFO) << "pv use label pv.";
  }
  if (param.GetAvgCtrUsePos()) {
    avg_ctr_weighted_sum_ += (ctr * bitmap.GetRequest());
    avg_ctr_weights_sum_ += (bitmap.GetRequest() * 1.0);
    DLOG(INFO) << "avg ctr use pos pv.";
  }
  else {
    avg_ctr_weighted_sum_ += (ctr * bitmap.GetPv());
    avg_ctr_weights_sum_ += (bitmap.GetPv() * 1.0);
    DLOG(INFO) << "avg ctr use label pv.";
  }

  double min_bid = param.GetMinBids(qd.platform, qd.advert_type);
  double avg_bid = result.bid_sum / (static_cast<double>(result.bid_num) + ESCAPE_ZERO);
  DLOG(INFO) << "min bid: " << min_bid << ", avg bid: " << avg_bid << ".";
  avg_bid = (avg_bid > min_bid) ? avg_bid : min_bid;
  if (param.GetAvgBidUsePos()) {
    avg_bid_weighted_sum_ += (avg_bid * bitmap.GetRequest());
    avg_bid_weights_sum_ += (bitmap.GetRequest() * 1.0);
    DLOG(INFO) << "avg bid use pos pv.";
  }
  else {
    avg_bid_weighted_sum_ += (avg_bid * bitmap.GetPv());
    avg_bid_weights_sum_ += (bitmap.GetPv() * 1.0);
    DLOG(INFO) << "avg bid use label pv.";
  }
  double win_rate_with_zero = result.win_num / (static_cast<double>(result.target_num) + ESCAPE_ZERO);

  if (param.GetWinRateUseDelta()) {
    double win_rate_without_zero = (result.win_num - result.zero_num) / (static_cast<double>(result.target_num - result.zero_num) + ESCAPE_ZERO);
    DLOG(INFO) << "win rate with zero: " << win_rate_with_zero << ", win rate without zero: " << win_rate_without_zero << ".";
    double delta = param.GetWinRateDelta(bitmap.GetRate());
    double win_rate = win_rate_with_zero * delta + win_rate_without_zero * (1.0 - delta);
    DLOG(INFO) << "win rate delta: " << delta << ", win rate: " << win_rate << ".";
    if (param.GetWinRateUsePos()) {
      win_rate_weighted_sum_ += (win_rate * bitmap.GetRequest());
      win_rate_weights_sum_ += (bitmap.GetRequest());
      DLOG(INFO) << "win rate use delta & pos pv.";
    }
    else {
      win_rate_weighted_sum_ += (win_rate * bitmap.GetPv());
      win_rate_weights_sum_ += (bitmap.GetPv());
      DLOG(INFO) << "win rate use delta & label pv.";
    }
  }
  else {
    DLOG(INFO) << "win rate: " << win_rate_with_zero << ".";
    if (param.GetWinRateUsePos()) {
      win_rate_weighted_sum_ += (win_rate_with_zero * bitmap.GetRequest());
      win_rate_weights_sum_ += (bitmap.GetRequest());
      DLOG(INFO) << "win rate use delta & pos pv.";
    }
    else {
      win_rate_weighted_sum_ += (win_rate_with_zero * bitmap.GetPv());
      win_rate_weights_sum_ += (bitmap.GetPv());
      DLOG(INFO) << "win rate use delta & label pv.";
    }
  }
}

void RequestHandler::Accumulate(const UvBidBitmap& bitmap,
                                const Result& result,
                                const Param& param) {
  if (param.GetUvUseDelta()) {
    uv_sum_ += (result.win_num * 1.0 / bitmap.GetSize() * param.GetUvDelta(bitmap.GetRate()) * bitmap.GetRequest());
    DLOG(INFO) << "uv use delta function: " << bitmap.GetRate() << " -> " << param.GetPvDelta(bitmap.GetRate());
  }
  else {
    pv_sum_ += (result.win_num * 1.0 / bitmap.GetSize() * bitmap.GetUv());
    DLOG(INFO) << "uv use label pv.";
  }
}

void RequestHandler::Calculate(const Param& param, const QueryData& qd) {
  pv_ = ceil(pv_sum_ / DURATION);
  DLOG(INFO) << "daily pv: " << pv_ << ".";
  uv_ = ceil(uv_sum_ * uv_merge_rate_ / DURATION);
  DLOG(INFO) << "daily uv: " << uv_ << ".";
  uv_ = ((uv_ > pv_ * param.GetUvPvMaxRate()) ? ceil(pv_ * param.GetUvPvMaxRate()) : uv_);
  uv_ = ((uv_ < pv_ * param.GetUvPvMinRate()) ? ceil(pv_ * param.GetUvPvMinRate()) : uv_);
  DLOG(INFO) << "daily uv after rate detection: " << uv_ << ".";
  if (pv_ > MIN_DISPLAY_PV) {
    avg_ctr_ = avg_ctr_weighted_sum_ / (avg_ctr_weights_sum_ + ESCAPE_ZERO);
    avg_bid_ = avg_bid_weighted_sum_ / (avg_bid_weights_sum_ + ESCAPE_ZERO);
    win_rate_ = win_rate_weighted_sum_ / (win_rate_weights_sum_ + ESCAPE_ZERO);
  }
  DLOG(INFO) << "avg ctr: " << avg_ctr_ << ".";
  DLOG(INFO) << "avg bid: " << avg_bid_ << ".";
  if (!param.GetWinRateUseDelta()) {
    DLOG(INFO) << "before price correction, win rate: " << win_rate_ << ".";
    double rate = pow(qd.price / param.GetWinRateThreshold(qd.platform, qd.advert_type), param.GetWinRateScale(qd.platform, qd.advert_type));
    win_rate_ = win_rate_ * ((rate < 1.0) ? rate : 1.0);
    DLOG(INFO) << "after price correction, win rate: " << win_rate_ << ".";
  }
  win_rate_ = ((win_rate_ > param.GetMaxWinRate())? param.GetMaxWinRate() : win_rate_);
  DLOG(INFO) << "win rate: " << win_rate_ << ", max win rate: " << param.GetMaxWinRate() << ".";
}

void RequestHandler::ToJson() {
  Json::Value root;
  root["PV"] = Json::Value(pv_);
  root["UV"] = Json::Value(uv_);
  root["avgCTR"] = Json::Value(avg_ctr_);
  root["avgPrice"] = Json::Value(avg_bid_);
  root["WinningRate"] = Json::Value(win_rate_);
  Json::FastWriter writer;
  json_str_ = writer.write(root);
  DLOG(INFO) << "to json result: " << json_str_ << ".";
}

void RequestHandler::Process(const SharedMemory& memory, const QueryData& qd) {
  
  // 清空内容
  Clear();
  // 对定向条件进行编码
  EncodeTargeting(memory, qd);

  const Index& index = memory.GetIndex();
  const UvMergeBitmap* uv_merge_map = index.GetUvMergeBitmap();
  const Param& param = memory.GetParam();

  if (uv_merge_map == NULL) {
    LOG(ERROR) << "uv merge bitmap is not exsit.";
    exit(1);
  }
  GetUvMergeRate(*uv_merge_map, qd);

  Result result;
  double hourly_ctr;

  for (uint32_t i = 0; i < qd.psids.size(); ++i) {
    const PvBidBitmap* pv_map = index.GetPvBidBitmap(qd.psids[i]);
    const UvBidBitmap* uv_map = index.GetUvBidBitmap(qd.psids[i]);
    if (pv_map != NULL) {
      GetTarget(*pv_map);
      hourly_ctr = pv_map->GetHourlyCtr(qd.hours);
      DLOG(INFO) << "ctr of " << qd.psids[i] << " in hours " << util::ToStringExt<uint32_t>::ToString(qd.hours) << ": " << hourly_ctr << ".";
      pv_map->Count(compute_buf_, qd.price, result);
      DLOG(INFO) << "pv result of psid " << qd.psids[i] \
                 << ": sample num -> " << pv_map->GetSize() \
                 << ", bid sum -> " << result.bid_sum \
                 << ", bid num -> " << result.bid_num \
                 << ", zero num -> " << result.zero_num \
                 << ", target num -> " << result.target_num \
                 << ", win num -> " << result.win_num \
                 << ", request -> " << pv_map->GetRequest() \
                 << ", pv -> " << pv_map->GetPv() \
                 << ", rate -> " << (static_cast<double>(pv_map->GetPv()) / static_cast<double>(pv_map->GetRequest())) << ".";
      Accumulate(*pv_map, result, param, qd, hourly_ctr);
      DLOG(INFO) << qd.psids[i] << " pv accumulate.";
    }
    else {
      DLOG(WARNING) << qd.psids[i] << " not in pv map.";
    }
    if (uv_map != NULL) {
      GetTarget(*uv_map);
      uv_map->Count(compute_buf_, qd.price, result);
      DLOG(INFO) << "pv result of psid " << qd.psids[i] \
                 << ": sample num -> " << uv_map->GetSize() \
                 << ", bid sum -> " << result.bid_sum \
                 << ", bid num -> " << result.bid_num \
                 << ", zero num -> " << result.zero_num \
                 << ", target num -> " << result.target_num \
                 << ", win num -> " << result.win_num \
                 << ", request -> " << uv_map->GetRequest() \
                 << ", uv -> " << uv_map->GetUv() \
                 << ", rate -> " << (static_cast<double>(uv_map->GetUv()) / static_cast<double>(uv_map->GetRequest())) << ".";
      Accumulate(*uv_map, result, param);
      DLOG(INFO) << qd.psids[i] << " uv accumulate.";
    }
    else {
      DLOG(WARNING) << qd.psids[i] << " not in uv map.";
    }
  }// for

  Calculate(param, qd);
  ToJson();
}

}  // end namespace
