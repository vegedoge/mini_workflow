# 学习记录

0801

## 第一章 Task

从Task入手开始学习workflow的精神， 主打目标就是一切都是Task, 封装好之后进行异步执行

### 1.1 MyTask class

这个是纯virtual的基类，一切Task都从这个类派生

#### 1.1.1 Include Guard

```cpp
  #ifndef #define #endif
  或者
  #pragma once
```

在头文件里 防止头文件被重复包含

### 1.2 MyGoTask class

这个类实现单纯的任务执行，把任意能调用的对象（函数指针，lambda）包装成一个任务，然后交给调度器异步执行

#### 1.2.1 callable object
  
+ 函数指针： 类型严格，无开销  
  int (*func_ptr)(int, int) = add; func_ptr(3,4);
+ 函数对象： 一个重载了operator()的类，编译期确定，是lambda的底层实现方法

  ```cpp
  struct Add{
      int operator() (int a, int b) {
          return a + b;
      }
  }
  Add add_obj;
  add_obj(3, 4);
  ```

+ lambda: 匿名函数，可以捕获外界变量（[=]或者[&]), 本质是函数对象
  
  ```cpp
  int offset = 10;
  auto lambda = [offset](int a) {return a + offset;};
  ```

+ std::function: 一个类模板，可以保存，复制，调用任意可调用对象,实现类型擦除，把不同的对象统一接口。  
  
  ```cpp
  std::function<return type(args...)> func;
  std::function<int(int, int)> f;
  f = add;    // pointer func
  f = Add();  // func obj
  f = [](int a, int b){ return a + b;}; // lambda

  f(3, 4);
  ```

### 1.3 MyScheduler class

调度器 用来调度异步执行

#### 1.3.1 概念解析

+ std::atomic\<T\> : 提供对类型T的原子操作, 避免数据竞争, 轻量级共享

  ```cpp
  std::atomic<int> counter(0);
  counter++;
  ```

+ std::condition_variable: 线程间通信，允许线程等待另一个线程中的条件成立并通知它，要和std::mutex一起用
  
+ std::semaphore: 信号量，用计数器阻塞实现对资源的有限访问  
  用cv模拟信号量

  ```cpp
  class semaphore{
    std::mutex mtx;
    std::condition_variable cv;
    int count;
    }
  public:
    semaphore(int c) : count(c) {}

    void acquire() {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [this]{return count > 0;});
      --count;
    }
    void release() {
      std::unique_lock<std::mutex> lock(mtx);
      ++count;
      cv.notify_one();
    }
  ```

+ mutex的临界区
  
  ```cpp
    func a() {
      {
        std::unique_lock<mutex> lock(mtx);
        ...
      }
      ...
    }
    // 用{}可以让锁更早地释放
  ```

