#ifndef SGP30_H_
#define SGP30_H_

#include <Arduino.h>
#include <Wire.h>

#define SGP30_I2C_ADDRESS   0x58

// Feature Set
#define SGP30_IAQ_INIT                      0x03
#define SGP30_MEASURE_IAQ                   0x08
#define SGP30_GET_IAQ_BASELINE              0x15
#define SGP30_SET_IAQ_BASELINE              0x1E
#define SGP30_SET_ABSOLUTE_HUMIDITY         0x61
#define SGP30_MEASURE_TEST                  0x32    // This use for product line testing, didn't implement
#define SGP30_GET_FEATURE_SET               0x2F
#define SGP30_MEASURE_RAW                   0x50
#define SGP30_GET_TVOC_INCEPTIVE_BASELINE   0xB3
#define SGP30_SET_TVOC_BASELINE             0x77

#define SGP30_SOFT_RESET                    0x06

// Checksum
#define SGP30_CRC8_INIT                     0xFF
#define SGP30_CRC8_POLYNOMIAL               0x31

typedef struct GAS {
    int TVOC = 0;       // Calculated value
    int CO2EQ = 0;      // Calculated value
    int Ethanol = 0;
    int H2 = 0;
};


class SGP30 {
    private:
        void I2CWrite(byte data[], byte dataSize);
        byte I2CRead(byte data[], byte byteToRead);
    public:
        GAS gas;
        unsigned int SerialID[3];

        unsigned int baseline_CO2eq = 0;
        unsigned int baseline_TVOC = 0;
        unsigned int baseline_TVOC_Inceptive = 0;

        byte initCheckCounter = 0;

        void init();
        void iaq_init();
        void measure();
        void getBaseline();
        void setBaseline(byte data[]);
        void setAbsoluteHumidity(byte RH, byte Temp);
        void getFeatureSet();
        void measureRaw();
        void getTVOCInceptiveBaseline();
        void setTVOCBaseline(byte data[]);

        void softReset();
        void getSerialID();
        byte checksum(byte data[], byte dataSize);
};

#endif  // SGP30_H_