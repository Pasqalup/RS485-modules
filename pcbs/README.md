# Making your own slave module
## Requirements
### Microcontroller
 - Must have at least 1 UART/USART peripheral
 - CH32V003 recommended (This is what I use)
### Serial
 - 2x 2-pin JST-PH conectors for RS485
 - MAX485 RS485 transciever
![max485 schematic](../assets/max485_wiring.png)
 - If you plan to use the template code provided, connect URX and UTX to the ***Remapped*** pins on the CH32 [^1]
 ![URX_, UTX_](../assets/uart_wiring.png)
### Power
 - 2x 3-pin KF301 Screw terminal connectors
 ![24v on left](../assets/pwr%20order.png)
 - Order will *always* be &emsp; **+24V/VCC, &emsp; +5V,  &emsp; GND** &emsp; from **left to right**
 - Power your microcontroller using the **+5V** rail
 - Power any high power electronics using the **+24V/VCC** rail.
 - Regardless if the rails are being used, you **MUST** connect the input terminals to the output terminals with high current **thick traces** or **copper pours**.
 ![thick traces](../assets/thick%20traces.png)

### Software
 - Master and Slaves must match baud rate (default 115200)
 - Maximum command length is 64 bytes
 - Refer to [commands.h](../code/slave/commands.h) for command details



[^1]: No particular reason for this, I just wired them based on remap and coded accordingly