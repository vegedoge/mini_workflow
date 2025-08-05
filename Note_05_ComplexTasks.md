# 学习记录

## 第五章 ComplexTasks

### 5.1 TaskFactory

我们希望用任务工厂来实现任务的创建，同时为MygoTask这一单一的计算任务 实现一个简单的内存对象池,从而复用MyGoTask,减少内存开销 优化性能

#### 5.1.1 Recycle

相比于原先对Task的new和delete 我们希望实现回收

```cpp
  go_func_ = nullptr;
  MyTaskFactory::recycle_go_task(std::static_pointer_cast<MyGoTask>(shared_from_this()));
```

首先清除之前的std::function,之后将自己(this指针)转换成shared\_ptr管理的指针 交给工厂。
这里static\_pointer\_cast是保险 一般基类转派生类采用。
交给工厂之后，工厂把他再放回obj pool供下一次使用。

```cpp
// 一个完整的声明周期：
1. create_go_task() 
   → 从池取 or new shared_ptr
   ↓
2. scheduler.schedule(task)
   ↓
3. worker 线程执行 task->execute()
   ↓
4. execute() 完成 → task->done()
   ↓
5. done() → 调用回调 → 调用 recycle()
   ↓
6. recycle() → 重置状态 → 放回池中
   ↓
7. 下次 create 可复用
```

#### 5.1.2 MyTaskFactory

+ create\_go\_task: 尝试从内存池里取出对象 然后重新set function复用，而不是新new一个。

+ recycle\_go\_task: 把task重新塞回pool里。

### 5.2 ParallelWork

#### 5.2.1 MyParallelWork

并发执行多个subTask，完成之后触发回调。对应WFCounterTask。

+ counter\_: 使用atomic counter来追踪子任务完成
+ execute(): 启动所有子任务，用sub\_task\_done()汇报完成
+ sub\_task\_done(): 最后一个完成者模式

```cpp
  if (--counter_ == 0) {
    if (parallel_callback_) {
      parallel_callback_(this);    
    }
    this->done();
  }
```

这里外接设置callback的时候 先捕捉可调用对象 然后执行对象 然后再执行callback里面的lambda内容。
