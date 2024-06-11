#pragma once
#include <iostream> // debug
#include <string>
#include <vector>
#include <atomic>
#include <memory>

#include "LogStream.h"
#include "Thread.h"
#include "Mutex.h"
#include "CountDownLatch.h"

extern const int kLargeBuffer;
static const off_t kRollSize = 1 * 1024 * 1024;

class AsyncLogging
{
public:
  AsyncLogging(const std::string &basename,
               off_t rollSize = kRollSize,
               int flushInterval = 3);

  ~AsyncLogging()
  {
    if (running_)
    {
      stop();
    }
  }

  void append(const char *logline, int len);

  void start()
  {
    running_ = true;
    thread_.start();
    latch_.wait();
    std::cout << "日志子线程启动成功！" << std::endl;
  }

  void stop()
  {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

private:
  void threadFunc();

  using Buffer = FixedBuffer<kLargeBuffer>;
  using BufferVector = std::vector<std::unique_ptr<Buffer>>;
  using BufferPtr = BufferVector::value_type;

  const int flushInterval_;
  std::atomic<bool> running_;
  const std::string basename_;
  const off_t rollSize_;
  Thread thread_;
  CountDownLatch latch_;
  MutexLock mutex_;
  Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};