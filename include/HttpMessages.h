#ifndef _HTTPMESSAGES_H_
#define _HTTPMESSAGES_H_

#include <string>
#include <map>
#include <sstream>
#include <algorithm>

// case Insensitive
struct CaseInsensitiveCompare {
  bool operator()(const std::string& a, const std::string& b) const{
    return std::lexicographical_compare(
      a.begin(), a.end(),
      b.begin(), b.end(),
      [](char c1, char c2) {
        return std::tolower(c1) < std::tolower(c2);
      }
    );
  }
};

struct HttpRequest {
  std::string method;
  std::string path;
  std::string version;
  std::map<std::string, std::string, CaseInsensitiveCompare> headers;
  std::string body;
};

struct HttpResponse {
  int status_code = 200;
  std::string status_message = "OK";
  std::map<std::string, std::string, CaseInsensitiveCompare> headers;
  std::string body;

  // serialize self to http response string
  std::string to_string() const {
    std::stringstream ss;
    ss << "HTTP/1.1" << status_code << " " << status_message << "\r\n";
    // 输出用户设置的headers
    for (const auto& header : headers) {
      ss << header.first << ": " << header.second << "\r\n";
    }

    // check and add Content-Length
    if (headers.find("Content-Length") == headers.end()) {
        ss << "Content-Length: " << body.length() << "\r\n";
    }

    // Connection: close
    if (headers.find("Connection") == headers.end()) {
        ss << "Connection: close\r\n";
    }

    ss << "\r\n";
    
    ss << body;
    return ss.str();
  }
};

bool parse_http_request(const std::string& raw, HttpRequest& req);


#endif