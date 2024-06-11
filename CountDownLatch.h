#pragma once
#include "Mutex.h"
#include "Condition.h"

class CountDownLatch
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};