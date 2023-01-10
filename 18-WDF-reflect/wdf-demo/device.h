#pragma once

#include "io_callbacks.h"
#include <ntddk.h>
#include <wdf.h>

NTSTATUS device_init(WDFDRIVER& driver, WDFDEVICE& device);