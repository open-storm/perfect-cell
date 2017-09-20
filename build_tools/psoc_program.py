#******************************************************************************
#* Copyright (2009), Cypress Semiconductor Corporation.
#******************************************************************************
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
#******************************************************************************

import win32com.client
import array
import PPCOM
from PPCOM import enumInterfaces
from PPCOM import enumFrequencies
from PPCOM import enumSonosArrays
import sys
import os

#Define global variables
m_sLastError = ""
PY3 = sys.version_info.major == 3

#Distinguishing identifier of PSoC3/5 families
LEOPARD_ID = 0xE0;
PANTHER_ID = 0xE1;

def SUCCEEDED(hr):
    return hr >= 0

def GetGenerationByJtagID(hr):
    return hr >= 0

#Check JTAG ID of device - identify family PSoC3 or PSoC5
def GetGenerationByJtagID(JtagID):
    if PY3:
        JtagID = [chr(c) for c in JtagID]
    distinguisher = (((ord(JtagID[0]) & 0x0F) << 4) | (ord(JtagID[1]) >> 4))
    return distinguisher

def IsPSoC3ES3(jtagID):
    global LEOPARD_ID
    if PY3:
        jtagID = [chr(c) for c in jtagID]
    if (GetGenerationByJtagID(jtagID) == LEOPARD_ID):
        if ((ord(jtagID[0]) >> 4) >= 0x01): return 1  #silicon version==0x01 saved in bits [4..7]
                                                 #For ES0-2==0x00, ES3==0x01
    return 0

def Is_PSoC5_TM_ID(jtagID):
    if PY3:
        jtagID = [chr(c) for c in jtagID]
    return ((ord(jtagID[0]) & 0xFF) == 0x0E) and ((ord(jtagID[1]) & 0xF0) == 0x10) and (ord(jtagID[3]) == 0x69)

def Is_PSoC_5_LP_ID(jtagID):
    #Check whether SWD ID belongs to PSoC5LP
    #PSoC5LP: 0x1BA01477 (SWD/JTAG read ID read request retuns CM3 ID always)
    #         0x2E1xx069 (LP) - this Device ID must be read from PANTHER_DEVICE_ID reg (0x4008001C)
    #2E0xx069
    if PY3:
        jtagID = [chr(c) for c in jtagID]
    return (((ord(jtagID[0]) & 0xFF) >= 0x2E) and ((ord(jtagID[1]) & 0xF0) == 0x10) and (ord(jtagID[3]) == 0x69))

def OpenPort():
    global m_sLastError
    # Open Port - get last (connected) port in the ports list
    hResult = pp.GetPorts()
    hr = hResult[0]
    portArray = hResult[1]
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)): return hr
    if (len(portArray) <= 0):
        m_sLastError = "Connect any Programmer to PC"
        return -1
    bFound = 0
    for i in range(0, len(portArray)):
        if (portArray[i].startswith("MiniProg3") or portArray[i].startswith("DVKProg") or portArray[i].startswith("FirstTouch") or portArray[i].startswith("Gen-FX2LP") or portArray[i].startswith("KitProg")):
            portName = portArray[i]
            print('FOUND DEVICE:', portName)
            bFound = 1;
            break
    if(bFound == 0):
        m_sLastError = "Connect any MiniProg3/DVKProg/FirstTouch/Gen-FX2LP/KitProg device to the PC"
        return -1

    #Port should be opened just once to connect Programmer device (MiniProg1/3,etc).
    #After that you can use Chip-/Programmer- specific APIs as long as you need.
    #No need to repoen port when you need to acquire chip 2nd time, just call Acquire() again.
    #This is true for all other APIs which get available once port is opened.
    #You have to call OpenPort() again if port was closed by ClosePort() method, or
    #when there is a need to connect to other programmer, or
    #if programmer was physically reconnected to USB-port.

    hr = pp.OpenPort(portName)
    m_sLastError = hr[1]
    return hr[0]

def ClosePort():
    hResult = pp.ClosePort()
    hr = hResult[0]
    strError = hResult[1]
    return hr

def Allow_Hard_Reset():
    listResult = []
    hResult = pp.DAP_GetJtagID();
    hr = hResult[0]
    chipJtagID = hResult[1]
    m_sLastError = hResult[2]
    if not SUCCEEDED(hr):
        listResult.append(hr)
        listResult.append(result)
        return listResult
    #Apply to PSoC5 LP only
    if Is_PSoC_5_LP_ID(chipJtagID):
        #Set 'allow_rst_hrd' bit in MLOGIC_DEBUG register (see PSoC5 IROS: 001-43078, Book 2)
        hr = pp.DAP_WriteIO(0x400046E8, 0x00000040)
    return hr;

def CheckHexAndDeviceCompatibility():
    global m_sLastError
    listResult = []
    result = 0
    hResult = pp.DAP_GetJtagID();
    hr = hResult[0]
    chipJtagID = hResult[1]
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)):
        listResult.append(hr)
        listResult.append(result)
        return listResult
    hResult = pp.HEX_ReadJtagID();
    hr = hResult[0]
    hexJtagID = hResult[1]
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)):
        listResult.append(hr)
        listResult.append(result)
        return listResult
    result = 1

    if PY3:
        hexJtagID = [chr(c) for c in hexJtagID]
        chipJtagID = [chr(c) for c in chipJtagID]

    for i in range(0, 4):
        if(ord(hexJtagID[i]) != ord(chipJtagID[i])):
            result = 0
            break
    listResult.append(0)
    listResult.append(result)
    return listResult

def IsNvlUpdatePossible():
    global m_sLastError
    listResult = []
    hResult = pp.GetPower1()
    hr = hResult[0]
    power = hResult[1]
    voltage = hResult[2]
    state = 0
    if (not SUCCEEDED(hr)):
        listResult.append(hr)
        listResult.append(0)
        return listResult
    if (voltage > 3700):
        state = 0
    else:
        state = 1
    listResult.append(hr)
    listResult.append(state)
    return listResult

def ProgramNvlArrays(nvlArrayType):
    global m_sLastError
    hResult = pp.PSoC3_GetSonosArrays(nvlArrayType)
    hr = hResult[0]
    arrayInfo = hResult[1]
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)): return hr
    addrHex = 0
    for i in range(0, len(arrayInfo[0])):
        arrayID = arrayInfo[0][i]
        arraySize = arrayInfo[1][i]
        #Read data from Hex file
        if (nvlArrayType == enumSonosArrays.ARRAY_NVL_USER):
            hResult = pp.PSoC3_ReadHexNvlCustom(addrHex, arraySize)
        else: #enumSonosArrays.ARRAY_NVL_WO_LATCHES
            hResult = pp.PSoC3_ReadHexNvlWo(addrHex, arraySize)
        hr = hResult[0]
        hexData = hResult[1]
        m_sLastError = hResult[2]
        if (not SUCCEEDED(hr)): return hr
        addrHex += arraySize
        #Read data from device
        hResult = pp.PSoC3_ReadNvlArray(arrayID)
        hr = hResult[0]
        chipData = hResult[1]
        m_sLastError = hResult[2]
        if (not SUCCEEDED(hr)): return hr
        #Compare data from Chip against corresponding Hex-block
        if (len(chipData) != len(hexData)):
            m_sLastError = "Hex file's NVL array differs from corresponding device's one!"
            return -1
        fIdentical = 1
        if PY3:
            chipData = [chr(c) for c in chipData]
            hexData = [chr(c) for c in hexData]
        for i in range(0, arraySize):
            if (ord(chipData[i]) != ord(hexData[i])):
                fIdentical = 0
                break
        if (fIdentical == 1): continue #Arrays are equal, skip programming, goto following array
        #Verify voltage range for TM silicon. Do not allow TM programming if voltage is greater than 3.3 V
        hResult = pp.DAP_GetJtagID()
        hr = hResult[0]
        chipJtagID = hResult[1]
        m_sLastError = hResult[2]
        if (not SUCCEEDED(hr)): return hr
        if (Is_PSoC5_TM_ID(chipJtagID)):
            hResult = IsNvlUpdatePossible()
            hr = hResult[0]
            state = hResult[1]
            if (state == 0):
                if(nvlArrayType == enumSonosArrays.ARRAY_NVL_USER):
                    m_sLastError = "User NVLs"
                else:
                    m_sLastError = "WO NVLs"
                m_sLastError = m_sLastError + " update failed. This operation can be completed in voltage range 1.8 - 3.3 V only."
                return -1
        #Program NVL array
        hResult = pp.PSoC3_WriteNvlArray(arrayID, hexData);
        hr = hResult[0]
        m_sLastError = hResult[1]
        if (not SUCCEEDED(hr)): return hr

        #PSoC3 ES3 support - check whether ECCEnable bit is changed and reacquire device
        if (nvlArrayType == enumSonosArrays.ARRAY_NVL_USER):
            hResult = pp.DAP_GetJtagID()
            hr = hResult[0]
            chipJtagID = hResult[1]
            m_sLastError = hResult[2]
            if (not SUCCEEDED(hr)): return hr
            #ES3 and probably newer revisions
            if (IsPSoC3ES3(chipJtagID) or Is_PSoC_5_LP_ID(chipJtagID)):
                eccEnableChanged = 0
                if (len(hexData) >= 4):
                    eccEnableChanged = ((ord(hexData[3]) ^ ord(chipData[3])) & 0x08) == 0x08
                #need to reacquire chip if EccEnable bit was changed to apply it for flash rows.
                if (eccEnableChanged):
                    hResult = pp.DAP_Acquire()
                    hr = hResult[0]
                    m_sLastError = hResult[1]
            return hr
    return 0

def GetEccOption(arrayID):
    global m_sLastError
    hResult = pp.PSoC3_GetFlashArrayInfo(arrayID)
    hr = hResult[0]
    rowSize = hResult[1]
    rowsPerArray = hResult[2]
    eccPresence = hResult[3]
    m_sLastError = hResult[4]
    if (not SUCCEEDED(hr)): return hr
    hResult = pp.PSoC3_GetEccStatus() #get ecc status of the acquired device
    hr = hResult[0]
    eccHwStatus = hResult[1]
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)): return hr
    #take into account data from the Config area
    if ((eccPresence != 0) and (eccHwStatus == 0)):
        eccOption = 1
    else:
        eccOption = 0
    return eccOption

def ProgramFlashArrays(flashSize):
    global m_sLastError
    hResult = pp.PSoC3_GetSonosArrays(enumSonosArrays.ARRAY_FLASH)
    hr = hResult[0]
    arrayInfo = hResult[1]
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)): return hr

    flashProgrammed = 0
    #Program Flash arrays
    for i in range(0, len(arrayInfo[0])):
        arrayID = arrayInfo[0][i]
        arraySize = arrayInfo[1][i]
        #Program flash array from hex file taking into account ECC (Config Data)
        hResult = pp.PSoC3_GetFlashArrayInfo(arrayID)
        hr = hResult[0]
        rowSize = hResult[1]
        rowsPerArray = hResult[2]
        eccPresence = hResult[3]
        m_sLastError = hResult[4]
        if (not SUCCEEDED(hr)): return hr
        eccOption = GetEccOption(arrayID); #take into account data from the Config area
        for rowID in range(0, rowsPerArray):
            hResult = pp.PSoC3_ProgramRowFromHex(arrayID, rowID, eccOption)
            hr = hResult[0]
            m_sLastError = hResult[1]
            if (not SUCCEEDED(hr)): return hr
            #Limit programming to the device flash size
            flashProgrammed += rowSize
            if (flashProgrammed >= flashSize): return hr
    return hr

def GetTotalFlashRowsCount(flashSize):
    global m_sLastError
    listResult = []
    totalRows = 0
    rowSize = 256
    totalRows = flashSize / rowSize
    listResult.append(0)
    listResult.append(totalRows)
    return listResult

def CheckSum_All(flashSize):
    global m_sLastError
    listResult = []
    cs = 0
    hResult = pp.PSoC3_GetSonosArrays(enumSonosArrays.ARRAY_FLASH)
    hr = hResult[0]
    arrayInfo = hResult[1]
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)):
        listResult.append(hr)
        listResult.append(cs)
        return listResult
    hResult = GetTotalFlashRowsCount(flashSize)
    hr = hResult[0]
    RowsToChecksum = hResult[1]
    for i in range(0, len(arrayInfo[0])):
        arrayID = arrayInfo[0][i]
        arraySize = arrayInfo[1][i]
        #Get info about flash array
        hResult = pp.PSoC3_GetFlashArrayInfo(arrayID)
        hr = hResult[0]
        rowSize = hResult[1]
        rowsPerArray = hResult[2]
        eccPresence = hResult[3]
        m_sLastError = hResult[4]
        if (not SUCCEEDED(hr)):
            listResult.append(hr)
            listResult.append(cs)
            return listResult
        #Find number of rows in array to be checksumed
        if ((RowsToChecksum - rowsPerArray) < 0):
            rowsPerArray = RowsToChecksum
        #Calculate checksum of the array
        arrayChecksum = 0;
        hResult = pp.PSoC3_CheckSum(arrayID, 0, rowsPerArray)
        hr = hResult[0]
        arrayChecksum = hResult[1]
        m_sLastError = hResult[2]
        if (not SUCCEEDED(hr)):
            listResult.append(hr)
            listResult.append(cs)
            return listResult
        #Sum checksum
        cs += arrayChecksum
        #Update number of rows to be checksumed
        RowsToChecksum -= rowsPerArray
        if (RowsToChecksum <= 0):
            break;
    listResult.append(hr)
    listResult.append(cs)
    return listResult

def ProgramAll(hex_file):
    global m_sLastError
    #Setup Power - "5.0V" and internal
    print('Setting Power Voltage to 5.0V')
    hResult = pp.SetPowerVoltage("5.0")
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    '''
    print('Turning on device')
    hResult = pp.PowerOn()
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    '''
    #Set protocol, connector and frequency
    print('Setting communication protocols')
    hResult = pp.SetProtocol(enumInterfaces.SWD)
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    #hResult = pp.SetProtocolConnector(1); #10-pin connector
    hResult = pp.SetProtocolConnector(0); #5-pin connector
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    hResult = pp.SetProtocolClock(enumFrequencies.FREQ_03_0); #3.0 MHz clock on SWD bus
    hr = hResult[0]
    m_sLastError = hResult[1]
    # Set Hex File
    print('Reading Hex file')
    hResult = pp.ReadHexFile(hex_file)
    hr = hResult[0]
    hexImageSize = int(hResult[1])
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)): return hr
    # Set Acquire Mode
    pp.SetAcquireMode("Reset")

    #The "Programming Flow" proper
    #Acquire Device
    print('Acquiring device')
    hResult = pp.DAP_Acquire()
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    #Check Hex File and Device compatibility
    fCompatibility = 0
    print('Checking Hex file and device compatability')
    hResult = CheckHexAndDeviceCompatibility()
    hr = hResult[0]
    fCompatibility = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    if (fCompatibility == 0):
        m_sLastError = "The Hex file does not match the acquired device, please connect the appropriate device";
        return -1
    #Erase All
    print('Erasing all flash')
    hResult = pp.PSoC3_EraseAll();
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr

    #Program User NVL arrays
    print('Programming User NVL arrays')
    hr = ProgramNvlArrays(enumSonosArrays.ARRAY_NVL_USER)
    if (not SUCCEEDED(hr)): return hr
    #Program Flash arrays
    print('Programming Flash arrays')
    hr = ProgramFlashArrays(hexImageSize)
    #Protect All arrays
    print('Protecting all arrays')
    hResult = pp.PSoC3_ProtectAll()
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr

    #CheckSum
    print('Generating checksum')
    hResult = CheckSum_All(hexImageSize)
    hr = hResult[0]
    flashChecksum = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    hResult = pp.ReadHexChecksum()
    hr = hResult[0]
    hexChecksum = hResult[1]
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)): return hr
    flashChecksum &= 0xFFFF
    if (flashChecksum != hexChecksum):
        print("Mismatch of Checksum: Expected 0x%x, Got 0x%x" %(flashChecksum, hexChecksum))
        return -1
    else:
        print("Checksum 0x%x" %(flashChecksum))
    #Release PSoC3 device
    #For PSoC5 LP call "Allow_Hard_Reset()" as in C#/C++ example
    Allow_Hard_Reset()
    hResult = pp.DAP_ReleaseChip()
    hr = hResult[0]
    m_sLastError = hResult[1]
    return hr

def UpgradeBlock():
    global m_sLastError
    #Setup Power - "5.0V" and internal
    hResult = pp.SetPowerVoltage("5.0")
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    hResult = pp.PowerOn()
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    #Set protocol, connector and frequency
    hResult = pp.SetProtocol(enumInterfaces.SWD)
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    #hResult = pp.SetProtocolConnector(1); #10-pin connector
    hResult = pp.SetProtocolConnector(0); #5-pin connector
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr
    hResult = pp.SetProtocolClock(enumFrequencies.FREQ_03_0); #<=3.0 MHz for Read operations
    hr = hResult[0]
    m_sLastError = hResult[1]

    #Set Acquire Mode
    pp.SetAcquireMode("Reset")

    #Acquire Device
    hResult = pp.DAP_Acquire()
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr

    #Write Row, use PSoC3_WriteRow() instead PSoC3_ProgramRow()
    writeData = [] #User and Config area of the Row (256+32)
    for i in range(0, 288):
        writeData.append(i & 0xFF)
    data = array.array('B',writeData)
    arrayID = 0
    rowID = 255
    hResult = pp.PSoC3_WriteRow(arrayID, rowID, buffer(data))
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr

    #Verify Row - only user area (without Config one)
    hResult = pp.PSoC3_ReadRow(arrayID, rowID, 0)
    hr = hResult[0]
    readRow = hResult[1]
    if PY3:
        readRow = [chr(c) for c in readRow]
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)): return hr

    for i in range(0, len(readRow)): #check 256 bytes
        if (ord(readRow[i]) != writeData[i]):
            hr = -1
            break
    if (not SUCCEEDED(hr)):
        m_sLastError = "Verification of User area failed!"
        return hr

    #Verify Row - Config are only
    hResult = pp.PSoC3_ReadRow(arrayID, rowID, 1)
    hr = hResult[0]
    readRow = hResult[1]
    m_sLastError = hResult[2]
    if (not SUCCEEDED(hr)): return hr

    for i in range(0, len(readRow)): #check 256 bytes
        if (ord(readRow[i]) != writeData[256+i]):
            hr = -1
            break
    if (not SUCCEEDED(hr)):
        m_sLastError = "Verification of Config area failed!"
        return hr

    #Release PSoC3 chip
    #For PSoC5 LP call "Allow_Hard_Reset()" as in C#/C++ example
    Allow_Hard_Reset()
    hResult = pp.DAP_ReleaseChip()
    hr = hResult[0]
    m_sLastError = hResult[1]
    return hr

def Execute(hex_file):
    print("Opening Port")
    hr = OpenPort()
    if (not SUCCEEDED(hr)): return hr
    hr = ProgramAll(hex_file)
    #hr = UpgradeBlock()
    print('Closing Port')
    ClosePort()
    return hr

def PowerOffDevice():
    global m_sLastError

    print("Opening Port")
    hr = OpenPort()
    if (not SUCCEEDED(hr)): return hr

    #Setup Power - "5.0V" and internal
    print('Setting Power Voltage to 5.0V')
    hResult = pp.SetPowerVoltage("5.0")
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr

    print('Turning off device')
    hResult = pp.PowerOff()
    hr = hResult[0]
    m_sLastError = hResult[1]
    if (not SUCCEEDED(hr)): return hr

    print('Closing Port')
    ClosePort()
    return hr

#Begin main program

if __name__ == '__main__':
    #Use Version Independent Prog ID to instantiate COM-object
    pp = win32com.client.Dispatch("PSoCProgrammerCOM.PSoCProgrammerCOM_Object")
    #For version dependent Prog ID use below commented line, but update there COM-object version (e.g. 14)
    #pp = win32com.client.Dispatch("PSoCProgrammerCOM.PSoCProgrammerCOM_Object.14")

    if len(sys.argv) == 2 and sys.argv[1] == '--power-off-device':
        hr = PowerOffDevice()

    else:
        print("Program All using COM-object interface only")

        hex_file = os.path.abspath(sys.argv[1])
        print("Using Hex File:", hex_file)

        hr = Execute(hex_file)

    if (SUCCEEDED(hr)):
        str = "Succeeded!"
        exit_code = 0
    else:
        str = "Failed! " + m_sLastError
        exit_code = 1

    print(str)
    sys.exit(exit_code)

#End main function
