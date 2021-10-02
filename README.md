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
1. Upload the hex from the my repository to the one board controlling - EMC software.
2. Upload the hex from this repository to the my emulator board - G29emu
