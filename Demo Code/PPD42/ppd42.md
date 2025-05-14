# ppd42

Sensor to detect polution within the air, pulse triggered interrupt sensor

## connections

On the top side of the sensor there is a pin labled 5.

<img src="../../Photos/unkown.jpg" alt="DPK SU2AV-0" width="200">

| pin | discription |
|-----|-------------|
| 1 | ground |
| 2 | vcc ( 3.3v ) |
| 3 |  |
| 4 | interrupt pin |
| 5 |  |

## usage

Every rising edge on the interrupt pin is the detection of a polution particle ( dust ).

## [code example](./PPD42.ino)
