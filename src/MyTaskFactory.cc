#include "MyTaskFactory.h"
#include "MyGoTask.h"

// 静态变量定义和初始化
// 实际分配内存 ODR
std::vector<std::shared_ptr<MyGoTask>> MyTaskFactory::go_task_pool_;
std::mutex MyTaskFactory::go_task_pool_mutex_;

std::shared_ptr<MyGoTask> MyTaskFactory::create_go_task(std::function<void()>&& func) {
  std::lock_guard<std::mutex> lock(go_task_pool_mutex_);
  std::shared_ptr<MyGoTask> task;

  if (!go_task_pool_.empty()) {
    task = go_task_pool_.back();
    go_task_pool_.pop_back();
    task->set_go_func(std::move(func));
  } else {
    task = std::make_shared<MyGoTask>(std::move(func));
  }

  return task;
}


// put task back into pool
void MyTaskFactory::recycle_go_task(std::shared_ptr<MyGoTask> task) {
  std::lock_guard<std::mutex> lock(go_task_pool_mutex_);
  go_task_pool_.emplace_back(task);
}