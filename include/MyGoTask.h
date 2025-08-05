#ifndef _MYGOTASK_H_
#define _MYGOTASK_H_

#include<functional> // functional: provides function objects, function pointers and binders
#include "MyTask.h"
#include "MyTaskFactory.h"

// GoTask: a specialized task that only does computation
class MyGoTask : public MyTask, public std::enable_shared_from_this<MyGoTask> {
public:
  // constructor: takes a function to exec, like a lambda or std::function
  MyGoTask(std::function<void()>&& func) : go_func_(std::move(func)) {}

  // instantiate the execute method of base class
  virtual void execute() override {
    if (go_func_) {
        go_func_(); // call the function

        this->done(); // nofity after execution, making it possible for serial processing
    }
  }

  virtual void recycle() override {
    // reset func to avoid memory leak
    go_func_ = nullptr;
    // giving back to factory
    MyTaskFactory::recycle_go_task(std::static_pointer_cast<MyGoTask>(shared_from_this()));
  }

  // set new func when reusing
  void set_go_func(std::function<void()>&& func) {
    go_func_ = std::move(func);
  }

private:
  // go_func_: private member, stores the callable obj
  std::function<void()> go_func_; // function to execute
};

#endif