#pragma once

#include <pthread.h>
#include "CurrentThread.h"

class MutexLock
{
public:
    MutexLock()
        : holder_(0)
    {
        pthread_mutex_init(&mutex_, NULL);
    }

    ~MutexLock()
    {

        pthread_mutex_destroy(&mutex_);
    }

    // must be called when locked, i.e. for assertion
    bool isLockedByThisThread() const
    {
        return holder_ == CurrentThread::tid();
    }

    void lock()
    {
        pthread_mutex_lock(&mutex_);
        assignHolder();
    }

    void unlock()
    {
        unassignHolder();
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t* getPthreadMutex()
    {
        return &mutex_;
    }

private:
    friend class Condition;

    class UnassignGuard // 放弃锁
    {
    public:
        explicit UnassignGuard(MutexLock &owner)
            : owner_(owner)
        {
            owner_.unassignHolder();
        }

        ~UnassignGuard()
        {
            owner_.assignHolder();
        }

    private:
        MutexLock &owner_;
    };

    void unassignHolder()
    {
        holder_ = 0;
    }

    void assignHolder()
    {
        holder_ = CurrentThread::tid();
    }

    pthread_mutex_t mutex_;
    pid_t holder_;
};

class MutexLockGuard
{
public:
    explicit MutexLockGuard(MutexLock &mutex)
        : mutex_(mutex)
    {
        mutex_.lock();
    }

    ~MutexLockGuard()
    {
        mutex_.unlock();
    }

private:
    MutexLock &mutex_;
};