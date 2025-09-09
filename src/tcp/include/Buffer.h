#pragma once

#include <cstring>
#include <memory>
#include <vector>
#include <string>

static const int kPrePendIndex = 8;  // prependindex长度(前置区域的大小)
static const int kInitalSize = 1024;  // 初始化缓冲区长度

class Buffer {
 public:
  Buffer();
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  Buffer(Buffer&&) = default;
  Buffer& operator=(Buffer&&) = default;
  ~Buffer();

  // buffer_的起始位置
  char* Begin();
  // begin的const版本，这样const对象返回的数据不能修改
  const char* Begin() const;

  // 可取区域的起始位置
  char* BeginRead();
  const char* BeginRead() const;

  // 可写区域的起始地址
  char* BeginWrite();
  const char* BeginWrite() const;

  // 添加数据
  void Append(const char *message, int len);
  void Append(const char *message);
  void Append(const std::string &message);
  
  // 获取前置区域，可读区域，可写区域大小
  int PrependAbleBytes() const;
  int ReadAbleBytes() const;
  int WriteAbleBytes() const;

  // 查看数据，但不修改read_index_
  char* Peek();
  const char* Peek() const;
  std::string PeekAsString(int len);  // 以string方式查看
  std::string PeekAllAsString();  // 以string查看所有数据

  // 取数据，会更新read_index_，也就是不能重复取
  // 定长取
  void Retrieve(int len);
  std::string RetrieveAsString(int len);
  // 全部
  void RetrieveAll();
  std::string RetrieveAllAsString();
  // 某个索引之前
  void RetrieveUntil(const char* end);
  std::string RetrieveUntilAsString(const char* end);

  // 查看空间
  // 查看是否能否写下len的空间
  // 如果能，什么都不做
  // 如果不能，先判断，收回已经读取的数据够不够
  // 如果够，则收回已经被读取的数据（即将所有数据先前移）
  // 如果不够则申请空间
  void EnsureWriteAbleBytes(int len);

 private:
  std::vector<char> buffer_;
  int read_index_;  // 可读区域的起始位置（已经写入数据）
  int write_index_;  // 可写区域的其实位置（未写入数据）
};
