# perfect-cell

General purpose firmware for cellular-enabled PSoC motes

[![Build Status](http://3936cf9b.ngrok.io/buildStatus/icon?job=perfect-cell%20CI&.png)](http://3936cf9b.ngrok.io/job/perfect-cell%20CI/)

## Installation

First, set up a [remote server](https://github.com/open-storm/docs.open-storm.org/wiki/Setting-up-the-server-environment) to receive data from the sensor node.

Next, download the firmware using git:

```
git clone https://github.com/open-storm/perfect-cell.git
```

Open `perfect-cell.cyprj` using PSoC creator.

Use PSoC Creator to [flash the firmware](https://github.com/open-storm/docs.open-storm.org/wiki/Using-PSoC-Creator) to a compatible device.

Schematics for the board can be found [here](https://github.com/open-storm/open-storm-hardware).

## Overview

The `perfect-cell` firmware contains a simple operating system that performs the following routines at a recurring interval.

* ''wakes'' the board from sleep mode
* takes measurements using the attached sensors
* connects to the cellular network
* sends sensor data to a web server
* updates device metadata and onboard parameters
* triggers attached actuators
* puts the board back into sleep mode.

![Main](https://s3.us-east-2.amazonaws.com/mdbartos-img/open-storm/main_diagram.svg)

An overview of the firmware can be found in the _open-storm_ [docs](https://github.com/open-storm/docs.open-storm.org/wiki/Firmware).

Auto-generated doxygen documentation can also be found [here](http://open-docs.s3-website-us-west-2.amazonaws.com).
