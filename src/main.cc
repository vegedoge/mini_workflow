/*
  main function for testing
  under development
*/
#include <iostream>
#include <chrono>
#include <memory>
#include "MyScheduler.h"
#include "MyGoTask.h"
#include "MySeriesWork.h"
#include "MyWaitGroup.h"

int main() {
  MyScheduler scheduler;
  MyWaitGroup wg(1); //等待一个任务，（我们的Series）完成

  // add 3 gotask
  auto series = std::make_shared<MySeriesWork>(&scheduler);

  series->add_task(std::make_shared<MyGoTask>([] {
    printf("Step 1: Reading user profiles...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }));

  series->add_task(std::make_shared<MyGoTask>([] {
    printf("Step 2: Reading user's posts...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }));

  series->add_task(std::make_shared<MyGoTask>([] {
    printf("Step 3: Computing...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
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

  // wait for deconstructor to print(if any)
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  return 0;
}
