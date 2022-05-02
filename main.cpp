#include "MPU6050_RaspbPi.h"

int main(void)
{
	printf("MPU6050 Driver Starting\n");

	MPU6050_RaspbPi* mpu = new MPU6050_RaspbPi(MPU6050_DEVICE_ADDRESS);
	//MPU6050_RaspbPi* obj = mpu;
	double* accelerometer = new double[3];
	double* gyrometer = new double[3];
	double temperature;

	for (int i = 0; i < 200; i++)
	{
		if (mpu->getDoubleSensorValues(accelerometer, gyrometer, temperature) > 0)
		{
			printf("\nAx : %.2f | Ay : %.2f	| Az : %.2f	| Tp : %.2f	| Gx : %.2f	| Gy : %.2f	| Gz: %.2f	\n"
																											, accelerometer[0]
																											, accelerometer[1]
																											, accelerometer[2]
																											, temperature
																											, gyrometer[0]
																											, gyrometer[1]
																											, gyrometer[2]
																											  );
		}
	}

	delete mpu;
	delete accelerometer;
	delete gyrometer;

	return 0;
}