使用c++11重构muduo网络库

### Channel类
封装文件描述符  
该文件描述符感兴趣的事件  
事件的回调   
每个Channel属于一个EventLoop  
Channel调用EventLoop的方法把自己更新到Poller上   

### Poller类
虚基类 提供方法管理要关注的Channel

### EPollPoller类
使用epoll实现的IO多路复用，封装管理的Channel的相关操作

### CurrentThread
返回当前线程的id，判断一个连接是否属于它的Loop