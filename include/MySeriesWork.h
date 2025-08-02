#ifndef _MYSERIESWORK_H_
#define _MYSERIESWORK_H_

#include <queue>
#include <memory>
#include <functional>
#include "MyTask.h"
#include "MyScheduler.h"

class MySeriesWork : public MyTask, public std::enable_shared_from_this<MySeriesWork> {
public:
  MySeriesWork(MyScheduler *scheduler);
  virtual ~MySeriesWork();

  void add_task(std::shared_ptr<MyTask> task);

  virtual void execute() override;

  // callback for seriesWork
  void set_series_callback(std::function<void(MySeriesWork *)> cb);

private:
  // 核心：处理调度下一个任务
  void handle_next();

private:
  std::queue<std::shared_ptr<MyTask>> tasks_;
  MyScheduler *scheduler_;
  std::function<void(MySeriesWork *)> series_callback_;
};


#endif // _MYSERIESWORK_H_