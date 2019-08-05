/*
	Neuropixel

	(c) Imec 2019

	Debug API exports
*/

#pragma once
#include "NeuropixAPI.h"
//#include "hardwareID.h"

#ifdef __cplusplus
extern "C" {
#endif	

	NP_EXPORT np::NP_ErrorCode NP_APIC getBSCHardwareID(int slotID, struct HardwareID* pHwid);
	NP_EXPORT np::NP_ErrorCode NP_APIC setBSCHardwareID(int slotID, const struct HardwareID* pHwid);
	NP_EXPORT np::NP_ErrorCode NP_APIC getHeadstageHardwareID(int slotID, int portID, struct HardwareID* pHwid);
	NP_EXPORT np::NP_ErrorCode NP_APIC setHeadstageHardwareID(int slotID, int portID, const struct HardwareID* pHwid);
	NP_EXPORT np::NP_ErrorCode NP_APIC getFlexHardwareID(int slotID, int portID, int dockID, struct HardwareID* pHwid);
	NP_EXPORT np::NP_ErrorCode NP_APIC setFlexHardwareID(int slotID, int portID, int dockID, const struct HardwareID* pHwid);
	NP_EXPORT np::NP_ErrorCode NP_APIC getProbeHardwareID(int slotID, int portID, int dockID, struct HardwareID* pHwid);
	NP_EXPORT np::NP_ErrorCode NP_APIC setProbeHardwareID(int slotID, int portID, int dockID, const struct HardwareID* pHwid);

#ifdef __cplusplus
}
#endif	