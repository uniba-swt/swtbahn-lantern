# SWTbahn-lantern

The SWTbahn-lantern is a synchronized LED driven point lantern based on an Attiny84A.
It is deployed on the SWTbahn Standard and connected with the BiDiB OneControl.
The OneControl supplies power and a sync signal via the GPIO to the ATtiny.

NOTE: the current working version is src/lantern.c

## Getting started


### Prerequisites

  - ISP- Programmer
  - Arduino IDE or Console + AVR-Toolchain

### Programming

When using Console + AVR-Toolchain:

  1. Configure the make.sh file regarding to your ISP-Programmer
  2. Supply 5V to the lantern and connect the programmer
  3. Run the make.sh file

When using Arduino IDE:

  1. Open lantern.c in Arduino IDE
  2. Select "Tools" in the upper drop-down bar and configure the settings regarding the programmer

  Example:

    ```
    Board: "ATtiny/24/48/84"
    Processor: "ATtiny84"
    Clock: "Internal 8 MHz"
    Port: "COM4"

    ```

  3. Click on compile (tick) and then click on upload (arrow)

If any errors occurs check the power supply or the ISP header.

For more details check out the [SWTbahn documentation](https://vc.uni-bamberg.de/moodle/course/view.php?id=26901 "SWTbahn").
