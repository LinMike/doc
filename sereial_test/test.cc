#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "serial.h"
using namespace std;

int main()
{
	Serial* ptr_serial = new Serial();
	int fd = ptr_serial->OpenSerial("/dev/ttyUSB0", 19200);
	if(fd > 0)
	{
		printf("connect to serial port successed...\n");	
	}
	else {
		printf("connect to serial failed...\n");	
	}
	short cmd_basket_y_ = 0;
	int cnt = 0;
	while(1)//cin >> cmd_basket_y_
	{
		usleep(5 * 1000);
		cmd_basket_y_ = random() % 350;
		if(cmd_basket_y_ % 2 == 0) cmd_basket_y_ = -cmd_basket_y_;
//		if(cmd_basket_y_ <= -400) cmd_basket_y_ = -400;
//		else if(cmd_basket_y_ >= 400) cmd_basket_y_ = 400;
//		int hex_y = ~cmd_basket_y_+0x01;
//		if(hex_y != hex_y2) std::cout<<"hex_y = "<<hex_y<<", hex_y2 = "<<hex_y2<<std::endl;
		//assert(hex_y == hex_y2);
//		else{
		int hex_y = (0xFFFF + 1 + cmd_basket_y_) % (0xFFFF + 1);
		
		unsigned char cmd_y[2] = {hex_y >> 8, hex_y & 0xFF};
		printf("%x, %x\n", cmd_y[0], cmd_y[1]);

		unsigned char data[15] = {0xee, 0xaa, 0x04, hex_y >> 8, hex_y & 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbb};
		unsigned char data2[15] = {0xee, 0xaa, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbb};
		cnt++;
		if(cnt == 5)
		{
			ptr_serial->WriteSerial(data, sizeof(data));
			for(int i=0;i<sizeof(data);i++)
				printf("%x ",data[i]);
			printf("\n");
			cnt = 0;
		}
		else {
			ptr_serial->WriteSerial(data2, sizeof(data2));
			for(int i=0;i<sizeof(data2);i++)
				printf("%x ",data2[i]);
			printf("\n");
		}
	}
//}
	return 0;
}
