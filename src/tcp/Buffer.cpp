#include "Buffer.h"
#include <string>
#include <assert.h>
#include <cstring>

Buffer::Buffer() : buffer_(kInitalSize), read_index_(kPrePendIndex), write_index_(kPrePendIndex) {}

Buffer::~Buffer() {}

char* Buffer::Begin() { return &(*buffer_.begin()); }
const char* Buffer::Begin() const { return &(*buffer_.begin()); }

char* Buffer::BeginRead() { return Begin() + read_index_; }
const char* Buffer::BeginRead() const { return Begin() + read_index_; }

char* Buffer::BeginWrite() { return Begin() + write_index_; }
const char* Buffer::BeginWrite() const { return Begin() + write_index_; }

void Buffer::Append(const char* msg, int len) {
  EnsureWriteAbleBytes(len);  // 确保能够写入len字节数据
  std::copy(msg, msg + len, BeginWrite());
  write_index_ += len;
}

void Buffer::Append(const char* msg) {
  Append(msg, static_cast<int>(strlen(msg)));
}

void Buffer::Append(const std::string& msg) {
  Append(msg.data(), static_cast<int>(msg.size()));
}

int Buffer::PrependAbleBytes() const { return read_index_; }
int Buffer::ReadAbleBytes() const { return write_index_ - read_index_; }
int Buffer::WriteAbleBytes() const { return static_cast<int>(buffer_.size() - write_index_); }

char* Buffer::Peek() { return BeginRead(); }
const char* Buffer::Peek() const { return BeginRead(); }

std::string Buffer::PeekAsString(int len) {
  if(len >= ReadAbleBytes()){
    return PeekAllAsString();
  }
  return std::string(BeginRead(), BeginRead() + len);
}

std::string Buffer::PeekAllAsString() {
  return std::string(BeginRead(), BeginWrite());
}

void Buffer::Retrieve(int len) {
  assert(ReadAbleBytes() >= len);
  if(len + read_index_ < write_index_) {
    // 读取的内容不超过可读空间
    read_index_ += len;
  }
  else {
    // 超过，读取所有数据
    RetrieveAll();
  }
}

void Buffer::RetrieveAll() {
  // 全部读取数据时，可以直接从头开始写入数据
  write_index_ = kPrePendIndex;
  read_index_ = write_index_;
}

void Buffer::RetrieveUntil(const char* end) {
  assert(BeginWrite() >= end);  // 确保没有超出可读区域
  read_index_ += static_cast<int>(end - BeginRead());
}

std::string Buffer::RetrieveAsString(int len) {
  assert(ReadAbleBytes() >= len);

  std::string ret = std::move(PeekAsString(len));
  Retrieve(len);
  return ret;
}

std::string Buffer::RetrieveAllAsString() {
  assert(ReadAbleBytes() > 0);

  std::string ret = std::move(PeekAllAsString());
  RetrieveAll();
  return ret;
}

std::string Buffer::RetrieveUntilAsString(const char* end) {
  assert(BeginWrite() >= end);

  std::string ret = std::move(PeekAsString(static_cast<int>(end - BeginRead())));
  RetrieveUntil(end);
  return ret;
}

void Buffer::EnsureWriteAbleBytes(int len) {
  if(WriteAbleBytes() >= len){  // 剩余可写空间足够
    return;
  }
  // 因为读取数据后，read_index_会后移，而不会在访问这些数据，因此它们没有用了，可以将它们收回
  // 如果可读区域+前置区域大于等于 前置下标+len，说明将read_index_前移后（收回已经发生读取的空间）空间足够
  if(WriteAbleBytes() + PrependAbleBytes() >= kPrePendIndex + len) {
    // 将可读区域的数据挪到最前端
    std::copy(BeginRead(), BeginWrite(), Begin() + kPrePendIndex);
    read_index_ = kPrePendIndex;
    write_index_ = kPrePendIndex + ReadAbleBytes();
  }
  else {
    buffer_.resize(write_index_ + len);
  }
}
