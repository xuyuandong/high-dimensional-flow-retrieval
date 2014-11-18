#include <signal.h>
#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <shared_memory.hpp>
#include <mg_thread_handler.h>

using namespace std;
using namespace ea;

DEFINE_string(port, "8778", "The server listen port.");
DEFINE_string(num_threads, "2", "Num of worker threads of server.");
DEFINE_string(access_log_file, "", "The server access log file.");
DEFINE_string(error_log_file, "", "The server error log file.");

DEFINE_string(param_filename, "", "The param filename.");
DEFINE_string(index_filename, "", "The index filename.");
DEFINE_string(targeting_conf, "", "The target config filename.");
DEFINE_int32(update_checktime, 60, "The checking interval for index update.");

int main(int argc, char* argv[]) {
  ::google::InitGoogleLogging(argv[0]);
  ::google::ParseCommandLineFlags(&argc, &argv, true);
  
  // Setup signal handler: quit on Ctrl-C 
  signal(SIGPIPE, SIG_IGN);
  FLAGS_logbufsecs  = 0; 

  int return_code;
  SharedMemory memory;
  return_code = memory.LoadTargetMap(FLAGS_targeting_conf);
  if (return_code) {
    LOG(ERROR) << "target map load failed.";
    exit(1);
  }
  LOG(INFO) << "targeting config load over";
  
  return_code = memory.UpdateIndex(FLAGS_index_filename);
  if (return_code) {
    LOG(ERROR) << "index file load failed.";
    exit(1);
  }
  LOG(INFO) << "index file update over";

  return_code = memory.UpdateParam(FLAGS_param_filename);
  if (return_code) {
    LOG(ERROR) << "param file load failed."; 
    exit(1);
  }
  LOG(INFO) << "param file update over";

  // List of mongoose options. Last element must be NULL.
  const char *options[] = {
    "listening_ports", FLAGS_port.c_str(),
    "num_threads", FLAGS_num_threads.c_str(),
    "access_log_file", FLAGS_access_log_file.c_str(),
    "error_log_file", FLAGS_error_log_file.c_str(),
    NULL};

  MgThreadHandler mghandler;
  mghandler.Init(&memory);

  // Start the web server.
  mghandler.Start(options);
  LOG(INFO) << "Server start running.";

  // Looping to check new index and updating.
  while (1) {
    sleep(FLAGS_update_checktime);
    return_code = memory.UpdateIndex(FLAGS_index_filename);
    if (return_code) {
      LOG(ERROR) << "index file load failed.";
      exit(1);
    }
    return_code = memory.UpdateParam(FLAGS_param_filename);
    if (return_code) {
      LOG(ERROR) << "param file load failed.";
      exit(1);
    }
  }

  // Stop the server.
  mghandler.Stop();
  LOG(INFO) << "Server has stoped.";

  return 0;
}

