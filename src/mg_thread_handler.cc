#include <unistd.h>

#include <json/json.h>
#include <glog/logging.h>
#include <timer.h>

#include <mg_thread_handler.h>
#include <request_handler.h>

namespace ea {

#define RESPONSE_ERROR(err) do { \
  Json::Value root;              \
  root["code"] = 404;            \
  root["message"] = err;         \
  std::string res = root.toStyledString();  \
  mg_printf(conn, "HTTP/1.1 200 \r\n" \
      "Content-Type: text/plain\r\n"  \
      "Content-Length: %lu\r\n"       \
      "\r\n"  \
      "%s\n", \
      res.length(), res.c_str()); \
  return 0; \
} while (0)

void MgThreadHandler::Start(const char** options) {
  memset(&callbacks_, 0, sizeof(callbacks_));
  callbacks_.thread_start = &MgThreadHandler::thread_start;
  callbacks_.begin_request = &MgThreadHandler::begin_request;
  callbacks_.thread_stop = &MgThreadHandler::thread_stop;
  ctx_ = mg_start(&callbacks_, memory_, options);
}

void MgThreadHandler::Stop() {
  mg_stop(ctx_);
}

void MgThreadHandler::thread_start(void *server_data, void **conn_data) {
  if (NULL != conn_data) { // worker thread
    RequestHandler** handler = (RequestHandler**)conn_data;
    *handler = new(std::nothrow) RequestHandler();
    if (NULL == *handler) {
      LOG(ERROR) << "failed to alloc memory for RequestHandler";
      exit(1);
    }
  }
  LOG(INFO) << "start thread.";
}

void MgThreadHandler::thread_stop(void *server_data, void **conn_data) {
  RequestHandler* handler = (RequestHandler*)conn_data;
  delete handler;
  handler = NULL;
  LOG(INFO) << "stop thread.";
}

int MgThreadHandler::begin_request(struct mg_connection *conn) {
  DLOG(INFO) << "begin to process request.";
  Timer tt;
  const struct mg_request_info *request_info = mg_get_request_info(conn);
  RequestHandler* handler = (RequestHandler*)request_info->conn_data;
  const SharedMemory* memory = (SharedMemory*)request_info->user_data;

  // read request content
  char* request_buffer = handler->GetRequestBuffer();
  memset(request_buffer, 0, RequestHandler::GetRequestBufferSize());
  DLOG(INFO) << "clear memory time: " << tt.ElapsedMilliSeconds();
  if (!strcmp(request_info->request_method, "GET")) {
    DLOG(WARNING) << "recv get request '" << request_info->query_string << "'"; 
    RESPONSE_ERROR("not support HTTP GET method!");
    return 0;
  } else { // post request
    int read_size = 0;
    char* buf_beg = request_buffer;
    int buf_size = RequestHandler::GetRequestBufferSize();
    do {
      read_size = mg_read(conn, buf_beg, buf_size);
      if (read_size > 0) {
        buf_beg += read_size;
        buf_size -= read_size;
      }
    } while(read_size > 0);
  }
  
  DLOG(INFO) << "request recv time: " << tt.ElapsedMilliSeconds();
  LOG(INFO) << "request json: " << request_buffer;

  QueryData qd;
  QueryParser parser;
  int return_code = parser.Parse(request_buffer, qd);
  if (return_code) {
    LOG(WARNING) << "parse request error: \"" << QueryParser::GetErrMsg(return_code) << "\"";
    RESPONSE_ERROR(QueryParser::GetErrMsg(return_code));
  }

  DLOG(INFO) << "request parse time: " << tt.ElapsedMilliSeconds();
  handler->Process(*memory, qd);

  // Send HTTP reply to the client
  const std::string& json_result = handler->GetJsonResult();
  mg_printf(conn,
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: %lu\r\n"        // Always set Content-Length
      "\r\n"
      "%s\n",
      json_result.length(), json_result.c_str());

  DLOG(INFO) << "request response time: " << tt.ElapsedMilliSeconds();
  return 0;
}

} // end namespace
