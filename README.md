﻿# Waves-that-clean-V1

 This repository contains de Firmware for the first prototype of a Solar Panel Cleaner Robot. It's based on Arduino Nano, programed with PlatformIO framework. 

 This Firmware is designed to implemnt an Standalone rutine, it comunicates through simple comands with the PC and does not have implemented the comands to work with the Software-GUI developed by CCTVal Team. 

 ### Some trouble shootings 

 1. Depending when and to whom you buy an Arduino Nano, the bootloader can change. Refer to the file platofrmio.ini and change the parameter board = nanoatmega328 to board = nanoatmega328new, or viceversa. 
