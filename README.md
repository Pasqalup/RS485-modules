# RS485-modules
<img width="520" alt="zine page" src="assets/Zine%20page.png" />    

## What is this?
This is a modular electronics system I made based on RS485. Using RS485 allows you to daisy chain up to **32 devices**[^1] on a single bus!     
The master board I've made is powered by an **STM32F405** and has **4 independent RS485 buses**, for a total of up to **128 devices**[^1]!    
    
I initially began this project when I wanted a modular control system for a 6-axis robotic arm. I designed individual closed-loop stepper motor driver PCBs and decided to use RS485 to connect them all to a master controller board. When I realized I also needed to add a grabber servo, I had the idea of turning this into a full modular control system.

## How do you use it?
On each module, there are two **2-pin JST-PH** connectors for RS485 and two **3-position screw terminals** providing 12-24v, 5v, and GND.     
You can daisy-chain up to 32 modules on a single bus by wiring each connector to the next module.       
Refer to the [coding page](code/) for software information.     
Refer to the [master board page](pcbs/master/) for more specifics.    

     
![master board](pcbs/master/image.png)





[^1]: If you use the **MAX1487** instead, you can put 128 devices per bus and 512 devices total. Overkill!
