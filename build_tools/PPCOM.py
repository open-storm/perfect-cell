#********************************************************************************
#* Copyright (2011), Cypress Semiconductor Corporation.
#********************************************************************************
#* This software is owned by Cypress Semiconductor Corporation (Cypress)
#* and is protected by and subject to worldwide patent protection (United
#* States and foreign), United States copyright laws and international treaty
#* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
#* non-transferable license to copy, use, modify, create derivative works of,
#* and compile the Cypress Source Code and derivative works for the sole
#* purpose of creating custom software in support of licensee product to be
#* used only in conjunction with a Cypress integrated circuit as specified in
#* the applicable agreement. Any reproduction, modification, translation,
#* compilation, or representation of this software except as specified above
#* is prohibited without the express, written permission of Cypress.
#********************************************************************************

class enumeCanPowerDevice:
	CAN_MEASURE_POWER             =0x4        # from enum enumCanPowerDevice
	CAN_POWER_DEVICE              =0x1        # from enum enumCanPowerDevice
	CAN_READ_POWER                =0x2        # from enum enumCanPowerDevice
	CAN_MEASURE_POWER_2           =0x8        # from enum enumCanPowerDevice

class enumCanProgram:
	CAN_PROGRAM_CARBON            =0x1        # from enum enumCanProgram
	CAN_PROGRAM_ENCORE            =0x2        # from enum enumCanProgram

class enumInterfaces:
	I2C                           =0x4        # from enum enumInterfaces
	ISSP                          =0x2        # from enum enumInterfaces
	JTAG                          =0x1        # from enum enumInterfaces
	SWD                           =0x8        # from enum enumInterfaces
	SPI                           =0x16       # from enum enumInterfaces

class enumFrequencies:
	FREQ_01_5                     =0xc0       # from enum enumFrequencies
	FREQ_01_6                     =0x98       # from enum enumFrequencies
	FREQ_03_0                     =0xe0       # from enum enumFrequencies
	FREQ_03_2                     =0x18       # from enum enumFrequencies
	FREQ_06_0                     =0x60       # from enum enumFrequencies
	FREQ_08_0                     =0x90       # from enum enumFrequencies
	FREQ_12_0                     =0x84       # from enum enumFrequencies
	FREQ_16_0                     =0x10       # from enum enumFrequencies
	FREQ_24_0                     =0x4        # from enum enumFrequencies
	FREQ_48_0                     =0x0        # from enum enumFrequencies
	FREQ_RESET                    =0xfc       # from enum enumFrequencies

class enumI2Cspeed:
	CLK_100K                      =0x1        # from enum enumI2Cspeed
	CLK_400K                      =0x2        # from enum enumI2Cspeed
	CLK_50K                       =0x4        # from enum enumI2Cspeed

class enumSonosArrays:
	ARRAY_ALL                     =0x1f       # from enum __MIDL___MIDL_itf_PSoCProgrammerCOM_0000_0001
	ARRAY_EEPROM                  =0x2        # from enum __MIDL___MIDL_itf_PSoCProgrammerCOM_0000_0001
	ARRAY_FLASH                   =0x1        # from enum __MIDL___MIDL_itf_PSoCProgrammerCOM_0000_0001
	ARRAY_NVL_FACTORY             =0x8        # from enum __MIDL___MIDL_itf_PSoCProgrammerCOM_0000_0001
	ARRAY_NVL_USER                =0x4        # from enum __MIDL___MIDL_itf_PSoCProgrammerCOM_0000_0001
	ARRAY_NVL_WO_LATCHES          =0x10       # from enum __MIDL___MIDL_itf_PSoCProgrammerCOM_0000_0001

class enumUpgradeFirmware:
	FINALIZE                      =0x3        # from enum enumUpgradeFirmware
	INITIALIZE                    =0x0        # from enum enumUpgradeFirmware
	UPGRADE_BLOCK                 =0x1        # from enum enumUpgradeFirmware
	VERIFY_BLOCK                  =0x2        # from enum enumUpgradeFirmware

class enumValidAcquireModes:
	CAN_POWER_CYCLE_ACQUIRE       =0x2        # from enum enumValidAcquireModes
	CAN_POWER_DETECT_ACQUIRE      =0x4        # from enum enumValidAcquireModes
	CAN_RESET_ACQUIRE             =0x1        # from enum enumValidAcquireModes

class enumVoltages:
	VOLT_18V                      =0x8        # from enum enumVoltages
	VOLT_25V                      =0x4        # from enum enumVoltages
	VOLT_33V                      =0x2        # from enum enumVoltages
	VOLT_50V                      =0x1        # from enum enumVoltages
