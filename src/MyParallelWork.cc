#include "MyParallelWork.h"

MyParallelWork::MyParallelWork(MyScheduler* scheduler) : scheduler_(scheduler), counter_(0) {}

void MyParallelWork::add_task(std::shared_ptr<MyTask> task) {
  tasks_.emplace_back(task);
}

void MyParallelWork::set_parallel_callback(std::function<void(MyParallelWork*)> cb) {
  parallel_callback_ = std::move(cb);
}

void MyParallelWork::execute() {
  size_t task_count = tasks_.size();
  counter_.store(task_count);

  if (task_count == 0) {
    this->done();
    return;
  }

  auto self = shared_from_this();

  for (auto& sub_task: tasks_) {
    // set cb for all sub-tasks, pointing to the same sub_task_done
    sub_task->set_callback([self](MyTask* t) {
      self->sub_task_done();
    });
    scheduler_->schedule(sub_task);
  }
}

void MyParallelWork::sub_task_done() {
  // counter--
  if (--counter_ == 0) {
    if (parallel_callback_) {
      parallel_callback_(this);
    }
    this->done();
  }
}