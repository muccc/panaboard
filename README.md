uCCC panaboard
==============

Panaboard to Interwebs converter

Content

1. Info
2. Pinout J3
3. Singal Analysis
4. Arduino
5. Picture Info
A. Licence

1. Info
=======

Hardware:
- Panasonic Panaboard KX-B550S
- Arduino Mega
- Raspberry Pi
- Some wireing

Goal:
The Goal of this project is, to grab the printer data during scanning and create an readable output file (png or pdf).
Additionaly it should still be possible to use the internal printer

Notes:
We use a version with a flex cable connection to the printer --> some soldering is needed.

2. Pinout J3
============
1  Data     11 D0
2  GND      12 D1
3  CLK      13 D2
4  GND      14 D3
5  NEWLINE  15 D4
6  GND      16 D5
7  5V       17 D6
8  GND      18 D7
9  NC       19 GND
10 NC       20 PULLUP

3. Singal Analysis
==================

No Scanning:
------------

Data: always zero
CLK: some bursts for 500us after NEWLINE
NEWLINE: 13 us pulse every 7.5ms

Scanning:
---------

Data: 1 = black, 0 = white on rising edge of CLK
CLK: about 650-700 kHz when line is scanned
NEWLINE: 13us pulse just before line gets scanned
D0: Is high somewhere in line for some CLK-cycles when line contains scanned piture

4. Arduino-Mega wireing
=======================

Panaboard | Arduino
---------------------
GND       | GND
Data      | 51 (SPI_MOSI)
CLK       | 52 (SPI_CLK)
NEWLINE   | 3  (INT1)
D0        | 2

5. Picture Info
===============

Size: 1440x1604 (pixel x lines)
Color: BW (1bit)
1st pixel: bottom left
lines: vertical

A. Licence
==========
All Parts (maybe instead of used libraries) are under GPLv2
