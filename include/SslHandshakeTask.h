#ifndef _SSLHANDSHAKETASK_H_
#define _SSLHANDSHAKETASK_H_

#include "MyTask.h"
#include "MyEpollPoller.h"
#include "HttpMessages.h"

class SslHandshakeTask : public MyTask, public std::enable_shared_from_this<SslHandshakeTask> {
public:
  SslHandshakeTask(std::shared_ptr<HttpContext> context, MyEpollPoller *poller);
  virtual void execute() override;

private:
  void do_handshake();

private:
  std::shared_ptr<HttpContext> context_;
  MyEpollPoller *poller_;
};

#endif