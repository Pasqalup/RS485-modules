#ifndef COMMAND_H
#define COMMAND_H
/*
    * commands.h - definitions for commands and RSBus class for STM32 master-slave communication
    byte 0: target (slave ID), 255 for broadcast, 0 for master
    byte 1: length of data (=n)
    byte 2: command
    byte 3: parameter (if needed), otherwise data starts from byte 2
    byte 4...n: data (if needed)

*/


//basic commands, byte 2
#define SCN 0x01 // command to scan for slaves; slaves will respond with 0x00(master) 0x03 0x01 0xXX (slave id), 0xXX (slave type) if they are alive
#define RQV 0x02 // command to request data from slave; slaves will respond with their data
#define RSP 0x03 // signifies a response packet from a slave
#define SET 0x04 // command to set a value on the slave; slaves will respond with ACK or NACK
#define SYNC 0x05 // command to synchronize slaves; expect timestamp starting from byte 2.
#define TLM 0xFF // command to send telemetry data from slave to master; expect data starting from byte 3, with length specified in byte 1

// request parameters, byte 3 when command is RQV
#define RQV_GP1 0x01 // request value for general purpose 1
#define RQV_GP2 0x02 // request value for general purpose 2
#define RQV_GP3 0x03 // request value for general purpose 3
#define RQV_GP4 0x04 // request value for general purpose 4
#define RQV_GP5 0x05 // request value for general purpose 5
#define RQV_GP6 0x06 // request value for general purpose 6
#define RQV_GP7 0x07 // request value for general purpose 7
#define RQV_GP8 0x08 // request value for general purpose 8
#define RQV_ANGLE 0x10 // request value for angle

// set parameters, byte 3 when command is SET. Expect data starting from byte 3
#define SET_GP1 0x01 // set value for general purpose 1
#define SET_GP2 0x02 // set value for general purpose 2
#define SET_GP3 0x03 // set value for general purpose 3
#define SET_GP4 0x04 // set value for general purpose 4
#define SET_GP5 0x05 // set value for general purpose 5
#define SET_GP6 0x06 // set value for general purpose 6
#define SET_GP7 0x07 // set value for general purpose 7
#define SET_GP8 0x08 // set value for general purpose 8
#define SET_ANGLE 0x10 // set value for angle. expect uint16_t for angle in byte 4 and 5, in degrees*100 (0-36000)
#define GOTO_ANGLE 0x11 // set value for angle, but with motion profile. Same bytes 4 and 5, bytes 6 and 7 now holds deadline timestamp


// response values, byte 3 when command is RSP
#define RSP_ACK 0x01 // acknowledgment for successful command execution. expect slave id on byte 4
#define RSP_NACK 0x02 // negative acknowledgment for failed command execution. expect error code on byte 4
#define RSP_ERR_FAT 0xFF // fatal error response, trigger fail safe!. expect err code on byte 4






#endif