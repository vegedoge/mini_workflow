# 学习记录

## 第三章 Epoller

### 3.1 MyEpollPoller

在一个线程里不断调用epoll\_wait(), 如果存在网络事件，不直接处理，而是打包成一个MyGoTask，交给调度器，从而实现解耦。

#### 3.1.1 相关概念

+ fd: 文件描述符  everything is a file. fd是一个非负整数，每个实例都分配一个fd，它是进程打开资源的索引。
  
  ```cpp
  stdin: 标准输入 -- 0    stdout: 标准输出 -- 1
  stderr: 标准错误 -- 2
  ```

+ epoll: I/O多路复用的手段 用一个线程处理上万个fd
  此外还有select和poll,但这两是O(n), epoll是O(1)

  epoll有三个核心系统调用：
  + epoll\_create: 创建epoll实例， 返回fd
  + epoll\_ctl: 向实例增删改fd
  + epoll\_wait: 等待被监控fd就绪(r/w)

  在workflow中 epoll工作流程：

  ```cpp
  ...创建epoll实例 -> 注册socket fd到epoll -> epoll_wait等待事件  
  -> 分发就绪事件到Task -> epollPoller监控fd -> epollerPoller 通知Task  
  -> Task执行回调
  ```

+ epoll\_event: epoll机制中的struct

  ```cpp
  struct epoll_event {
    uint32_t events;
    epoll_data_t data;
  }
  typedef union epoll_data{
    void* ptr;   // 指向any obj       
    int   fd;    // fd
    uint32_t u32;
    uint64_t u64;
  }
  ```

#### 3.1.2 MyEpollPoller

+ 构造函数接收一个Scheduler 用来作为分发事件的调度器
+ 禁止拷贝和赋值 系统资源的生命周期由一个对象独占
+ add()接口： is\_oneshot 事件触发后，epoll会自动禁止该fd，避免一个事件被多个线程处理。

