#include "HttpContext.h"
#include "HttpRequest.h"
#include <memory>

HttpContext::HttpContext() : state_(HttpRequestParaseState::START) {
  request_ = std::make_unique<HttpRequest>();
}

HttpContext::~HttpContext() {}

bool HttpContext::ParaseRequest(const char *begin, int len) {
  char *start = const_cast<char *>(begin);
  char *end = start;
  char *colon = end; // 对于URL:PARAMS 和 HeaderKey: HeaderValue保存中间的位置
  
  while(state_ != HttpRequestParaseState::kINVALID
  && state_ != HttpRequestParaseState::COMPLETE
  && end - begin < len) {
    char ch = *end;  // 当前字符

    switch(state_) {
      // 开始
      case HttpRequestParaseState::START: {
        if(ch == CR || ch == LF || isblank(ch)){  // 如果是空白符则跳过

        }
        else if(isupper(ch)) {  // 大写字母，则开始解析请求方法
          state_ = HttpRequestParaseState::METHOD;
        }
        else {  // 其他字符则无效
          state_ = HttpRequestParaseState::kINVALID;
        }
        break;
      }
      // 求情方法解析
      case HttpRequestParaseState::METHOD: {
        if(isupper(ch)) {  // 大写字母，则继续

        }
        else if(isblank(ch)) {  // 空格，说明METHOD解析完成
          request_->SetMethod(std::string(start, end));  // 设置请求方法
          state_ = HttpRequestParaseState::BEFORE_URL;  // 进入URL解析的前一个阶段
          start = end + 1;  // 更新开始位置
        }
        else {  // 其他字符则无效
          state_ = HttpRequestParaseState::kINVALID_METHOD;
        }
        break;
      }
      // URL解析的前一个阶段，即将进行URL解析
      case HttpRequestParaseState::BEFORE_URL: {
        if(ch == '/') {  // 遇到'/'，说明遇到了URL，开始解析URL
          state_ = HttpRequestParaseState::IN_URL;
        }
        else if(isblank(ch)) {  // 空格，跳过

        }
        else{  // 其他字符则无效
          state_ = HttpRequestParaseState::kINVALID_URL;
        }
        break;
      }
      // URL解析
      case HttpRequestParaseState::IN_URL: {
        if(ch == '?') {  // 遇到'?'，说明接下来是请求参数（首先解析参数key）
          request_->SetUrl(std::string(start, end));  // 设置请求路径
          start = end + 1;  // 更新开始位置
          state_ = HttpRequestParaseState::BEFORE_URL_PARAM_KEY;  // 进入URL参数key解析的前一个阶段
        }
        else if(isblank(ch)) {  // 空格，说明没有请求参数且URL解析完成
          request_->SetUrl(std::string(start, end));  // 设置请求路径
          start = end + 1;
          state_ = HttpRequestParaseState::BEFORE_PROTOCOL;  // 进入协议解析的前一个阶段
        }
        else {  // 其它字符为URL的内容，继续

        }
        break;
      }
      // URL参数键key析的前一个阶段，即将进行URL参数解析
      case HttpRequestParaseState::BEFORE_URL_PARAM_KEY: {
        if(ch == CR || ch == LF || isblank(ch)) {  // 参数解析中不能出现空白符
          state_ = HttpRequestParaseState::kINVALID;
        }
        else {
          state_ = HttpRequestParaseState::URL_PARAM_KEY;  // 进入URL参数key解析
        }
        break;
      }
      // URL参数key解析
      case HttpRequestParaseState::URL_PARAM_KEY: {
        if(ch == '=') {  // 遇到'='，说明接下来是参数value，key解析完毕
          colon = end;  // key和value的分割位置
          state_ = HttpRequestParaseState::BEFORE_URL_PARAM_VALUE;  // 进入URL参数value解析的前一个阶段
        }
        else if(isblank(ch)) {  // 不能出现空格
          state_ = HttpRequestParaseState::kINVALID;
        }
        else {  // 其它字符为key的内容，继续

        }
        break;
      }
      // URL参数value解析的前一个阶段，即将进行URL参数value解析
      case HttpRequestParaseState::BEFORE_URL_PARAM_VALUE: {
        if(ch == CR || ch == LF || isblank(ch)) {  // 参数解析中不能出现空白符
          state_ = HttpRequestParaseState::kINVALID;
        }
        else {
          state_ = HttpRequestParaseState::URL_PARAM_VALUE;  // 进入URL参数value解析
        }
        break;
      }
      // URL参数value解析
      case HttpRequestParaseState::URL_PARAM_VALUE: {
        if(ch == '&') {  // 遇到'&'，说明接下来是下一个参数，当前参数解析完毕
          request_->SetRequestParams(std::string(start, colon), std::string(colon + 1, end));
          start = end + 1;
          state_ = HttpRequestParaseState::BEFORE_URL_PARAM_KEY;  // 重新进入URL参数key解析的前一个阶段，解析下一个参数
        }
        else if(isblank(ch)) {  // 遇到空格，说明URL参数解析完毕
          request_->SetRequestParams(std::string(start, colon), std::string(colon + 1, end));
          start = end + 1;
          state_ = HttpRequestParaseState::BEFORE_PROTOCOL;  // 进入协议解析的前一个阶段
        }
        else {  // 其它字符为value的内容，继续

        }
        break;
      }
      // 协议解析的前一个阶段，即将进行协议解析
      case HttpRequestParaseState::BEFORE_PROTOCOL: {
        if(isblank(ch)) {  // 空格，跳过

        }
        else {  // 其它字符为协议的内容，进入协议解析
          state_ = HttpRequestParaseState::PROTOCOL;
        }
        break;
      }
      // 协议解析
      case HttpRequestParaseState::PROTOCOL: {
        if(ch == '/') {  // 遇到'/'，说明接下来是协议版本，协议解析完毕
          request_->SetProtocol(std::string(start, end));  // 设置协议类型
          start = end + 1;
          state_ = HttpRequestParaseState::BEFORE_VERSION;  // 进入版本解析的前一个阶段
        }
        else {  // 其它字符为协议的内容，继续

        }
        break;
      }
      // 版本解析的前一个阶段，即将进行版本解析
      case HttpRequestParaseState::BEFORE_VERSION: {
        if(isdigit(ch)) {  // 数字，说明是版本内容，进入版本解析
          state_ = HttpRequestParaseState::VERSION;
        }
        else {  // 其它字符无效
          state_ = HttpRequestParaseState::kINVALID;
        }
        break;
      }
      // 版本解析
      case HttpRequestParaseState::VERSION: {
        if(isdigit(ch) || ch == '.') {  // 数字和'.'为版本内容，继续

        }
        else if(ch == CR) {  // 遇到回车'\r'，说明版本解析完毕（也说明请求行结束），接下来进入请求头的解析
          request_->SetVersion(std::string(start, end));  // 设置版本
          start = end + 1;
          state_ = HttpRequestParaseState::WHEN_CR;  // 进入遇到回车的状态
        }
        else {  // 其它字符无效
          state_ = HttpRequestParaseState::kINVALID;
        }
        break;
      }
      // 遇到回车'\r'
      case HttpRequestParaseState::WHEN_CR: {
        if(ch == LF) {  // 遇到换行'\n'，说明该行结束
          start = end + 1;  // 进入下一行
          state_ = HttpRequestParaseState::CR_LF;  // 进入回车换行的状态
        }
        else {  // 其它字符无效
          state_ = HttpRequestParaseState::kINVALID;
        }
        break;
      }
      // 回车换行的状态（下一行的开始）
      case HttpRequestParaseState::CR_LF: {
        if(ch == CR) {  // 遇到回车'\r'，说明遇到了空行
          state_ = HttpRequestParaseState::CR_LF_CR;  // 进入空行的状态
        }
        else if(isblank(ch)) {  // 不能出现空格
          state_ = HttpRequestParaseState::kINVALID;
        }
        else {  // 其它字符为请求头的内容，进入请求头key解析
          state_ = HttpRequestParaseState::HEADER_KEY;
        }
        break;
      }
      // 请求头key解析
      case HttpRequestParaseState::HEADER_KEY: {
        if(ch == ':') {  // 遇到':'，说明接下来是请求头的value，key解析完毕
          colon = end;
          state_ = HttpRequestParaseState::HEADER_VALUE;  // 进入请求头value解析
        }
        else {  // 其它字符为请求头key的内容，继续

        }
        break;
      }
      // 请求头value解析
      case HttpRequestParaseState::HEADER_VALUE: {
        if(ch == CR) {  // 遇到回车'\r'，说明请求头解析完毕
          request_->AddHeader(std::string(start, colon), std::string(colon + 1, end));
          start = end + 1;
          state_ = HttpRequestParaseState::WHEN_CR;  // 进入遇到回车的状态
        }
        break;
      }
      // 空行的状态
      case HttpRequestParaseState::CR_LF_CR: {
        if(ch == LF) {  // 遇到换行'\n'，说明遇到空行，接下来解析请求体
          if(request_->GetHeaders().count("Content-Length") > 0) {
            // 如果请求头中设置了请求体的长度，则使用设置的长度
            if(atoi(request_->GetHeaderValue("Content-Length").c_str()) > 0) {
              state_ = HttpRequestParaseState::BODY;  // 进入请求体解析
            }
            else {  // 请求体长度为0，则直接完成请求
              state_ = HttpRequestParaseState::COMPLETE;
            }
          }
          else {
            // 请求头中没有设置请求体的长度，则使用传入的len
            if(end+1 - begin < len) {  // 传入的数据还没有用完，说明需要处理请求体
              state_ = HttpRequestParaseState::BODY;
            }
            else {  // 否则直接完成请求
              state_ = HttpRequestParaseState::COMPLETE;
            }
          }
          start = end + 1;
        }
        else {  // 其它字符无效
          state_ = HttpRequestParaseState::kINVALID;
        }
        break;
      }
      // 请求体解析
      case HttpRequestParaseState::BODY: {
        int body_len = len - (end - begin);  // 请求体的长度
        request_->SetBody(std::string(start, start + body_len));  // 设置请求体
        state_ = HttpRequestParaseState::COMPLETE;  // 完成请求解析
        break;
      }
      // 无效状态
      default: {
        state_ = HttpRequestParaseState::kINVALID;
        break;
      }
    }

    ++end;
  }

  return state_ == HttpRequestParaseState::COMPLETE;  // 返回请求是否完成
}

bool HttpContext::IsCompleteRequest() { return state_ == HttpRequestParaseState::COMPLETE; }

HttpRequest *HttpContext::GetRequest() { return request_.get(); }

void HttpContext::ResetContextStatus() { state_ = HttpRequestParaseState::START;}
