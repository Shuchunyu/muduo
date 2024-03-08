使用c++11重构muduo网络库

### Channel类
封装文件描述符
该文件描述符感兴趣的事件
事件的回调
每个Channel属于一个EventLoop
Channel调用EventLoop的方法把自己更新到Poller上