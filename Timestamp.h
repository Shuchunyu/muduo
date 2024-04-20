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
    int64_t microSecondsSinceEpoch() const { return microSecondSinceEpoch_; }
    // 1秒=1000*1000微妙
    static const int kMicroSecondsPerSecond = 1000 * 1000;
    
private:
    int64_t microSecondSinceEpoch_;
};

inline bool operator<(const Timestamp &lhs, const Timestamp& rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(const Timestamp& lhs, const Timestamp& rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

// 如果是重复定时任务就会对此时间戳进行增加。
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    // 将延时的秒数转换为微妙
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    // 返回新增时后的时间戳
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}