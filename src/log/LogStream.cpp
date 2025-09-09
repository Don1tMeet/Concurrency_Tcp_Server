#include "LogStream.h"
#include <cstring>
#include <cstdio>

LogStream::LogStream(){}
LogStream::~LogStream(){}

void LogStream::Append(const char *data, int len){
    buffer_.Append(data, len);
}

const LogStream::Buffer & LogStream::GetBuffer() const { return buffer_; }

void LogStream::ResetBuffer() {
    buffer_.Clear();
    buffer_.Reset();
}


LogStream& LogStream:: operator<<(bool v){
    buffer_.Append(v ? "1" : "0", 1);
    return *this;
}
// 整形数据的字符串转换、保存到缓冲区； 内部均调用 void formatInteger(T); 
LogStream& LogStream:: operator<<(short num){
    return (*this) << static_cast<int>(num);
}
LogStream& LogStream:: operator<<(unsigned short num){
    return (*this) << static_cast<unsigned int>(num);
}
LogStream& LogStream:: operator<<(int num){
    formatInteger(num);
    return *this;
}
LogStream& LogStream:: operator<<(unsigned int num){
    formatInteger(num);
    return *this;
}
LogStream& LogStream:: operator<<(long num){
    formatInteger(num);
    return *this;
}
LogStream& LogStream:: operator<<(unsigned long num){
    formatInteger(num);
    return *this;
}
LogStream& LogStream:: operator<<(long long num){
    formatInteger(num);
    return *this;
}
LogStream& LogStream:: operator<<(unsigned long long num){
    formatInteger(num);
    return *this;
}

// 浮点类型数据转换成字符串
LogStream& LogStream:: operator<<(const float num){
    return (*this) << static_cast<const double>(num);
}

LogStream& LogStream:: operator<<(const double num){
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%g", num);
    buffer_.Append(buf, len);
    return *this;
}

LogStream& LogStream:: operator<<(char v){
    buffer_.Append(&v, 1);
    return *this;
}

// 原生字符串输出到缓冲区
LogStream& LogStream:: operator<<(const char* str){
    if (str) {
      buffer_.Append(str, strlen(str)); 
    }
    else {
      buffer_.Append("(null)", 6);
    }

    return *this;
}

// 标准字符串std::string输出到缓冲区
LogStream& LogStream:: operator<<(const std::string& v){
    buffer_.Append(v.c_str(), v.size());
    return *this;
}
