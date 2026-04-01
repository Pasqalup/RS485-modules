#include <Adafruit_NeoPixel.h>

// 9600,
#include <HardwareSerial.h>
#include "commands.h"
#include "rsbus.h"

#define MAX_SLAVES_PER_BUS 10
/*
    * commands.h - definitions for commands and RSBus class for STM32 master-slave communication
    byte 0: target (slave ID), 255 for broadcast, 0 for master
    byte 1: length of data (=n)
    byte 2: command
    byte 3: parameter (if needed), otherwise data starts from byte 2
    byte 4...n: data (if needed)

*/

typedef enum{
  SCANNING,
  IDLE,
  DONE_SCANNING,
  ERROR,
  WORKING,
} CMD_state_t; // command state type; what is the master doing?
CMD_state_t cmd_state = SCANNING;
typedef enum{
  TX,
  RX,
  DONE,
} SUBSTATE_t; // substate type; whether we are waiting for transmission to complete or waiting for response after transmission
SUBSTATE_t scanning_sbstate = TX; // substate for scanning process
typedef struct{
  uint8_t bus; // which bus the slave is on, 1-4
  uint8_t id;
  uint8_t type; // type of slave, can be used to determine what commands to send
  bool active; // whether the slave is active, determined by whether we receive a response from it during scanning
} SlaveInfo;

typedef struct {
    uint8_t id;
    uint8_ bus;
    uint8_t command;
    uint8_t param;
    uint8_t data[54];
    uint8_t dataLen;
    bool expectsResponse;

} JobInfo;

JobInfo jobQueue[20]; // simple job queue for commands to be sent to slaves. In a real implementation, this would likely be more complex and dynamic.


#define S1E PA8 
#define S2E PC8
#define S3E PA4
#define S4E PB12
HardwareSerial Serial1(USART1);
HardwareSerial Serial2(USART6);
HardwareSerial Serial3(USART2);
HardwareSerial Serial4(USART3);

RSBus bus1(Serial1, S1E, USART1);
RSBus bus2(Serial2, S2E, USART6);
RSBus bus3(Serial3, S3E, USART2);
RSBus bus4(Serial4, S4E, USART3);

Adafruit_NeoPixel leds = Adafruit_NeoPixel(3, PB8, NEO_GRB + NEO_KHZ800);


void setup() {
    strip.begin();
    bus1.init();
    bus2.init();
    bus3.init();
    bus4.init();

    while(!bus1.serialReady() || !bus2.serialReady() || !bus3.serialReady() || !bus4.serialReady()){
    ;// wait for all serial ports to be ready
    }



    // start scanning for slaves immediately
    cmd_state = SCANNING;
    scanning_sbstate = TX; // start with transmitting scan command

}

/*
* master will always talk first; slaves will only respond
* master will send a command to a slave, then wait for buf
* responses will be sent in one line, ending with \n

*/
void handleAllBuses(){
  bus1.handle();
  bus2.handle();
  bus3.handle();
  bus4.handle();
}

void led_setStatus(uint8_t status) {
    switch (status) {
    case 0: // all good
		strip.setPixelColor(0, strip.Color(0, 255, 0)); // green
	case 1: // warning
        strip.setPixelColor(0, strip.Color(255, 255, 0)); // yellow
		break;
    case 2: // error
		strip.setPixelColor(0, strip.Color(255, 0, 0)); // red
    break;
    default:
		strip.setPixelColor(0, strip.Color(255, 0, 255)); // magenta for unknown status
}
// tx a string over bus 4 for debug
// only for testing, will remove later
void telemetry(const uint8_t* str){
  uint8_t len = strlen((const char*)str);
  uint8_t msg[64];
  msg[0] = 0x00; // won't accidentally be interpreted as a slave id
  msg[1] = len;
  msg[2] = 0xFF; // command for telemetry, won't be interpreted
  memcpy(&msg[3], str, len);
  bus4.startSending(msg); // send telemetry data over bus 4
}

uint32_t time_of_last_sync = 0;
void sync(){
	time_of_last_sync = millis();
	// send sync command to all slaves, passing current time for synchronization
	uint32_t currentTime = millis();
	uint8_t syncMsg[7];
	syncMsg[0] = 0xFF; // broadcast
	syncMsg[1] = 7; // length of data
	syncMsg[2] = 0x05; // sync command
	memcpy(&syncMsg[3], &currentTime, sizeof(currentTime)); // copy current time into message
	bus1.startSending(syncMsg);
	bus2.startSending(syncMsg);
	bus3.startSending(syncMsg);
	bus4.startSending(syncMsg);
}
#define TOTAL_SLAVES 4
SlaveInfo slaveList[TOTAL_SLAVES] = { // Initialize slave list with default values
  {1, 1, 1, false},
  {2, 2, 2, false}, 
  {3, 3, 2, false}, 
  {3, 4, 1, false},
};
//only populated with real slaves
uint8_t scanningidx = 0;
uint8_t currentBus = 1;
SlaveInfo currentSlave;
uint32_t lasttim = 0;
RSBus* buses[] = {&bus1, &bus2, &bus3, &bus4}; 
void handleScan(){
  if(scanning_sbstate == DONE){
    cmd_state = DONE_SCANNING
  }

  switch (scanning_sbstate){
      case TX:
        {
          if (scanningidx >= TOTAL_SLAVES) {
            scanning_sbstate = DONE;
            break;
          }

          currentSlave = slaveList[scanningidx]; // get current slave
          if(currentSlave.active){ // if slave is already active, skip
            scanningidx++; // continue to next slave
            if(scanningidx >= TOTAL_SLAVES){ // last slave is active, we are done scanning
              scanning_sbstate = DONE;
            }
            break;
          }
          uint8_t scanCmd[3] = {scanningidx+1, 0x00, SCN}; // 0xnn slave id, 0x00 length placeholder, scn command
          buses[currentSlave.bus - 1]->startSending(scanCmd); // send scan command on the appropriate bus
          lasttim = millis(); // record time of transmission to check for timeout
          // wait for response in handle method; response will be processed there and update slaveList
          scanning_sbstate = RX; // move to waiting response state

        }
        break;
      case RX:
          if(buses[currentSlave.bus - 1]->hasResponse()){
            uint8_t* response = buses[currentSlave.bus - 1]->getResponse();
            if(response[2] == RSP && response[3] == RSP_ACK && response[4] == currentSlave.id){
              // valid response from slave
              slaveList[scanningidx].active = true; // mark slave as active
            } else{
              // invalid response, keep slave as inactive
            }
            scanningidx++;
            if(scanningidx >= TOTAL_SLAVES){
              scanning_sbstate = DONE; // move to idle state after scanning all slaves
            } else{
              scanning_sbstate = TX; // scan next slave
            }
          } else{
            //no response! check for timeout
            if(millis() - lasttim > 5){ // if it's been more than 5ms since we sent the scan command, consider it a timeout and move on to next slave. 
              //5ms is plenty of time for high speed uart. if it actually takes more than 5ms, its too slow so consider it failed
              // timeout, move on to next slave
              scanningidx++;
              if(scanningidx >= TOTAL_SLAVES){
                scanning_sbstate = DONE; // move to idle state after scanning all slaves
              } else{
                scanning_sbstate = TX; // scan next slave
              }
            }
            ;
            ;
			// no timeout yet, keep waiting for response

          }

        break;
      default:
      break;
    }
}
/*
1. SCAN - ping each slave, confirm working. must wait for timeout <100ms
2. DONE_SCANNING - verify or throw error. 0ms
=========== ALL SLAVES ARE WORKING; 
*/

void loop() {
  handleAllBuses();

  switch (cmd_state){
     case IDLE:
         // not currently transmitting or recieveing
         // Here we do checks and whatnot and look for actions to perform

		 if (time_of_last_sync == 0 || millis() - time_of_last_sync > 1000) { // if we havent synced yet or its been more than 1s since last sync, sync again
          sync();
         }

        break;
    case SCANNING:
      handleScan();
      break;
    case DONE_SCANNING:
		// for testing, print out active slaves over telemetry
		bool allGood = true;
        for(int i = 0; i < TOTAL_SLAVES; i++){
            if(slaveList[i].active){
              char buf[32];
              sprintf(buf, "Slave %d active on bus %d\n", slaveList[i].id, slaveList[i].bus);
              telemetry((const uint8_t*)buf);
            }
			if (slaveList[i].active == false) {
              char buf[32];
              sprintf(buf, "Slave %d inactive on bus %d\n", slaveList[i].id, slaveList[i].bus);
              telemetry((const uint8_t*)buf);
			  allGood = false;
              // throw an error!
            }
		}
		if (!allGood) {
          cmd_state = ERROR; // if any slave is inactive, set state to error
          led_setStatus(2); // set LED to red for error
        } else{
          cmd_state = IDLE; // if all slaves are active, we are good to go!
          led_setStatus(0); // set LED to green for all good
        }
        
      break;
    case ERROR:
        ; //dont do anything! somethings wrong!
		led_setStatus(2); // set LED to red for error
		// display error code on LEDs or something?
		break;
  }
}
