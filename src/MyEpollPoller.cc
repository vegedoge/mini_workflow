#include "MyEpollPoller.h"
#include "MyGoTask.h"
#include <unistd.h> // for closing 
#include <iostream>

MyEpollPoller::MyEpollPoller(MyScheduler* scheduler) 
  : scheduler_(scheduler), stop_flag_(false) {

  // 和 epoll_create()的区别是：epoll_create1()可以指定创建的标志位
  // 这里 0 表示没有特殊标志
  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    perror("epoll_create1");
    exit(EXIT_FAILURE);
  }

  // bind thread
  poller_thread_ = std::thread(&MyEpollPoller::poller_loop, this);
}

MyEpollPoller::~MyEpollPoller() {
  printf("[Poller] Destructor called. Stopping poller...\n");
  stop_flag_.store(true);
  printf("[Poller] Joining poller thread...\n");
  poller_thread_.join();
  printf("[Poller] Poller thread joined. Destructor finished...\n");
  close(epoll_fd_);
}

bool MyEpollPoller::add(int fd, 
                        std::function<void()> read_cb, 
                        std::function<void()> write_cb, 
                        bool is_oneshot) {
  
  // 检查是否已存在
  bool exists = false;

  {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    if (handlers_.find(fd) != handlers_.end())
    {
      exists = true;
      // 已存在，改为 MOD
      std::cout << "Already exists add, use mod" << std::endl;
    }
  }

  if (exists) {
    return mod(fd, std::move(read_cb), std::move(write_cb), is_oneshot);
  }

  // epoll_event: 事件结构体
  epoll_event ev;
  ev.data.fd = fd;

  // 位操作
  ev.events = EPOLLIN | EPOLLOUT | EPOLLET;   // 监听读写 ET:边缘触发
  
  if (is_oneshot) {
    ev.events |= EPOLLONESHOT;
  }

  // epoll_ctl: 这里用来添加或修改事件
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    perror("epoll_ctl: add");
    return false;
  }

  std::lock_guard<std::mutex> lock(handlers_mutex_);
  // modify fd
  handlers_[fd] = {fd, std::move(read_cb), std::move(write_cb)};
  std::cout << "Added fd: " << fd << " to epoll" << std::endl;

  return true;
}

bool MyEpollPoller::mod(int fd,
                        std::function<void()> read_cb,
                        std::function<void()> write_cb,
                        bool is_oneshot ) {
  struct epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLIN | EPOLLOUT | EPOLLET;    // 保持读写事件监听
  if (is_oneshot) {
    ev.events |= EPOLLONESHOT;
  }

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
    perror("epoll_ctl: mod");
    return false;
  }

  std::lock_guard<std::mutex> lock(handlers_mutex_);
  auto it = handlers_.find(fd);
  if (it != handlers_.end()) {
    it->second.read_handler = std::move(read_cb);
    it->second.write_handler = std::move(write_cb);
  }
  std::cout << "Modified fd:" << fd << std::endl;
  return true;
}

bool MyEpollPoller::del(int fd) {
  std::lock_guard<std::mutex> lock(handlers_mutex_);
  
  // 检查文件描述符是否已经存在
  auto it = handlers_.find(fd);
  if (it == handlers_.end()) {
    // 文件描述符已经被删除，避免重复删除
    return true;
  }
  
  // 从 epoll 中删除
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    perror("epoll_ctl: del");
  }

  // 从 handlers_ 中删除
  handlers_.erase(fd);

  return true;
}

void MyEpollPoller::poller_loop() {
  const int MAX_EVENTS = 128;
  std::vector<epoll_event> events(MAX_EVENTS); // buffer, storing triggered events

  while(!stop_flag_.load()) {
    // n is the number of trigered events
    int n = epoll_wait(epoll_fd_, events.data(), MAX_EVENTS, 1000); // wait 1s
    if (n == -1) {
      perror("epoll_wait");
      continue;
    }

    for (int i = 0; i < n; ++i) {
      // get fd of each triggered event
      int fd = events[i].data.fd;
      std::function<void()> handler_func;

      {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        auto it = handlers_.find(fd);

        // fail to find
        if (it == handlers_.end()) {
          continue;
        }

              // readable or writable event
      if (events[i].events & EPOLLIN) {
        handler_func = it->second.read_handler;
        std::cout << "EPOLLIN event triggered for fd: " << fd << std::endl;
      } else if (events[i].events & EPOLLOUT) {
        handler_func = it->second.write_handler;
        std::cout << "EPOLLOUT event triggered for fd: " << fd << std::endl;
      }
      }

      if (handler_func) {
        // wrap func into MyGoTask and pass to scheduler
        scheduler_->schedule(std::make_shared<MyGoTask>(std::move(handler_func)));
      }
    }
  }
}
