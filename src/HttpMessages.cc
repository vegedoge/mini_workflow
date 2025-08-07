#include "HttpMessages.h"

bool parse_http_request(const std::string& raw, HttpRequest& req) {
  size_t pos = 0;

  // parse起始行： GET /path HTTP/1.1
  size_t line_end = raw.find("\r\n", pos);
  if (line_end == std::string::npos) {
    return false;
  }

  std::string first_line = raw.substr(pos, line_end - pos);
  pos = line_end + 2;

  std::istringstream iss(first_line); // isringstream: 字符串流，仅继承istream(读取)
  // 没成功匹配到
  if (!(iss >> req.method >> req.path >> req.version)) {
    return false;
  }

  // parse url中的query --- 暂时不支持
  // size_t query_pos = req.path.find("?");
  // if (query_pos != std::string::npos) {
  //   req.query = req.path.substr(query_pos + 1);
  //   req.path = req.path.substr(0, query_pos);
  // }

  // 解析headers
  while (true) {
    line_end = raw.find("\r\n", pos);
    if (line_end == std::string::npos) {
      return false;
    }

    std::string line = raw.substr(pos, line_end - pos);
    pos = line_end + 2;

    if (line.empty()) {
      break;
    }

    size_t separator = line.find(":");
    if (separator == std::string::npos) {
      continue;   // 没： 这个header无效
    }

    std::string key = line.substr(0, separator);
    std::string value = line.substr(separator + 1);

    // remove " " near value
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    req.headers[key] = value;
  }

  // parse body
  size_t body_start = pos;
  size_t body_end = raw.size();

  if (body_start < body_end) {
    req.body = raw.substr(body_start, body_end - body_start);

    auto content_type_it = req.headers.find("content_type");
    if (content_type_it != req.headers.end()) {
      const std::string &content_type = content_type_it->second;
      // 可拓展操作 比如查表
      printf("specific body action to be added\n");
    }
  }

  return true;
}