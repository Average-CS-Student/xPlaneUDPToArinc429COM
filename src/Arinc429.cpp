#include <iostream>
#include <vector>
#include "Arinc429.h"

void xPlane12UDPtoArinc429(float source[], std::vector<bool> options, std::vector<uint32_t>& output) {
	int optionsSize = options.size();;
	int sourceRow = 0; //row = 8 elements + 1 separator
	output.clear();
	std::vector<uint32_t> temp;
	for (int i = 0; i < optionsSize; i++) {
		if (options[i]) {
			floatDataToArincProto(i, source + sourceRow * 9, temp);
			for (int j = 0; j < temp.size(); j++) {
				output.push_back(temp[j]);
			}
			sourceRow++;
		}
	}

}

void floatDataToArincProto(int optionId, float dataRow[], std::vector<uint32_t>& output) {
	switch (optionId) {
		//option 0 = computed airspeed, ground speed, true speed, equivalent airspeed
	case 0: {
		//data is emulated as if sent by ADIRS (eqpt. ID HEX: 038), equivalent speed - by Supersonic Air Data Computer (eqpt. ID HEX: 140)

		uint32_t computedAirspeed = encodeArincBinary(0b01100001, 0, dataRow[0], 14, 1024); //label 206
		uint32_t equivalentAirspeed = encodeArincBinary(0b01110101, 0, dataRow[1], 14, 1024); //label 256
		uint32_t trueAirspeed = encodeArincBinary(0b00010001, 0, dataRow[2], 15, 2048); //label 210
		uint32_t groundSpeed = encodeArincBinary(0b01010011, 0, dataRow[3], 15, 4096); //label 312

		output.resize(4);
		output[0] = computedAirspeed;
		output[1] = equivalentAirspeed;
		output[2] = trueAirspeed;
		output[3] = groundSpeed;

		break;
	}

		  //option 1 = Mach, Internal Verical Velocity
	case 1: {
		//data is emulated as if sent by ADIRS (eqpt. ID HEX: 038)

		uint32_t mach = encodeArincBinary(0b10100001, 0, dataRow[0], 16, 4.096f); //label 205
		uint32_t IVV = encodeArincBinary(0b10101111, 0, dataRow[2], 15, 32768); //label 365

		output.resize(2);
		output[0] = mach;
		output[1] = IVV;

		break;
	}

		  //option 2 = AOA and AOS
	case 2: {
		// data is emulated as if sent by ADIRS (eqpt. ID HEX: 038)

		// Convert angles to range -180 to 180 degrees per Arinc429 specification
		float AOA = dataRow[0];
		if (AOA > 180) {
			AOA = AOA - 360;
		}
		else if (AOA < -180) {
			AOA = AOA + 360;
		}

		float AOS = dataRow[7];
		if (AOS > 180) {
			AOS = AOS - 360;
		}
		else if (AOS < -180) {
			AOS = AOS + 360;
		}

		uint32_t angleOfAttack = encodeArincBinary(0b10001001, 0, AOA, 12, 180); //label 221
		uint32_t angleOfSlip = encodeArincBinary(0b00010101, 0, AOS, 12, 180); //label 250

		output.resize(2);
		output[0] = angleOfAttack;
		output[1] = angleOfSlip;
		break;
	}

		  //option 3 = pitch, roll, true heading, magnetic heading
	case 3: {
		//data is emulated as if sent by ADIRS (eqpt. ID HEX: 038)

		float pitchAngle = dataRow[0];
		if (pitchAngle > 180) {
			pitchAngle = pitchAngle - 360;
		}
		else if (pitchAngle < -180) {
			pitchAngle = pitchAngle + 360;
		}

		float rollAngle = dataRow[1];
		if (rollAngle > 180) {
			rollAngle = rollAngle - 360;
		}
		else if (rollAngle < -180) {
			rollAngle = rollAngle + 360;
		}

		float trueHeadAngle = dataRow[2];
		if (trueHeadAngle > 180) {
			trueHeadAngle = trueHeadAngle - 360;
		}
		else if (trueHeadAngle < -180) {
			trueHeadAngle = trueHeadAngle + 360;
		}

		float magHeadAngle = dataRow[4];
		if (magHeadAngle > 180) {
			magHeadAngle = magHeadAngle - 360;
		}
		else if (magHeadAngle < -180) {
			magHeadAngle = magHeadAngle + 360;
		}

		uint32_t pitch = encodeArincBinary(0b00101011, 0, pitchAngle, 14, 180); //label 324
		uint32_t roll = encodeArincBinary(0b10101011, 0, rollAngle, 14, 180); //label 325
		uint32_t trueHeading = encodeArincBinary(0b00110011, 0, trueHeadAngle, 15, 180); //label 314
		uint32_t magHeading = encodeArincBinary(0b00001011, 0, magHeadAngle, 15, 180); //label 320

		output.resize(4);
		output[0] = pitch;
		output[1] = roll;
		output[2] = trueHeading;
		output[3] = magHeading;

		break;
	}

		  //option 4 = latitude, longitude, altitude
	case 4: {
		//data is emulated as if sent by ADIRS (eqpt. ID HEX: 038)

		float latAngle = dataRow[0];
		if (latAngle > 180) {
			latAngle = latAngle - 360;
		}
		else if (latAngle < -180) {
			latAngle = latAngle + 360;
		}

		float longAngle = dataRow[1];
		if (longAngle > 180) {
			longAngle = longAngle - 360;
		}
		else if (longAngle < -180) {
			longAngle = longAngle + 360;
		}

		//latitude and longitude are only parameters that occupy more than 18 bits and store data in SDI part of a message
		uint32_t latitude = encodeArincBinary(0b00010011, 0, latAngle, 20, 180); //label 310
		uint32_t longitude = encodeArincBinary(0b10010011, 0, longAngle, 20, 180); //label 311

		uint32_t altitude = encodeArincBinary(0b11000001, 0, dataRow[5], 17, 131072); //label 203

		output.resize(3);
		output[0] = latitude;
		output[1] = longitude;
		output[2] = altitude;

		break;
	}

		  //option 5 = body pitch, roll and yaw acceleration
	case 5: {
		//data is emulated as if sent by ADIRS (eqpt. ID HEX: 038)

		uint32_t pitchAccel = encodeArincBinary(0b01010100, 0, dataRow[1], 15, 64); //label 052
		uint32_t rollAccel = encodeArincBinary(0b11010100, 0, dataRow[0], 15, 64); //label 053
		uint32_t yawAccel = encodeArincBinary(0b00110100, 0, dataRow[2], 15, 64); //label 054

		output.resize(3);
		output[0] = pitchAccel;
		output[1] = rollAccel;
		output[2] = yawAccel;

		break;
	}

		  //option 6 = body pitch, roll and yaw rate
	case 6: {
		//data is emulated as if sent by ADIRS (eqpt. ID HEX: 038)

		uint32_t pitchRate = encodeArincBinary(0b01101011, 0, dataRow[1], 13, 128); //label 326
		uint32_t rollRate = encodeArincBinary(0b11101011, 0, dataRow[0], 13, 128); //label 327
		uint32_t yawRate = encodeArincBinary(0b00011011, 0, dataRow[2], 13, 128); //label 330

		output.resize(3);
		output[0] = pitchRate;
		output[1] = rollRate;
		output[2] = yawRate;

		break;
	}

		  //option 7 = static pressure, static air temperature, total air temperature, air density ratio
	case 7: {
		//data is emulated as if sent by Supersonic Air Data Computer (eqpt. ID HEX: 140)

		uint32_t TAT = encodeArincBinary(0b10010001, 0, dataRow[2], 12, 512); //label 211
		uint32_t SAT = encodeArincBinary(0b11010001, 0, dataRow[1], 11, 512); //label 213
		uint32_t staticPressure = encodeArincBinary(0b11110001, 0, dataRow[0], 16, 64); //label 217
		uint32_t airDensity = encodeArincBinary(0b01000111, 0, dataRow[3], 12, 4); //label 342

		output.resize(4);
		output[0] = TAT;
		output[1] = SAT;
		output[2] = staticPressure;
		output[3] = airDensity;

		break;
	}

		  //option 8 = EPR actual
	case 8: {
		//data is emulated as if sent by ADDCS and EICAS (eqpt. ID HEX: 029)

		// SDI L = 01, R = 10
		uint32_t EPRLeft = encodeArincBinary(0b00000111, 0, dataRow[0], 12, 4, 0b01); //label 340
		uint32_t EPRRight = encodeArincBinary(0b00000111, 0, dataRow[1], 12, 4, 0b10); //label 340

		output.resize(2);
		output[0] = EPRLeft;
		output[1] = EPRRight;

		break;
	}

		  //option 9 = fuel flow
	case 9: {
		//data is emulated as if sent by ADDCS and EICAS (eqpt. ID HEX: 029)

		// Least Significant Bit = 8
		float FFL = dataRow[0] / 8;
		float FFR = dataRow[1] / 8;

		// SDI L = 01, R = 10
		uint32_t fuelFlowL = encodeArincBinary(0b11100111, 0, FFL, 12, 32768 / 8, 0b01); //label 347
		uint32_t fuelFlowR = encodeArincBinary(0b11100111, 0, FFL, 12, 32768 / 8, 0b10); //label 347

		output.resize(2);
		output[0] = fuelFlowL;
		output[1] = fuelFlowR;

		break;
	}

		  //option 10 = Exhaust Gas Temperature
	case 10: {
		//data is emulated as if sent by ADDCS and EICAS (eqpt. ID HEX: 029)

		// SDI L = 01, R = 10
		uint32_t EGTLeft = encodeArincBinary(0b10100111, 0, dataRow[0], 12, 2048, 0b01); //label 345
		uint32_t EGTRight = encodeArincBinary(0b10100111, 0, dataRow[1], 12, 2048, 0b10); //label 345

		output.resize(2);
		output[0] = EGTLeft;
		output[1] = EGTRight;

		break;
	}

		   //option 11 = Oil pressure engine
	case 11: {
		//data is emulated as if sent by ADDCS and EICAS (eqpt. ID HEX: 029)

		// SDI L = 01, R = 10
		uint32_t oilPressureL = encodeArincBinary(0b11110011, 0, dataRow[0], 12, 4096, 0b01); //label 317
		uint32_t oilPressureR = encodeArincBinary(0b11110011, 0, dataRow[1], 12, 4096, 0b10); //label 317

		output.resize(2);
		output[0] = oilPressureL;
		output[1] = oilPressureR;

		break;
	}

		   //option 12 = Oil temperature engine
	case 12: {
		//data is emulated as if sent by ADDCS and EICAS (eqpt. ID HEX: 029)

		// SDI L = 01, R = 10
		uint32_t oilTemperatureL = encodeArincBinary(0b01110011, 0, dataRow[0], 12, 2048, 0b01); //label 316
		uint32_t oilTemperatureR = encodeArincBinary(0b01110011, 0, dataRow[1], 12, 2048, 0b10); //label 316

		output.resize(2);
		output[0] = oilTemperatureL;
		output[1] = oilTemperatureR;

		break;
	}

	default: {
		output.clear();
		break;
	}
	}
}

uint32_t encodeArincBinary(uint8_t label, bool bit29, float value, uint8_t sigBits, float maxRange) {
	uint32_t Arinc429Word = 0;

	// Determine if value is negative
	bool isNegative = (value < 0);

	// Normalize value
	value = abs(value);

	// set label
	Arinc429Word |= label;

	//set bits 31 & 30
	if (value > maxRange) {
		//if value out of range, set 01 (No Computed data for binary data)
		Arinc429Word |= 0b01 << 29;
		Arinc429Word |= (~std::_Popcount(Arinc429Word) % 2 << 31); // set parity bit
		return Arinc429Word;
	}
	else {
		//11 (Normal Operation for binary data)
		Arinc429Word |= 0b11 << 29;
	}
	// set bit 29
	Arinc429Word |= bit29 << 28;

	//----- encode data ------

	// Convert value to an integer representation
	uint32_t floatData = round((value / (maxRange / 2)) * (1 << sigBits - 1));

	// if value doesn't fit in designated bits (too close to maxRange) subtract 1 to make it fit
	if (floatData == 1 << sigBits) {
		floatData--;
	}

	// If original value was negative, take 2's complement
	if (isNegative) {
		uint32_t mask = (1 << sigBits) - 1;
		floatData = ~floatData & mask;
		floatData++;
		floatData |= (1 << sigBits); // Set sign bit
	}
	//------ data encoding end ------

	//set data bits
	Arinc429Word |= floatData << 32 - sigBits - 4; // align data into right bits

	// set parity bit
	Arinc429Word |= (~std::_Popcount(Arinc429Word) % 2 << 31);

	return Arinc429Word;
}

uint32_t encodeArincBinary(uint8_t label, bool bit29, float value, uint8_t sigBits, float maxRange, uint8_t SDI) {
	uint32_t Arinc429Word = 0;

	// Determine if value is negative
	bool isNegative = (value < 0);

	// Normalize value
	value = abs(value);

	// set label
	Arinc429Word |= label;

	//set SDI bits 9 & 10
	Arinc429Word |= SDI << 8;

	//set SSM bits 31 & 30
	if (value > maxRange) {
		//if value out of range, set 01 (No Computed data for binary data)
		Arinc429Word |= 0b01 << 29;
		Arinc429Word |= (~std::_Popcount(Arinc429Word) % 2 << 31); // set parity bit
		return Arinc429Word;
	}
	else {
		//11 (Normal Operation for binary data)
		Arinc429Word |= 0b11 << 29;
	}
	// set bit 29
	Arinc429Word |= bit29 << 28;

	//----- encode data ------

	// Convert value to an integer representation
	uint32_t floatData = round((value / (maxRange / 2)) * (1 << sigBits - 1));

	// if value doesn't fit in designated bits (too close to maxRange) subtract 1 to make it fit
	if (floatData == 1 << sigBits) {
		floatData--;
	}

	// If original value was negative, take 2's complement
	if (isNegative) {
		uint32_t mask = (1 << sigBits) - 1;
		floatData = ~floatData & mask;
		floatData++;
		floatData |= (1 << sigBits); // Set sign bit
	}
	//------ data encoding end ------

	//set data bits
	Arinc429Word |= floatData << 32 - sigBits - 4; // align data into right bits

	// set parity bit
	Arinc429Word |= (~std::_Popcount(Arinc429Word) % 2 << 31);

	return Arinc429Word;
}