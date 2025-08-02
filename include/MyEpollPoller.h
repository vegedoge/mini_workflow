#ifndef _MYEPOLLPOLLER_H_
#define _MYEPOLLPOLLER_H_

#include <sys/epoll.h>
#include <functional>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <memory>
#include <atomic>

#include "MyScheduler.h"

// 事件处理结构体
struct FdHandler {
  int fd;
  std::function<void()> read_handler;     // fd可读时执行的回调
  std::function<void()> write_handler;    // fd可写时执行的回调
};

class MyEpollPoller {
public:
  MyEpollPoller(MyScheduler *scheduler);
  ~MyEpollPoller();

  // 禁止拷贝和赋值 系统资源由一个对象独占
  MyEpollPoller(const MyEpollPoller &) = delete;
  MyEpollPoller &operator=(const MyEpollPoller &) = delete;

  // 事件监听
  // 增加和修改
  bool add(int fd,
           std::function<void()> read_cb,   // 可读cb 
           std::function<void()> write_cb,  // 可写cb
           bool is_oneshot);
  // 删除
  bool del(int fd);

private:
  void poller_loop();

private:
  int epoll_fd_;
  MyScheduler *scheduler_;
  std::thread poller_thread_;
  std::atomic<bool> stop_flag_;
  std::map<int, FdHandler> handlers_;
  std::mutex handlers_mutex_;
};

#endif