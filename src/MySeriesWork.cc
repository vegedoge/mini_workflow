#include "MySeriesWork.h"

MySeriesWork::MySeriesWork(MyScheduler* scheduler) : scheduler_(scheduler) {}

MySeriesWork::~MySeriesWork() {
  // clean logic to be added later
}

void MySeriesWork::add_task(MyTask* task) {
  tasks_.push(task);
}

// start the first task when the seriesWork is called
void MySeriesWork::execute() {
  handle_next();
}

void MySeriesWork::set_series_callback(std::function<void(MySeriesWork*)> cb) {
  // 按值传递+std::move
  series_callback_ = std::move(cb);
}

void MySeriesWork::handle_next() {
  if (tasks_.empty()) {
    // all sub tasks done,
    // trigger series call_back
    if (series_callback_) {
      series_callback_(this);
    }

    this->done();
    return;
  }

  // get the next task
  MyTask* next_task = tasks_.front();
  tasks_.pop();

  // 给下一个任务设置回调，回调就是继续call seriesWork的下一个任务
  next_task->set_callback([this](MyTask* task) {
    // 这个lambda在子任务完成时被调用
    this->handle_next();
  });

  scheduler_->schedule(next_task);
}