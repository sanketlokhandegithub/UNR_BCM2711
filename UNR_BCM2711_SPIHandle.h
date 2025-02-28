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
* 
*/


#pragma once
#ifndef __UNR_BCM2711_SPIHANDLE_H__
#define __UNR_BCM2711_SPIHANDLE_H__

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <exception>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>         // errno
#include <system_error>   // std::error_code, std::generic_category
						// std::error_condition

constexpr unsigned int RPI3_SPI_INSTANCE0            = 0;
constexpr unsigned int RPI3_SPI_INSTANCE1            = 1;
constexpr char RPI3_SPI_DEV_INSTANCE0[]              = "/dev/spidev0.0";
constexpr char RPI3_SPI_DEV_INSTANCE1[]              = "/dev/spidev0.1";
constexpr signed int UNR_IOCTRL_FAIL                 = -1;


	class UNR_SPIHandle
	{
	private:
		int m_intFile_descriptor;
		unsigned int m_spifrequency;
		ssize_t m_s4Return_in;
		ssize_t m_s4Return_out;
		void init_file_descriptor(const char _charFD[]) noexcept(false);
		void set_spi_mode(const unsigned char &_u1MODE) noexcept(false);
		void set_spi_databits(const unsigned char &_u1BITS) noexcept(false);
		void set_spi_frequency(const unsigned int &_u4FREQ) noexcept(false);
	
	public:
		UNR_SPIHandle(unsigned int _u4SPI_instance,
			unsigned char _u1Mode,
			unsigned char _u1Bits,
			unsigned int _u4Freq) noexcept(false);
		~UNR_SPIHandle(void);
		UNR_SPIHandle() = delete;
		UNR_SPIHandle(const UNR_SPIHandle&) = delete;
		UNR_SPIHandle(const UNR_SPIHandle&&) = delete;
		UNR_SPIHandle & operator = (const UNR_SPIHandle&) = delete;
		UNR_SPIHandle & operator = (const UNR_SPIHandle&&) = delete;
	//protected:
		int spi_write(const unsigned char &_u1TX, const unsigned int &_u4Size) noexcept(false);
		int spi_read(unsigned char&_u1RX, const unsigned int &_u4Size) noexcept(false);
		
	};
#endif // __UNR_BCM2711_SPIHANDLE_H__
