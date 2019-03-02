#pragma once

#define __LText(text)  L##text
#define LText(text)  __LText(text)
#define PlayerKnownsDriverName  "playerknownskrnl"
#define PlayerKnownsDriverDeviceName  PlayerKnownsDriverName

#define LPlayerKnownsDriverName  LText(PlayerKnownsDriverName)
#define LPlayerKnownsDriverDeviceName  LPlayerKnownsDriverName

// use for argument of mmap's IoCreateDriver
#define LPlayerKnownsDriverPath  L"\\Driver\\" LPlayerKnownsDriverName

// use for device creation
#define LPlayerKnownsDevicePath  L"\\Device\\" LPlayerKnownsDriverDeviceName

// use for creating symbol link which make user space application to locate our device
#define LPlayerKnownsDeviceSymbolPath  L"\\DosDevices\\" LPlayerKnownsDriverDeviceName
