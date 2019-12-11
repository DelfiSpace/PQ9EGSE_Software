# PQ9EGSE Software

This repository contains the software running on the PQ9EGSE board. 

## USB chipset configuration

The USB chipset (FT232RT) needs to be configured using FT_PROG from FTDI. This configuration is required for:
- selecting the PWREN# pin
- selecting the RXLED and TXLED pins
- enabling pull-down on the GPIO pins
- set the device as bus powered
- selecting max current consumption of 90 mA
