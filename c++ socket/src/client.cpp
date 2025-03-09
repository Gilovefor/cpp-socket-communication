#include <iostream>
#include <string>
#include<vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <format>

void start_client() {
	//1、创建通信的套接字
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("socket");
		return;
	}

	//2、连接服务器的IP，port
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(9999);	//将主机字节序的字符串转化为网络字节序（小端）的IP地址
	inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr.s_addr);
	int ret = connect(fd, (struct sockaddr*)&saddr, sizeof(saddr));
	if (ret == -1) {
		perror("connect");
		return;
	}

	//3、通信
	int number = 0;
	while (1) {
		// 发送数据
		std::string message = std::format("你好，hello，world，{}...\n", number++);
		send(fd, message.c_str(), message.size() + 1, 0);  // 注意：发送末尾的 '\0'

		sleep(1);

		// 接收数据
		std::vector<char> buffer(1024, 0);  // 使用动态缓冲区
		int len = recv(fd, buffer.data(), buffer.size(), 0);
		if (len > 0) {
			std::string received_data(buffer.data(), len);  // 转换为 std::string
			std::cout << "Received: " << received_data << std::endl;
		}
		else if (len == 0) {
			std::cout << "服务器已经断开了连接。。。" << std::endl;
			break;
		}
		else {
			perror("recv");
			break;
		}
	}

	//关闭文件描述符
	close(fd);
}