# Distance-Calculator
This program calculates the distance between ultrasonic sensor and the nearest boundary using Tiva C.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

## Prerequisites

You need to have TM4C123GXL kit. You can order from [here](http://www.ti.com/tool/EK-TM4C123GXL). Also you need to 
have 3 7-Sigma displays and ultrasonic sensor. 

For the software, you can use Code Composer Studio (CCS) Integrated Development Environment (IDE).
You can download from [here](http://www.ti.com/tool/CCSTUDIO-TM4X).

## Deployment

Just import the main.c file inside a new project and click Run.

## How code works

The program interface TivaC with ultrasonic sensor. It sends pulse and waits to receive it. 
The elapsed time to send and receive is used to determine the distance. Then the result appears on 7 sigma display.

## Test and results

I tested the accuracy of the program till 100 cm distance and the results was 100% accurate. Over 100 cm, it begins to have an error between +/- 1 cm.
