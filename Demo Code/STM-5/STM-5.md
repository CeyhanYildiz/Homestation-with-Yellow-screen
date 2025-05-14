# STM-5

Sensor module for multiple air condition related values, uses i2c communication

## connections

STM-5 uses i2c communication. On the board there are 5 pins, the square pin is pin 1.

<img src="../../Photos/Stm-5_1.jpg" alt="STM-5" width="200">

| pin | discription |
|-----|-------------|
| 1 | ground |
| 2 | / |
| 3 | scl |
| 4 | sda |
| 5 | vcc( 3.3 v ) |

## usage

As mentioned before, this sensor uses i2c communication. The Sensor's slave adress is 0x2A. To initialize the chip 0xFF is written to register 0xC0. After this data transfer the sensor needs to update it's values. Then, starting from address 0x00, the first 15 values will contain the sensor data.

```c
    float temperature = ((data[1] << 8) | data[0]) / 10.0;  // Temperature in °C
    float humidity = ((data[3] << 8) | data[2]) / 10.0;     // Humidity in %
    uint16_t light = (data[5] << 8) | data[4];              // Light in Lux
    uint8_t audio = data[6];                                // Audio in dB
    uint16_t eCO2 = (data[8] << 8) | data[7];               // eCO2 in PPM
    uint16_t VOC = (data[10] << 8) | data[9];               // VOC in PPB
    float batteryVoltage = ((data[14] << 8) | data[13]) / 1024.0 * (3.3 / 0.330);  // Battery Voltage
```

## [code example](./STM-5.ino)
