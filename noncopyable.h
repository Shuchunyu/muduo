#pragma once
/* 
    noncopyable被继承后，子类不可以进行赋值和拷贝构造，
    但是可以执行默认的构造和析构函数
*/

class noncopyable
{
public:
    noncopyable (const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};