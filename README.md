# FM stereo radio receiver with RDS

This tuner circuit is a quick prototype which I build to test the *RDA5807M* FM radio tuner IC. *RDA5807M* is a single-chip tuner IC with RDS and MPX decoder, and it equipped with I2C interface for control. 

![Prototype version of the RDA5807M receiver](https://raw.githubusercontent.com/dilshan/rda5807m-receiver/master/resources/rda5807m-radio.jpg)

This receiver builds around Atmel's *ATmega16A* 8-bit MCU. The output stage of this design consists of *AN7147N*, 2×5.3W audio power amplifier. 

Based on the specifications of the switching-regulator and power amplifier, this circuit can drive between 9V to 20V DC input. The most recommended working voltage for this receiver is 12V.

As mentioned above this design is a quick prototype to test the features of *RDA5807M* tuner. Most of the parameters of this tuner are hardcoded in the firmware and not exposed in UI. 

-------------------------------------------------------------------------------------------------------------------------

The firmware source codes of this project are released under the terms of the [MIT license](https://github.com/dilshan/rda5807m-receiver/blob/master/LICENSE). All the design files and schematics are released under the terms of the [Creative Commons - Attribution 4.0 International license](https://creativecommons.org/licenses/by/4.0/). 

