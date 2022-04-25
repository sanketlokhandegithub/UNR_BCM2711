#pragma once
//#ifndef __UNR_BCM2711_I2CHANDLE_H__
//#define __UNR_BCM2711_I2CHANDLE_H__

#include <iostream>
#include <linux/i2c-dev.h>
#include <exception>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <inttypes.h>
#include <cerrno>         // errno
#include <system_error>   // std::error_code, std::generic_category
// std::error_condition
//# define DEBUG 1

constexpr unsigned int RPI4_I2C_INSTANCE0 = 0;
constexpr unsigned int RPI4_I2C_INSTANCE1 = 1;
constexpr char RPI4_I2C_DEV_INSTANCE0[] = "/dev/i2c-0";  // do not use this for raspberry pi 4 3 3b+ zero and zero w
constexpr char RPI4_I2C_DEV_INSTANCE1[] = "/dev/i2c-1";
constexpr signed int I2OCTRL_FAIL = -1;
constexpr unsigned int UNR_I2C_MAX_BYTES = 255;

class UNR_I2CHandle {
private:
	int m_intFile_descriptor;
	unsigned char m_ucDecive_Address;
	unsigned char m_tempBuffer[UNR_I2C_MAX_BYTES];
	void init_file_descriptor(const char _charFD[]) noexcept(false);
	void set_device_mode(unsigned int _u1Mode) const noexcept(false);
	
#ifdef DEBUG
	ssize_t m_s4Return_in;
	ssize_t m_s4Return_out;
#endif
public:
	UNR_I2CHandle(unsigned char _instance, unsigned char _dev_address, unsigned short int _u1Mode) noexcept(false);
	~UNR_I2CHandle();
	UNR_I2CHandle() = delete;
	UNR_I2CHandle(const UNR_I2CHandle&) = delete;
	UNR_I2CHandle(const UNR_I2CHandle&&) = delete;
	UNR_I2CHandle & operator=(UNR_I2CHandle&) = delete;
//protected:
	int i2c_write_simple(unsigned char& _buffer, const unsigned short& numByte) noexcept(false);
	int i2c_read_simple(unsigned char& buffer, const unsigned short& numBytes) noexcept(false);
	int i2c_writeReg(unsigned char& buffer, unsigned char& register_address  , const unsigned short& numBytes   ) noexcept(false);
	int i2c_readReg(unsigned char& buffer, unsigned char& register_address  , const unsigned short& numBytes   ) noexcept(false);

};

//#endif // UNR_BCM2711_I2CHANDLE_H_


