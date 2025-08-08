#include "MyHttpClientTask.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

// url parser, static: only useful here
static bool parse_url(const std::string& url, std::string& host, int& port, std::string& path) {
  // only supports: http://host:port/path

  std::string temp = url;
  // find the last index of http://
  if (temp.rfind("http://", 0) != 0) {
    return false;
  }
  temp = temp.substr(7);    // exclude the scheme

  size_t path_pos = temp.find("/");
  if (path_pos == std::string::npos) {
    path = "/";
    host = temp;
  } else {
    path = temp.substr(path_pos);
    host = temp.substr(0, path_pos);
  }

  size_t port_pos = host.find(":");
  if (port_pos != std::string::npos) {
    port = std::stoi(host.substr(port_pos + 1));
    host = host.substr(0, port_pos);
  } else {
    port = 80;    // default for http
  }

  return true;
}

MyHttpClientTask::MyHttpClientTask(MyScheduler* scheduler, MyEpollPoller* poller, std::string url) 
  : scheduler_(scheduler), poller_(poller), url_(std::move(url)), sockfd_(-1), cleaned_up_(false)
{
  if (!parse_url(url_, host_, port_, path_)) {
    // error handling
    std::cerr << "Failed to parse URL: " << url_ << std::endl;
    throw std::invalid_argument("Invalid URL format");
  }
}

void MyHttpClientTask::execute() {
  // addrinfo 存地址信息的结构体
  // hints: 用于指定地址信息的格式，res: 用于存储解析
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;      // 地址族，AF_UNSPEC表示IPv4或IPv6
  hints.ai_socktype = SOCK_STREAM;  // 套接字类型，SOCK_STREAM表示TCP

  // get address info, store in res
  if (getaddrinfo(host_.c_str(), std::to_string(port_).c_str(), &hints, &res) != 0) {
    handle_error();
    return;
  }

  // create socket
  sockfd_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd_, F_SETFL, O_NONBLOCK);  // non-blocking mode

  auto self = shared_from_this();

  // start non-blocking connect
  int ret = connect(sockfd_, res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);  // 作用： 释放addrinfo结构体

  if (ret == 0) { // immediate success
    handle_connect();
  } else if (errno == EINPROGRESS) { // in progress
    // register for write event to check connection status
    poller_->add(sockfd_,
                 nullptr,
                 [self]() { self->handle_connect(); },
                 true);
  } else { // error occurred
    handle_error();
  }
}

void MyHttpClientTask::handle_connect() {
  std::cout << "handle_connect() called, sockfd: " << sockfd_ << std::endl;
  
  // construct HTTP request
  std::string request = "GET " + path_ + " HTTP/1.1\r\n"
                       "Host: " + host_ + "\r\n"
                       "User-Agent: MyHttpClient/1.0\r\n"
                       "Accept: */*\r\n"
                       "Connection: close\r\n\r\n";
  ssize_t written = write(sockfd_, request.c_str(), request.length());
  std::cout << "HTTP request written: " << written << " bytes" << std::endl;

  // set the monitoring event as "readable", set cb as handle_read
  // 注意：这里不使用 oneshot，因为我们需要持续监听读取事件
  auto self = shared_from_this();
  poller_->add(sockfd_, 
              [self] { 
                self->handle_read(); 
              }, 
              nullptr, 
              false);  // 不使用 oneshot
}

void MyHttpClientTask::handle_read() {
  // 如果已经清理过了，直接返回
  if (cleaned_up_) {
    return;
  }
  
  std::cout << "handle_read() called, sockfd: " << sockfd_ << std::endl;
  
  char buf[4096];
  ssize_t total_read = 0;
  
  // 读取所有可用数据
  while (true) {
    ssize_t n = read(sockfd_, buf, sizeof(buf));
    
    if (n > 0) {
    response_.append(buf, n);
      total_read += n;
      std::cout << "Read " << n << " bytes, total response size: " << response_.length() << std::endl;
    } else if (n == 0) {
      // 连接关闭
      std::cout << "Connection closed by server, finishing task" << std::endl;
      if (!cleaned_up_) {
        cleaned_up_ = true;
        poller_->del(sockfd_);
        close(sockfd_);
      }
      this->done();
      return;
    } else if (n == -1) {
      if (errno == EAGAIN) {
        // 暂时没有更多数据
        break;
      } else {
        // 其他错误
        std::cout << "Read error, errno=" << errno << std::endl;
        if (!cleaned_up_) {
          cleaned_up_ = true;
          poller_->del(sockfd_);
          close(sockfd_);
        }
        this->done();
        return;
      }
    }
  }
  
  // 检查是否已经收到了完整的HTTP响应
  if (response_.find("\r\n\r\n") != std::string::npos && response_.length() > 100) {
    std::cout << "HTTP response appears complete, finishing task" << std::endl;
    if (!cleaned_up_) {
      cleaned_up_ = true;
    poller_->del(sockfd_);
    close(sockfd_);
    }
    this->done();
    return;
  }
  
  // 继续等待更多数据
  std::cout << "Continuing to wait for more data..." << std::endl;
}

void MyHttpClientTask::handle_error() {
  std::cerr << "Task error: " << strerror(errno) << std::endl;
  if (sockfd_ != -1 && !cleaned_up_) {
    cleaned_up_ = true;
    poller_->del(sockfd_);
    close(sockfd_);
  }

  this->done();
}
