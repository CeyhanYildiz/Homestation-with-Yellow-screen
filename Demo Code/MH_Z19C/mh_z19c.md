# mh_z19c

Sensor to read co2 levels via uart communication between the sensor and the main board

## connections

Sensor has multiple connection, all are documented on the back of the board.
The sensor uses uart to communicate it's data.

<img src="../../Photos/mh-z19c.jpg" alt="DPK SU2AV-0" width="200">

## usage

connect it to the board and setup a uart interface for the sensor to communicate with. The data returned is binary encoded and needs to be parsed in the following way:

```c
int parseCO2(byte response[]) 
{
    if (response[0] != 0xFF || response[1] != 0x86)
    {
        return -1;  // Invalid response
    }

    int high = response[2];
    int low = response[3];
    
    return (high << 8) + low;
}
```

## [code example](./MH_Z19C.ino)