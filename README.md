# perfect-cell

General purpose firmware for cellular-enabled PSoC motes

### Master:         [![Build Status](http://ec2-13-58-145-29.us-east-2.compute.amazonaws.com:8080/buildStatus/icon?job=open-storm/perfect-cell/master&.png)](http://ec2-13-58-145-29.us-east-2.compute.amazonaws.com:8080/job/open-storm/job/perfect-cell/job/master/)

### Development:    [![Build Status](http://ec2-13-58-145-29.us-east-2.compute.amazonaws.com:8080/buildStatus/icon?job=open-storm/perfect-cell/development&.png)](http://ec2-13-58-145-29.us-east-2.compute.amazonaws.com:8080/job/open-storm/job/perfect-cell/job/development/)


## Installation

First, set up a [remote server](https://github.com/open-storm/docs.open-storm.org/wiki/Setting-up-the-server-environment) to receive data from the sensor node.

Next, download the firmware using git:

```
git clone https://github.com/open-storm/perfect-cell.git
```

Open `perfect-cell.cyprj` using PSoC Creator (4.1 or greater).

Set the following parameters in `config.h` to match your server settings:

```
// Default user info
#define DEFAULT_HOME_USER   "home_user"
#define DEFAULT_HOME_PASS   "home_pass"
#define DEFAULT_HOME_DB     "HOME_DB"
#define DEFAULT_HOME_PORT   8086
#define DEFAULT_HOME_HOST   "ec2-13-58-145-29.us-east-2.compute.amazonaws.com"

// Meta user info
#define DEFAULT_META_USER   "meta_user"
#define DEFAULT_META_PASS   "meta_pass"
#define DEFAULT_META_DB     "META_DB"
#define DEFAULT_META_PORT   8086
#define DEFAULT_META_HOST   "ec2-52-87-156-130.compute-1.amazonaws.com"

...

// SSL parameters
#define SSL_ENABLED            1u
```

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
