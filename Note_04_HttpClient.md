# 学习记录

## 第四章 HttpClient

### 3.1 MyHttpClientTask

作为一个Task,首先保有了对MyTask的继承,使用事件驱动：  

+ connect: tcp三次握手
+ write: 发送http请求
+ read: 接收http响应
+ done: 触发回调

#### 3.1.1 http协议

+ url 统一资源定位符
  
  ```cpp
  http://www.example.com:8080/api/users?id=123#top    
    │     │               │     │          │         └─ fragment
    │     │               │     │          └─ query
    │     │               │     └─ path
    │     │               └─ port
    │     └─ host
    └─ scheme
  ```

  + scheme: http或者https 使用的协议
  + host: 服务器域名或者ip, dns会把域名解析为ip地址
  + port: 服务器端口号，http默认80, https默认443
  + path: 服务器上资源路径，可以是服务api
  + query: 查询参数 key/value pair
  + fragment: 页面锚点(前端 不发给服务器)

+ HTTP请求 HTTP Methods
  + GET: 获取资源
    幂等，每次请求相同。参数在url里，可缓存可收藏，但是也因此不安全
  + POST: 提交数据
    非幂等，浏览器不可缓存，不可收藏，数据请求在body里，因此可传大量数据
  + PUT: 更新资源
  + DELETE: 删除资源

#### 3.1.2 Http connect flow

```cpp
execute()
    ↓
getaddrinfo(host, port) → res (addrinfo 链表)
    ↓
socket(af, type, proto) → sockfd_
    ↓
fcntl(sockfd_, O_NONBLOCK)
    ↓
connect(sockfd_, addr) 
    ↓
   ┌───────────────────────┐
   ↓                       ↓
ret == 0             errno == EINPROGRESS
   ↓                       ↓
handle_connect()    poller 监听 sockfd_ 可写
                        ↓
                  连接成功 → handle_connect()
```
  