/*
  main function for testing
  under development
*/
#include <iostream>
#include <chrono>
#include "MyScheduler.h"
#include "MyGoTask.h"

int main() {
  MyScheduler scheduler;

  std::cout << "Scheduling 10 tasks..." << '\n';

  // create 10 tasks
  for (int i = 0; i < 10; ++i) {
    MyTask* task = new MyGoTask([i] {
      std::cout << "Task " << i << " is running on thread "
                << std::this_thread::get_id() << std::endl;
      // simulate certain jobs
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    scheduler.schedule(task);
  }

  std::cout << "All tasks scheduled, main sleep for 2 seconds" << '\n';
  std::this_thread::sleep_for(std::chrono::seconds(2));

  std::cout << "Main thread finished" << '\n';
  return 0;
}
