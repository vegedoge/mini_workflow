# 学习记录

0802

## 第二章 SeriesTask

MyGoTask和MyScheduler解决了任务的执行和并发 但是没有对任务的顺序调度和依赖。SeriesTask解决的就是这个问题。

**回调地狱：** 多个异步执行的组合，会导致回调函数层层嵌套，影响可读性和设计。

### 升级MyTask

在MyTask和MyGoTask里面，添加任务完成时的callback函数和标志任务完成的done()函数，为
框架提供串联任务的能力。