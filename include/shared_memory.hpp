#ifndef _SHARED_MEMORY_H_
#define _SHARED_MEMORY_H_

#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glog/logging.h>

#include <target_map.h>
#include <index.h>
#include <param.h>

namespace ea {

class SharedMemory {

  public:

    SharedMemory() : index_ts_(0), param_ts_(0), index_flag_(false), param_flag_(false) { }


    int LoadTargetMap(const char* path) {
      return dict_.Load(path);
    }
    int LoadTargetMap(const std::string& path) {
      return dict_.Load(path);
    }

    int UpdateIndex(const char* path) {
      return Update<Index>(path, index_, index_ts_, index_flag_);
    }
    int UpdateIndex(const std::string& path) {
      return Update<Index>(path.c_str(), index_, index_ts_, index_flag_);
    }

    int UpdateParam(const char* path) {
      return Update<Param>(path, param_, param_ts_, param_flag_);
    }
    int UpdateParam(const std::string& path) {
      return Update<Param>(path.c_str(), param_, param_ts_, param_flag_);
    }

    const Index& GetIndex() const {
      return index_[static_cast<size_t>(index_flag_)];
    }
    const Param& GetParam() const {
      return param_[static_cast<size_t>(param_flag_)];
    }

    int GetTargetId(const std::string& target) const {
      return dict_.QueryOnline(target);
    }

  private:

    template<class type>
    int Update(const char* path, type* buffer, time_t& ts, bool& flag) {
      time_t mtime = GetMtime(path);
      if (mtime <= ts) {
        DLOG(INFO) << "file doesn't need to update, ts of memory: " << ts << ", ts of file: " << mtime << ".";
        return 0;
      }
      sleep(10);
      int return_code = buffer[static_cast<size_t>(!flag)].Load(path);
      if (return_code != 0)
        return return_code;
      DLOG(INFO) << "file load over, switch flag.";
      flag = !flag;
      ts = mtime;
      sleep(5);
      buffer[static_cast<size_t>(!flag)].Clear();
      DLOG(INFO) << "overdue data is clear!";
      return 0;
    }

    time_t GetMtime(const char* path) const {
      struct stat attr;
      return (0 == stat(path, &attr) ? attr.st_mtime : static_cast<time_t>(0));
    }
    time_t GetMtime(const std::string& path) const {
      return GetMtime(path.c_str());
    }

    time_t index_ts_;
    time_t param_ts_;

    bool index_flag_;
    bool param_flag_;

    Index index_[2];
    Param param_[2];
    TargetMap dict_;
};

}

#endif 
