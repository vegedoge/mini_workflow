#include "MyScheduler.h"
#include "MyGoTask.h"
#include <iostream>
#include <memory>

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
  std::cout << "[Scheduler] Destructor called. Stopping scheduler..." << std::endl;
  
  // stop
  stop_.store(true);

  // notify all worker threads to wake up
  condition_.notify_all();

  // wait and join all worker threads
  for (size_t i = 0; i < workers_.size(); ++i) {
    printf("[Scheduler] Joining thread %zu/%zu (%p)...\n", i+1, workers_.size(), workers_[i].get_id());
    workers_[i].join();
    printf("[Scheduler] Thread %zu joined.\n", i+1);
  }
  std::cout << "[Scheduler] Scheduler stopped" << '\n';
}

void MyScheduler::schedule(std::shared_ptr<MyTask> task) {
  // reject if stop confirmed
  if (stop_.load()) {
    printf("[Scheduler] Warning: Scheduling task on a stopped scheduler. Task rejected.\n");
    return;
  }
  
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    tasks_.push(task);
  } // mutex released here, {} helpes early release

  condition_.notify_one(); // notify one worker thread
}

void MyScheduler::worker_loop() {
  while(true) {
    // use shared ptr 
    std::shared_ptr<MyTask> task;
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

    // 1.0: shared ptr 会自动处理引用计数
    // 不用delete

    // 2.0: 内存池 不销毁而回收
    // 只有在调度器没有停止时才回收任务
    if (!stop_.load()) {
      task->recycle();
    }
  }
}
