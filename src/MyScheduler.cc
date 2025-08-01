#include "MyScheduler.h"
#include <iostream>

MyScheduler::MyScheduler() : stop_(false) {
  // get the number of hardware threads
  const size_t num_threads = std::thread::hardware_concurrency();
  std::cout << "Scheduler Starting " << num_threads << " worker threads" << '\n';

  // create worker threads
  for (size_t i = 0; i < num_threads; ++i) {
    // 对每一个thread element 绑定到 worker_loop 函数，指明这个成员函数的上下文是this
    workers_.emplace_back([this] {
      this->worker_loop();
    });
  }
}

MyScheduler::~MyScheduler() {
  // stop
  stop_.store(true);

  // notify all worker threads to wake up
  condition_.notify_all();

  // wait and join all worker threads
  for (std::thread& worker : workers_) {
    worker.join();
  }
  std::cout << "Scheduler stopped" << '\n';
}

void MyScheduler::schedule(MyTask* task) {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    tasks_.push(task);
  } // mutex released here, {} helpes early release

  condition_.notify_one(); // notify one worker thread
}

void MyScheduler::worker_loop() {
  while(true) {
    MyTask *task;
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);

      condition_.wait(lock, [this] { 
        return this->stop_.load() || !this->tasks_.empty(); 
      });

      // quit if stop and no tasks
      if (this->stop_.load() && this->tasks_.empty()) {
        return;
      }

      // get a task
      task = tasks_.front();
      tasks_.pop();
    }

    task->execute();

    // destory the task obj, may use memory pool later
    delete task;
  }
}
