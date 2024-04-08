/*
	Project  : LaLiMat project (https://www.youtube.com/playlist?list=PLJBKmE2nNweRXOebZjydkMEiq2pHtBMOS in Chinese)
 	file     : SGP30.cpp
	Author   : ykchau
 	youtube  : youtube.com/ykchau888
  	Licenese : GPL-3.0
   	Please let me know if you use it commercial project.
*/

#include "SGP30.h"

/**************************
 * Private
 *************************/
void SGP30::I2CWrite(byte data[], byte dataSize) {
    Wire.beginTransmission(SGP30_I2C_ADDRESS);
    for ( int i = 0; i < dataSize; i++ ) {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

byte SGP30::I2CRead(byte data[], byte byteToRead) {
    byte rec = Wire.requestFrom((int)SGP30_I2C_ADDRESS, (int)byteToRead);

    if ( rec != byteToRead ) {
        return rec;
    }

    while (Wire.available()) {
        *data = Wire.read();
        data++;
    }

    return byteToRead;
}

/**************************
 * Public
 *************************/
void SGP30::init() {
    // Wire.begin();
    // Serial.begin(115200);
    iaq_init();
}

void SGP30::iaq_init() {
    byte command[2] = {0};
    command[0] = 0x20;
    command[1] = SGP30_IAQ_INIT;
    I2CWrite(command, 2);

    delay(10);
}

void SGP30::measure() {
    byte command[2] = {0};
    command[0] = 0x20;
    command[1] = SGP30_MEASURE_IAQ;
    I2CWrite(command, 2);

    delay(12);

    byte data[6] = {0};
    if ( I2CRead(data, 6) == 6 ) {
        byte dataSet1[3] = {data[0],data[1],data[2]};
        byte dataSet2[3] = {data[3],data[4],data[5]};
        // 2 set of data
        // 1st set is CO2eq
        if ( checksum(dataSet1,2) == dataSet1[2] ) {
            gas.CO2EQ = dataSet1[0] * 256 + dataSet1[1];
        } else {
            Serial.println("CO2eq read error!");
        }

        // 2nd set is TVOC
        if ( checksum(dataSet2,2) == dataSet2[2] ) {
            gas.TVOC = dataSet2[0] * 256 + dataSet2[1];
        } else {
            Serial.println("TVOC read error!");
        }

        if ( gas.TVOC == 0 && gas.CO2EQ == 400 ) {
            if ( initCheckCounter >= 5 ) {
                Serial.println("Initialize not complete yet, please wait at least 15s after power up.");
                initCheckCounter = 0;
            } else {
                initCheckCounter++;
            }
        }
    }
}

void SGP30::getBaseline() {
    byte command[2] = {0};
    command[0] = 0x20;
    command[1] = SGP30_GET_IAQ_BASELINE;
    I2CWrite(command, 2);

    delay(10);

    byte data[6] = {0};
    if ( I2CRead(data, 6) == 6 ) {
        byte dataSet1[3] = {data[0],data[1],data[2]};
        byte dataSet2[3] = {data[3],data[4],data[5]};
        // 2 set of data
        // 1st set is CO2eq
        if ( checksum(dataSet1,2) == dataSet1[2] ) {
            baseline_CO2eq = dataSet1[0] * 256 + dataSet1[1];
        } else {
            Serial.println("Baseline CO2eq read error!");
        }

        // 2nd set is TVOC
        if ( checksum(dataSet2,2) == dataSet2[2] ) {
            baseline_TVOC = dataSet2[0] * 256 + dataSet2[1];
        } else {
            Serial.println("Baseline TVOC read error!");
        }
    }
}

/*
If no stored baseline is available after initializing the baseline algorithm, the sensor has to run for 12 hours until the baseline
can be stored. This will ensure an optimal behavior for subsequent startups. Reading out the baseline prior should be avoided
unless a valid baseline is restored first. Once the baseline is properly initialized or restored, the current baseline value should
be stored approximately once per hour. While the sensor is off, baseline values are valid for a maximum of seven days
*/
void SGP30::setBaseline(byte data[]) {
    byte command[2] = {0};
    command[0] = 0x20;
    command[1] = SGP30_SET_IAQ_BASELINE;
    I2CWrite(command, 2);

    I2CWrite(data, 6);
    
    delay(10);
}

void SGP30::setAbsoluteHumidity(byte RH, byte Temp) {
    // Ref : https://planetcalc.com/2167/
    // https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
    // Absolute Humidity, gram/m3

    float AH = (6.112 * pow(2.71828,((17.67 * Temp)/(Temp + 243.5))) * RH * 2.1674) / (273.15 + Temp);

    // send compensation absolute humidity to SGP30
    byte AH_H = (byte)AH;
    byte AH_L = (byte)((AH - AH_H)*256);

    byte command[2] = {0};
    command[0] = 0x20;
    command[1] = SGP30_SET_ABSOLUTE_HUMIDITY;
    I2CWrite(command, 2);

    byte data[3] = { AH_H, AH_L, 0 };
    data[2] = checksum(data, 2);

    I2CWrite(data, 3);

    delay(10);
}


void SGP30::getFeatureSet() {
    byte command[2] = {0};
    command[0] = 0x20;
    command[1] = SGP30_GET_FEATURE_SET;
    I2CWrite(command, 2);

    delay(10);

    byte data[3] = {0};
    if ( I2CRead(data, 3) == 3 ) {
        if ( checksum(data, 2) == data[2] ) {
            Serial.println(String("Product type : ") + (data[0] & 0x0F));
            Serial.println(String("Product Version : ") + data[1]);
        } else {
            Serial.println("Feature Set read error.");
        }
    }
}

void SGP30::measureRaw() {
    byte command[2] = {0};
    command[0] = 0x20;
    command[1] = SGP30_MEASURE_RAW;
    I2CWrite(command, 2);

    delay(25);

    byte data[6] = {0};
    if ( I2CRead(data, 6) == 6 ) {
        byte dataSet1[3] = {data[0],data[1],data[2]};
        byte dataSet2[3] = {data[3],data[4],data[5]};
        // 2 set of data
        // 1st set is H2
        if ( checksum(dataSet1,2) == dataSet1[2] ) {
            gas.H2 = dataSet1[0] * 256 + dataSet1[1];
        } else {
            Serial.println("H2 read error!");
        }

        // 2nd set is Ethanol
        if ( checksum(dataSet2,2) == dataSet2[2] ) {
            gas.Ethanol = dataSet2[0] * 256 + dataSet2[1];
        } else {
            Serial.println("Ethanol read error!");
        }
    }
}

void SGP30::getTVOCInceptiveBaseline() {
    byte command[2] = {0};
    command[0] = 0x20;
    command[1] = SGP30_GET_TVOC_INCEPTIVE_BASELINE;
    I2CWrite(command, 2);

    delay(10);

    byte data[3] = {0};
    if ( I2CRead(data, 3) == 3 ) {
        // extract inceptive baseline
        if ( checksum(data,2) == data[2] ) {
            baseline_TVOC_Inceptive = data[0] * 256 + data[1];
        } else {
            Serial.println("Baseline TVOC Inceptive read error!");
        }
    }
}

void SGP30::setTVOCBaseline(byte data[]) {
    byte command[2] = {0};
    command[0] = 0x20;
    command[1] = SGP30_SET_TVOC_BASELINE;
    I2CWrite(command, 2);
    I2CWrite(data, 3);

    delay(10);
}

void SGP30::softReset() {
    byte command[2] = { 0x00, 0x06 };
    I2CWrite(command, 2);

    iaq_init();
}

void SGP30::getSerialID() {
    byte command[2] = { 0x36, 0x82 };
    I2CWrite(command, 2);

    // wait 500 us for before read
    delay(1);

    byte data[9] = {0};

    // The get serial ID command returns 3 words, and every word is followed by an 8-bit CRC checksum. 
    // Together the 3 words constitute a unique serial ID with a length of 48 bits.
    if ( I2CRead(data, 9) == 9 ) {
        for ( int i = 0; i < 3; i++ ) {
            byte id[2] = {data[i * 3], data[i * 3 + 1]};
            byte crc = data[i * 3 + 2];
            if ( checksum(id,2) == crc ){
                SerialID[i] = id[0] * 256 + id[1];
            } else {
                Serial.println(String("ID Read Error, byte : ") + i);
            }
        }
    }
}



byte SGP30::checksum(byte data[], byte dataSize) {
    byte crc = SGP30_CRC8_INIT;

    for ( int i = 0; i < dataSize; i++ ) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++ ) {
            crc = (crc & 0x80) ? ((crc << 1)^ SGP30_CRC8_POLYNOMIAL) : (crc << 1);
        }
    }
    return crc;
}
