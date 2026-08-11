// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#include <string>

#include "Core/HLE/HLE.h"
#include "Core/HLE/HLETables.h"
#include "Core/HLE/FunctionWrappers.h"
#include "Core/MemMap.h"

#include "sceAtrac.h"
#include "sceAudio.h"
#include "sceAudiocodec.h"
#include "sceAudioRouting.h"
#include "sceCcc.h"
#include "sceChnnlsv.h"
#include "sceCtrl.h"
#include "sceDeflt.h"
#include "sceDisplay.h"
#include "sceDmac.h"
#include "sceFont.h"
#include "sceGameUpdate.h"
#include "sceGe.h"
#include "sceHeap.h"
#include "sceHprm.h"
#include "sceHttp.h"
#include "sceImpose.h"
#include "sceIo.h"
#include "sceJpeg.h"
#include "sceKernel.h"
#include "sceKernelEventFlag.h"
#include "sceKernelHeap.h"
#include "sceKernelMemory.h"
#include "sceKernelInterrupt.h"
#include "sceKernelModule.h"
#include "sceKernelSemaphore.h"
#include "sceKernelThread.h"
#include "sceKernelTime.h"
#include "sceMd5.h"
#include "sceMp4.h"
#include "sceAac.h"
#include "sceMp3.h"
#include "sceNet.h"
#include "sceNetAdhoc.h"
#include "sceNetAdhocMatching.h"
#include "sceNp.h"
#include "sceMpeg.h"
#include "sceOpenPSID.h"
#include "sceP3da.h"
#include "sceParseHttp.h"
#include "sceParseUri.h"
#include "scePauth.h"
#include "scePower.h"
#include "scePspNpDrm_user.h"
#include "scePsmf.h"
#include "sceReg.h"
#include "sceRtc.h"
#include "sceSas.h"
#include "sceSircs.h"
#include "sceSsl.h"
#include "sceUmd.h"
#include "sceUsb.h"
#include "sceUsbAcc.h"
#include "sceUsbCam.h"
#include "sceUsbGps.h"
#include "sceUsbMic.h"
#include "sceUtility.h"
#include "sceVaudio.h"
#include "sceMt19937.h"
#include "sceSha256.h"
#include "sceAdler.h"
#include "sceSfmt19937.h"
#include "sceG729.h"
#include "KUBridge.h"
#include "sceNetInet.h"
#include "sceNetResolver.h"
// #include "sceNp2.h"
#include "sceNet_lib.h"

#define N(s) s

//\*\*\ found\:\ {[a-zA-Z]*}\ {0x[a-zA-Z0-9]*}\ \*\*
//{FID(\2),0,N("\1")},

//Metal Gear Acid modules:
//kjfs
//sound
//zlibdec
const HLEFunction FakeSysCalls[] = {
	{NID_THREADRETURN, __KernelReturnFromThread, "__KernelReturnFromThread", 'x', ""},
	{NID_CALLBACKRETURN, __KernelReturnFromMipsCall, "__KernelReturnFromMipsCall", 'x', ""},
	{NID_INTERRUPTRETURN, __KernelReturnFromInterrupt, "__KernelReturnFromInterrupt", 'x', ""},
	{NID_EXTENDRETURN, __KernelReturnFromExtendStack, "__KernelReturnFromExtendStack", 'x', ""},
	{NID_MODULERETURN, __KernelReturnFromModuleFunc, "__KernelReturnFromModuleFunc", 'x', ""},
	{NID_IDLE, __KernelIdle, "_sceKernelIdle", 'x', ""},
	{NID_GPUREPLAY, &WrapI_V<__KernelGPUReplay>, "__KernelGPUReplay", 'x', ""},
	{NID_HLECALLRETURN, HLEReturnFromMipsCall, "HLEReturnFromMipsCall", 'x', ""},
};

const HLEFunction UtilsForUser[] = 
{
	{0X91E4F6A7, &WrapU_V<sceKernelLibcClock>,                       "sceKernelLibcClock",                      'x', ""   },
	{0X27CC57F0, &WrapU_U<sceKernelLibcTime>,                        "sceKernelLibcTime",                       'x', "x"  },
	{0X71EC4271, &WrapU_UU<sceKernelLibcGettimeofday>,               "sceKernelLibcGettimeofday",               'x', "xx" },
	{0XBFA98062, &WrapI_UI<sceKernelDcacheInvalidateRange>,          "sceKernelDcacheInvalidateRange",          'i', "xi" },
	{0XC8186A58, &WrapI_UIU<sceKernelUtilsMd5Digest>,                "sceKernelUtilsMd5Digest",                 'i', "xix"},
	{0X9E5C5086, &WrapI_U<sceKernelUtilsMd5BlockInit>,               "sceKernelUtilsMd5BlockInit",              'i', "x"  },
	{0X61E1E525, &WrapI_UUI<sceKernelUtilsMd5BlockUpdate>,           "sceKernelUtilsMd5BlockUpdate",            'i', "xxi"},
	{0XB8D24E78, &WrapI_UU<sceKernelUtilsMd5BlockResult>,            "sceKernelUtilsMd5BlockResult",            'i', "xx" },
	{0X840259F1, &WrapI_UIU<sceKernelUtilsSha1Digest>,               "sceKernelUtilsSha1Digest",                'i', "xix"},
	{0XF8FCD5BA, &WrapI_U<sceKernelUtilsSha1BlockInit>,              "sceKernelUtilsSha1BlockInit",             'i', "x"  },
	{0X346F6DA8, &WrapI_UUI<sceKernelUtilsSha1BlockUpdate>,          "sceKernelUtilsSha1BlockUpdate",           'i', "xxi"},
	{0X585F1C09, &WrapI_UU<sceKernelUtilsSha1BlockResult>,           "sceKernelUtilsSha1BlockResult",           'i', "xx" },
	{0XE860E75E, &WrapU_UU<sceKernelUtilsMt19937Init>,               "sceKernelUtilsMt19937Init",               'x', "xx" },
	{0X06FB8A63, &WrapU_U<sceKernelUtilsMt19937UInt>,                "sceKernelUtilsMt19937UInt",               'x', "x"  },
	{0X37FB5C42, &WrapU_V<sceKernelGetGPI>,                          "sceKernelGetGPI",                         'x', ""   },
	{0X6AD345D7, &WrapV_U<sceKernelSetGPO>,                          "sceKernelSetGPO",                         'v', "x"  },
	{0X79D1C3FA, &WrapI_V<sceKernelDcacheWritebackAll>,              "sceKernelDcacheWritebackAll",             'i', ""   },
	{0XB435DEC5, &WrapI_V<sceKernelDcacheWritebackInvalidateAll>,    "sceKernelDcacheWritebackInvalidateAll",   'i', ""   },
	{0X3EE30821, &WrapI_UI<sceKernelDcacheWritebackRange>,           "sceKernelDcacheWritebackRange",           'i', "xi" },
	{0X34B9FA9E, &WrapI_UI<sceKernelDcacheWritebackInvalidateRange>, "sceKernelDcacheWritebackInvalidateRange", 'i', "xi" },
	{0XC2DF770E, &WrapI_UI<sceKernelIcacheInvalidateRange>,          "sceKernelIcacheInvalidateRange",          'i', "xi" },
	{0X80001C4C, nullptr,                                            "sceKernelDcacheProbe",                    '?', ""   },
	{0X16641D70, nullptr,                                            "sceKernelDcacheReadTag",                  '?', ""   },
	{0X4FD31C9D, nullptr,                                            "sceKernelIcacheProbe",                    '?', ""   },
	{0XFB05FAD0, nullptr,                                            "sceKernelIcacheReadTag",                  '?', ""   },
	{0X920F104A, &WrapU_V<sceKernelIcacheInvalidateAll>,             "sceKernelIcacheInvalidateAll",            'x', ""   }
};				   

const HLEFunction LoadCoreForKernel[] = 
{
	{0XACE23476, nullptr,                                            "sceKernelCheckPspConfig",                 '?', ""   },
	{0X7BE1421C, nullptr,                                            "sceKernelCheckExecFile",                  '?', ""   },
	{0XBF983EF2, nullptr,                                            "sceKernelProbeExecutableObject",          '?', ""   },
	{0X7068E6BA, nullptr,                                            "sceKernelLoadExecutableObject",           '?', ""   },
	{0XB4D6FECC, nullptr,                                            "sceKernelApplyElfRelSection",             '?', ""   },
	{0X54AB2675, nullptr,                                            "sceKernelApplyPspRelSection",             '?', ""   },
	{0X2952F5AC, nullptr,                                            "sceKernelDcacheWBinvAll",                 '?', ""   },
	{0xD8779AC6, &WrapU_V<sceKernelIcacheClearAll>,                  "sceKernelIcacheClearAll",                 'x', "",       HLE_KERNEL_SYSCALL },
	{0X99A695F0, nullptr,                                            "sceKernelRegisterLibrary",                '?', ""   },
	{0X5873A31F, nullptr,                                            "sceKernelRegisterLibraryForUser",         '?', ""   },
	{0X0B464512, nullptr,                                            "sceKernelReleaseLibrary",                 '?', ""   },
	{0X9BAF90F6, nullptr,                                            "sceKernelCanReleaseLibrary",              '?', ""   },
	{0X0E760DBA, nullptr,                                            "sceKernelLinkLibraryEntries",             '?', ""   },
	{0X0DE1F600, nullptr,                                            "sceKernelLinkLibraryEntriesForUser",      '?', ""   },
	{0XDA1B09AA, nullptr,                                            "sceKernelUnLinkLibraryEntries",           '?', ""   },
	{0XC99DD47A, nullptr,                                            "sceKernelQueryLoadCoreCB",                '?', ""   },
	{0X616FCCCD, nullptr,                                            "sceKernelSetBootCallbackLevel",           '?', ""   },
	{0XF32A2940, nullptr,                                            "sceKernelModuleFromUID",                  '?', ""   },
	{0XCD0F3BAC, nullptr,                                            "sceKernelCreateModule",                   '?', ""   },
	{0X6B2371C2, nullptr,                                            "sceKernelDeleteModule",                   '?', ""   },
	{0X7320D964, nullptr,                                            "sceKernelModuleAssign",                   '?', ""   },
	{0X44B292AB, nullptr,                                            "sceKernelAllocModule",                    '?', ""   },
	{0XBD61D4D5, nullptr,                                            "sceKernelFreeModule",                     '?', ""   },
	{0XAE7C6E76, nullptr,                                            "sceKernelRegisterModule",                 '?', ""   },
	{0X74CF001A, nullptr,                                            "sceKernelReleaseModule",                  '?', ""   },
	{0XFB8AE27D, nullptr,                                            "sceKernelFindModuleByAddress",            '?', ""   },
	{0XCCE4A157, &WrapU_U<sceKernelFindModuleByUID>,                 "sceKernelFindModuleByUID",                'x', "x" ,     HLE_KERNEL_SYSCALL },
	{0X82CE54ED, nullptr,                                            "sceKernelModuleCount",                    '?', ""   },
	{0XC0584F0C, nullptr,                                            "sceKernelGetModuleList",                  '?', ""   },
	{0XCF8A41B1, &WrapU_C<sceKernelFindModuleByName>,                "sceKernelFindModuleByName",               'x', "s",      HLE_KERNEL_SYSCALL },
	{0XB95FA50D, nullptr,                                            "LoadCoreForKernel_B95FA50D",              '?', ""   },
};


const HLEFunction KDebugForKernel[] = 
{
	{0XE7A3874D, nullptr,                                            "sceKernelRegisterAssertHandler",          '?', ""   },
	{0X2FF4E9F9, nullptr,                                            "sceKernelAssert",                         '?', ""   },
	{0X9B868276, nullptr,                                            "sceKernelGetDebugPutchar",                '?', ""   },
	{0XE146606D, nullptr,                                            "sceKernelRegisterDebugPutchar",           '?', ""   },
	{0X7CEB2C09, &WrapU_V<sceKernelRegisterKprintfHandler>,          "sceKernelRegisterKprintfHandler",         'x', "",       HLE_KERNEL_SYSCALL },
	{0X84F370BC, nullptr,                                            "Kprintf",                                 '?', ""   },
	{0X5CE9838B, nullptr,                                            "sceKernelDebugWrite",                     '?', ""   },
	{0X66253C4E, nullptr,                                            "sceKernelRegisterDebugWrite",             '?', ""   },
	{0XDBB5597F, nullptr,                                            "sceKernelDebugRead",                      '?', ""   },
	{0XE6554FDA, nullptr,                                            "sceKernelRegisterDebugRead",              '?', ""   },
	{0XB9C643C9, nullptr,                                            "sceKernelDebugEcho",                      '?', ""   },
	{0X7D1C74F0, nullptr,                                            "sceKernelDebugEchoSet",                   '?', ""   },
	{0X24C32559, nullptr,                                            "sceKernelDipsw",                          '?', ""   },
	{0XD636B827, nullptr,                                            "sceKernelRemoveByDebugSection",           '?', ""   },
	{0X5282DD5E, nullptr,                                            "sceKernelDipswSet",                       '?', ""   },
	{0X9F8703E4, nullptr,                                            "sceKernelDipswCpTime",                    '?', ""   },
	{0X333DCEC7, nullptr,                                            "sceKernelSm1RegisterOperations",          '?', ""   },
	{0XE892D9A1, nullptr,                                            "sceKernelSm1ReferOperations",             '?', ""   },
	{0XA126F497, nullptr,                                            "KDebugForKernel_A126F497",                '?', ""   },
	{0XB7251823, nullptr,                                            "sceKernelAcceptMbogoSig",                 '?', ""   },
};

const HLEFunction pspeDebug[] =
{
	{0XDEADBEAF, nullptr,                                            "pspeDebugWrite",                          '?', ""   },
};

#define VSH_STUB(name) static int name() { return hleNoLog(0); }

static u32 scePafMemset(u32 dst, int value, u32 size) {
	if (Memory::IsValidRange(dst, size)) {
		memset(Memory::GetPointerWriteUnchecked(dst), value & 0xFF, size);
	}
	return dst;
}

static u32 scePafAllocStubBlock(const char *tag) {
	u32 size = 0x100;
	u32 addr = userMemory.Alloc(size, false, tag);
	if (addr != 0 && Memory::IsValidRange(addr, size)) {
		memset(Memory::GetPointerWriteUnchecked(addr), 0, size);
	}
	return addr;
}

static void scePafInitStubContext(u32 addr) {
	if (addr == 0 || !Memory::IsValidRange(addr, 0x40)) {
		return;
	}
	Memory::Write_U32(addr + 0x00, addr + 0x10);
	Memory::Write_U32(addr + 0x04, addr + 0x14);
	Memory::Write_U32(addr + 0x08, addr + 0x18);
	Memory::Write_U32(addr + 0x0C, addr + 0x1C);
	Memory::Write_U32(addr + 0x10, addr + 0x20);
	Memory::Write_U32(addr + 0x14, addr + 0x24);
	Memory::Write_U32(addr + 0x18, addr + 0x28);
	Memory::Write_U32(addr + 0x1C, addr + 0x2C);
	Memory::Write_U32(addr + 0x20, addr + 0x30);
	Memory::Write_U32(addr + 0x24, addr + 0x34);
	Memory::Write_U32(addr + 0x28, addr + 0x38);
}

static u32 scePafAllocStubContextBlock(const char *tag) {
	u32 addr = scePafAllocStubBlock(tag);
	scePafInitStubContext(addr);
	return addr;
}

static u32 scePafGetSharedStubContext() {
	static u32 block = 0;
	if (block == 0 || !Memory::IsValidAddress(block)) {
		block = scePafAllocStubContextBlock("scePaf_SharedContext");
	}
	return block;
}

VSH_STUB(sceVshBridge_01730088)
VSH_STUB(sceVshBridge_0543156C)
VSH_STUB(sceVshBridge_0C0D5913)
VSH_STUB(sceVshBridge_0D684A0B)
VSH_STUB(sceVshBridge_0D7A4FE4)
VSH_STUB(sceVshBridge_12B07B05)
VSH_STUB(sceVshBridge_1D5C579F)
VSH_STUB(sceVshBridge_21C243FE)
VSH_STUB(sceVshBridge_21D4D038)
VSH_STUB(sceVshBridge_2380DC08)
VSH_STUB(sceVshBridge_27BDA326)
VSH_STUB(sceVshBridge_27CD418C)
VSH_STUB(sceVshBridge_29CDFFBA)
VSH_STUB(sceVshBridge_2EBD2323)
VSH_STUB(sceVshBridge_3785D08B)
VSH_STUB(sceVshBridge_3A46C639)
VSH_STUB(sceVshBridge_3C90E435)
VSH_STUB(sceVshBridge_3D30FEB6)
VSH_STUB(sceVshBridge_4A596D2D)
VSH_STUB(sceVshBridge_4DB43867)
VSH_STUB(sceVshBridge_53BFD101)
VSH_STUB(sceVshBridge_582B5281)
VSH_STUB(sceVshBridge_59197BE8)
VSH_STUB(sceVshBridge_5B7F3339)
VSH_STUB(sceVshBridge_5C2983C2)
VSH_STUB(sceVshBridge_5E0F5543)
VSH_STUB(sceVshBridge_5E5AF7A2)
VSH_STUB(sceVshBridge_5F35E8FE)
VSH_STUB(sceVshBridge_61001D64)
VSH_STUB(sceVshBridge_63047647)
VSH_STUB(sceVshBridge_639C3CB3)
VSH_STUB(sceVshBridge_63E69956)
VSH_STUB(sceVshBridge_6CAEB765)
VSH_STUB(sceVshBridge_7423151D)
VSH_STUB(sceVshBridge_74DA9D25)
VSH_STUB(sceVshBridge_74DBE57E)
VSH_STUB(sceVshBridge_791FCD43)
VSH_STUB(sceVshBridge_79B916E1)
VSH_STUB(sceVshBridge_7A90D816)
VSH_STUB(sceVshBridge_7B14CE2B)
VSH_STUB(sceVshBridge_7D1C13B5)
VSH_STUB(sceVshBridge_7E117907)
VSH_STUB(sceVshBridge_81682A40)
VSH_STUB(sceVshBridge_837C457A)
VSH_STUB(sceVshBridge_88C35487)
VSH_STUB(sceVshBridge_9056DE3A)
VSH_STUB(sceVshBridge_9347D693)
VSH_STUB(sceVshBridge_9427C909)
VSH_STUB(sceVshBridge_9940D95C)
VSH_STUB(sceVshBridge_A29B5A33)
VSH_STUB(sceVshBridge_A5628F0D)
VSH_STUB(sceVshBridge_AAB9A9EF)
VSH_STUB(sceVshBridge_ABB84565)
VSH_STUB(sceVshBridge_B8B07CAF)
VSH_STUB(sceVshBridge_C51A6C26)
VSH_STUB(sceVshBridge_C6395C03)
VSH_STUB(sceVshBridge_C949966C)
VSH_STUB(sceVshBridge_CCD27632)
VSH_STUB(sceVshBridge_CD1A2C46)
VSH_STUB(sceVshBridge_CE32CBEF)
VSH_STUB(sceVshBridge_D39DE400)
VSH_STUB(sceVshBridge_D47041CA)
VSH_STUB(sceVshBridge_D7D7E7B6)
VSH_STUB(sceVshBridge_DB7C3D5A)
VSH_STUB(sceVshBridge_E174218C)
VSH_STUB(sceVshBridge_E533E98C)
VSH_STUB(sceVshBridge_E5DA5E95)
VSH_STUB(sceVshBridge_EBC3A334)

VSH_STUB(sceVshCommonUtil_0A708F40)
VSH_STUB(sceVshCommonUtil_1BC70F77)
VSH_STUB(sceVshCommonUtil_1D7E0B27)
static int sceVshCommonUtil_2058CEAA() {
	return hleNoLog(0);
}
VSH_STUB(sceVshCommonUtil_2D320307)
static int sceVshCommonUtil_34826464() {
	return hleNoLog(0);
}
VSH_STUB(sceVshCommonUtil_38C93C16)
VSH_STUB(sceVshCommonUtil_392A75EF)
VSH_STUB(sceVshCommonUtil_3CA4266A)
VSH_STUB(sceVshCommonUtil_3F28E621)
VSH_STUB(sceVshCommonUtil_4A92625E)
VSH_STUB(sceVshCommonUtil_5A1359E0)
VSH_STUB(sceVshCommonUtil_649C3568)
VSH_STUB(sceVshCommonUtil_8381781A)
VSH_STUB(sceVshCommonUtil_9869763D)
VSH_STUB(sceVshCommonUtil_A16F632D)
VSH_STUB(sceVshCommonUtil_A721B1D1)
VSH_STUB(sceVshCommonUtil_BC01322B)
VSH_STUB(sceVshCommonUtil_E6C36E64)
VSH_STUB(sceVshCommonUtil_F9DB4018)
VSH_STUB(sceVshCommonUtil_FC3BC7EE)

VSH_STUB(sceUtility_netparam_internal_02598DB8)
VSH_STUB(sceUtility_netparam_internal_09905636)
VSH_STUB(sceUtility_netparam_internal_6B1BC62C)

static u32 scePaf_03B20F85() {
	return hleNoLog(scePafGetSharedStubContext());
}
static u32 scePaf_30AF7A24() {
	return hleNoLog(scePafGetSharedStubContext());
}
static u32 scePaf_44AE2A91() {
	return hleNoLog(scePafGetSharedStubContext());
}
static u32 scePaf_4CA9F1A7() {
	return hleNoLog(scePafGetSharedStubContext());
}
static u32 scePaf_4CF09BA2() {
	return hleNoLog(scePafGetSharedStubContext());
}
static u32 scePaf_4CFAB8D4() {
	return hleNoLog(scePafGetSharedStubContext());
}
static u32 scePaf_4D45C2CF() {
	return hleNoLog(scePafGetSharedStubContext());
}
#define PAF_ALLOC_STUB(name) static u32 name() { return hleNoLog(scePafAllocStubBlock(#name)); }
PAF_ALLOC_STUB(scePaf_0085C4A5)
PAF_ALLOC_STUB(scePaf_00B832B2)
PAF_ALLOC_STUB(scePaf_02A60EBD)
PAF_ALLOC_STUB(scePaf_04A7B506)
PAF_ALLOC_STUB(scePaf_04E015E4)
PAF_ALLOC_STUB(scePaf_05E14B72)
PAF_ALLOC_STUB(scePaf_075FACE4)
PAF_ALLOC_STUB(scePaf_0929EF0D)
PAF_ALLOC_STUB(scePaf_0A1E907A)
PAF_ALLOC_STUB(scePaf_0B95292E)
PAF_ALLOC_STUB(scePaf_0C2CD696)
PAF_ALLOC_STUB(scePaf_0E8CA0D0)
#undef PAF_ALLOC_STUB



#define PAF_VSH_UNKNOWN_STUB(name) VSH_STUB(scePaf_##name)
PAF_VSH_UNKNOWN_STUB(0EE8FFFF)
PAF_VSH_UNKNOWN_STUB(0F19FF55)
PAF_VSH_UNKNOWN_STUB(0FC5E0EF)
PAF_VSH_UNKNOWN_STUB(0FC66D1C)
PAF_VSH_UNKNOWN_STUB(10E9CF5B)
PAF_VSH_UNKNOWN_STUB(11D950F0)
PAF_VSH_UNKNOWN_STUB(126454E6)
PAF_VSH_UNKNOWN_STUB(13B2C7BB)
PAF_VSH_UNKNOWN_STUB(14EF73B2)
PAF_VSH_UNKNOWN_STUB(167BA176)
PAF_VSH_UNKNOWN_STUB(16CA066C)
PAF_VSH_UNKNOWN_STUB(1775C8E3)
PAF_VSH_UNKNOWN_STUB(177C1E4F)
PAF_VSH_UNKNOWN_STUB(17B4723E)
PAF_VSH_UNKNOWN_STUB(1941469E)
PAF_VSH_UNKNOWN_STUB(1C80A4DC)
PAF_VSH_UNKNOWN_STUB(1E443486)
PAF_VSH_UNKNOWN_STUB(1FB4579B)
PAF_VSH_UNKNOWN_STUB(22420CC7)
PAF_VSH_UNKNOWN_STUB(22560E39)
PAF_VSH_UNKNOWN_STUB(23A245C3)
PAF_VSH_UNKNOWN_STUB(26717465)
PAF_VSH_UNKNOWN_STUB(279F57C3)
PAF_VSH_UNKNOWN_STUB(28C3DFA5)
PAF_VSH_UNKNOWN_STUB(2B889C55)
PAF_VSH_UNKNOWN_STUB(2C0F0975)
PAF_VSH_UNKNOWN_STUB(2C22C6FB)
PAF_VSH_UNKNOWN_STUB(2C23C4E2)
PAF_VSH_UNKNOWN_STUB(2D138A74)
PAF_VSH_UNKNOWN_STUB(2D92A298)
PAF_VSH_UNKNOWN_STUB(304F6CF4)
PAF_VSH_UNKNOWN_STUB(319A6571)
PAF_VSH_UNKNOWN_STUB(327B34BF)
PAF_VSH_UNKNOWN_STUB(330BD953)
PAF_VSH_UNKNOWN_STUB(362F4258)
PAF_VSH_UNKNOWN_STUB(36E2C527)
PAF_VSH_UNKNOWN_STUB(3710132E)
PAF_VSH_UNKNOWN_STUB(3782A7C2)
PAF_VSH_UNKNOWN_STUB(379A3D64)
PAF_VSH_UNKNOWN_STUB(37A80B89)
PAF_VSH_UNKNOWN_STUB(37ABB4A1)
PAF_VSH_UNKNOWN_STUB(3874A5F8)
PAF_VSH_UNKNOWN_STUB(3876721B)
PAF_VSH_UNKNOWN_STUB(3A4504D5)
PAF_VSH_UNKNOWN_STUB(3A5E1022)
PAF_VSH_UNKNOWN_STUB(3BF0DB62)
PAF_VSH_UNKNOWN_STUB(3C0E9C2D)
PAF_VSH_UNKNOWN_STUB(3E22B04E)
PAF_VSH_UNKNOWN_STUB(3ED6EA7F)
PAF_VSH_UNKNOWN_STUB(3F65FC71)
PAF_VSH_UNKNOWN_STUB(428E7562)
PAF_VSH_UNKNOWN_STUB(430D11DC)
PAF_VSH_UNKNOWN_STUB(438B6847)
PAF_VSH_UNKNOWN_STUB(440665DB)
PAF_VSH_UNKNOWN_STUB(465B3A83)
PAF_VSH_UNKNOWN_STUB(4792CA66)
PAF_VSH_UNKNOWN_STUB(485B96FB)
PAF_VSH_UNKNOWN_STUB(491E69CE)
PAF_VSH_UNKNOWN_STUB(4AA46320)
PAF_VSH_UNKNOWN_STUB(4AF28AF7)
PAF_VSH_UNKNOWN_STUB(4BC342F7)
PAF_VSH_UNKNOWN_STUB(54807A37)
PAF_VSH_UNKNOWN_STUB(58BD2AEB)
PAF_VSH_UNKNOWN_STUB(5905DF98)
PAF_VSH_UNKNOWN_STUB(5A6CAFE2)
PAF_VSH_UNKNOWN_STUB(5B257A25)
PAF_VSH_UNKNOWN_STUB(5B46340C)
PAF_VSH_UNKNOWN_STUB(5CE2ACF4)
PAF_VSH_UNKNOWN_STUB(5D22ED89)
PAF_VSH_UNKNOWN_STUB(5E9078F9)
PAF_VSH_UNKNOWN_STUB(6195CF7C)
PAF_VSH_UNKNOWN_STUB(6332FF84)
PAF_VSH_UNKNOWN_STUB(66832B20)
PAF_VSH_UNKNOWN_STUB(675047B1)
PAF_VSH_UNKNOWN_STUB(67FF4EBC)
PAF_VSH_UNKNOWN_STUB(685F8622)
PAF_VSH_UNKNOWN_STUB(68685412)
PAF_VSH_UNKNOWN_STUB(68B4E5A8)
PAF_VSH_UNKNOWN_STUB(698098B9)
PAF_VSH_UNKNOWN_STUB(69A1E33B)
PAF_VSH_UNKNOWN_STUB(69D71FBF)
PAF_VSH_UNKNOWN_STUB(6A17A477)
PAF_VSH_UNKNOWN_STUB(6E36E1D9)
PAF_VSH_UNKNOWN_STUB(6F3415DD)
PAF_VSH_UNKNOWN_STUB(6FBEAC03)
PAF_VSH_UNKNOWN_STUB(706ABBFF)
PAF_VSH_UNKNOWN_STUB(713256AB)
PAF_VSH_UNKNOWN_STUB(72B16452)
PAF_VSH_UNKNOWN_STUB(7364C1F7)
PAF_VSH_UNKNOWN_STUB(73DA59C1)
PAF_VSH_UNKNOWN_STUB(73F0D607)
PAF_VSH_UNKNOWN_STUB(73F36F1F)
PAF_VSH_UNKNOWN_STUB(777E5AC9)
PAF_VSH_UNKNOWN_STUB(78037E0C)
PAF_VSH_UNKNOWN_STUB(7869A05E)
PAF_VSH_UNKNOWN_STUB(79174B51)
PAF_VSH_UNKNOWN_STUB(7B7133D5)
PAF_VSH_UNKNOWN_STUB(7B77B7E7)
PAF_VSH_UNKNOWN_STUB(7C92BF10)
PAF_VSH_UNKNOWN_STUB(7D7DEB7E)
PAF_VSH_UNKNOWN_STUB(7DF192FA)
PAF_VSH_UNKNOWN_STUB(7E0A6B25)
PAF_VSH_UNKNOWN_STUB(7E160F18)
PAF_VSH_UNKNOWN_STUB(7E3A3D9B)
PAF_VSH_UNKNOWN_STUB(7E545DA3)
PAF_VSH_UNKNOWN_STUB(80390D7B)
PAF_VSH_UNKNOWN_STUB(81F7BDC2)
PAF_VSH_UNKNOWN_STUB(85BFE0DD)
PAF_VSH_UNKNOWN_STUB(8871CAB9)
PAF_VSH_UNKNOWN_STUB(88CE8467)
PAF_VSH_UNKNOWN_STUB(8BC9927A)
PAF_VSH_UNKNOWN_STUB(8DB83D04)
PAF_VSH_UNKNOWN_STUB(8EDF4126)
PAF_VSH_UNKNOWN_STUB(8FA84E15)
PAF_VSH_UNKNOWN_STUB(918BCA8B)
PAF_VSH_UNKNOWN_STUB(91973397)
PAF_VSH_UNKNOWN_STUB(942E8711)
PAF_VSH_UNKNOWN_STUB(94AFA971)
PAF_VSH_UNKNOWN_STUB(9684511F)
PAF_VSH_UNKNOWN_STUB(96ECA540)
PAF_VSH_UNKNOWN_STUB(995294D1)
PAF_VSH_UNKNOWN_STUB(9A418CCC)
PAF_VSH_UNKNOWN_STUB(9AE88FFF)
PAF_VSH_UNKNOWN_STUB(9AF12ADC)
PAF_VSH_UNKNOWN_STUB(9D93A13C)
PAF_VSH_UNKNOWN_STUB(9E236520)
PAF_VSH_UNKNOWN_STUB(9EEFEF9D)
PAF_VSH_UNKNOWN_STUB(9EF44AF8)
#undef PAF_VSH_UNKNOWN_STUB

VSH_STUB(scePaf_9F12D4D1)
VSH_STUB(scePaf_9F3E4ED9)
VSH_STUB(scePaf_A04B4F40)

static bool scePafGetVarArg(int argIndex, u32 *value) {
	if (argIndex <= 1) {
		*value = currentMIPS->r[MIPS_REG_A2 + argIndex];
		return true;
	}
	if (argIndex <= 5) {
		*value = currentMIPS->r[MIPS_REG_T0 + argIndex - 2];
		return true;
	}
	int stackIndex = argIndex - 6;
	u32 stackAddr = currentMIPS->r[MIPS_REG_SP] + stackIndex * 4;
	if (!Memory::IsValidAddress(stackAddr)) {
		return false;
	}
	*value = Memory::Read_U32(stackAddr);
	return true;
}

static int scePaf_A138A376(u32 dst, u32 fmt) {
	if (!Memory::IsValidNullTerminatedString(fmt)) {
		return hleNoLog(0);
	}

	std::string result;
	bool processingSpecifier = false;
	std::string specifier;
	int bytesToRead = 0;
	int argIndex = 0;

	for (const char *c = Memory::GetCharPointerUnchecked(fmt); *c != '\0'; ++c) {
		if (!processingSpecifier) {
			if (*c == '%') {
				specifier = "%";
				processingSpecifier = true;
				bytesToRead = 0;
			} else {
				result.push_back(*c);
			}
			continue;
		}

		specifier.push_back(*c);
		switch (*c) {
		case '%':
			result.append(specifier);
			processingSpecifier = false;
			break;
		case 's': {
			u32 val = 0;
			if (!scePafGetVarArg(argIndex, &val)) {
				return hleNoLog(0);
			}
			argIndex++;
			if (!Memory::IsValidNullTerminatedString(val)) {
				return hleNoLog(0);
			}
			result.append(Memory::GetCharPointerUnchecked(val));
			processingSpecifier = false;
			break;
		}
		case 'd':
		case 'i':
		case 'u':
		case 'o':
		case 'x':
		case 'X':
		case 'f':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
		case 'c':
		case 'p':
		case 'n': {
			u64 val = 0;
			if (bytesToRead == 0) {
				bytesToRead = 4;
			}
			int readCnt = 0;
			while (bytesToRead > 0) {
				u32 valFromArg = 0;
				if (!scePafGetVarArg(argIndex, &valFromArg)) {
					return hleNoLog(0);
				}
				argIndex++;
				val |= (u64)valFromArg << (readCnt * 32);
				bytesToRead -= 4;
				readCnt++;
			}
			char buf[128] = {};
			snprintf(buf, sizeof(buf), specifier.c_str(), val);
			buf[sizeof(buf) - 1] = '\0';
			result.append(buf);
			processingSpecifier = false;
			break;
		}
		case 'h':
			bytesToRead = 4;
			break;
		case 'l':
			bytesToRead += 4;
			break;
		default:
			result.append(specifier);
			processingSpecifier = false;
			break;
		}
	}

	if (processingSpecifier) {
		result.append(specifier);
	}

	if (!Memory::IsValidRange(dst, (u32)result.size() + 1)) {
		return hleNoLog(0);
	}
	memcpy(Memory::GetPointerWriteUnchecked(dst), result.c_str(), result.size() + 1);
	return hleNoLog((int)result.size());
}

VSH_STUB(scePaf_A140233F)
VSH_STUB(scePaf_A2ADF6EC)
VSH_STUB(scePaf_A4EAD0A0)
VSH_STUB(scePaf_A5BBFD24)
VSH_STUB(scePaf_A69A5266)
VSH_STUB(scePaf_A7353F2A)
VSH_STUB(scePaf_A7784A89)
VSH_STUB(scePaf_A9645067)
VSH_STUB(scePaf_A989A2C4)
VSH_STUB(scePaf_AB0731F2)
VSH_STUB(scePaf_ABB3AE55)
VSH_STUB(scePaf_AC29C247)
VSH_STUB(scePaf_AC98B0C3)
VSH_STUB(scePaf_AD454A8F)
VSH_STUB(scePaf_AE5D4943)
VSH_STUB(scePaf_AF067FA2)
VSH_STUB(scePaf_B03CB9A0)
static int scePaf_B05D9677(u32 dst, u32 src, u32 size) {
	if (Memory::IsValidRange(dst, size) && Memory::IsValidRange(src, size)) {
		return hleNoLog(memcmp(Memory::GetCharPointerUnchecked(dst), Memory::GetCharPointerUnchecked(src), size));
	}
	return hleNoLog(0);
}
VSH_STUB(scePaf_B4C7DE8E)
VSH_STUB(scePaf_B6A3AB1E)
VSH_STUB(scePaf_B6D58D98)
VSH_STUB(scePaf_B73A78AF)
VSH_STUB(scePaf_B94AD2A4)
VSH_STUB(scePaf_BCA8DE39)
VSH_STUB(scePaf_C0E15A55)
VSH_STUB(scePaf_C1B36CF9)
VSH_STUB(scePaf_C1D8F2DC)
VSH_STUB(scePaf_C22BACF3)
VSH_STUB(scePaf_C327FF14)
VSH_STUB(scePaf_C3B97D0D)
VSH_STUB(scePaf_C3D5A672)
VSH_STUB(scePaf_C59FC3D0)
VSH_STUB(scePaf_C5C17E46)
VSH_STUB(scePaf_C5C4707A)
VSH_STUB(scePaf_C735969B)
VSH_STUB(scePaf_C7F002F1)
VSH_STUB(scePaf_C839B167)
VSH_STUB(scePaf_C853CAF6)
VSH_STUB(scePaf_C8F930E9)
VSH_STUB(scePaf_CCC1E8F7)
VSH_STUB(scePaf_D0D9615E)
VSH_STUB(scePaf_D0DFA412)
VSH_STUB(scePaf_D1755221)
VSH_STUB(scePaf_D21E2DFC)
VSH_STUB(scePaf_D229572C)
VSH_STUB(scePaf_D2FCE506)
VSH_STUB(scePaf_D31D6132)
VSH_STUB(scePaf_D5F2C162)
VSH_STUB(scePaf_D6A63A45)
VSH_STUB(scePaf_D9FC24EA)
VSH_STUB(scePaf_DA8D097D)
VSH_STUB(scePaf_DB1612F1)
VSH_STUB(scePaf_DD10B322)
VSH_STUB(scePaf_DD23970A)
VSH_STUB(scePaf_DD588D1C)
VSH_STUB(scePaf_DDE74F3B)
VSH_STUB(scePaf_E0B32AE8)
VSH_STUB(scePaf_E1B1B2F9)
VSH_STUB(scePaf_E4B97CA1)
VSH_STUB(scePaf_E67517D2)
VSH_STUB(scePaf_E6D56019)
VSH_STUB(scePaf_E7263320)
VSH_STUB(scePaf_E9570AA3)
VSH_STUB(scePaf_EA681A0B)
VSH_STUB(scePaf_F09010AF)
VSH_STUB(scePaf_F207376F)
VSH_STUB(scePaf_F265E4F0)
VSH_STUB(scePaf_F4C130B8)
VSH_STUB(scePaf_F4EBA045)
VSH_STUB(scePaf_FA60E9C4)
VSH_STUB(scePaf_FAB38CB0)
VSH_STUB(scePaf_FD048927)
VSH_STUB(scePaf_FD38EF6C)
VSH_STUB(scePaf_FE99F522)
VSH_STUB(scePaf_FF01EBA9)
VSH_STUB(scePaf_FFBAD89D)

static u32 scePaf_D9E2D6E1(u32 dst, int value, u32 size) {
	return scePafMemset(dst, value, size);
}


VSH_STUB(scePaf_4F7B8133)
VSH_STUB(scePaf_505EA775)
VSH_STUB(scePaf_512737D3)
VSH_STUB(scePaf_537B3828)
VSH_STUB(scePaf_53B0C35F)
VSH_STUB(scePaf_5476FD2B)
VSH_STUB(scePaf_5CC6AA3C)
VSH_STUB(scePaf_5E909060)
VSH_STUB(scePaf_726DFBA9)
static int scePaf_77ADEAEB() {
	return hleNoLog((int)scePafGetSharedStubContext());
}
VSH_STUB(scePaf_781E5F1E)
VSH_STUB(scePaf_85CFB6E6)
VSH_STUB(scePaf_9FA3F3F6)
static u32 scePaf_B9DDDCC8() {
	return hleNoLog(scePafGetSharedStubContext());
}
VSH_STUB(scePaf_CC3426BF)
VSH_STUB(scePaf_D590412B)
VSH_STUB(scePaf_D7DCB972)
VSH_STUB(scePaf_E00895CD)
VSH_STUB(scePaf_F48A9040)
VSH_STUB(scePaf_FC5C8A32)
static u32 scePaf_40A2FAC3() {
	return hleNoLog(scePafAllocStubBlock("scePaf_40A2FAC3"));
}

static u32 scePaf_412B2F09() {
	return hleNoLog(scePafGetSharedStubContext());
}

static u32 scePaf_BB89C9EA(u32 dst, int value, u32 size) {
	return scePafMemset(dst, value, size);
}

#undef VSH_STUB

const HLEFunction sceVshBridge[] =
{
	{0X01730088, &WrapI_V<sceVshBridge_01730088>,                     "sceVshBridge_01730088",                   'i', ""   },
	{0X0543156C, &WrapI_V<sceVshBridge_0543156C>,                     "sceVshBridge_0543156C",                   'i', ""   },
	{0X0C0D5913, &WrapI_V<sceVshBridge_0C0D5913>,                     "sceVshBridge_0C0D5913",                   'i', ""   },
	{0X0D684A0B, &WrapI_V<sceVshBridge_0D684A0B>,                     "sceVshBridge_0D684A0B",                   'i', ""   },
	{0X0D7A4FE4, &WrapI_V<sceVshBridge_0D7A4FE4>,                     "sceVshBridge_0D7A4FE4",                   'i', ""   },
	{0X12B07B05, &WrapI_V<sceVshBridge_12B07B05>,                     "sceVshBridge_12B07B05",                   'i', ""   },
	{0X1D5C579F, &WrapI_V<sceVshBridge_1D5C579F>,                     "sceVshBridge_1D5C579F",                   'i', ""   },
	{0X21C243FE, &WrapI_V<sceVshBridge_21C243FE>,                     "sceVshBridge_21C243FE",                   'i', ""   },
	{0X21D4D038, &WrapI_V<sceVshBridge_21D4D038>,                     "sceVshBridge_21D4D038",                   'i', ""   },
	{0X2380DC08, &WrapI_V<sceVshBridge_2380DC08>,                     "sceVshBridge_2380DC08",                   'i', ""   },
	{0X27BDA326, &WrapI_V<sceVshBridge_27BDA326>,                     "sceVshBridge_27BDA326",                   'i', ""   },
	{0X27CD418C, &WrapI_V<sceVshBridge_27CD418C>,                     "sceVshBridge_27CD418C",                   'i', ""   },
	{0X29CDFFBA, &WrapI_V<sceVshBridge_29CDFFBA>,                     "sceVshBridge_29CDFFBA",                   'i', ""   },
	{0X2EBD2323, &WrapI_V<sceVshBridge_2EBD2323>,                     "sceVshBridge_2EBD2323",                   'i', ""   },
	{0X3785D08B, &WrapI_V<sceVshBridge_3785D08B>,                     "sceVshBridge_3785D08B",                   'i', ""   },
	{0X3A46C639, &WrapI_V<sceVshBridge_3A46C639>,                     "sceVshBridge_3A46C639",                   'i', ""   },
	{0X3C90E435, &WrapI_V<sceVshBridge_3C90E435>,                     "sceVshBridge_3C90E435",                   'i', ""   },
	{0X3D30FEB6, &WrapI_V<sceVshBridge_3D30FEB6>,                     "sceVshBridge_3D30FEB6",                   'i', ""   },
	{0X4A596D2D, &WrapI_V<sceVshBridge_4A596D2D>,                     "sceVshBridge_4A596D2D",                   'i', ""   },
	{0X4DB43867, &WrapI_V<sceVshBridge_4DB43867>,                     "sceVshBridge_4DB43867",                   'i', ""   },
	{0X53BFD101, &WrapI_V<sceVshBridge_53BFD101>,                     "sceVshBridge_53BFD101",                   'i', ""   },
	{0X582B5281, &WrapI_V<sceVshBridge_582B5281>,                     "sceVshBridge_582B5281",                   'i', ""   },
	{0X59197BE8, &WrapI_V<sceVshBridge_59197BE8>,                     "sceVshBridge_59197BE8",                   'i', ""   },
	{0X5B7F3339, &WrapI_V<sceVshBridge_5B7F3339>,                     "sceVshBridge_5B7F3339",                   'i', ""   },
	{0X5C2983C2, &WrapI_V<sceVshBridge_5C2983C2>,                     "sceVshBridge_5C2983C2",                   'i', ""   },
	{0X5E0F5543, &WrapI_V<sceVshBridge_5E0F5543>,                     "sceVshBridge_5E0F5543",                   'i', ""   },
	{0X5E5AF7A2, &WrapI_V<sceVshBridge_5E5AF7A2>,                     "sceVshBridge_5E5AF7A2",                   'i', ""   },
	{0X5F35E8FE, &WrapI_V<sceVshBridge_5F35E8FE>,                     "sceVshBridge_5F35E8FE",                   'i', ""   },
	{0X61001D64, &WrapI_V<sceVshBridge_61001D64>,                     "sceVshBridge_61001D64",                   'i', ""   },
	{0X63047647, &WrapI_V<sceVshBridge_63047647>,                     "sceVshBridge_63047647",                   'i', ""   },
	{0X639C3CB3, &WrapI_V<sceVshBridge_639C3CB3>,                     "sceVshBridge_639C3CB3",                   'i', ""   },
	{0X63E69956, &WrapI_V<sceVshBridge_63E69956>,                     "sceVshBridge_63E69956",                   'i', ""   },
	{0X6CAEB765, &WrapI_V<sceVshBridge_6CAEB765>,                     "sceVshBridge_6CAEB765",                   'i', ""   },
	{0X7423151D, &WrapI_V<sceVshBridge_7423151D>,                     "sceVshBridge_7423151D",                   'i', ""   },
	{0X74DA9D25, &WrapI_V<sceVshBridge_74DA9D25>,                     "sceVshBridge_74DA9D25",                   'i', ""   },
	{0X74DBE57E, &WrapI_V<sceVshBridge_74DBE57E>,                     "sceVshBridge_74DBE57E",                   'i', ""   },
	{0X791FCD43, &WrapI_V<sceVshBridge_791FCD43>,                     "sceVshBridge_791FCD43",                   'i', ""   },
	{0X79B916E1, &WrapI_V<sceVshBridge_79B916E1>,                     "sceVshBridge_79B916E1",                   'i', ""   },
	{0X7A90D816, &WrapI_V<sceVshBridge_7A90D816>,                     "sceVshBridge_7A90D816",                   'i', ""   },
	{0X7B14CE2B, &WrapI_V<sceVshBridge_7B14CE2B>,                     "sceVshBridge_7B14CE2B",                   'i', ""   },
	{0X7D1C13B5, &WrapI_V<sceVshBridge_7D1C13B5>,                     "sceVshBridge_7D1C13B5",                   'i', ""   },
	{0X7E117907, &WrapI_V<sceVshBridge_7E117907>,                     "sceVshBridge_7E117907",                   'i', ""   },
	{0X81682A40, &WrapI_V<sceVshBridge_81682A40>,                     "sceVshBridge_81682A40",                   'i', ""   },
	{0X837C457A, &WrapI_V<sceVshBridge_837C457A>,                     "sceVshBridge_837C457A",                   'i', ""   },
	{0X88C35487, &WrapI_V<sceVshBridge_88C35487>,                     "sceVshBridge_88C35487",                   'i', ""   },
	{0X9056DE3A, &WrapI_V<sceVshBridge_9056DE3A>,                     "sceVshBridge_9056DE3A",                   'i', ""   },
	{0X9347D693, &WrapI_V<sceVshBridge_9347D693>,                     "sceVshBridge_9347D693",                   'i', ""   },
	{0X9427C909, &WrapI_V<sceVshBridge_9427C909>,                     "sceVshBridge_9427C909",                   'i', ""   },
	{0X9940D95C, &WrapI_V<sceVshBridge_9940D95C>,                     "sceVshBridge_9940D95C",                   'i', ""   },
	{0XA29B5A33, &WrapI_V<sceVshBridge_A29B5A33>,                     "sceVshBridge_A29B5A33",                   'i', ""   },
	{0XA5628F0D, &WrapI_V<sceVshBridge_A5628F0D>,                     "sceVshBridge_A5628F0D",                   'i', ""   },
	{0XAAB9A9EF, &WrapI_V<sceVshBridge_AAB9A9EF>,                     "sceVshBridge_AAB9A9EF",                   'i', ""   },
	{0XABB84565, &WrapI_V<sceVshBridge_ABB84565>,                     "sceVshBridge_ABB84565",                   'i', ""   },
	{0XB8B07CAF, &WrapI_V<sceVshBridge_B8B07CAF>,                     "sceVshBridge_B8B07CAF",                   'i', ""   },
	{0XC51A6C26, &WrapI_V<sceVshBridge_C51A6C26>,                     "sceVshBridge_C51A6C26",                   'i', ""   },
	{0XC6395C03, &WrapI_V<sceVshBridge_C6395C03>,                     "sceVshBridge_C6395C03",                   'i', ""   },
	{0XC949966C, &WrapI_V<sceVshBridge_C949966C>,                     "sceVshBridge_C949966C",                   'i', ""   },
	{0XCCD27632, &WrapI_V<sceVshBridge_CCD27632>,                     "sceVshBridge_CCD27632",                   'i', ""   },
	{0XCD1A2C46, &WrapI_V<sceVshBridge_CD1A2C46>,                     "sceVshBridge_CD1A2C46",                   'i', ""   },
	{0XCE32CBEF, &WrapI_V<sceVshBridge_CE32CBEF>,                     "sceVshBridge_CE32CBEF",                   'i', ""   },
	{0XD39DE400, &WrapI_V<sceVshBridge_D39DE400>,                     "sceVshBridge_D39DE400",                   'i', ""   },
	{0XD47041CA, &WrapI_V<sceVshBridge_D47041CA>,                     "sceVshBridge_D47041CA",                   'i', ""   },
	{0XD7D7E7B6, &WrapI_V<sceVshBridge_D7D7E7B6>,                     "sceVshBridge_D7D7E7B6",                   'i', ""   },
	{0XDB7C3D5A, &WrapI_V<sceVshBridge_DB7C3D5A>,                     "sceVshBridge_DB7C3D5A",                   'i', ""   },
	{0XE174218C, &WrapI_V<sceVshBridge_E174218C>,                     "sceVshBridge_E174218C",                   'i', ""   },
	{0XE533E98C, &WrapI_V<sceVshBridge_E533E98C>,                     "sceVshBridge_E533E98C",                   'i', ""   },
	{0XE5DA5E95, &WrapI_V<sceVshBridge_E5DA5E95>,                     "sceVshBridge_E5DA5E95",                   'i', ""   },
	{0XEBC3A334, &WrapI_V<sceVshBridge_EBC3A334>,                     "sceVshBridge_EBC3A334",                   'i', ""   },
};

const HLEFunction sceVshCommonUtil[] =
{
	{0X0A708F40, &WrapI_V<sceVshCommonUtil_0A708F40>,                 "sceVshCommonUtil_0A708F40",               'i', ""   },
	{0X1BC70F77, &WrapI_V<sceVshCommonUtil_1BC70F77>,                 "sceVshCommonUtil_1BC70F77",               'i', ""   },
	{0X1D7E0B27, &WrapI_V<sceVshCommonUtil_1D7E0B27>,                 "sceVshCommonUtil_1D7E0B27",               'i', ""   },
	{0X2058CEAA, &WrapI_V<sceVshCommonUtil_2058CEAA>,                 "sceVshCommonUtil_2058CEAA",               'i', ""   },
	{0X2D320307, &WrapI_V<sceVshCommonUtil_2D320307>,                 "sceVshCommonUtil_2D320307",               'i', ""   },
	{0X34826464, &WrapI_V<sceVshCommonUtil_34826464>,                 "sceVshCommonUtil_34826464",               'i', ""   },
	{0X38C93C16, &WrapI_V<sceVshCommonUtil_38C93C16>,                 "sceVshCommonUtil_38C93C16",               'i', ""   },
	{0X392A75EF, &WrapI_V<sceVshCommonUtil_392A75EF>,                 "sceVshCommonUtil_392A75EF",               'i', ""   },
	{0X3CA4266A, &WrapI_V<sceVshCommonUtil_3CA4266A>,                 "sceVshCommonUtil_3CA4266A",               'i', ""   },
	{0X3F28E621, &WrapI_V<sceVshCommonUtil_3F28E621>,                 "sceVshCommonUtil_3F28E621",               'i', ""   },
	{0X4A92625E, &WrapI_V<sceVshCommonUtil_4A92625E>,                 "sceVshCommonUtil_4A92625E",               'i', ""   },
	{0X5A1359E0, &WrapI_V<sceVshCommonUtil_5A1359E0>,                 "sceVshCommonUtil_5A1359E0",               'i', ""   },
	{0X649C3568, &WrapI_V<sceVshCommonUtil_649C3568>,                 "sceVshCommonUtil_649C3568",               'i', ""   },
	{0X8381781A, &WrapI_V<sceVshCommonUtil_8381781A>,                 "sceVshCommonUtil_8381781A",               'i', ""   },
	{0X9869763D, &WrapI_V<sceVshCommonUtil_9869763D>,                 "sceVshCommonUtil_9869763D",               'i', ""   },
	{0XA16F632D, &WrapI_V<sceVshCommonUtil_A16F632D>,                 "sceVshCommonUtil_A16F632D",               'i', ""   },
	{0XA721B1D1, &WrapI_V<sceVshCommonUtil_A721B1D1>,                 "sceVshCommonUtil_A721B1D1",               'i', ""   },
	{0XBC01322B, &WrapI_V<sceVshCommonUtil_BC01322B>,                 "sceVshCommonUtil_BC01322B",               'i', ""   },
	{0XE6C36E64, &WrapI_V<sceVshCommonUtil_E6C36E64>,                 "sceVshCommonUtil_E6C36E64",               'i', ""   },
	{0XF9DB4018, &WrapI_V<sceVshCommonUtil_F9DB4018>,                 "sceVshCommonUtil_F9DB4018",               'i', ""   },
	{0XFC3BC7EE, &WrapI_V<sceVshCommonUtil_FC3BC7EE>,                 "sceVshCommonUtil_FC3BC7EE",               'i', ""   },
};


const HLEFunction sceUtility_netparam_internal[] =
{
	{0X02598DB8, &WrapI_V<sceUtility_netparam_internal_02598DB8>,     "sceUtility_netparam_internal_02598DB8", 'i', ""   },
	{0X09905636, &WrapI_V<sceUtility_netparam_internal_09905636>,     "sceUtility_netparam_internal_09905636", 'i', ""   },
	{0X6B1BC62C, &WrapI_V<sceUtility_netparam_internal_6B1BC62C>,     "sceUtility_netparam_internal_6B1BC62C", 'i', ""   },
};

const HLEFunction scePaf[] =
{
	{0X0085C4A5, &WrapU_V<scePaf_0085C4A5>,                          "scePaf_0085C4A5",                        'x', ""   },
	{0X00B832B2, &WrapU_V<scePaf_00B832B2>,                          "scePaf_00B832B2",                        'x', ""   },
	{0X02A60EBD, &WrapU_V<scePaf_02A60EBD>,                          "scePaf_02A60EBD",                        'x', ""   },
	{0X04A7B506, &WrapU_V<scePaf_04A7B506>,                          "scePaf_04A7B506",                        'x', ""   },
	{0X04E015E4, &WrapU_V<scePaf_04E015E4>,                          "scePaf_04E015E4",                        'x', ""   },
	{0X05E14B72, &WrapU_V<scePaf_05E14B72>,                          "scePaf_05E14B72",                        'x', ""   },
	{0X075FACE4, &WrapU_V<scePaf_075FACE4>,                          "scePaf_075FACE4",                        'x', ""   },
	{0X0929EF0D, &WrapU_V<scePaf_0929EF0D>,                          "scePaf_0929EF0D",                        'x', ""   },
	{0X0A1E907A, &WrapU_V<scePaf_0A1E907A>,                          "scePaf_0A1E907A",                        'x', ""   },
	{0X0B95292E, &WrapU_V<scePaf_0B95292E>,                          "scePaf_0B95292E",                        'x', ""   },
	{0X0C2CD696, &WrapU_V<scePaf_0C2CD696>,                          "scePaf_0C2CD696",                        'x', ""   },
	{0X0E8CA0D0, &WrapU_V<scePaf_0E8CA0D0>,                          "scePaf_0E8CA0D0",                        'x', ""   },
	{0X0EE8FFFF, &WrapI_V<scePaf_0EE8FFFF>,                          "scePaf_0EE8FFFF",                        'i', ""   },
	{0X0F19FF55, &WrapI_V<scePaf_0F19FF55>,                          "scePaf_0F19FF55",                        'i', ""   },
	{0X0FC5E0EF, &WrapI_V<scePaf_0FC5E0EF>,                          "scePaf_0FC5E0EF",                        'i', ""   },
	{0X0FC66D1C, &WrapI_V<scePaf_0FC66D1C>,                          "scePaf_0FC66D1C",                        'i', ""   },
	{0X10E9CF5B, &WrapI_V<scePaf_10E9CF5B>,                          "scePaf_10E9CF5B",                        'i', ""   },
	{0X11D950F0, &WrapI_V<scePaf_11D950F0>,                          "scePaf_11D950F0",                        'i', ""   },
	{0X126454E6, &WrapI_V<scePaf_126454E6>,                          "scePaf_126454E6",                        'i', ""   },
	{0X13B2C7BB, &WrapI_V<scePaf_13B2C7BB>,                          "scePaf_13B2C7BB",                        'i', ""   },
	{0X14EF73B2, &WrapI_V<scePaf_14EF73B2>,                          "scePaf_14EF73B2",                        'i', ""   },
	{0X167BA176, &WrapI_V<scePaf_167BA176>,                          "scePaf_167BA176",                        'i', ""   },
	{0X16CA066C, &WrapI_V<scePaf_16CA066C>,                          "scePaf_16CA066C",                        'i', ""   },
	{0X1775C8E3, &WrapI_V<scePaf_1775C8E3>,                          "scePaf_1775C8E3",                        'i', ""   },
	{0X177C1E4F, &WrapI_V<scePaf_177C1E4F>,                          "scePaf_177C1E4F",                        'i', ""   },
	{0X17B4723E, &WrapI_V<scePaf_17B4723E>,                          "scePaf_17B4723E",                        'i', ""   },
	{0X1941469E, &WrapI_V<scePaf_1941469E>,                          "scePaf_1941469E",                        'i', ""   },
	{0X1C80A4DC, &WrapI_V<scePaf_1C80A4DC>,                          "scePaf_1C80A4DC",                        'i', ""   },
	{0X1E443486, &WrapI_V<scePaf_1E443486>,                          "scePaf_1E443486",                        'i', ""   },
	{0X1FB4579B, &WrapI_V<scePaf_1FB4579B>,                          "scePaf_1FB4579B",                        'i', ""   },
	{0X22420CC7, &WrapI_V<scePaf_22420CC7>,                          "scePaf_22420CC7",                        'i', ""   },
	{0X22560E39, &WrapI_V<scePaf_22560E39>,                          "scePaf_22560E39",                        'i', ""   },
	{0X23A245C3, &WrapI_V<scePaf_23A245C3>,                          "scePaf_23A245C3",                        'i', ""   },
	{0X26717465, &WrapI_V<scePaf_26717465>,                          "scePaf_26717465",                        'i', ""   },
	{0X279F57C3, &WrapI_V<scePaf_279F57C3>,                          "scePaf_279F57C3",                        'i', ""   },
	{0X28C3DFA5, &WrapI_V<scePaf_28C3DFA5>,                          "scePaf_28C3DFA5",                        'i', ""   },
	{0X2B889C55, &WrapI_V<scePaf_2B889C55>,                          "scePaf_2B889C55",                        'i', ""   },
	{0X2C0F0975, &WrapI_V<scePaf_2C0F0975>,                          "scePaf_2C0F0975",                        'i', ""   },
	{0X2C22C6FB, &WrapI_V<scePaf_2C22C6FB>,                          "scePaf_2C22C6FB",                        'i', ""   },
	{0X2C23C4E2, &WrapI_V<scePaf_2C23C4E2>,                          "scePaf_2C23C4E2",                        'i', ""   },
	{0X2D138A74, &WrapI_V<scePaf_2D138A74>,                          "scePaf_2D138A74",                        'i', ""   },
	{0X2D92A298, &WrapI_V<scePaf_2D92A298>,                          "scePaf_2D92A298",                        'i', ""   },
	{0X304F6CF4, &WrapI_V<scePaf_304F6CF4>,                          "scePaf_304F6CF4",                        'i', ""   },
	{0X319A6571, &WrapI_V<scePaf_319A6571>,                          "scePaf_319A6571",                        'i', ""   },
	{0X327B34BF, &WrapI_V<scePaf_327B34BF>,                          "scePaf_327B34BF",                        'i', ""   },
	{0X330BD953, &WrapI_V<scePaf_330BD953>,                          "scePaf_330BD953",                        'i', ""   },
	{0X362F4258, &WrapI_V<scePaf_362F4258>,                          "scePaf_362F4258",                        'i', ""   },
	{0X36E2C527, &WrapI_V<scePaf_36E2C527>,                          "scePaf_36E2C527",                        'i', ""   },
	{0X3710132E, &WrapI_V<scePaf_3710132E>,                          "scePaf_3710132E",                        'i', ""   },
	{0X3782A7C2, &WrapI_V<scePaf_3782A7C2>,                          "scePaf_3782A7C2",                        'i', ""   },
	{0X379A3D64, &WrapI_V<scePaf_379A3D64>,                          "scePaf_379A3D64",                        'i', ""   },
	{0X37A80B89, &WrapI_V<scePaf_37A80B89>,                          "scePaf_37A80B89",                        'i', ""   },
	{0X37ABB4A1, &WrapI_V<scePaf_37ABB4A1>,                          "scePaf_37ABB4A1",                        'i', ""   },
	{0X3874A5F8, &WrapI_V<scePaf_3874A5F8>,                          "scePaf_3874A5F8",                        'i', ""   },
	{0X3876721B, &WrapI_V<scePaf_3876721B>,                          "scePaf_3876721B",                        'i', ""   },
	{0X3A4504D5, &WrapI_V<scePaf_3A4504D5>,                          "scePaf_3A4504D5",                        'i', ""   },
	{0X3A5E1022, &WrapI_V<scePaf_3A5E1022>,                          "scePaf_3A5E1022",                        'i', ""   },
	{0X3BF0DB62, &WrapI_V<scePaf_3BF0DB62>,                          "scePaf_3BF0DB62",                        'i', ""   },
	{0X3C0E9C2D, &WrapI_V<scePaf_3C0E9C2D>,                          "scePaf_3C0E9C2D",                        'i', ""   },
	{0X3E22B04E, &WrapI_V<scePaf_3E22B04E>,                          "scePaf_3E22B04E",                        'i', ""   },
	{0X3ED6EA7F, &WrapI_V<scePaf_3ED6EA7F>,                          "scePaf_3ED6EA7F",                        'i', ""   },
	{0X3F65FC71, &WrapI_V<scePaf_3F65FC71>,                          "scePaf_3F65FC71",                        'i', ""   },
	{0X428E7562, &WrapI_V<scePaf_428E7562>,                          "scePaf_428E7562",                        'i', ""   },
	{0X430D11DC, &WrapI_V<scePaf_430D11DC>,                          "scePaf_430D11DC",                        'i', ""   },
	{0X438B6847, &WrapI_V<scePaf_438B6847>,                          "scePaf_438B6847",                        'i', ""   },
	{0X440665DB, &WrapI_V<scePaf_440665DB>,                          "scePaf_440665DB",                        'i', ""   },
	{0X465B3A83, &WrapI_V<scePaf_465B3A83>,                          "scePaf_465B3A83",                        'i', ""   },
	{0X4792CA66, &WrapI_V<scePaf_4792CA66>,                          "scePaf_4792CA66",                        'i', ""   },
	{0X485B96FB, &WrapI_V<scePaf_485B96FB>,                          "scePaf_485B96FB",                        'i', ""   },
	{0X491E69CE, &WrapI_V<scePaf_491E69CE>,                          "scePaf_491E69CE",                        'i', ""   },
	{0X4AA46320, &WrapI_V<scePaf_4AA46320>,                          "scePaf_4AA46320",                        'i', ""   },
	{0X4AF28AF7, &WrapI_V<scePaf_4AF28AF7>,                          "scePaf_4AF28AF7",                        'i', ""   },
	{0X4BC342F7, &WrapI_V<scePaf_4BC342F7>,                          "scePaf_4BC342F7",                        'i', ""   },
	{0X54807A37, &WrapI_V<scePaf_54807A37>,                          "scePaf_54807A37",                        'i', ""   },
	{0X58BD2AEB, &WrapI_V<scePaf_58BD2AEB>,                          "scePaf_58BD2AEB",                        'i', ""   },
	{0X5905DF98, &WrapI_V<scePaf_5905DF98>,                          "scePaf_5905DF98",                        'i', ""   },
	{0X5A6CAFE2, &WrapI_V<scePaf_5A6CAFE2>,                          "scePaf_5A6CAFE2",                        'i', ""   },
	{0X5B257A25, &WrapI_V<scePaf_5B257A25>,                          "scePaf_5B257A25",                        'i', ""   },
	{0X5B46340C, &WrapI_V<scePaf_5B46340C>,                          "scePaf_5B46340C",                        'i', ""   },
	{0X5CE2ACF4, &WrapI_V<scePaf_5CE2ACF4>,                          "scePaf_5CE2ACF4",                        'i', ""   },
	{0X5D22ED89, &WrapI_V<scePaf_5D22ED89>,                          "scePaf_5D22ED89",                        'i', ""   },
	{0X5E9078F9, &WrapI_V<scePaf_5E9078F9>,                          "scePaf_5E9078F9",                        'i', ""   },
	{0X6195CF7C, &WrapI_V<scePaf_6195CF7C>,                          "scePaf_6195CF7C",                        'i', ""   },
	{0X6332FF84, &WrapI_V<scePaf_6332FF84>,                          "scePaf_6332FF84",                        'i', ""   },
	{0X66832B20, &WrapI_V<scePaf_66832B20>,                          "scePaf_66832B20",                        'i', ""   },
	{0X675047B1, &WrapI_V<scePaf_675047B1>,                          "scePaf_675047B1",                        'i', ""   },
	{0X67FF4EBC, &WrapI_V<scePaf_67FF4EBC>,                          "scePaf_67FF4EBC",                        'i', ""   },
	{0X685F8622, &WrapI_V<scePaf_685F8622>,                          "scePaf_685F8622",                        'i', ""   },
	{0X68685412, &WrapI_V<scePaf_68685412>,                          "scePaf_68685412",                        'i', ""   },
	{0X68B4E5A8, &WrapI_V<scePaf_68B4E5A8>,                          "scePaf_68B4E5A8",                        'i', ""   },
	{0X698098B9, &WrapI_V<scePaf_698098B9>,                          "scePaf_698098B9",                        'i', ""   },
	{0X69A1E33B, &WrapI_V<scePaf_69A1E33B>,                          "scePaf_69A1E33B",                        'i', ""   },
	{0X69D71FBF, &WrapI_V<scePaf_69D71FBF>,                          "scePaf_69D71FBF",                        'i', ""   },
	{0X6A17A477, &WrapI_V<scePaf_6A17A477>,                          "scePaf_6A17A477",                        'i', ""   },
	{0X6E36E1D9, &WrapI_V<scePaf_6E36E1D9>,                          "scePaf_6E36E1D9",                        'i', ""   },
	{0X6F3415DD, &WrapI_V<scePaf_6F3415DD>,                          "scePaf_6F3415DD",                        'i', ""   },
	{0X6FBEAC03, &WrapI_V<scePaf_6FBEAC03>,                          "scePaf_6FBEAC03",                        'i', ""   },
	{0X706ABBFF, &WrapI_V<scePaf_706ABBFF>,                          "scePaf_706ABBFF",                        'i', ""   },
	{0X713256AB, &WrapI_V<scePaf_713256AB>,                          "scePaf_713256AB",                        'i', ""   },
	{0X72B16452, &WrapI_V<scePaf_72B16452>,                          "scePaf_72B16452",                        'i', ""   },
	{0X7364C1F7, &WrapI_V<scePaf_7364C1F7>,                          "scePaf_7364C1F7",                        'i', ""   },
	{0X73DA59C1, &WrapI_V<scePaf_73DA59C1>,                          "scePaf_73DA59C1",                        'i', ""   },
	{0X73F0D607, &WrapI_V<scePaf_73F0D607>,                          "scePaf_73F0D607",                        'i', ""   },
	{0X73F36F1F, &WrapI_V<scePaf_73F36F1F>,                          "scePaf_73F36F1F",                        'i', ""   },
	{0X777E5AC9, &WrapI_V<scePaf_777E5AC9>,                          "scePaf_777E5AC9",                        'i', ""   },
	{0X78037E0C, &WrapI_V<scePaf_78037E0C>,                          "scePaf_78037E0C",                        'i', ""   },
	{0X7869A05E, &WrapI_V<scePaf_7869A05E>,                          "scePaf_7869A05E",                        'i', ""   },
	{0X79174B51, &WrapI_V<scePaf_79174B51>,                          "scePaf_79174B51",                        'i', ""   },
	{0X7B7133D5, &WrapI_V<scePaf_7B7133D5>,                          "scePaf_7B7133D5",                        'i', ""   },
	{0X7B77B7E7, &WrapI_V<scePaf_7B77B7E7>,                          "scePaf_7B77B7E7",                        'i', ""   },
	{0X7C92BF10, &WrapI_V<scePaf_7C92BF10>,                          "scePaf_7C92BF10",                        'i', ""   },
	{0X7D7DEB7E, &WrapI_V<scePaf_7D7DEB7E>,                          "scePaf_7D7DEB7E",                        'i', ""   },
	{0X7DF192FA, &WrapI_V<scePaf_7DF192FA>,                          "scePaf_7DF192FA",                        'i', ""   },
	{0X7E0A6B25, &WrapI_V<scePaf_7E0A6B25>,                          "scePaf_7E0A6B25",                        'i', ""   },
	{0X7E160F18, &WrapI_V<scePaf_7E160F18>,                          "scePaf_7E160F18",                        'i', ""   },
	{0X7E3A3D9B, &WrapI_V<scePaf_7E3A3D9B>,                          "scePaf_7E3A3D9B",                        'i', ""   },
	{0X7E545DA3, &WrapI_V<scePaf_7E545DA3>,                          "scePaf_7E545DA3",                        'i', ""   },
	{0X80390D7B, &WrapI_V<scePaf_80390D7B>,                          "scePaf_80390D7B",                        'i', ""   },
	{0X81F7BDC2, &WrapI_V<scePaf_81F7BDC2>,                          "scePaf_81F7BDC2",                        'i', ""   },
	{0X85BFE0DD, &WrapI_V<scePaf_85BFE0DD>,                          "scePaf_85BFE0DD",                        'i', ""   },
	{0X8871CAB9, &WrapI_V<scePaf_8871CAB9>,                          "scePaf_8871CAB9",                        'i', ""   },
	{0X88CE8467, &WrapI_V<scePaf_88CE8467>,                          "scePaf_88CE8467",                        'i', ""   },
	{0X8BC9927A, &WrapI_V<scePaf_8BC9927A>,                          "scePaf_8BC9927A",                        'i', ""   },
	{0X8DB83D04, &WrapI_V<scePaf_8DB83D04>,                          "scePaf_8DB83D04",                        'i', ""   },
	{0X8EDF4126, &WrapI_V<scePaf_8EDF4126>,                          "scePaf_8EDF4126",                        'i', ""   },
	{0X8FA84E15, &WrapI_V<scePaf_8FA84E15>,                          "scePaf_8FA84E15",                        'i', ""   },
	{0X918BCA8B, &WrapI_V<scePaf_918BCA8B>,                          "scePaf_918BCA8B",                        'i', ""   },
	{0X91973397, &WrapI_V<scePaf_91973397>,                          "scePaf_91973397",                        'i', ""   },
	{0X942E8711, &WrapI_V<scePaf_942E8711>,                          "scePaf_942E8711",                        'i', ""   },
	{0X94AFA971, &WrapI_V<scePaf_94AFA971>,                          "scePaf_94AFA971",                        'i', ""   },
	{0X9684511F, &WrapI_V<scePaf_9684511F>,                          "scePaf_9684511F",                        'i', ""   },
	{0X96ECA540, &WrapI_V<scePaf_96ECA540>,                          "scePaf_96ECA540",                        'i', ""   },
	{0X995294D1, &WrapI_V<scePaf_995294D1>,                          "scePaf_995294D1",                        'i', ""   },
	{0X9A418CCC, &WrapI_V<scePaf_9A418CCC>,                          "scePaf_9A418CCC",                        'i', ""   },
	{0X9AE88FFF, &WrapI_V<scePaf_9AE88FFF>,                          "scePaf_9AE88FFF",                        'i', ""   },
	{0X9AF12ADC, &WrapI_V<scePaf_9AF12ADC>,                          "scePaf_9AF12ADC",                        'i', ""   },
	{0X9D93A13C, &WrapI_V<scePaf_9D93A13C>,                          "scePaf_9D93A13C",                        'i', ""   },
	{0X9E236520, &WrapI_V<scePaf_9E236520>,                          "scePaf_9E236520",                        'i', ""   },
	{0X9EEFEF9D, &WrapI_V<scePaf_9EEFEF9D>,                          "scePaf_9EEFEF9D",                        'i', ""   },
	{0X9EF44AF8, &WrapI_V<scePaf_9EF44AF8>,                          "scePaf_9EF44AF8",                        'i', ""   },
	{0X03B20F85, &WrapU_V<scePaf_03B20F85>,                          "scePaf_03B20F85",                        'x', ""   },
	{0X30AF7A24, &WrapU_V<scePaf_30AF7A24>,                          "scePaf_30AF7A24",                        'x', ""   },
	{0X40A2FAC3, &WrapU_V<scePaf_40A2FAC3>,                          "scePaf_40A2FAC3",                        'x', ""   },
	{0X412B2F09, &WrapU_V<scePaf_412B2F09>,                          "scePaf_412B2F09",                        'x', ""   },
	{0X44AE2A91, &WrapU_V<scePaf_44AE2A91>,                          "scePaf_44AE2A91",                        'x', ""   },
	{0X4CA9F1A7, &WrapU_V<scePaf_4CA9F1A7>,                          "scePaf_4CA9F1A7",                        'x', ""   },
	{0X4CF09BA2, &WrapU_V<scePaf_4CF09BA2>,                          "scePaf_4CF09BA2",                        'x', ""   },
	{0X4CFAB8D4, &WrapU_V<scePaf_4CFAB8D4>,                          "scePaf_4CFAB8D4",                        'x', ""   },
	{0X4D45C2CF, &WrapU_V<scePaf_4D45C2CF>,                          "scePaf_4D45C2CF",                        'x', ""   },
	{0X4F7B8133, &WrapI_V<scePaf_4F7B8133>,                          "scePaf_4F7B8133",                        'i', ""   },
	{0X505EA775, &WrapI_V<scePaf_505EA775>,                          "scePaf_505EA775",                        'i', ""   },
	{0X512737D3, &WrapI_V<scePaf_512737D3>,                          "scePaf_512737D3",                        'i', ""   },
	{0X537B3828, &WrapI_V<scePaf_537B3828>,                          "scePaf_537B3828",                        'i', ""   },
	{0X53B0C35F, &WrapI_V<scePaf_53B0C35F>,                          "scePaf_53B0C35F",                        'i', ""   },
	{0X5476FD2B, &WrapI_V<scePaf_5476FD2B>,                          "scePaf_5476FD2B",                        'i', ""   },
	{0X5CC6AA3C, &WrapI_V<scePaf_5CC6AA3C>,                          "scePaf_5CC6AA3C",                        'i', ""   },
	{0X5E909060, &WrapI_V<scePaf_5E909060>,                          "scePaf_5E909060",                        'i', ""   },
	{0X726DFBA9, &WrapI_V<scePaf_726DFBA9>,                          "scePaf_726DFBA9",                        'i', ""   },
	{0X77ADEAEB, &WrapI_V<scePaf_77ADEAEB>,                          "scePaf_77ADEAEB",                        'i', ""   },
	{0X781E5F1E, &WrapI_V<scePaf_781E5F1E>,                          "scePaf_781E5F1E",                        'i', ""   },
	{0X85CFB6E6, &WrapI_V<scePaf_85CFB6E6>,                          "scePaf_85CFB6E6",                        'i', ""   },
	{0X9FA3F3F6, &WrapI_V<scePaf_9FA3F3F6>,                          "scePaf_9FA3F3F6",                        'i', ""   },
	{0XB9DDDCC8, &WrapU_V<scePaf_B9DDDCC8>,                          "scePaf_B9DDDCC8",                        'x', ""   },
	{0XBB89C9EA, &WrapU_UIU<scePaf_BB89C9EA>,                        "scePaf_BB89C9EA",                        'x', "xix"},
	{0XCC3426BF, &WrapI_V<scePaf_CC3426BF>,                          "scePaf_CC3426BF",                        'i', ""   },
	{0XD590412B, &WrapI_V<scePaf_D590412B>,                          "scePaf_D590412B",                        'i', ""   },
	{0XD7DCB972, &WrapI_V<scePaf_D7DCB972>,                          "scePaf_D7DCB972",                        'i', ""   },
	{0XE00895CD, &WrapI_V<scePaf_E00895CD>,                          "scePaf_E00895CD",                        'i', ""   },
	{0XF48A9040, &WrapI_V<scePaf_F48A9040>,                          "scePaf_F48A9040",                        'i', ""   },
	{0XFC5C8A32, &WrapI_V<scePaf_FC5C8A32>,                          "scePaf_FC5C8A32",                        'i', ""   },
	{0X9F12D4D1, &WrapI_V<scePaf_9F12D4D1>,                          "scePaf_9F12D4D1",                        'i', ""   },
	{0X9F3E4ED9, &WrapI_V<scePaf_9F3E4ED9>,                          "scePaf_9F3E4ED9",                        'i', ""   },
	{0XA04B4F40, &WrapI_V<scePaf_A04B4F40>,                          "scePaf_A04B4F40",                        'i', ""   },
	{0XA138A376, &WrapI_UU<scePaf_A138A376>,                         "scePaf_A138A376",                        'i', "xx"  },
	{0XA140233F, &WrapI_V<scePaf_A140233F>,                          "scePaf_A140233F",                        'i', ""   },
	{0XA2ADF6EC, &WrapI_V<scePaf_A2ADF6EC>,                          "scePaf_A2ADF6EC",                        'i', ""   },
	{0XA4EAD0A0, &WrapI_V<scePaf_A4EAD0A0>,                          "scePaf_A4EAD0A0",                        'i', ""   },
	{0XA5BBFD24, &WrapI_V<scePaf_A5BBFD24>,                          "scePaf_A5BBFD24",                        'i', ""   },
	{0XA69A5266, &WrapI_V<scePaf_A69A5266>,                          "scePaf_A69A5266",                        'i', ""   },
	{0XA7353F2A, &WrapI_V<scePaf_A7353F2A>,                          "scePaf_A7353F2A",                        'i', ""   },
	{0XA7784A89, &WrapI_V<scePaf_A7784A89>,                          "scePaf_A7784A89",                        'i', ""   },
	{0XA9645067, &WrapI_V<scePaf_A9645067>,                          "scePaf_A9645067",                        'i', ""   },
	{0XA989A2C4, &WrapI_V<scePaf_A989A2C4>,                          "scePaf_A989A2C4",                        'i', ""   },
	{0XAB0731F2, &WrapI_V<scePaf_AB0731F2>,                          "scePaf_AB0731F2",                        'i', ""   },
	{0XABB3AE55, &WrapI_V<scePaf_ABB3AE55>,                          "scePaf_ABB3AE55",                        'i', ""   },
	{0XAC29C247, &WrapI_V<scePaf_AC29C247>,                          "scePaf_AC29C247",                        'i', ""   },
	{0XAC98B0C3, &WrapI_V<scePaf_AC98B0C3>,                          "scePaf_AC98B0C3",                        'i', ""   },
	{0XAD454A8F, &WrapI_V<scePaf_AD454A8F>,                          "scePaf_AD454A8F",                        'i', ""   },
	{0XAE5D4943, &WrapI_V<scePaf_AE5D4943>,                          "scePaf_AE5D4943",                        'i', ""   },
	{0XAF067FA2, &WrapI_V<scePaf_AF067FA2>,                          "scePaf_AF067FA2",                        'i', ""   },
	{0XB03CB9A0, &WrapI_V<scePaf_B03CB9A0>,                          "scePaf_B03CB9A0",                        'i', ""   },
	{0XB05D9677, &WrapI_UUU<scePaf_B05D9677>,                        "scePaf_B05D9677",                        'i', "xxx" },
	{0XB4C7DE8E, &WrapI_V<scePaf_B4C7DE8E>,                          "scePaf_B4C7DE8E",                        'i', ""   },
	{0XB6A3AB1E, &WrapI_V<scePaf_B6A3AB1E>,                          "scePaf_B6A3AB1E",                        'i', ""   },
	{0XB6D58D98, &WrapI_V<scePaf_B6D58D98>,                          "scePaf_B6D58D98",                        'i', ""   },
	{0XB73A78AF, &WrapI_V<scePaf_B73A78AF>,                          "scePaf_B73A78AF",                        'i', ""   },
	{0XB94AD2A4, &WrapI_V<scePaf_B94AD2A4>,                          "scePaf_B94AD2A4",                        'i', ""   },
	{0XBCA8DE39, &WrapI_V<scePaf_BCA8DE39>,                          "scePaf_BCA8DE39",                        'i', ""   },
	{0XC0E15A55, &WrapI_V<scePaf_C0E15A55>,                          "scePaf_C0E15A55",                        'i', ""   },
	{0XC1B36CF9, &WrapI_V<scePaf_C1B36CF9>,                          "scePaf_C1B36CF9",                        'i', ""   },
	{0XC1D8F2DC, &WrapI_V<scePaf_C1D8F2DC>,                          "scePaf_C1D8F2DC",                        'i', ""   },
	{0XC22BACF3, &WrapI_V<scePaf_C22BACF3>,                          "scePaf_C22BACF3",                        'i', ""   },
	{0XC327FF14, &WrapI_V<scePaf_C327FF14>,                          "scePaf_C327FF14",                        'i', ""   },
	{0XC3B97D0D, &WrapI_V<scePaf_C3B97D0D>,                          "scePaf_C3B97D0D",                        'i', ""   },
	{0XC3D5A672, &WrapI_V<scePaf_C3D5A672>,                          "scePaf_C3D5A672",                        'i', ""   },
	{0XC59FC3D0, &WrapI_V<scePaf_C59FC3D0>,                          "scePaf_C59FC3D0",                        'i', ""   },
	{0XC5C17E46, &WrapI_V<scePaf_C5C17E46>,                          "scePaf_C5C17E46",                        'i', ""   },
	{0XC5C4707A, &WrapI_V<scePaf_C5C4707A>,                          "scePaf_C5C4707A",                        'i', ""   },
	{0XC735969B, &WrapI_V<scePaf_C735969B>,                          "scePaf_C735969B",                        'i', ""   },
	{0XC7F002F1, &WrapI_V<scePaf_C7F002F1>,                          "scePaf_C7F002F1",                        'i', ""   },
	{0XC839B167, &WrapI_V<scePaf_C839B167>,                          "scePaf_C839B167",                        'i', ""   },
	{0XC853CAF6, &WrapI_V<scePaf_C853CAF6>,                          "scePaf_C853CAF6",                        'i', ""   },
	{0XC8F930E9, &WrapI_V<scePaf_C8F930E9>,                          "scePaf_C8F930E9",                        'i', ""   },
	{0XCCC1E8F7, &WrapI_V<scePaf_CCC1E8F7>,                          "scePaf_CCC1E8F7",                        'i', ""   },
	{0XD0D9615E, &WrapI_V<scePaf_D0D9615E>,                          "scePaf_D0D9615E",                        'i', ""   },
	{0XD0DFA412, &WrapI_V<scePaf_D0DFA412>,                          "scePaf_D0DFA412",                        'i', ""   },
	{0XD1755221, &WrapI_V<scePaf_D1755221>,                          "scePaf_D1755221",                        'i', ""   },
	{0XD21E2DFC, &WrapI_V<scePaf_D21E2DFC>,                          "scePaf_D21E2DFC",                        'i', ""   },
	{0XD229572C, &WrapI_V<scePaf_D229572C>,                          "scePaf_D229572C",                        'i', ""   },
	{0XD2FCE506, &WrapI_V<scePaf_D2FCE506>,                          "scePaf_D2FCE506",                        'i', ""   },
	{0XD31D6132, &WrapI_V<scePaf_D31D6132>,                          "scePaf_D31D6132",                        'i', ""   },
	{0XD5F2C162, &WrapI_V<scePaf_D5F2C162>,                          "scePaf_D5F2C162",                        'i', ""   },
	{0XD6A63A45, &WrapI_V<scePaf_D6A63A45>,                          "scePaf_D6A63A45",                        'i', ""   },
	{0XD9E2D6E1, &WrapU_UIU<scePaf_D9E2D6E1>,                          "scePaf_D9E2D6E1",                        'x', "xix"},
	{0XD9FC24EA, &WrapI_V<scePaf_D9FC24EA>,                          "scePaf_D9FC24EA",                        'i', ""   },
	{0XDA8D097D, &WrapI_V<scePaf_DA8D097D>,                          "scePaf_DA8D097D",                        'i', ""   },
	{0XDB1612F1, &WrapI_V<scePaf_DB1612F1>,                          "scePaf_DB1612F1",                        'i', ""   },
	{0XDD10B322, &WrapI_V<scePaf_DD10B322>,                          "scePaf_DD10B322",                        'i', ""   },
	{0XDD23970A, &WrapI_V<scePaf_DD23970A>,                          "scePaf_DD23970A",                        'i', ""   },
	{0XDD588D1C, &WrapI_V<scePaf_DD588D1C>,                          "scePaf_DD588D1C",                        'i', ""   },
	{0XDDE74F3B, &WrapI_V<scePaf_DDE74F3B>,                          "scePaf_DDE74F3B",                        'i', ""   },
	{0XE0B32AE8, &WrapI_V<scePaf_E0B32AE8>,                          "scePaf_E0B32AE8",                        'i', ""   },
	{0XE1B1B2F9, &WrapI_V<scePaf_E1B1B2F9>,                          "scePaf_E1B1B2F9",                        'i', ""   },
	{0XE4B97CA1, &WrapI_V<scePaf_E4B97CA1>,                          "scePaf_E4B97CA1",                        'i', ""   },
	{0XE67517D2, &WrapI_V<scePaf_E67517D2>,                          "scePaf_E67517D2",                        'i', ""   },
	{0XE6D56019, &WrapI_V<scePaf_E6D56019>,                          "scePaf_E6D56019",                        'i', ""   },
	{0XE7263320, &WrapI_V<scePaf_E7263320>,                          "scePaf_E7263320",                        'i', ""   },
	{0XE9570AA3, &WrapI_V<scePaf_E9570AA3>,                          "scePaf_E9570AA3",                        'i', ""   },
	{0XEA681A0B, &WrapI_V<scePaf_EA681A0B>,                          "scePaf_EA681A0B",                        'i', ""   },
	{0XF09010AF, &WrapI_V<scePaf_F09010AF>,                          "scePaf_F09010AF",                        'i', ""   },
	{0XF207376F, &WrapI_V<scePaf_F207376F>,                          "scePaf_F207376F",                        'i', ""   },
	{0XF265E4F0, &WrapI_V<scePaf_F265E4F0>,                          "scePaf_F265E4F0",                        'i', ""   },
	{0XF4C130B8, &WrapI_V<scePaf_F4C130B8>,                          "scePaf_F4C130B8",                        'i', ""   },
	{0XF4EBA045, &WrapI_V<scePaf_F4EBA045>,                          "scePaf_F4EBA045",                        'i', ""   },
	{0XFA60E9C4, &WrapI_V<scePaf_FA60E9C4>,                          "scePaf_FA60E9C4",                        'i', ""   },
	{0XFAB38CB0, &WrapI_V<scePaf_FAB38CB0>,                          "scePaf_FAB38CB0",                        'i', ""   },
	{0XFD048927, &WrapI_V<scePaf_FD048927>,                          "scePaf_FD048927",                        'i', ""   },
	{0XFD38EF6C, &WrapI_V<scePaf_FD38EF6C>,                          "scePaf_FD38EF6C",                        'i', ""   },
	{0XFE99F522, &WrapI_V<scePaf_FE99F522>,                          "scePaf_FE99F522",                        'i', ""   },
	{0XFF01EBA9, &WrapI_V<scePaf_FF01EBA9>,                          "scePaf_FF01EBA9",                        'i', ""   },
	{0XFFBAD89D, &WrapI_V<scePaf_FFBAD89D>,                          "scePaf_FFBAD89D",                        'i', ""   },
};


const HLEModule moduleList[] =
{
	{"FakeSysCalls", ARRAY_SIZE(FakeSysCalls), FakeSysCalls},
	{"UtilsForUser", ARRAY_SIZE(UtilsForUser), UtilsForUser},
	{"KDebugForKernel", ARRAY_SIZE(KDebugForKernel), KDebugForKernel},
	{"sceSAScore"},
	{"SceBase64_Library"},
	{"sceCert_Loader"},
	{"SceFont_Library"},
	{"sceNetApctl"},
	{"sceSIRCS_IrDA_Driver"},
	{"Pspnet_Scan"},
	{"Pspnet_Show_MacAddr"},
	{"pspeDebug", ARRAY_SIZE(pspeDebug), pspeDebug},
};

static const int numModules = ARRAY_SIZE(moduleList);

void RegisterAllModules() {
	Register_Kernel_Library();
	Register_ThreadManForUser();
	Register_ThreadManForKernel();
	Register_LoadExecForUser();
	Register_UtilsForKernel();
	Register_SysMemUserForUser();
	Register_InterruptManager();
	Register_IoFileMgrForUser();
	Register_ModuleMgrForUser();
	Register_ModuleMgrForKernel();
	Register_StdioForUser();

	Register_sceHprm();
	Register_sceCcc();
	Register_sceCtrl();
	Register_sceDisplay();
	Register_sceAudio();
	Register_sceSasCore();
	Register_sceFont();
	Register_sceNet();
	Register_sceNetResolver();
	Register_sceNetInet();
	Register_sceNetApctl();
	Register_sceNetAdhoc();
	Register_sceNetAdhocMatching();
	Register_sceNetAdhocDiscover();
	Register_sceNetAdhocctl();
	Register_sceRtc();
	Register_sceWlanDrv();
	Register_sceMpeg();
	Register_sceMp3();
	Register_sceHttp();
	Register_scePower();
	Register_sceImpose();
	Register_sceSuspendForUser();
	Register_sceGe_user();
	Register_sceUmdUser();
	Register_sceDmac();
	Register_sceUtility();
	Register_sceAtrac3plus();
	Register_scePsmf();
	Register_scePsmfPlayer();
	Register_sceOpenPSID();
	Register_sceParseUri();
	Register_sceSsl();
	Register_sceParseHttp();
	Register_sceVaudio();
	Register_sceUsb();
	Register_sceChnnlsv();
	Register_sceNpDrm();
	Register_sceP3da();
	Register_sceGameUpdate();
	Register_sceDeflt();
	Register_sceMp4();
	Register_sceAac();
	Register_scePauth();
	Register_sceNp();
	Register_sceNpCommerce2();
	Register_sceNpService();
	Register_sceNpAuth();
	Register_sceMd5();
	Register_sceJpeg();
	Register_sceAudiocodec();
	Register_sceHeap();

	for (int i = 0; i < numModules; i++) {
		RegisterHLEModule(moduleList[i].name, moduleList[i].numFunctions, moduleList[i].funcTable);
	}

	// IMPORTANT: New modules have to be added at the end, or they will break savestates.

	Register_StdioForKernel();
	RegisterHLEModule("LoadCoreForKernel", ARRAY_SIZE(LoadCoreForKernel), LoadCoreForKernel);
	Register_IoFileMgrForKernel();
	Register_LoadExecForKernel();
	Register_InitForKernel();
	Register_SysMemForKernel();
	Register_sceMt19937();
	Register_SysclibForKernel();
	Register_sceCtrl_driver();
	Register_sceDisplay_driver();
	Register_sceMpegbase();
	Register_sceUsbGps();
	Register_sceLibFttt();
	Register_sceSha256();
	Register_sceAdler();
	Register_sceSfmt19937();
	Register_sceAudioRouting();
	Register_sceUsbCam();
	Register_sceG729();
	Register_sceNetUpnp();
	Register_sceNetIfhandle();
	Register_KUBridge();
	Register_sceUsbAcc();
	Register_sceUsbMic();
	Register_sceOpenPSID_driver();
	Register_semaphore();
	Register_sceDdrdb();
	Register_mp4msv();
	Register_InterruptManagerForKernel();
	Register_sceSircs();
	Register_sceNet_lib();
	Register_sceReg();
	// Not ready to enable this due to apparent softlocks in Patapon 3.
	// Register_sceNpMatching2();

	RegisterHLEModule("sceVshBridge", ARRAY_SIZE(sceVshBridge), sceVshBridge);
	RegisterHLEModule("sceVshCommonUtil", ARRAY_SIZE(sceVshCommonUtil), sceVshCommonUtil);
	RegisterHLEModule("sceUtility_netparam_internal", ARRAY_SIZE(sceUtility_netparam_internal), sceUtility_netparam_internal);
	RegisterHLEModule("scePaf", ARRAY_SIZE(scePaf), scePaf);

	// add new modules here.
}

