# 学习记录

0802

## 第二章 SeriesTask

MyGoTask和MyScheduler解决了任务的执行和并发 但是没有对任务的顺序调度和依赖。SeriesTask解决的就是这个问题。

**回调地狱：** 多个异步执行的组合，会导致回调函数层层嵌套，影响可读性和设计。

### 升级MyTask

在MyTask和MyGoTask里面，添加任务完成时的callback函数和标志任务完成的done()函数，为
框架提供串联任务的能力。

### class MySeriesWork

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
