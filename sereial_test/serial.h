#ifndef SERIAL_H
#define SERIAL_H

#include <boost/shared_ptr.hpp>

//#include "share/log/logger.h"

class Serial
{
private:
	int fd_;
//	boost::shared_ptr<share::log::Logger> logger_;

	int SetInterfaceAttribs (int speed, int parity);
	void SetBlocking (int should_block);

public:
	Serial();
	int OpenSerial(const char *devFile, int baud);
	int WriteSerial(unsigned char* data, int len);
	int ReadSerial(unsigned char* buffer, int sz);
};

#endif //SERIAL_H
