#include <iostream>
#include <memory>
#include <csignal>
#include <chrono>
#include "MyScheduler.h"
#include "MyEpollPoller.h"
#include "MyHttpServer.h"

static bool running = true;
void sig_handler(int sig) {
  std::cout << "Received shutdown signal" << std::endl;
  running = false;
}

int main() {
  // set signal, for closing server
  // 讲讲中断
  signal(SIGINT, sig_handler);

  // initialize
  MyScheduler scheduler;
  MyEpollPoller poller(&scheduler);
  MyHttpServer server(&scheduler, &poller);

  // work logic
  auto my_logic = [](const HttpRequest &req, HttpResponse &resp)
  {
    std::cout << "Handling request: " << req.method << " " << req.path << std::endl;

    if (req.path == "/hello")
    {
      resp.body = "<h1>Hello, mini-workflow!</h1>";
      resp.headers["Content-Type"] = "text/html";
    }
    else if (req.path == "/time")
    {
      auto now = std::chrono::system_clock::now();
      auto a_time = std::chrono::system_clock::to_time_t(now);
      resp.body = std::ctime(&a_time);
    }
    else
    {
      resp.status_code = 404;
      resp.status_message = "Not Found";
      resp.body = "404 Not Found";
    }
  };

  // start
  server.start(8080, my_logic);

  while(running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  server.stop();
  printf("Server shutdown.\n");

  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  return 0;
}