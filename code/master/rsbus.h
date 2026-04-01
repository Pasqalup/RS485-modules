#ifndef RBUS_H
#define RBUS_H
#include <HardwareSerial.h>
#include <cstdint>
#include <cstddef>
class RSBus {
  private:
    enum State {IDLE, RX, TX} state; // internal state of the bus
    uint8_t buf[64]; // buffer for incoming data
	int bufindex; // index for buffer
	bool needToSend; // flag to indicate if we need to send data
	uint8_t tosend[64]; // buffer for outgoing data
	uint8_t sendlen; // length of data to send
	uint8_t responseLen; // length of response data
	bool responseReady; // flag to indicate if response is ready
   public:
    HardwareSerial &serial;   // store reference, not copy
    uint8_t DE;
    USART_TypeDef *port;      // pointer to USART registers
    uint8_t response[64]; // buffer for response data
    bool awaitingResponse;
    // Constructor
    RSBus(HardwareSerial &s, uint8_t D, USART_TypeDef *p)
        : serial(s), DE(D), port(p), state(IDLE), bufindex(0), needToSend(false), responseLen(64), awaitingResponse(false), sendlen(64){}
    void init();
    bool serialReady();
    template <size_t N>
    void startSending(const uint8_t(&data)[N]) {
		responseReady = false; // reset response ready flag for new message
        awaitingResponse = true;
        needToSend = true;
        sendlen = N; // N is automatically deduced!

        size_t copyLen = N > sizeof(tosend) ? sizeof(tosend) : N;
        memcpy(tosend, data, copyLen);
        tosend[1] = sendlen;
    }
    bool isBusy();
    void DELOW();
    int available();
    int read();
    // handle method will be called in loop
	void handle();
	bool hasResponse() const { return responseReady; } // check if we have a response ready
    uint8_t* getResponse() { 
		responseReady = false; // reset flag for next message
        return response; } // get pointer to response buffer
};

#endif