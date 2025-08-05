#ifndef _MYTASKFACTORY_H_
#define _MYTASKFACTORY_H_

#include <memory>
#include <functional>
#include <mutex>
#include <vector>
#include "MyTask.h"

class MyGoTask; //前向声明 避免MyGoTask循环依赖

class MyTaskFactory {
public:
  // try get one task from pool, otherwise create a new one
  static std::shared_ptr<MyGoTask> create_go_task(std::function<void()> &&func);

  // other task creation methods to be added

  // recycle back into pool
  static void recycle_go_task(std::shared_ptr<MyGoTask> task);

private:
  // store reusable MyGoTask obj
  static std::vector<std::shared_ptr<MyGoTask>> go_task_pool_;
  static std::mutex go_task_pool_mutex_;
};

#endif