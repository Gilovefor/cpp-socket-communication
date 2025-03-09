#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../head/thread_pool.h" // 包含线程池头文件

// 客户端处理函数
void handle_client(int cfd, struct sockaddr_in caddr) {
    char ip[32];
    std::cout << "新客户端连接："
        << inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ip, sizeof(ip))
        << "，端口：" << ntohs(caddr.sin_port) << std::endl;

    // 通信
    while (true) {
        std::vector<char> buffer(1024, 0);
        int len = recv(cfd, buffer.data(), buffer.size(), 0);
        if (len > 0) {
            std::string received_data(buffer.data(), len);
            std::cout << "客户端 [" << ip << "] 说: " << received_data << std::endl;

            // 准备响应数据
            std::string response = "服务器已收到: " + received_data;
            send(cfd, response.c_str(), response.size(), 0);
        }
        else if (len == 0) {
            std::cout << "客户端 [" << ip << "] 已断开连接。" << std::endl;
            break;
        }
        else {
            perror("recv");
            break;
        }
    }

    close(cfd);
    std::cout << "客户端 [" << ip << "] 连接已关闭。" << std::endl;
}

// 主服务器函数
void start_server() {
    // 创建线程池
    Thread_Pool thread_pool(4); // 线程池大小设置为 4

    // 创建监听套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return;
    }

    // 绑定IP和端口
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
        perror("bind");
        return;
    }

    if (listen(fd, 2) == -1) {
        perror("listen");
        return;
    }

    std::cout << "服务器启动，等待客户端连接..." << std::endl;

    // 接受客户端连接
    while (true) {
        struct sockaddr_in caddr;
        socklen_t addrlen = sizeof(caddr);

        int cfd = accept(fd, (struct sockaddr*)&caddr, &addrlen);
        if (cfd == -1) {
            perror("accept");
            continue;
        }

        // 将任务提交到线程池中
        thread_pool.enqueue(handle_client, cfd, caddr);
    }

    close(fd);
}
