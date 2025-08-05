#ifndef _MYPARALLELWORK_H_
#define _MYPARALLELWORK_H_

#include <vector>
#include <atomic>
#include <memory>
#include "MyTask.h"
#include "MyScheduler.h"

class MyParallelWork : public MyTask, public std::enable_shared_from_this<MyParallelWork> {
public:
  MyParallelWork(MyScheduler *scheduler);

  void add_task(std::shared_ptr<MyTask> task);
  virtual void execute() override;

  // cb
  void set_parallel_callback(std::function<void(MyParallelWork *)> cb);

private:
  void sub_task_done();

private:
  std::vector<std::shared_ptr<MyTask>> tasks_;
  MyScheduler *scheduler_;
  std::atomic<size_t> counter_;
  std::function<void(MyParallelWork *)> parallel_callback_;
};

#endif