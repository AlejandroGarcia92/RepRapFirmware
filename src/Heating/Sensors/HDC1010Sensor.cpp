/*
 * HDC1010Sensor.cpp
 *
 *  Created on: 26 abr. 2019
 *      Author: agarciamoreno
 */

#include "HDC1010Sensor.h"
#include "RepRap.h"
#include "Platform.h"
#include "GCodes/GCodeBuffer.h"
#include "GCodes/GCodes.h"
#include "Hardware/I2C.h"
#ifdef BCN3D_DEV

// Define the minimum interval between readings.
const uint32_t MinimumReadInterval = 2000;		// minimum interval between reads, in milliseconds
const uint32_t Default_delay = 10;		// minimum interval between request and send, in milliseconds

HDC1010Sensor::HDC1010Sensor(unsigned int addr_offset)
	: I2CTemHumSensor(addr_offset, "HDC1010 Sensor I2C")// addr between 64-67
{
	addr = addr_offset - FirstHDC1010Channel + 64;
}
void HDC1010Sensor::Init()
{
	I2C::Init();
	TemperatureError rslt;
	for (unsigned int i = 0; i < 3; ++i)		// try 3 times
	{
		rslt = TryInitI2C();
		if (rslt == TemperatureError::success)
		{
			break;
		}
		delay(MinimumReadInterval);
	}

	lastReadingTime = millis();
	lastResult = rslt;
	lastTemperature = 0.0;

	if (rslt != TemperatureError::success)
	{
		reprap.GetPlatform().MessageF(ErrorMessage, "Failed to initialise I2C: %s\n", TemperatureErrorString(rslt));
	}

}
// Configure this temperature sensor
GCodeResult HDC1010Sensor::Configure(unsigned int mCode, unsigned int heater, GCodeBuffer& gb, const StringRef& reply)
{
	if (mCode == 305)
	{
		bool seen = false;
		if(gb.Seen('A')){
			seen = true;
			temorhum = true;
		}else{
			temorhum = false;
		}
		TryConfigureHeaterName(gb, seen);

		if (!gb.Seen('X'))
		{
			CopyBasicHeaterDetails(heater, reply);
			reply.catf(", Something wrong");
		}
	}
	return GCodeResult::ok;
}
// Try to initialise the I2C
TemperatureError HDC1010Sensor::TryInitI2C() const
{

	const uint8_t command_1[3] = {0x02, 0x80, 0x00};			// Configure device reset software
	const uint8_t command_2[3] = {0x02, 0x00, 0x00};			// Configure device normal operation
	//const uint16_t addr = 80;//default dir
	uint32_t rawVal;

	TemperatureError sts = DoI2CTransaction(command_1, ARRAY_SIZE(command_1), 0, rawVal, addr);

	if(sts!=TemperatureError::success) return sts;

	delay(5);

	sts = DoI2CTransaction(command_2, ARRAY_SIZE(command_2), 0, rawVal, addr);

	return sts;
}
TemperatureError HDC1010Sensor::TryGetTemperature(float& t)
{
	// Sample time slot for temp HDC1010Sensor sensor with addr 64 is millis()
	// Sample time slot for temp HDC1010Sensor sensor with addr 65 is millis() + 100
	// Sample time slot for temp HDC1010Sensor sensor with addr 66 is millis() + 200
	// Sample time slot for temp HDC1010Sensor sensor with addr 67 is millis() + 300
	// Sample time slot for hum HDC1010Sensor sensor with addr 64 is millis() + 1000
	// Sample time slot for hum HDC1010Sensor sensor with addr 65 is millis() + 1100
	// Sample time slot for hum HDC1010Sensor sensor with addr 66 is millis() + 1200
	// Sample time slot for hum HDC1010Sensor sensor with addr 67 is millis() + 1300
	if (inInterrupt() || millis()-((temorhum?1000:0)+250*(addr-64)) - lastReadingTime < MinimumReadInterval)// Reserve time slots for different Sensors i2c
	{
		t = lastTemperature;
	}
	else
	{
		lastReadingTime = millis() + ((temorhum?1000:0)+250*(addr-64));

		static bool rw = false; // request data
		static const uint8_t command[1] = {0x01};			// Read Memory from dir 0x52 hum
		static const uint8_t command2[1] = {0x00};			// Read Memory from dir 0x52 hum


		TemperatureError sts;
		uint32_t rawVal;
		//reprap.GetPlatform().MessageF(GenericMessage, "%u millis(): %lu \n",addr, millis());

		if(rw){
			if(temorhum){
			sts = DoI2CTransaction(command2,0, 2, rawVal, addr);
			}else{
			sts = DoI2CTransaction(command,0, 2, rawVal, addr);
			}
			if (sts != TemperatureError::success)
			{
				//lastResult = sts;
				//reprap.GetPlatform().MessageF(HttpMessage, "I2c Rx Error\n");
				t = lastTemperature;
			}
			else
			{

				if(temorhum){
					t = ((float) rawVal *100.0 / 65536.0);
				}else{
					t = ((float) rawVal *165.0 / 65536.0)-40.0;
				}

				lastTemperature = t;
			}

			rw = false;
		}else{
			if(temorhum){
			sts = DoI2CTransaction(command, ARRAY_SIZE(command), 0, rawVal, addr); //request data
			}else{
			sts = DoI2CTransaction(command2, ARRAY_SIZE(command2), 0, rawVal, addr); //request data
			}
			//lastResult = sts;
			t = lastTemperature;
			rw = true;

		}
		//delay(10);

	}
	return lastResult;
}

#endif
