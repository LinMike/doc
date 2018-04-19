#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include "serial.h"
using namespace std;

int main()
{
	Serial* ptr_serial = new Serial();
	ptr_serial->OpenSerial("/dev/ttyUSB0", 19200);
	int cmd_basket_y_ = 0;
	while(cin >> cmd_basket_y_)
	{
		cmd_basket_y_ = cmd_basket_y_ * 10 -6;
		if(cmd_basket_y_ <= -285) cmd_basket_y_ = -285;
		else if(cmd_basket_y_ >= 295) cmd_basket_y_ = 295;
		int hex_y = (0xFFFF + 1 + cmd_basket_y_) % (0xFFFF + 1);
		unsigned char cmd_y[2] = {hex_y >> 8, hex_y & 0xFF};
		printf("%x, %x", cmd_y[0], cmd_y[1]);
		unsigned char data[12] = {0xee, 0xaa, 0x00, hex_y >> 8, hex_y & 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbb};
		ptr_serial->WriteSerial(data, sizeof(data));
//		cout << hex_y << ", " << cmd_y[0] << ", " << cmd_y[1] << endl;
	}
	return 0;
}
