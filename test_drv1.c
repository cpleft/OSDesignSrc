#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define  DEVICE_NAME  "/dev/yy_drv_lab1"

int main()
{
	int fd;
	int ret;
	char* i;
	char buf[1024] = "Hello, I'm from user space.\n";
	printf("\nstart yy_drv test\n\n");
	
	fd = open(DEVICE_NAME, O_RDWR);  //调用打开设备函数
	printf("fd=%d\n", fd);	
	if (fd == -1)	
	{	
		printf("open device %s error\n", DEVICE_NAME);
	}	
	else
	{
		int i = 0;
		for (i = 0; i < 50; i++)
			printf("%c", buf[i]);
		printf("\n");

		write(fd, buf, 30);    //调用驱动程序的写操作接口函数
		read(fd, buf, 40);	//调用驱动程序的读操作接口函数

		for (i = 0; i < 50; i++)
			printf("%c", buf[i]);

		printf("\n");
		ioctl(fd);		//调用驱动程序的ioctl接口函数
		ret = close(fd);		//若ret的值为0，表示设备成功关闭
		printf("ret = %d\n", ret);	
		printf("close Demo_drive test\n");
	}
	return 0;
}
