/* Author: Sanket L. (slokhande@unr.edu)

	Copyright (C) 2022  Sanket Lokhande
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
*
*  File Description: This driver file is intended to interface the BCM2711 I2C with POSIX / *Unix sytems.
*					Make sure the /dev/i2c-X (X=0 or 1) file is present in the system
*					Do not use i2c-0 
* Unauthorized Distrubution is strictly prohibited.
* Rev 1: I2C support added with simple read/ write and Register Read /write options. 
* Rev 2: Took the buffer dependency out of the system for write/read operations. 
*		 Reading process for I2C did not yield good results for more than 70 odd bytes at a time. One can read in blocks of 50bytes at a time for now.
* Rev 3: Reverting back on the I2C databuffers on write operation. Write does now work without the buffers.
*/


// Basic Includes
#include "UNR_BCM2711_I2CHandle.h"

/*
* This function allows opening of I2C port from kernel space to user space.
* Check if the file /dev/i2c-X is present in your device. 
* If not then use the "sudo raspi-config" command and set the correct setting to turn on the I2C peripheral device.
* Input to the function is the instance of I2C device
*/
void UNR_I2CHandle::init_file_descriptor(const char _charFD[]) noexcept(false)
{

	m_intFile_descriptor = open(_charFD, O_RDWR);
	if (m_intFile_descriptor < 0)
	{
		throw std::runtime_error(std::string("Error Opening I2C Port"));
	}

}

/* 
* This function sets the mode for I2C peripheral device. 
* input mode include : I2C_SLAVE, I2C_SLAVE_FORCE
* outputs an error message if error is thrown by IOCTL function
*/
void UNR_I2CHandle::set_device_mode(unsigned int _u1Mode) const noexcept(false)
{
	int _s4Return = 0;
	_s4Return = ioctl(m_intFile_descriptor, _u1Mode, m_ucDecive_Address);
	if (_s4Return == I2OCTRL_FAIL)
	{
		throw std::runtime_error(std::string("Error Setting Device Mode"));
	}
}

/*
* Explicit constructor which takes in the I2C peripheral instance (either 0 or 1), the device address which is usually given in the datasheet, and mode of operation usually I2C_SLAVE.
* Outputs error message if anything goes wrong in initializing. 
*/
UNR_I2CHandle::UNR_I2CHandle(unsigned char _instance,
							unsigned char _dev_address,
							unsigned short int _u1Mode) noexcept(false) : m_intFile_descriptor(0)
												      , m_ucDecive_Address(_dev_address)
{
	memset((void*)m_tempBuffer, 0x00, UNR_I2C_MAX_BYTES);

	switch (_instance)
	{
	case 0:
	{
		try {
			init_file_descriptor(RPI4_I2C_DEV_INSTANCE0);
		}
		catch (std::exception& e)
		{
			throw(e.what());
		}
		break;
	}
	case 1:
	{
		try {
			init_file_descriptor(RPI4_I2C_DEV_INSTANCE1);
		}
		catch (std::exception& e)
		{
			throw(e.what());
		}
		break;
	}
	default:
	{
		try {
			init_file_descriptor(RPI4_I2C_DEV_INSTANCE1);
		}
		catch (std::exception& e)
		{
			throw(e.what());
		}
		break;
	}
	}
	try {
		set_device_mode(_u1Mode);
	}
	catch (std::exception& e)
	{
		throw(e.what());
	}
#ifdef DEBUG
	m_s4Return_in = 0;
	m_s4Return_out = 0;
#endif
}

/*
* Write function for traditional I2C protocol writes which are directed towards a particular register
* Inputs are 1. Starting address of the buffer to write, 2. Register address to write, 3. Number of bytes to write
* Output: In debug mode, the errors will be thrown on screen with a return value of -1, 
* in non debug mode,  returns -1 for failed write transactions and returns number of bytes for successful writes 
*/
int UNR_I2CHandle::i2c_writeReg(unsigned char& _buffer, unsigned char& register_address, const unsigned short& numBytes) noexcept(false)
{
	// the write buffer needs to be preloaded with the register address.
	memset((void *)m_tempBuffer, 0x00, UNR_I2C_MAX_BYTES);
	m_tempBuffer[0] = register_address;
	memcpy((void *)&m_tempBuffer[1], (const void*)&_buffer, numBytes);
	
#ifdef DEBUG
	m_s4Return_in = write(m_intFile_descriptor, (void*)(&m_tempBuffer), static_cast<size_t>(numBytes+1));
	if (m_s4Return_in < 0)
	{
		std::error_code ec(errno, std::generic_category());
		std::error_condition ok;
		std::string msg = ec.message();
		if (ec != ok) puts(ec.message().c_str());
		throw std::runtime_error(std::string("I2C Could not handle write Operation"));
	}
	return (m_s4Return_in-1);
#else	
	return write(m_intFile_descriptor, (void*)(&m_tempBuffer), static_cast<size_t>(numBytes + 1)) == -1 ? -1 : numBytes;
#endif
}


/*
* Read function for traditional I2C protocol reads which are directed from a particular register
* Inputs are 1. Starting address of the empty buffer to read, 2. Register address to read from, 3. Number of bytes to read
* Output: In debug mode, the errors will be thrown on screen with a return value of -1,
* in non debug mode,  returns -1 for failed transactions and returns number of bytes for successful writes
*/
int UNR_I2CHandle::i2c_readReg(unsigned char& buffer, unsigned char& register_address, const unsigned short& numBytes) noexcept(false) 
{
#ifdef DEBUG
	m_s4Return_in = write(m_intFile_descriptor, (const void*)&register_address, 1U);
	if (m_s4Return_in < 0)
	{
		std::error_code ec(errno, std::generic_category());
		std::error_condition ok;
		std::string msg = ec.message();
		if (ec != ok) puts(ec.message().c_str());
		throw std::runtime_error(std::string("I2C Could not handle write Operation"));
	}
	return m_s4Return_in;
	m_s4Return_in = write(m_intFile_descriptor, (const void*)&register_address, 1U);  // this line needs to be here so that the debug process can happen on this read function
	m_s4Return_in = read(m_intFile_descriptor, (void*)(&buffer), static_cast<size_t>(numBytes));
	if (m_s4Return_in < 0)
	{
		std::error_code ec(errno, std::generic_category());
		std::error_condition ok;
		std::string msg = ec.message();
		if (ec != ok) puts(ec.message().c_str());
		throw std::runtime_error(std::string("I2C Could not handle Read Operation"));
	}
	return m_s4Return_in;
#else
	/*if (write(m_intFile_descriptor, (const void*)( & register_address ), 1U) < 0)
		return -1;
	return read(m_intFile_descriptor, (void*)(&buffer), static_cast<size_t>(numBytes)) == -1 ? -1 : numBytes;*/

	if (write(m_intFile_descriptor, (const void*)(&register_address), 1U) > 0)
		return read(m_intFile_descriptor, (void*)(&buffer), static_cast<size_t>(numBytes));
	else
		return -1;

#endif
}

/*
* ****** IMP ******** Not Sure this will work for I2C ics which have specific register read / writes. Can be used for DIGI pots I guess
* This function is a simple write function to write to slave devices which does not have specific registers to write.
* First Input to the function is reference to first byte of the buffer address to write, second Input is the number of bytes to write.
* Output of this function in debug mode is the Error as returned by "write" function. And in non debug mode, it returns a -1 on unsuccessful writes, and
*/
int UNR_I2CHandle::i2c_write_simple(unsigned char& _buffer, const unsigned short& numBytes) noexcept(false)
{
#ifdef DEBUG
	m_s4Return_out = write(m_intFile_descriptor, (void*)(&_buffer), static_cast<size_t>(numBytes));
	if (m_s4Return_out < 0)
	{
		std::error_code ec(errno, std::generic_category());
		std::error_condition ok;
		std::string msg = ec.message();
		if (ec != ok) puts(ec.message().c_str());
		throw std::runtime_error(std::string("I2C Could not handle write Operation"));
	}
	return m_s4Return_out;
#else
	return write(m_intFile_descriptor, (void*)(&_buffer), static_cast<size_t>(numBytes)) == -1 ? -1 : numBytes;
#endif
}

/*
* ****** IMP ******** Not Sure this will work for I2C ics which have specific register read / writes. Can be used for DIGI pots I guess
* Simple read function follows similar instruction as simple write function.
* input : starting address / pointer of an empty buffer. if the buffer is not empty, this function will overwrite the buffer.
* Second input is the number of bytes to read.
* Output is errors thrown on screen if unsuccessful operation otherwise it will return the number of bytes read
*/
int UNR_I2CHandle::i2c_read_simple(unsigned char& buffer, const unsigned short& numBytes) noexcept(false)
{
#ifdef DEBUG

	m_s4Return_in = read(m_intFile_descriptor, (void*)(&buffer), static_cast<size_t>(numBytes));
	if (m_s4Return_in < 0)
	{
		std::error_code ec(errno, std::generic_category());
		std::error_condition ok;
		std::string msg = ec.message();
		if (ec != ok) puts(ec.message().c_str());
		throw std::runtime_error(std::string("I2C Could not handle Read Operation"));
	}
	return m_s4Return_in;
#else
	return read(m_intFile_descriptor, (void*)(&buffer), static_cast<size_t>(numBytes)) == -1 ? -1 : numBytes;
#endif
}

/*
* Destructor for the i2c device driver class
*/
UNR_I2CHandle::~UNR_I2CHandle(void)
{
	if (m_intFile_descriptor != 0) close(m_intFile_descriptor);
	m_intFile_descriptor = 0;
	m_ucDecive_Address = 0;
}
