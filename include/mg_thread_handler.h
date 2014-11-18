#ifndef _MG_THREAD_HANDLER_H_
#define _MG_THREAD_HANDLER_H_

#include <mongoose.h>

#include <common.h>


namespace ea {

class SharedMemory;

class MgThreadHandler {
  public:
    MgThreadHandler() { }
    ~MgThreadHandler() { }

    void Init(SharedMemory* memory) {
      this->memory_ = memory;
    }
    void Start(const char** options);
    void Stop();

  private:
    static void thread_start(void *server_data, void **conn_data);
    static void thread_stop(void *server_data, void **conn_data);
    static int begin_request(struct mg_connection *conn);
 
  private:
    SharedMemory* memory_;

    struct mg_context *ctx_;
    struct mg_callbacks callbacks_;
    
  DISALLOW_COPY_AND_ASSIGN(MgThreadHandler);
};


}

#endif  // _MG_THREAD_HANDLER_H_
