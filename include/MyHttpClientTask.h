#ifndef _MYHTTPCLIENTTASK_H_
#define _MYHTTPCLIENTTASK_H_

#include <string>
#include <memory>
#include "MyTask.h"
#include "MyScheduler.h"
#include "MyEpollPoller.h"

class MyHttpClientTask : public MyTask, public std::enable_shared_from_this<MyHttpClientTask> {
public:
  MyHttpClientTask(MyScheduler *scheduler, MyEpollPoller *poller, std::string url);

  std::string get_response() const {
    return response_;
  }

protected:
  // override execute method
  virtual void execute() override;

private:
  // 4 stages of http client task
  void handle_connect();
  void handle_read();
  void handle_write();
  void handle_error();

private:
  MyEpollPoller *poller_;
  MyScheduler *scheduler_;
  std::string url_;
  std::string host_;
  int port_;
  std::string path_;

  int sockfd_;
  std::string response_;
  bool cleaned_up_;  // 防止重复清理的标志
};

#endif
