/*
  main function for testing
  under development
*/
#include <iostream>
#include <chrono>
#include "MyScheduler.h"
#include "MyGoTask.h"
#include "MySeriesWork.h"
#include "MyWaitGroup.h"

int main() {
  MyScheduler scheduler;
  MyWaitGroup wg(1); //等待一个任务，（我们的Series）完成

  MySeriesWork *series = new MySeriesWork(&scheduler);

  // add 3 gotask
  series->add_task(new MyGoTask([] {
    printf("Step 1: reading the user profile...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }));

  series->add_task(new MyGoTask([] {
    printf("Step 1: reading the user's post...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }));

  series->add_task(new MyGoTask([] {
    printf("Step 1: computing...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
  }));

  // set callback for the whole series
  series->set_series_callback([&wg](MySeriesWork *series) {
    printf("All steps finished\n");
    wg.done();
  });

  printf("Starting the whole series in scheduler...\n");
  scheduler.schedule(series);

  wg.wait();
  printf("Main thread finished\n");

  // 清理MySeriesWork对象
  delete series;

  // wait for deconstructor to print(if any)
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  return 0;
}
