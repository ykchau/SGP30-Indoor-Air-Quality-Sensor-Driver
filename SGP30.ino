/*
	Project  : LaLiMat project (https://www.youtube.com/playlist?list=PLJBKmE2nNweRXOebZjydkMEiq2pHtBMOS in Chinese)
 	file     : SGP30.ino
	Author   : ykchau
 	youtube  : youtube.com/ykchau888
  	Licenese : GPL-3.0
   	Please let me know if you use it commercial project.
*/

#include "SGP30.h"

byte RH = 69;
byte temp = 27;
SGP30 IAQSensor;
unsigned long startTime = 0;
unsigned long baselineTime = 0;
unsigned long initTime = 0;

void setup() {
    Wire.begin();
    Serial.begin(115200);

	startTime = millis();
    baselineTime = millis();
    
    // Init SGP30, this command should run every power-up or soft-reset (done in the code)
    // it requires 15s to initialize before the first reading
    // in the mean time, we can set absolute humidity
    // and baseline (if ava.) which saved before ( younger than 1 week)
    IAQSensor.init();
    IAQSensor.setAbsoluteHumidity(RH, temp);

    IAQSensor.getSerialID();
    Serial.print("ID : ");
    Serial.print(IAQSensor.SerialID[0],HEX);
    Serial.print(IAQSensor.SerialID[1],HEX);
    Serial.print(IAQSensor.SerialID[2],HEX);

    IAQSensor.getFeatureSet();

    Serial.println("Waiting 15s for initialize");
}

void loop() {
    if ( millis() > initTime + 15000 ) {
        if ( millis() > startTime + 1000 ) {
            IAQSensor.measure();
            IAQSensor.measureRaw();
            Serial.print(String("TVOC : ") + IAQSensor.gas.TVOC + " ppb | ");
            Serial.print(String("CO2eq : ") + IAQSensor.gas.CO2EQ + " ppm | ");
            Serial.print(String("RAW H2 : ") + IAQSensor.gas.H2 + " ppm | ");
            Serial.println(String("RAW Ethanol : ") + IAQSensor.gas.Ethanol + " ppm");
        }

        if ( millis() > baselineTime + 30000 ) {
            IAQSensor.getBaseline();
            Serial.print(String("Baseline TVOC : ") + IAQSensor.baseline_TVOC);
            Serial.println(String(" | Baseline CO2eq : ") + IAQSensor.baseline_CO2eq);
            baselineTime = millis();
        }

        delay(1000);
    }
}
