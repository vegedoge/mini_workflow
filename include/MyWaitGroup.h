#ifndef _MYWAITGROUP_H_
#define _MYWAITGROUP_H_

#include <mutex>
#include <condition_variable>
#include <atomic>

class MyWaitGroup {
public:
  explicit MyWaitGroup(int n) : count_(n) {}

  void done() {
    if (--count_ == 0) {
      cv_.notify_one();
    }
  }

  void wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]
             { return count_ == 0; });
  }

private:
  std::atomic<int> count_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

#endif