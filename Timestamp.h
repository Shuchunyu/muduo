#pragma once

#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondSinceEpoch); // explicit防止隐式转换 必须明确写出构造函数
    static Timestamp now();
    std::string toString() const;

private:
    int64_t microSecondSinceEpoch_;
};