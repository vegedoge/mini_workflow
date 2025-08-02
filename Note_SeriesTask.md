# 学习记录

0802

## 第二章 SeriesTask

MyGoTask和MyScheduler解决了任务的执行和并发 但是没有对任务的顺序调度和依赖。SeriesTask解决的就是这个问题。

**回调地狱：** 多个异步执行的组合，会导致回调函数层层嵌套，影响可读性和设计。

### 2.1 升级MyTask

在MyTask和MyGoTask里面，添加任务完成时的callback函数和标志任务完成的done()函数，为
框架提供串联任务的能力。

### 2.2 class MySeriesWork

用来调度和处理任务链的work，由于内容可能较为复杂，需要继承一个*virtual deconstructor*,而不像MyGoTask一样直接调用默认的deconstructor。

+ 值传递+std::move : 既可以高效处理lambda等右值临时对象，避免拷贝，又可以兼容左值。
+ Series callback设计  
  seriesWork的精髓是给被执行的任务设置callback，让它执行结束后回头再次调用seriesWork里面的handle_next();

  ```cpp
  ...handle_next() -> 取得next_task -> next_task设置callback 
  -> next_task.execute() -> execute结束调用done() 
  -> done()发现callback,调用callback(this) 
  -> 之前设置好的callback()捕获seriesWork,继续调用handle_next()...
  ```

### 2.3 life-span management

这个mini版本有个问题，对每个Task使用裸指针管理。比如SeriesWork的指针，在scheduler里面单次执行结束后会被delete，但实际还要被handle_next()作为this捕获 这样就爆了 -- segmentation fault。  

**智能指针** 来解决这个问题，改成shared\_ptr, 用引用计数统计，只有最后一个指向它的ptr也被delete，再真正delete。

通过在lambda中捕获self（一个shared_ptr），我们人为给MySeriesWork对象增加了一个引用计数。这个lambda会和子任务next_task一起存活，直到子任务完成并被销毁。因此，MySeriesWork对象至少会活到它的子任务回调被执行的那一刻，解决悬垂指针问题。

**shared\_from\_this** 获取继承了这个trait的类的shared\_ptr. 因为某些回调函数会调用this，which可能已经被外部销毁了。利用shared\_from\_this可以首先捕获this作为共享指针，用引用计数保证存活。

