# STM32F405 Master Code
## Details
## Dependencies
1. Install [STM32duino](https://github.com/stm32duino/Arduino_Core_STM32) core into Arduino IDE
2. Select Generic STM32F4 Series under Tools>Board>STM32 MCU Based Boards > Generic STM32F4 Series
3. Select Generic STM32F405RGTx under Tools>Board part number>STM32F405RGTx
4. Install [Adafruit Neopixel](https://github.com/adafruit/Adafruit_NeoPixel)
## Use
I made an "RSBus" class to handle basic receiving and transmitting over the RS485.
- During setup(), initialize the bus with bus.init()
- Wait for the serial to initialize with
```
while (!bus1.serialReady()) ;
```
During loop() you must keep in mind several things:
- Call bus.handle() every loop
- Keep loop blocking minimal - make sure the program doesn't "hang" in the middle of a loop. Use state machines and flags instead, so the loop can keep running.
- bus.startSending() does NOT guarantee transmission is finished. You cannot make another transmission until bus.isBusy() is false.
- It is safe to call bus.startSending() while receiving; RSBus will automatically wait until it is finished recieving before transmitting
- bus.hasResponse() returns `true` if there is received data that hasn't been read yet. bus.getResponse() will clear this.
### Example Snippet
```c++
#include <HardwareSerial.h>
#include "commands.h"
#include "rsbus.h"
typedef enum{
  IDLE,
  TX,
  RX,
  DONE,
} State_t;
State_t state = IDLE:
#define S1E PA8 
#define S2E PC8
#define S3E PA4
#define S4E PB12
HardwareSerial Serial1(USART1);
HardwareSerial Serial2(USART6);
HardwareSerial Serial3(USART2);
HardwareSerial Serial4(USART3);
void init(){
    bus1.init();
    bus2.init();
    bus3.init();
    bus4.init();

    while(!bus1.serialReady() || !bus2.serialReady() || !bus3.serialReady() || !bus4.serialReady()){
        ;// wait for all serial ports to be ready
    }
}
void loop(){
  switch(state){
      case IDLE:
          uint8_t cmd[3] = {1, 3, SCN}; //scan for slave id 1
          bus1.startSending(cmd); // on bus 1
          state = RX; // move to waiting for response
          break;
      case RX:
          if(bus1.hasResponse()){
              uint8_t* response = bus1.getResponse(); // we have a response
              if (response[2] == RSP && response[3] == RSP_ACK && response[4] == 1){ // response acknowledge confirming slave 1
                  state = DONE; // valid response
              }
          }
          break;
      case DONE:
          ; //do nothing
          break;
      case TX:
          ;//idk
          break;
  }

}


```
