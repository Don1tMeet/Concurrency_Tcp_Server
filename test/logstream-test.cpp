#include "LogStream.h"
#include <iostream>
int main(){

    LogStream os;
    os << "hello world";
    std::cout << os.GetBuffer().Data() << std::endl;
    os.ResetBuffer();

    os << 11;
    std::cout << os.GetBuffer().Data() << std::endl;
    os.ResetBuffer();

    os << 0.1;
    std::cout << os.GetBuffer().Data() << std::endl;
    os.ResetBuffer();

    os << Fmt("%0.5f", 0.1);
    std::cout << os.GetBuffer().Data() << std::endl;
    os.ResetBuffer();

    return 0;
}