#include <Arduino.h>
#include "rsbus.h"
#define BAUD 115200 
// RSBus::RSBus(HardwareSerial& s, uint8_t D, USART_TypeDef* p)
//     : serial(s), DE(D), port(p), state(IDLE), bufindex(0) {
// }
void RSBus::init() {
    pinMode(DE, OUTPUT); // Set driver enable to OUTPUT
    digitalWrite(DE, LOW); // Immediately set LOW
    serial.begin(BAUD); // Start serial communication
}
bool RSBus::serialReady() {
    return serial;
}
// if startsending() is called while state=RX, we are cooked
// move serial.write() into handle() and startsending() just sets a flag and copies data to a buffer. handle() will check the flag and send data when state=TX. This way, we can avoid blocking in startsending() and handle() can manage the state transitions more effectively.
// 
// 
// THIS HAS BEEN MOVED TO A TEMPLATE IN HEADER FILE
// ALLOWS FOR AUTOMATIC DEDUCTION OF DATA LENGTH
//void RSBus::startSending(const uint8_t* data, uint8_t len) {
//
//	awaitingResponse = true; // data is sent but still awaiting response, so set flag to true. start wait here instead of only after transmission is complete to avoid polling for response before we have even sent the data.
//	needToSend = true; // set flag to indicate we need to send
//	sendlen = len; // store length of data to send
//	memcpy(tosend, data, sizeof(tosend)); // copy data to tosend buffer
//	tosend[1] = len; // automatically add length
//}
bool RSBus::isBusy() {
	return !(port->SR & USART_SR_TC) | needToSend; // either still transmitting or havent begun transmission of pending data.
}
void RSBus::DELOW() {
    digitalWrite(DE, LOW);
}
int RSBus::available() {
    return serial.available();
}
int RSBus::read() {
    return serial.read();
}
void RSBus::handle() {
    switch (state) {
    case IDLE:
        // waiting for data
        if (available()) { 
            state = RX; 
			responseLen = 64; // reset response length for new message
        }
		if (needToSend) { // will only begin send when in IDLE
			digitalWrite(DE, HIGH); // set DE HIGH to start transmission
            serial.write(tosend, sendlen); // send data !!!!! this can fail silently and cause issues with incomplete transmission.
			needToSend = false; // data is set now; reset flag.
			state = TX; // switch to TX state to wait for transmission to complete
        }
        break;
    case RX:
        // read incoming data
                    /*
                * commands.h - definitions for commands and RSBus class for STM32 master-slave communication
                byte 0: target (slave ID), 255 for broadcast, 0 for master
                byte 1: length of data (=n)
                byte 2: command
                byte 3: parameter (if needed), otherwise data starts from byte 2
                byte 4...n: data (if needed)
            */
        if (available()) {
            uint8_t byte = read();
            buf[bufindex++] = byte; // store into buffer
            // !!!!!!!!!
			// BUFINDEX IS NOW +1 BECAUSE OF POST INCREMENT, SO IT POINTS TO THE NEXT POSITION TO WRITE INTO, NOT THE LAST BYTE WE JUST READ.

            if (bufindex == 2) { // because of post increment, reading byte 1 means bufindex is now 2
                //byte 1: length of data, not relying on \n
				responseLen = byte; // length of FULL FRAME
            }
            // when (++)bufindex = responselen, bufindex-1 is what was just read, which is the last byte
            // 0x01 0x04 0xnn 0xnn
            // responselen = 4
			// bufindex: 0 1 2 3
            // reach here after we read at buf[3]. bufindex is now 4 because of post increment
			// bufindex == responselen, so we have read the full frame. We can process it now.
			if (bufindex == responseLen || bufindex >= sizeof(buf)) { // end of message or buffer overflow
                if (buf[0] == 0) { // if first byte is 0, it's addressed to master. 
                    memcpy(response, buf, bufindex); // copy buffer to response
					awaitingResponse = false; // we have received a response, so set flag to false
					responseReady = true; // set response ready flag
                } // otherwise, ignore message
                bufindex = 0; // reset index for next message
                state = IDLE; // reset state to IDLE
            }
        }
        break;
    case TX:
        if (!isBusy()) { // wait for transmission to complete
            DELOW(); // set DE LOW after transmission is complete
            state = IDLE; // reset state to IDLE
        }
        break;
    }
}