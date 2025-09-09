# Concurrency_Tcp_Server

参考：https://github.com/yuesong-feng/30dayMakeCppServer

实现高性能TCP通用服务器

// 在linux下
// cd到build目录，执行以下命令
```
cmake ..
make
```

此时，build/bin下会有两个可执行文件：http-client, http-server

// 在终端执行
```
./bin/http-server
```

// 再开一个或多个终端执行
```
./bin/http-client
```

在http-client的终端输入数据，数据传到http-server，并在http-client回显
