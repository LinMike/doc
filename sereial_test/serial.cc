#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <sstream>
#include "serial.h"
#include <iostream>
//#include "share/core/service_configuration.h"

using namespace std;

static int name_arr[] = { 115200, 38400,  19200,  9600,  4800,  2400,  1200,  300};

static int speed_arr[] = { B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};

Serial::Serial() {
	fd_ = -1;
//	logger_ = share::core::ServiceConfiguration::Instance()->GetService<share::log::Logger>();
}

int Serial::OpenSerial(const char* devfile, int baud)
{
	std::ostringstream oss;
	cout<<"start to open serial dev = "<<devfile << ",baud = "<<baud<<endl;
	fd_ = open (devfile, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
	if (fd_ < 0)
	{
		oss << "error " << errno << " opening "<< devfile;
//		logger_->Error(oss.str());
		std::cout<<oss.str()<<std::endl;
		return -1;
	}
	else
	{
		oss.str("");
		oss << "success opening " << devfile;
		std::cout<<oss.str()<<std::endl;
//		logger_->Info(oss.str());
	}
	SetInterfaceAttribs (baud, 0);  // set speed to 115,200 bps, 8n1 (no parity)
//	set_blocking (0);                // set no blocking
	return fd_;
}


int Serial::WriteSerial(unsigned char* data, int len)
{
	int sz = (int)write(fd_, data, len);

	tcflush(fd_, TCIOFLUSH);
    return sz;
}

int Serial::ReadSerial(unsigned char* buffer, int sz)
{
    return read(fd_, buffer, sz);
}

/*************************** Private methods **************************************/
int Serial::SetInterfaceAttribs (int speed, int parity)
{
	std::ostringstream oss;

    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd_, &tty) != 0)
    {
    	oss << "error " << errno << " from tcgetattr";
	std::cout<<oss.str()<<std::endl;
//    	logger_->Error(oss.str());
        return -1;
    }

	for (unsigned int i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) 
	{ 
		if  (speed == name_arr[i]) 
		{     
    		cfsetospeed (&tty, speed_arr[i]);
	        cfsetispeed (&tty, speed_arr[i]);
	        break;
	    }
	}

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd_, TCSANOW, &tty) != 0)
    {
    	oss.str("");
    	oss << "error " << errno << " from tcsetattr";
	std::cout<<oss.str()<<std::endl;
    	//logger_->Error(oss.str());
        return -1;
    }
    return 0;
}

void Serial::SetBlocking (int should_block)
{
	std::ostringstream oss;

    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd_, &tty) != 0)
    {
    	oss << "error " << errno << " from tggetattr";
    	//logger_->Error(oss.str());
	std::cout<<oss.str()<<std::endl;
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd_, TCSANOW, &tty) != 0)
    {
    	oss.str("");
    	oss << "error " << errno <<" setting term attributes";
    	//logger_->Error(oss.str());
	std::cout<<oss.str()<<std::endl;
        return;
    }
}

