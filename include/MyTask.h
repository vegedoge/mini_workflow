#ifndef _MYTASK_H_
#define _MYTASK_H_

#include <functional>
// base class
// it only defines the minimum capability of tasks: being executed

class MyTask {
public:
  // all sub classes succeed this virtual execute
  virtual void execute() = 0;

  // deconstruction 
  virtual ~MyTask() {}

  // set call back function when task finished
  void set_callback(std::function<void(MyTask*)> cb) {
    this->callback_ = std::move(cb);
  }

protected:
  // when task finishes, use this method to trigger callback
  void done() {
    if (callback_) {
      callback_(this);
    }
  }

private:
  std::function<void(MyTask *)> callback_;
};

#endif