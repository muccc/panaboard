uCCC panaboard
==============

Panaboard to Interwebs converter

Content

1. Info
2. Pinout J3
3. Singal Analysis
A. Licence

1. Info
=======

Hardware:
- Panasonic Panaboard KX-B550S
- Raspberry Pi
- 5V/3.3V Level-Converter
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
NEWLINW: 13 us pulse every 7.5ms

Scanning:
---------

Data: 1 = black, 0 = white on rising(?) edge of CLK
CLK:

A. Licence
==========
All Parts (maybe instead of used libraries) are under GPLv2
