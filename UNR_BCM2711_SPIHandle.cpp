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
*  File Description: This driver file is intended to interface the BCM2711 SPI with POSIX / *Unix sytems.
*					Make sure the /dev/spidev0.X (X=0 or 1) file is present in the system 
* Unauthorized Distrubution is strictly prohibited.
* Rev 1: First revision. Seems to run without bugs. Will do more testing 
*/

#include "UNR_BCM2711_SPIHandle.h"


	void UNR_SPIHandle::init_file_descriptor(const char _charFD[]) noexcept(false)
	{
		m_intFile_descriptor = open(_charFD, O_RDWR);
		if (m_intFile_descriptor < 0)
		{
			throw std::runtime_error(std::string("Can not open port"));
		}
	}

	void UNR_SPIHandle::set_spi_mode(const unsigned char &_u1MODE) noexcept(false)
	{
		if ((ioctl(m_intFile_descriptor, SPI_IOC_WR_MODE, &_u1MODE)) == OV_IOCTRL_FAIL)
		{
			throw std::runtime_error(std::string("Error Setting SPI Write Mode"));
		}

		if ((ioctl(m_intFile_descriptor, SPI_IOC_RD_MODE, &_u1MODE)) == OV_IOCTRL_FAIL)
		{
			throw std::runtime_error(std::string("Error Setting SPI Read Mode"));
		}
	}

	void UNR_SPIHandle::set_spi_databits(const unsigned char &_u1BITS) noexcept(false)
	{
		if ((ioctl(m_intFile_descriptor, SPI_IOC_WR_BITS_PER_WORD, &_u1BITS)) == OV_IOCTRL_FAIL)
		{
			throw std::runtime_error(std::string("Error in setting Write Bits per word"));
		}

		if ((ioctl(m_intFile_descriptor, SPI_IOC_RD_BITS_PER_WORD, &_u1BITS)) == OV_IOCTRL_FAIL)
		{
			throw std::runtime_error(std::string("Error in setting Read Bits per word"));
		}
	}

	void UNR_SPIHandle::set_spi_frequency(const unsigned int &_u4FREQ) noexcept(false)
	{

		if ((ioctl(m_intFile_descriptor, SPI_IOC_WR_MAX_SPEED_HZ, &_u4FREQ)) == OV_IOCTRL_FAIL)
		{
			throw std::runtime_error(std::string("Error Setting Write Speed"));
		}

		if ((ioctl(m_intFile_descriptor, SPI_IOC_RD_MAX_SPEED_HZ, &_u4FREQ)) == OV_IOCTRL_FAIL)
		{
			throw std::runtime_error(std::string("Error Setting Read Speed"));
		}
	}

	UNR_SPIHandle::UNR_SPIHandle(unsigned int _u4SPI_instance,
		unsigned char _u1Mode,
		unsigned char _u1Bits,
		unsigned int _u4Freq) noexcept(false) : m_intFile_descriptor(0) , m_spifrequency(0)
	{
		switch (_u4SPI_instance)
		{
		case RPI3_SPI_INSTANCE0:
		{
			try {
				init_file_descriptor(RPI3_SPI_DEV_INSTANCE0);
			}
			catch (std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
			break;
		}

		case RPI3_SPI_INSTANCE1:
		{
			try {
				init_file_descriptor(RPI3_SPI_DEV_INSTANCE1);
			}
			catch (std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
			break;
		}
		default:
			break;
		}

		try {
			set_spi_mode(_u1Mode);
		}
		catch (std::exception& e)
		{
			throw std::runtime_error(e.what());
		}

		try {
			set_spi_databits(_u1Bits);
		}
		catch (std::exception& e)
		{
			throw std::runtime_error(e.what());
		}

		try {
			set_spi_frequency(_u4Freq);
			m_spifrequency = _u4Freq;
		}
		catch (std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
		m_s4Return_in = 0;
		m_s4Return_out = 0;
	}

	/*Funct: spi_write returns write number or -1 for error*/
	int UNR_SPIHandle::spi_write(const unsigned char &_u1TX, const unsigned int &_u4Size) noexcept(false)
	{
		m_s4Return_in = write(m_intFile_descriptor, (void*)(&_u1TX), _u4Size);
		if (m_s4Return_in == OV_IOCTRL_FAIL)
		{
			std::error_code ec(errno, std::generic_category());
			std::error_condition ok;
			std::string msg = ec.message();
			if (ec != ok) puts(ec.message().c_str());
			throw std::runtime_error(std::string("Write Bytes to Port Error"));
		}
		return m_s4Return_in;
	}

	/*Funct: spi_read returns read number or -1 for error*/
	int UNR_SPIHandle::spi_read(unsigned char &_u1RX, const unsigned int &_u4Size) noexcept(false)
	{
		m_s4Return_out = read(m_intFile_descriptor, (void*)(&_u1RX), _u4Size);
		if (m_s4Return_out == OV_IOCTRL_FAIL)
		{
			std::error_code ec(errno, std::generic_category());
			std::error_condition ok;
			std::string msg = ec.message();
			if (ec != ok) puts(ec.message().c_str());
			throw std::runtime_error(std::string("Read Bytes from Port Error"));
		}
		return m_s4Return_out;
	}

	UNR_SPIHandle::~UNR_SPIHandle(void)
	{
		close(m_intFile_descriptor);
		m_intFile_descriptor = 0;
		m_spifrequency = 0;
	}
