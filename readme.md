# swtbahn-lantern

The SWTbahn platforms use lanterns to indicate the current positions of the
servo-driven points. A lantern has constant brightness if its associated point
is in the normal position, and has pulsing brightness if its point is in the reverse
position. The pulsing effect of all lanterns are synchronised via a synchronisation
pulse generated from the OneControl boards.

The SWTbahn Standard uses an ATtiny84A to control the lantern LED (powered from
the OneControl GPIO ports), while the SWTbahn Full uses an Adafruit Metro Mini
(powered externally by a 5V supply).


## Prerequisites

  * Programmer for the ATtiny: [STK500](doc/stk500-isp-programmer.pdf)
  * [Arduino IDE](https://www.arduino.cc/en/Main/Software)
  * [ATtiny board support for Arduino](https://github.com/damellis/attiny)
    1. Open the Arduino IDE
    2. Go to the settings and add the following board manager URL: https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json
    3. Go to Tools > Boards > Board Manager
    4. Search for "attiny" by David A. Mellis and install the board support package


## Programming with Arduino IDE

### ATtiny84A
1. Open `attiny84a.ino` in Arduino IDE
2. Plug the STK500 programmer into the computer, and record the serial port it has been assigned
3. Select Tools in the menu bar and configure the following settings:
   * Board: ATtiny/24/48/84
   * Processor: ATtiny84
   * Clock: Internal 8 MHz
   * Port: Whichever port the programmer has been assigned
   * Programmer: Atmel STK500 development board
4. Plug the STK500 programmer to the ATtiny84A
5. Compile and upload to the ATtiny84A  


### Adafruit Metro Mini
1. Download and install the USB serial drivers
   * [Virtual COM port drivers](http://www.ftdichip.com/Drivers/VCP.htm)
   * [USB to UART bridge VCP drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads)
2. Open `adafruit-metro-mini.ino` in Arduino IDE
3. Plug the Adafruit Metro Mini into a USB port of your computer, and record the serial port it has been assigned
4. Select Tools in the menu bar and configure the following settings:
   * Board: Arduino UNO
   * Port: Whichever port the programmer has been assigned
   * Programmer: USBtinyISP
5. Compile and upload to the Adafruit Metro Mini


## Notes on Setting the ATtiny84A Interrupt and PWM Timer

Interrupt for the synchronisation pulse pin (see [Datasheet p. 50](doc/attiny84a-datasheet.pdf))

```
GIMSK = (1 << PCIE1); // Pin Change Interrupt Enable 1
GIFR = (1 << PCIF1); // Pin Change Interrupt Flag 1
PCMSK1 = (1 << SYNC_INT); //Pin Change Enable Mask 9
```

PWM for the glowing effect (see [Datasheet p. 85](doc/attiny84a-datasheet.pdf))

```
// Timer/Counter 1 at 800
TCCR1A = 0; // Clear register TCCR1A
TCCR1B = 0; // Clear register TCCR1B
TCNT1  = 0; // Reset counter value
// Set compare match register for 600Hz increments
OCR1A = 40;  // = 8MHz/(3kHz*64 prescaler) - 1, (OCR0A value must be less than 65536)
TCCR1B |= (1 << WGM12); // Enable Clear Timer on Capture mode
TCCR1B |= (1 << CS11) | (1 << CS10);  // Prescaler of 64
TIMSK1 |= (1 << OCIE1A);  // Enable timer compare interrupt
```
