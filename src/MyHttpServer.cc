#include "MyHttpServer.h"
#include "MySeriesWork.h"
#include "MyGoTask.h"
// more headers to be added later

#include <sys/socket.h>
#include <netinet/in.h> // 功能：提供Internet地址族的定义
#include <unistd.h>
#include <arpa/inet.h>  // 功能：提供Internet地址族的定义
#include <iostream>

struct HttpContext {
  int fd; 
  HttpRequest request;
  HttpResponse response;
  std::string request_buffer;   // 累积读取的数据
};

// 内部任务: 读取解析HTTP请求
class HttpReadTask : public MyTask, public std::enable_shared_from_this<HttpReadTask> {
public: 
  HttpReadTask(std::shared_ptr<HttpContext> context, MyEpollPoller* poller)
      : context_(std::move(context)), poller_(poller) {}

  virtual void execute() override {
    readHttpRequest();
  }

private:
  void readHttpRequest() {
    char buf[4096];
    std::string &buffer = context_->request_buffer;

    while (true) {
      ssize_t n = read(context_->fd, buf, sizeof(buf));
      if (n > 0) {
        buffer.append(buf, n);
        if (buffer.find("\r\n\r\n") != std::string::npos) {
          // parse http
          if (parse_http_request(buffer, context_->request)) {
            this->done();
          } else {
            // 解析失败
            context_->response.status_code = 400;
            context_->response.status_message = "Bad Request";
            this->done();
          }

          return;
        }
      } else if (n == 0) {
        // close
        this->done();
        return;
      } else if (errno == EAGAIN) {
        // wait for more
        auto self = shared_from_this();
        poller_->add(context_->fd,
                     [self]
                     { self->execute(); },
                    nullptr,
                  true);
        return;
      } else {
        // 其他错误
        this->done();
        return;
      }
    }
  }


private:
  std::shared_ptr<HttpContext> context_;
  MyEpollPoller *poller_;
};


// 内部任务: 发送HTTP响应,将HttpContext中response序列化，后异步写入fd
class HttpWriteTask : public MyTask, public std::enable_shared_from_this<HttpWriteTask> {
public:
  HttpWriteTask(std::shared_ptr<HttpContext> context, MyEpollPoller* poller)
    : context_(context), poller_(poller) {}

    virtual void execute() override {
      writeHttpResponse();
    }

private:
  void writeHttpResponse() {
    std::string response_str = context_->response.to_string();
    const char *data = response_str.c_str();
    size_t len = response_str.length();
    size_t offset = 0;

    while (offset < len) {
      ssize_t n = write(context_->fd, data + offset, len - offset);
      if (n > 0) {
        offset += n;
      } else if (n == -1 && errno == EAGAIN) {
        auto self = shared_from_this();
        poller_->add (context_->fd,
                      nullptr,
                      [self]
                      { self->execute(); },
                      true);

        return;
      } else {
        break;
      }
    }

    close(context_->fd);
    this->done();
  }

private:
  std::shared_ptr<HttpContext> context_;
  MyEpollPoller *poller_;
};

MyHttpServer::MyHttpServer(MyScheduler* scheduler, MyEpollPoller* poller)
  : scheduler_(scheduler), poller_(poller), listen_fd_(-1) {}

void MyHttpServer::start(unsigned short port, HttpProcessCallback process_callback) {
  process_callback_ = std::move(process_callback);
  // listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);   // 改成non-block

  sockaddr_in serv_addr;        // 服务器地址
  serv_addr.sin_family = AF_INET;   // IPv4
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 绑定到所有可用地址
  serv_addr.sin_port = htons(port);               // 端口号转换为网络字节序

  // 设置套接字选项，允许端口复用
  // 这样可以在服务器重启后立即重新绑定到同一端口
  // 否则可能会因为TIME_WAIT状态而无法立即绑定
  // &opt: 指向选项值的指针
  int opt = 1;
  setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(listen_fd_, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("bind");
    exit(1);
  }

  // 最多监听1024个
  if (listen(listen_fd_, 1024) < 0) {
    perror("listen");
    exit(1);
  }

  // 把listen_fd_加入poller, cb是接收新的连接
  poller_->add(listen_fd_,
               [this] { handle_accept(); },
               nullptr,
               false);  // 监听套接字不使用oneshot,因为需要持续监听新连接

  std::cout << "HTTP server started listening on port " << port << std::endl;
}

void MyHttpServer::stop() {
  if (listen_fd_ != -1) {
    poller_->del(listen_fd_);
    close(listen_fd_);
  }
}

void MyHttpServer::handle_accept() {
  sockaddr_in client_addr;  // 客户端地址
  socklen_t client_len = sizeof(client_addr);

  // ET模式，循环accept直到出错
  // ET: 边缘触发
  // 我们保留while(true)给ET，但是提供快速break
  while(true) {

    // *** 这个没有新连接的话会一直卡住 哪怕主线程被ctrl C了 ***
    // *** 改成non-block的socket 并且改成non-block的accept 解决 ***
    // int conn_fd = accept(listen_fd_, (sockaddr *)&client_addr, &client_len);
    int conn_fd = accept4(listen_fd_, (sockaddr *)&client_addr, &client_len, SOCK_NONBLOCK); // 改用asynic I/O
    
    if (conn_fd < 0) {
      // EAGAIN 代表没再多连接了 退出
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      } else {
        perror("accept");
        // continue;
        break;
      }
    }

    // create a new workflow for new conn
    auto context = std::make_shared<HttpContext>();
    context->fd = conn_fd;

    auto series = std::make_shared<MySeriesWork>(scheduler_);

    // task 1: async read
    series->add_task(std::make_shared<HttpReadTask>(context, poller_));

    // task 2: func logic
    series->add_task(std::make_shared<MyGoTask>([this, context] {
      this->process_callback_(context->request, context->response);
    }));

    // task 3: async write
    series->add_task(std::make_shared<HttpWriteTask>(context, poller_));

    scheduler_->schedule(series);
  }
}