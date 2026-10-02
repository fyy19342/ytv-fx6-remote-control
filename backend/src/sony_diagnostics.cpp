#include "sony_diagnostics.h"
#include <sstream>

std::string sdk_error_name(SCRSDK::CrError error) {
    switch (error) {
    case SCRSDK::CrError_None: return "CrError_None";
    case SCRSDK::CrError_Adaptor_Create: return "CrError_Adaptor_Create";
    case SCRSDK::CrError_Connect_HandlePlugin: return "CrError_Connect_HandlePlugin";
    case SCRSDK::CrError_Connect_Disconnected: return "CrError_Connect_Disconnected";
    case SCRSDK::CrError_Connect_TimeOut: return "CrError_Connect_TimeOut";
    case SCRSDK::CrError_Connect_GetProperty: return "CrError_Connect_GetProperty";
    default: return "Sony SDK error";
    }
}

std::string sdk_property_name(SCRSDK::CrDevicePropertyCode code) {
    switch (code) {
    case SCRSDK::CrDeviceProperty_FNumber: return "FNumber";
    case SCRSDK::CrDeviceProperty_IsoSensitivity: return "IsoSensitivity";
    case SCRSDK::CrDeviceProperty_GainUnitSetting: return "GainUnitSetting";
    case SCRSDK::CrDeviceProperty_GainBaseIsoSensitivity: return "GainBaseIsoSensitivity";
    case SCRSDK::CrDeviceProperty_NDFilter: return "NDFilter";
    case SCRSDK::CrDeviceProperty_NDFilterValue: return "NDFilterValue";
    case SCRSDK::CrDeviceProperty_NDFilterModeSetting: return "NDFilterModeSetting";
    case SCRSDK::CrDeviceProperty_NDFilterSwitchingSetting: return "NDFilterSwitchingSetting";
    case SCRSDK::CrDeviceProperty_NDFilterOpticalDensityValue: return "NDFilterOpticalDensityValue";
    default: return "Property " + std::to_string(code);
    }
}

std::string sdk_warning_text(SCRSDK::CrError error, CrInt32 a, CrInt32 b, CrInt32 c) {
    std::ostringstream text;
    text << "Sony SDK warning 0x" << std::hex << error << std::dec
         << " (" << a << ", " << b << ", " << c << ')';
    return text.str();
}
