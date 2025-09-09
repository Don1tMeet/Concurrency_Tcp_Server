#include <string>
#include <algorithm>
#include <cstring>
#include <assert.h>
#include <iostream>

static const int FixedBufferSize = 4096;
static const int FixedLargeBufferSize = 4096 * 1000;
static const int kMaxNumericSize = 48;

template <int SIZE>
class FixedBuffer {
 public:
  FixedBuffer();
  // FixedBuffer(const FixedBuffer&) = delete;
  // FixedBuffer& operator=(const FixedBuffer&) = delete;
  // FixedBuffer(FixedBuffer&&) = delete;
  // FixedBuffer& operator=(FixedBuffer&&) = delete;
  ~FixedBuffer();

  void Append(const char *buf, int len);  // 在缓冲区末尾增加数据
  const char *Data() const;  // 返回缓冲区数据
  int Size() const;  // 返回缓冲区数据的大小

  char* Current();  // 返回缓冲区当前可写位置
  int Available() const;  // 返回缓冲区剩余可写空间大小
  void Add(int len);  // 增加cur_的位置

  void Reset();  // 重置缓冲区（cur_指向data_，即数据的开头）
  void Clear();  // 清空缓冲区
  const char* End() const;  // 返回缓冲区末尾位置

  void Bzero();  // 将data_的内容置零

 private:
  char data_[SIZE];  // 缓冲区数据 
  char* cur_;  // 当前可写位置
};


class LogStream {
 public:
  using Buffer = FixedBuffer<FixedBufferSize>;
  using self = LogStream;

  LogStream();
  LogStream(const LogStream&) = delete;
  LogStream& operator=(const LogStream&) = delete;
  LogStream(LogStream&&) = delete;
  LogStream& operator=(LogStream&&) = delete;
  ~LogStream();

  void Append(const char* data, int len);  // 向缓冲区末尾加入数据
  const Buffer& GetBuffer() const;  // 返回缓冲区
  void ResetBuffer();  // 重置缓冲区

  // 将浮点类型转换为字符串，并存入缓冲区
  self& operator<<(bool v);
  // 整形（内部调用void formatInteger(T); 函数）
  self& operator<<(short num);
  self& operator<<(unsigned short num);
  self& operator<<(int num);
  self& operator<<(unsigned int num);
  self& operator<<(long num);
  self& operator<<(unsigned long num);
  self& operator<<(long long num);
  self& operator<<(unsigned long long num);
  // 浮点
  self& operator<<(const float num);
  self& operator<<(const double num);
  // 原始字符
  self& operator<<(char c);
  // 原始字符串
  self& operator<<(const char* str);
  // std::string
  self& operator<<(const std::string &str);

 private:
  template <typename T>
  void formatInteger(T value);  // 格式化整形
  Buffer buffer_;  // 缓冲区
};

template<typename T>
void LogStream::formatInteger(T value) {
  if(buffer_.Available() >= kMaxNumericSize) {  // 缓冲区空间足够
    char *buf = buffer_.Current();
    char *cur = buf;

    // 依次填入value的每一位数（逆序填入）
    do {
      int remainder = value % 10;
      *(cur++) = remainder + '0';
      value /= 10;
    } while(value != 0);
    
    if(value < 0){
      *(cur++) = '-';
    }
    std::reverse(buf, cur);
    // buffer_的cur向后移动cur-buf位置，因为数据已经写入了，只需要移动cur即可
    buffer_.Add(cur - buf);
  }
}


class Fmt {
 public:
  template <typename T>
  Fmt(const char *fmt, T val);

  const char* Data() const { return buf_; }

  int Size() const { return size_; }

 private:
  char buf_[32];
  int size_;
};

template <typename T>
Fmt::Fmt(const char *fmt, T val) {
  // 判断是否是算数类型
  static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");
  // 将val以fmt的形式格式化
  size_ = snprintf(buf_, sizeof(buf_), fmt, val);
  assert(static_cast<size_t>(size_) < sizeof(buf_));
};

inline LogStream& operator<< (LogStream& s, const Fmt& fmt){
  s.Append(fmt.Data(), fmt.Size());
  return s;
}

// Explicit instantiations

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);


// FixedBuffer的定义，不支持分离编译
template <int SIZE>
FixedBuffer<SIZE>::FixedBuffer():cur_(data_){};

template <int SIZE>
FixedBuffer<SIZE>::~FixedBuffer(){};

template <int SIZE>
void FixedBuffer<SIZE>::Append(const char *buf, int len){
    if(Available() > len){
        memcpy(cur_, buf, len);
        cur_ += len;
    }
}
template <int SIZE>
const char *FixedBuffer<SIZE>::Data() const { return data_; }

template <int SIZE>
int FixedBuffer<SIZE>::Size() const {
  return static_cast<int>(cur_ - data_);
}

template <int SIZE>
char *FixedBuffer<SIZE>::Current() { return cur_; }

template <int SIZE>
int FixedBuffer<SIZE>::Available() const {
  return static_cast<int>(End() - cur_); 
}

template <int SIZE>
void FixedBuffer<SIZE>::Add(int len) { cur_ += len; }

template <int SIZE>
void FixedBuffer<SIZE>::Reset() { cur_ = data_; }

template <int SIZE>
void FixedBuffer<SIZE>::Clear() { memset(data_, 0, sizeof(data_)); }

template <int SIZE>
const char *FixedBuffer<SIZE>::End() const { return data_ + sizeof(data_); }

template <int SIZE>
void FixedBuffer<SIZE>::Bzero() {
    memset(data_, '\0', sizeof(data_));
    cur_ = data_;
}