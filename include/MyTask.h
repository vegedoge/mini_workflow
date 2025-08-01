#ifndef _MYTASK_H_
#define _MYTASK_H_

// base class
// it only defines the minimum capability of tasks: being executed

class MyTask {
public:
  // all sub classes succeed this virtual execute
  virtual void execute() = 0;

  // deconstruction 
  virtual ~MyTask() {}
};

#endif