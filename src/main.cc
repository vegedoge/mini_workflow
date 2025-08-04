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

int main() {
  MyScheduler scheduler;
  MyWaitGroup wg(1); //等待一个任务，（我们的Series）完成
  MyEpollPoller poller(&scheduler);

  // creat an http client task
  auto http_task = std::make_shared<MyHttpClientTask>(&scheduler, &poller, "http://httpbin.org/get");

  // an computing task for http
  auto process_task = std::make_shared<MyGoTask>([http_task] {
    printf("Http request finished. Response size: %zu\n", http_task->get_response().length());
    std::string resp_header = http_task->get_response().substr(0, 256);
    printf("Response header: \n%s\n...\n", resp_header.c_str());
  });

  // serires
  auto series = std::make_shared<MySeriesWork>(&scheduler);
  series->add_task(http_task);
  series->add_task(process_task);

  series->set_series_callback([&wg](MySeriesWork* s) {
    printf("Whole seires finished.\n");
    wg.done();
  });

  printf("Starting http request test...\n");
  scheduler.schedule(series);

  wg.wait();
  printf("Main thread finished.\n");
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  return 0;
}
