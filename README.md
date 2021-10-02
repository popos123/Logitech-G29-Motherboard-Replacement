# Logitech-G29-Motherboard-Replacement
This is a new edition of the motherboard from Logiteh G29 using EMC software

How to build own Logitech G29 Motherboard

What you need:
- 2x Bluepill (F103) or Blackpill (F103) or better F401 (for my board you can use even Arduino Leonardo, just change pins in src)
- 9x resistors 10K
- 9x tranzistors NPN (eg. BC546)
- 1x DC motor driver (eg. BTS7960B, CYTRON MD30C or cheaper some driver for couple Amps for couple $)
- couple wires :)

Simple:
(if you are using STM32 prefer to uplada a code via ST Link using ST Link Utility)
1. Upload the hex from the my repository to the one board controlling - EMC software.
2. Upload the hex from this repository to the my emulator board - G29emu

To bulid a code just use the Arduino ide and paste a code from main.cpp and attach the libraries. You can use platform io too.

Simple presentation of the working system:
https://youtu.be/wHjuSKlw6mE

In F103 version the Play Station button is centering the wheel, in F401 there is no option yet (1.07 software)
In F103 is a bug in software - EEPROM doesnt work properly, need to run software every reset / turn off the steering wheel.
So in version 1.07 is working EEPROM but no centering button, but in version 1.06 is opposite.
