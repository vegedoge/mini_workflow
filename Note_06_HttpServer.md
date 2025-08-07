# 学习记录

## 第六章 HttpServer

### 6.1 MyHttpServer

利用之前的scheduler poller SeriesWork,创建一个Http服务器，把外部请求动态创建工作流来处理。

#### 6.1.1 定义Request和Response

将Http请求和响应抽象为struct

#### 6.1.2 HttpContext

用一个Context结构体，让所有任务共享上下文，避免全局变量。

+ 具体结构

```cpp
struct HttpContext {
  int fd; 
  HttpRequest request;
  HttpResponse response;
  std::string request_buffer;   // 累积读取的数据
};
```

用一个buffer来作为流信息的缓冲区，实现： 循环读->累积buffer->检查分隔符->解析->完成 的标准流程.

+ “\r\n\r\n”是Http header和body之间的分隔符

```cpp
    请求行 + 头部字段
    \r\n
    字段名: 字段值
    \r\n
    字段名: 字段值
    \r\n
    \r\n          ← 这里就是 "\r\n\r\n"
    正文内容...
```

```cpp
    POST /api/login HTTP/1.1
    Host: example.com
    Content-Type: application/json
    Content-Length: 16
    
    {"user":"admin"}
```

#### 6.1.3 HttpRequest解析

用内部任务HttpReadTask实现。
相比于客户端的parse\_url, 作为服务端的parse\_request要复杂很多。因为url实际就是解析一个地址，但是requeset要拆解完整的http请求。

典型Http请求： 要拆出不同的部分

```cpp
GET /search?q=workflow HTTP/1.1
Host: example.com
User-Agent: curl/7.68.0
Accept: */*
Connection: keep-alive

(optional body) 
----------
<method> <path> <version>\r\n
<header>: <value>\r\n
... (更多 headers)
\r\n
[body]
```

+ 用istringstream流，因为它只支持读取，继承istream 内存效率高.
+ 目前仅支持最简单的GET

#### 6.1.4 HttpResponse

用内部任务HttpWriteTask实现。
主要用于发送Http的响应。

#### 6.1.5 整体流程

```cpp
Client → TCP 连接 → accept() → 创建 SeriesWork
                                  ↓
                          HttpReadTask (异步读)
                                  ↓
                       用户业务逻辑（process_callback）
                                  ↓
                         HttpWriteTask (异步写)
                                  ↓
                               连接关闭
```

+ start函数： 启动服务器
  1. 创建监听socket
  2. 设置端口复用setsockopt(..reuse..)
  3. 绑定端口bind(listen_fd_, port)
  4. 开始监听listen()
  5. 注册事件回调poller->add

+ handle\_accept函数： 客户端连接
  1. 循环accept所有新连接
  2. 为每个连接创建服务流程 context,记录fd request和respose
  3. 创建服务流水线SeriesWork
  4. 在流水线添加request->func->response流程
  5. 调度执行
