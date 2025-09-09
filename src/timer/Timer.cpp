#include "Timer.h"
#include "TimeStamp.h"
#include <functional>

Timer::Timer(TimeStamp timestamp, std::function<void()> const &cb, double interval)
    : expiration_(timestamp),
      callback_(cb),
      interval_(interval),
      repeat_(interval > 0.0) {}

Timer::~Timer() {}

void Timer::ReStart(TimeStamp now) {
  expiration_ = TimeStamp::AddTime(now, interval_);
}

void Timer::Run() const {
  if (callback_) {
    callback_();
  }
}

TimeStamp Timer::Expiration() const {
  return expiration_;
}

bool Timer::Repeat() const {
  return repeat_;
}