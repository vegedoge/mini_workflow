#ifndef _MYHTTPSERVER_H_
#define _MYHTTPSERVER_H_

#include <functional>
#include <HttpMessages.h>
#include <MyScheduler.h>
#include <MyEpollPoller.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

using HttpProcessCallback = std::function<void(const HttpRequest &, HttpResponse &)>;

class MyHttpServer {
public:
  MyHttpServer(MyScheduler *scheduler, MyEpollPoller *poller);
  void start(unsigned short port, HttpProcessCallback process_callback);  // short: 因为端口号最大65535
  void stop();

private:
  void handle_accept();

  // 初始化和清理SSL上下文用
  bool init_ssl_context(const char *cert_file, const char *key_file);
  void cleanup_ssl_context();

private:
  int listen_fd_;  // 监听套接字
  MyScheduler *scheduler_;
  MyEpollPoller *poller_;
  HttpProcessCallback process_callback_;  // HTTP处理回调函数
  SSL_CTX *ssl_ctx_;  // 全局ssl上下文
};

#endif