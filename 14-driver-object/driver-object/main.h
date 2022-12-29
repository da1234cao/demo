#pragma once

#include "common/log.h"
#include <ntddk.h>

#define DRIVER_OBJECT_DEVICE_NAME L"\\Device\\object_device"

#define DRIVER_OBJECT_SYM_NAME L"\\DosDevices\\object_device"

#define IOCTL_OBJECT_TEST_REFLECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)

#define IOCTL_OBJECT_TEST_READ CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)

#define BUFFER_SIZE 1024