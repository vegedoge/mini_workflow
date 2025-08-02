#ifndef _MYSCHEDULER_H_
#define _MYSCHEDULER_H_

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include "MyTask.h"

class MyScheduler {
public:
  MyScheduler();
  ~MyScheduler();

  // public interface for submitting tasks
  void schedule(std::shared_ptr<MyTask> ptr);

private:
  // thread looping function
  void worker_loop();

private:
  std::vector<std::thread> workers_;    // 工作线程列表
  std::queue<std::shared_ptr<MyTask>> tasks_;          // 任务队列
  std::mutex queue_mutex_;
  std::condition_variable condition_;   // 线程通信的条件变量
  std::atomic<bool> stop_;              // 调度器停止
};

#endif