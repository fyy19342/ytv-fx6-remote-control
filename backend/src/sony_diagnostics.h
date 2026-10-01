#pragma once
#include <string>
#include "CrDeviceProperty.h"
#include "CrError.h"

// Project-owned diagnostics for the limited property surface used by this app.
std::string sdk_error_name(SCRSDK::CrError error);
std::string sdk_property_name(SCRSDK::CrDevicePropertyCode code);
std::string sdk_warning_text(SCRSDK::CrError error, CrInt32 a, CrInt32 b, CrInt32 c);
