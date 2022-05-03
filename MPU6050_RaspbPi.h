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
*/


#pragma once
#include "UNR_BCM2711_I2CHandle.h"
#include "MPU6050_RegisterMap.h"

#define SINGLE_BYTE_TRANSACTION		1U
#define MPU6050_DEVICE_ADDRESS		0x68
#define SWAP_BYTES(X)	(((0x00FF & X)<<8) | ((0xFF00 & X) >> 8)) 

class MPU6050_RaspbPi : public UNR_I2CHandle
{
private:
	union MPU6050_SensorValues 
	{
		struct sensorValues 
		{
			union Accelx 
			{
				uint8_t u8_AXData[2];
				uint16_t u16_AXData;
			}Accelx_t;

			union Accely
			{
				uint8_t u8_AYData[2];
				uint16_t u16_AYData;
			}Accely_t;

			union Accelz
			{
				uint8_t u8_AZData[2];
				uint16_t u16_AZData;
			}Accelz_t;

			union Temperature 
			{
				uint8_t u8_TData[2];
				uint16_t u16_TData;
			}Temperature_t;

			union Gyrox
			{
				uint8_t u8_GXData[2];
				uint16_t u16_GXData;
			}Gyrox_t;

			union Gyroy
			{
				uint8_t u8_GYData[2];
				uint16_t u16_GYData;
			}Gyroy_t;

			union Gyroz
			{
				uint8_t u8_GZData[2];
				uint16_t u16_GZData;
			}Gyroz_t;
		}sensorValues_t;

		uint8_t byteData[14];
		void swapAllBytes() 
		{
			sensorValues_t.Accelx_t.u16_AXData = SWAP_BYTES(sensorValues_t.Accelx_t.u16_AXData);
			sensorValues_t.Accely_t.u16_AYData = SWAP_BYTES(sensorValues_t.Accely_t.u16_AYData);
			sensorValues_t.Accelz_t.u16_AZData = SWAP_BYTES(sensorValues_t.Accelz_t.u16_AZData);
			sensorValues_t.Temperature_t.u16_TData = SWAP_BYTES(sensorValues_t.Temperature_t.u16_TData);
			sensorValues_t.Gyrox_t.u16_GXData = SWAP_BYTES(sensorValues_t.Gyrox_t.u16_GXData);
			sensorValues_t.Gyroy_t.u16_GYData = SWAP_BYTES(sensorValues_t.Gyroy_t.u16_GYData);
			sensorValues_t.Gyroz_t.u16_GZData = SWAP_BYTES(sensorValues_t.Gyroz_t.u16_GZData);
		}
	};


	MPU6050_SensorValues* m_sensorvalues;
	unsigned char tempBuffer[2];
	unsigned char accessRegisterID;
	unsigned char accessSensorValuesRegister; 

	signed int _f2sComplement(uint16_t& );
	void inline resetBuffer(void) { memset(tempBuffer, NULL, 2U); }
		

	double gyroScale;
	double accelScale;

	/** Get and Set clock source setting.
 * An internal 8MHz oscillator, gyroscope based clock, or external sources can
 * be selected as the MPU-60X0 clock source. When the internal 8 MHz oscillator
 * or an external source is chosen as the clock source, the MPU-60X0 can operate
 * in low power modes with the gyroscopes disabled.
 *
 * Upon power up, the MPU-60X0 clock source defaults to the internal oscillator.
 * However, it is highly recommended that the device be configured to use one of
 * the gyroscopes (or an external clock source) as the clock reference for
 * improved stability. The clock source can be selected according to the following table:
 *
 * <pre>
 * CLK_SEL | Clock Source
 * --------+--------------------------------------
 * 0       | Internal oscillator
 * 1       | PLL with X Gyro reference
 * 2       | PLL with Y Gyro reference
 * 3       | PLL with Z Gyro reference
 * 4       | PLL with external 32.768kHz reference
 * 5       | PLL with external 19.2MHz reference
 * 6       | Reserved
 * 7       | Stops the clock and keeps the timing generator in reset
 * </pre>
 *
 */
	int setClockSource(unsigned char);
	int getClockSource(unsigned char);


/** Get and set full-scale gyroscope range.
* The FS_SEL parameter allows setting the full-scale range of the gyro sensors,
* as described in the table below.
*
* <pre>
* 0 = +/- 250 degrees/sec
* 1 = +/- 500 degrees/sec
* 2 = +/- 1000 degrees/sec
* 3 = +/- 2000 degrees/sec
* </pre>
*
* @return Current full-scale gyroscope range setting
* @see MPU6050_GYRO_FS_250
* @see MPU6050_RA_GYRO_CONFIG
* @see MPU6050_GCONFIG_FS_SEL_BIT
* @see MPU6050_GCONFIG_FS_SEL_LENGTH
*/
	int setFullScaleGyroRange(unsigned char);
	int getFullScaleGyroRange(unsigned char);

/** Get and Set full-scale accelerometer range.
* The FS_SEL parameter allows setting the full-scale range of the accelerometer
* sensors, as described in the table below.
*
* <pre>
* 0 = +/- 2g
* 1 = +/- 4g
* 2 = +/- 8g
* 3 = +/- 16g
* </pre>
*
* @return Current full-scale accelerometer range setting
* @see MPU6050_ACCEL_FS_2
* @see MPU6050_RA_ACCEL_CONFIG
* @see MPU6050_ACONFIG_AFS_SEL_BIT
* @see MPU6050_ACONFIG_AFS_SEL_LENGTH
*/
	int getFullScaleAccelRange(unsigned char);
	int setFullScaleAccelRange(unsigned char);

/** Get and sleep sleep mode status.
* Setting the SLEEP bit in the register puts the device into very low power
* sleep mode. In this mode, only the serial interface and internal registers
* remain active, allowing for a very low standby current. Clearing this bit
* puts the device back into normal mode. To save power, the individual standby
* selections for each of the gyros should be used if any gyro axis is not used
* by the application.
* @return Current sleep mode enabled status
* @see MPU6050_RA_PWR_MGMT_1
* @see MPU6050_PWR1_SLEEP_BIT
*/
	bool getSleepEnabled();
	int setSleepEnabled(bool);

public:
    MPU6050_RaspbPi(unsigned char _devAddress) : UNR_I2CHandle(RPI4_I2C_INSTANCE1, _devAddress, I2C_SLAVE)
												, accessRegisterID(0x00) 
												, tempBuffer({0x00 , 0x00})
												, accessSensorValuesRegister(MPU6050_RA_ACCEL_XOUT_H)
												, gyroScale(MPU6050_GYRO_FS_250_SCALE)
												, accelScale(MPU6050_ACCEL_FS_2_SCALE)
	{
		m_sensorvalues = new MPU6050_SensorValues;
	}

	MPU6050_RaspbPi() = delete;
	MPU6050_RaspbPi(const MPU6050_RaspbPi&) = delete;
	MPU6050_RaspbPi(const MPU6050_RaspbPi&&) = delete;
	MPU6050_RaspbPi& operator = (MPU6050_RaspbPi&) = delete;

/*
* Destructor function
*/
    ~MPU6050_RaspbPi(void);

	/** Power on and prepare for general usage.
 * This will activate the device and take it out of sleep mode (which must be done
 * after start-up). This function also sets both the accelerometer and the gyroscope
 * to their most sensitive settings, namely +/- 2g and +/- 250 degrees/sec, and sets
 * the clock source to use the X Gyro for reference, which is slightly better than
 * the default internal clock source.
 */
    int initialize(void);

	/** Get raw 6-axis motion sensor readings (accel/gyro) and Temperature values.
	 * Retrieves all currently available motion sensor values.
	 * @param ax 16-bit signed integer container for accelerometer X-axis value
	 * @param ay 16-bit signed integer container for accelerometer Y-axis value
	 * @param az 16-bit signed integer container for accelerometer Z-axis value
	 * @param temp 
	 * @param gx 16-bit signed integer container for gyroscope X-axis value
	 * @param gy 16-bit signed integer container for gyroscope Y-axis value
	 * @param gz 16-bit signed integer container for gyroscope Z-axis value
	 * @see 
	 * @see 
	 * @see MPU6050_RA_ACCEL_XOUT_H
	 */

	int getSensorValues();
	int getDoubleSensorValues(double*, double*, double*);
	//int getRAWSensorValues(uint16_t*);

};

