#include "TimeStamp.h"
#include "util.h"
#include <cstdio>
#include <sys/time.h>
#include <string>

// 微秒到秒的转换
const int kMicrosecond2Second = 1000000; 

TimeStamp::TimeStamp() : micro_seconds_(0) {}

TimeStamp::TimeStamp(int64_t micro_seconds) : micro_seconds_(micro_seconds) {}

TimeStamp::~TimeStamp() {}

bool TimeStamp::operator< (const TimeStamp& rhs) const {
  return micro_seconds_ < rhs.micro_seconds_;
}

bool TimeStamp::operator> (const TimeStamp& ths) const {
  return micro_seconds_ > ths.micro_seconds_;
}

bool TimeStamp::operator<= (const TimeStamp& ths) const {
  return micro_seconds_ <= ths.micro_seconds_;
}

bool TimeStamp::operator>= (const TimeStamp& ths) const {
  return micro_seconds_ >= ths.micro_seconds_;
}

bool TimeStamp::operator== (const TimeStamp& ths) const {
  return micro_seconds_ == ths.micro_seconds_;
}

std::string TimeStamp::ToDefaultLogString() const {
  char buf[64] = {0};
  time_t seconds = static_cast<time_t>(micro_seconds_ / kMicrosecond2Second);
  struct tm tm_time;
  Errif(localtime_r(&seconds, &tm_time) == nullptr, "localtime_r failed");
  
  snprintf(buf, sizeof(buf), "%4d%02d%02d_%02d%02d%02d",
           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  return buf;
}

std::string TimeStamp::ToString() const {
  char buf[64] = {0};
  int microseconds = static_cast<int>(micro_seconds_ % kMicrosecond2Second);
  time_t seconds = static_cast<time_t>(micro_seconds_ / kMicrosecond2Second);
  struct tm tm_time;
  Errif(localtime_r(&seconds, &tm_time) == nullptr, "localtime_r failed");
  
  snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
  return buf;
}

int64_t TimeStamp::GetMicroSeconds() const {
  return micro_seconds_;
}

// static function
TimeStamp TimeStamp::Now() {
  struct timeval time;
  Errif(gettimeofday(&time, nullptr) != 0, "gettimeofday failed");
  return TimeStamp(time.tv_sec * kMicrosecond2Second + time.tv_usec);
}

TimeStamp TimeStamp::AddTime(TimeStamp timestamp, double add_seconds) {
  int64_t add_micro_seconds = static_cast<int64_t>(add_seconds * kMicrosecond2Second);
  return TimeStamp(timestamp.GetMicroSeconds() + add_micro_seconds);
}