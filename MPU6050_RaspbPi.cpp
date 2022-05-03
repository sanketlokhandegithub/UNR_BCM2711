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
*  File Description: MPU6050 Driver File
* 
* Unauthorized Distrubution is strictly prohibited.
* Rev 1: Added Startup code 
* Rev 2: Finished the ACCEL GYRO and TEMP sensor data acquition functions // TODO: OFFSET data calibration
*/


#include "MPU6050_RaspbPi.h"

int MPU6050_RaspbPi::initialize(void)
{
	//Check for the correct IC
	resetBuffer();
	accessRegisterID = MPU6050_RA_WHO_AM_I;
	if (this->i2c_readReg(tempBuffer[0], accessRegisterID, SINGLE_BYTE_TRANSACTION) < 0) return -1;
	if (tempBuffer[0] == 0x68)
	{
		printf("MPU6050 IC Detected!\n");
	}
	else
	{
		printf("Could no detect MPU6050!\n");
		return -2;
	}
	setClockSource(MPU6050_CLOCK_PLL_XGYRO);
	setFullScaleGyroRange(MPU6050_GYRO_FS_250);
	setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
	setSleepEnabled(false); 
}

int MPU6050_RaspbPi::getClockSource(unsigned char _in)
{
	resetBuffer();
	accessRegisterID = MPU6050_RA_PWR_MGMT_1;
	return (this->i2c_readReg(_in, accessRegisterID, SINGLE_BYTE_TRANSACTION));	
}

int MPU6050_RaspbPi::setClockSource(unsigned char _source) 
{
	if (getClockSource(tempBuffer[0]) < 0) return -1;
	tempBuffer[1] = tempBuffer[0] |  ((_source) & 0b00000111);
	return (this->i2c_writeReg(tempBuffer[1], accessRegisterID, SINGLE_BYTE_TRANSACTION));
}

int MPU6050_RaspbPi::getFullScaleGyroRange(unsigned char _in)
{
	resetBuffer();
	accessRegisterID = MPU6050_RA_GYRO_CONFIG;
	return this->i2c_readReg(_in, accessRegisterID, SINGLE_BYTE_TRANSACTION);
}

int MPU6050_RaspbPi::setFullScaleGyroRange(unsigned char _scale)
{
	switch (_scale)
	{
		case MPU6050_GYRO_FS_250:  gyroScale = MPU6050_GYRO_FS_250_SCALE; break;
		case MPU6050_GYRO_FS_500:  gyroScale = MPU6050_GYRO_FS_500_SCALE; break;
		case MPU6050_GYRO_FS_1000: gyroScale = MPU6050_GYRO_FS_1000_SCALE; break;
		case MPU6050_GYRO_FS_2000: gyroScale = MPU6050_GYRO_FS_2000_SCALE; break;
		default: gyroScale = MPU6050_GYRO_FS_250_SCALE; break;
	}
	if (getFullScaleGyroRange(tempBuffer[0]) < 0) return -1;
	tempBuffer[1] = tempBuffer[0] | ((_scale << (MPU6050_GCONFIG_FS_SEL_BIT-1)) & 0b00011000);
	return (this->i2c_writeReg(tempBuffer[1], accessRegisterID, SINGLE_BYTE_TRANSACTION));
}


int MPU6050_RaspbPi::getFullScaleAccelRange(unsigned char _in)
{
	resetBuffer();
	accessRegisterID = MPU6050_RA_ACCEL_CONFIG;
	return (this->i2c_readReg(_in, accessRegisterID, SINGLE_BYTE_TRANSACTION));
}

int MPU6050_RaspbPi::setFullScaleAccelRange(unsigned char _scale)
{
	switch (_scale)
	{
		case MPU6050_ACCEL_FS_2: accelScale = MPU6050_ACCEL_FS_2_SCALE; break;
		case MPU6050_ACCEL_FS_4: accelScale = MPU6050_ACCEL_FS_4_SCALE; break;
		case MPU6050_ACCEL_FS_8: accelScale = MPU6050_ACCEL_FS_8_SCALE; break;
		case MPU6050_ACCEL_FS_16: accelScale = MPU6050_ACCEL_FS_16_SCALE; break;
		default: accelScale = MPU6050_ACCEL_FS_2_SCALE; break;
	}
	if (getFullScaleAccelRange(tempBuffer[0]) < 0) return -1;
	tempBuffer[1] = tempBuffer[0] | ((_scale << (MPU6050_ACONFIG_AFS_SEL_BIT-1)) & 0b00011000);
	return (this->i2c_writeReg(tempBuffer[1], accessRegisterID, SINGLE_BYTE_TRANSACTION));
}

bool MPU6050_RaspbPi::getSleepEnabled()
{
	resetBuffer();
	accessRegisterID = MPU6050_RA_PWR_MGMT_1;
	if (this->i2c_readReg(tempBuffer[0], accessRegisterID, SINGLE_BYTE_TRANSACTION) < 0)
		return false;
	return (tempBuffer[0] && (1U << (MPU6050_PWR1_SLEEP_BIT)));
}

int MPU6050_RaspbPi::setSleepEnabled(bool _in)
{
	resetBuffer();
	accessRegisterID = MPU6050_RA_PWR_MGMT_1;
	if (this->i2c_readReg(tempBuffer[0], accessRegisterID, SINGLE_BYTE_TRANSACTION) < 0)
		return -1;
	
	_in == true ? tempBuffer[0] |= (1U << MPU6050_PWR1_SLEEP_BIT) : tempBuffer[0] &= 0xBF;

	if (this->i2c_writeReg(tempBuffer[0], accessRegisterID, SINGLE_BYTE_TRANSACTION) < 0)
		return -1;
	if (this->i2c_readReg(tempBuffer[0], accessRegisterID, SINGLE_BYTE_TRANSACTION) < 0)
		return -1;
	return 1;
}

int MPU6050_RaspbPi::getSensorValues()
{ 
	if (this->i2c_readReg(m_sensorvalues->sensorValues_t.Accelx_t.u8_AXData[0], accessSensorValuesRegister, 14U) < 0) return -1;
	m_sensorvalues->swapAllBytes();		
	return 1;
}

int MPU6050_RaspbPi::getDoubleSensorValues(double* accel, double* gyro, double* temperature)
{
	if (getSensorValues() > 0)
	{
		accel[0] = (double)_f2sComplement(m_sensorvalues->sensorValues_t.Accelx_t.u16_AXData) / accelScale;
		accel[1] = (double)_f2sComplement(m_sensorvalues->sensorValues_t.Accely_t.u16_AYData) / accelScale;
		accel[2] = (double)_f2sComplement(m_sensorvalues->sensorValues_t.Accelz_t.u16_AZData) / accelScale;

		temperature[0] = (double)(_f2sComplement(m_sensorvalues->sensorValues_t.Temperature_t.u16_TData)) / 340.00 + 36.53;

		gyro[0] = (double)_f2sComplement(m_sensorvalues->sensorValues_t.Gyrox_t.u16_GXData)/ gyroScale;
		gyro[1] = (double)_f2sComplement(m_sensorvalues->sensorValues_t.Gyroy_t.u16_GYData) / gyroScale;
		gyro[2] = (double)_f2sComplement(m_sensorvalues->sensorValues_t.Gyroz_t.u16_GZData) / gyroScale;

		return 1;
	}

	else
		return -1;
	
	
}

signed int MPU6050_RaspbPi::_f2sComplement(uint16_t& _in)
{
	if (_in >= 0b1000000000000000)
	{
		_in = ~(_in);
		_in = _in + 0b0000000000000001;
		return (0 - _in);
	}
	else
		return _in;
}

MPU6050_RaspbPi::~MPU6050_RaspbPi(void)
{
	delete m_sensorvalues;
}
