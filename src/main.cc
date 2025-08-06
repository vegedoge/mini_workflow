/*
  main function for testing
  under development
*/
#include <iostream>
#include <chrono>
#include <memory>
#include <thread>
#include "MyScheduler.h"
#include "MyGoTask.h"
#include "MySeriesWork.h"
#include "MyWaitGroup.h"
#include "MyHttpClientTask.h"
#include "MyEpollPoller.h"
#include "MyTaskFactory.h"
#include "MyParallelWork.h"

int main() {
  MyScheduler scheduler;
  MyWaitGroup wg(1); //等待一个任务，（我们的Series）完成
  MyEpollPoller poller(&scheduler);

  // creat an serie work
  auto series = std::make_shared<MySeriesWork>(&scheduler);

  // task 1: simple goTask
  series->add_task(MyTaskFactory::create_go_task([] {
    printf("Series: start, preparing for parallel tasksk...\n");
  }));

  // task 2: complex parallel task
  auto parallel = std::make_shared<MyParallelWork>(&scheduler);

  // sub-tasks for parallel task
  parallel->add_task(MyTaskFactory::create_go_task([] {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    printf("Parallel: GoTask finished.\n");
  }));

  parallel->add_task(std::make_shared<MyHttpClientTask>(&scheduler, &poller, "http://httpbin.org/delay/2"));
  parallel->add_task(std::make_shared<MyHttpClientTask>(&scheduler, &poller, "http://httpbin.org/delay/3"));

  parallel->set_parallel_callback([] (MyParallelWork* p) {
    printf("Parallel: all tasks finished.\n");
  });

series->add_task(parallel);

// task 3: final task
series->add_task(MyTaskFactory::create_go_task([] {
  printf("Sereis: all parallel work is done... \n");
}));

series->set_series_callback([&wg](MySeriesWork* s) {
  printf("Series: whole series finished. \n");
  wg.done();  // 通知 WaitGroup 任务完成
});

scheduler.schedule(series);

wg.wait();
printf("Main thread finished.\n");
std::this_thread::sleep_for(std::chrono::milliseconds(300));

return 0;
}
