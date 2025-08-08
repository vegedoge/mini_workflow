# 学习记录

## 第七章 HTTPs + OpenSSL

TLS / SSL 处于应用层之下，传输层之上，进行加密  
为了支持https访问，我们引入OpenSSL.

### 7.1 HttpServer

让HttpServer持有一个全局SSL工厂（SSL\_CTX) 并且让每个连接的上下文持有自己的SSL会话对象。
