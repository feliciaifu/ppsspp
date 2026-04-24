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

#include "Core/HLE/HLE.h"
#include "Core/HLE/HLETables.h"
#include "Core/HLE/FunctionWrappers.h"

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

VSH_STUB(sceVshBridge_01730088)
VSH_STUB(sceVshBridge_0543156C)
VSH_STUB(sceVshBridge_61001D64)
VSH_STUB(sceVshBridge_C949966C)
VSH_STUB(sceVshBridge_DB7C3D5A)
VSH_STUB(sceVshBridge_21C243FE)

VSH_STUB(sceVshCommonUtil_1D7E0B27)
VSH_STUB(sceVshCommonUtil_2058CEAA)
VSH_STUB(sceVshCommonUtil_34826464)
VSH_STUB(sceVshCommonUtil_38C93C16)
VSH_STUB(sceVshCommonUtil_3CA4266A)
VSH_STUB(sceVshCommonUtil_A721B1D1)
VSH_STUB(sceVshCommonUtil_649C3568)

VSH_STUB(scePaf_03B20F85)
VSH_STUB(scePaf_30AF7A24)
VSH_STUB(scePaf_44AE2A91)
VSH_STUB(scePaf_4CA9F1A7)
VSH_STUB(scePaf_4CF09BA2)
VSH_STUB(scePaf_4CFAB8D4)
VSH_STUB(scePaf_4D45C2CF)
VSH_STUB(scePaf_4F7B8133)
VSH_STUB(scePaf_505EA775)
VSH_STUB(scePaf_512737D3)
VSH_STUB(scePaf_537B3828)
VSH_STUB(scePaf_53B0C35F)
VSH_STUB(scePaf_5476FD2B)
VSH_STUB(scePaf_5CC6AA3C)
VSH_STUB(scePaf_5E909060)
VSH_STUB(scePaf_726DFBA9)
VSH_STUB(scePaf_77ADEAEB)
VSH_STUB(scePaf_781E5F1E)
VSH_STUB(scePaf_85CFB6E6)
VSH_STUB(scePaf_9FA3F3F6)
VSH_STUB(scePaf_CC3426BF)
VSH_STUB(scePaf_D590412B)
VSH_STUB(scePaf_D7DCB972)
VSH_STUB(scePaf_E00895CD)
VSH_STUB(scePaf_F48A9040)
VSH_STUB(scePaf_FC5C8A32)

#undef VSH_STUB

const HLEFunction sceVshBridge[] =
{
	{0X01730088, &WrapI_V<sceVshBridge_01730088>,                     "sceVshBridge_01730088",                   'i', ""   },
	{0X0543156C, &WrapI_V<sceVshBridge_0543156C>,                     "sceVshBridge_0543156C",                   'i', ""   },
	{0X21C243FE, &WrapI_V<sceVshBridge_21C243FE>,                     "sceVshBridge_21C243FE",                   'i', ""   },
	{0X61001D64, &WrapI_V<sceVshBridge_61001D64>,                     "sceVshBridge_61001D64",                   'i', ""   },
	{0XC949966C, &WrapI_V<sceVshBridge_C949966C>,                     "sceVshBridge_C949966C",                   'i', ""   },
	{0XDB7C3D5A, &WrapI_V<sceVshBridge_DB7C3D5A>,                     "sceVshBridge_DB7C3D5A",                   'i', ""   },
};

const HLEFunction sceVshCommonUtil[] =
{
	{0X1D7E0B27, &WrapI_V<sceVshCommonUtil_1D7E0B27>,                 "sceVshCommonUtil_1D7E0B27",               'i', ""   },
	{0X2058CEAA, &WrapI_V<sceVshCommonUtil_2058CEAA>,                "sceVshCommonUtil_2058CEAA",              'i', ""   },
	{0X34826464, &WrapI_V<sceVshCommonUtil_34826464>,                "sceVshCommonUtil_34826464",              'i', ""   },
	{0X38C93C16, &WrapI_V<sceVshCommonUtil_38C93C16>,                "sceVshCommonUtil_38C93C16",              'i', ""   },
	{0X3CA4266A, &WrapI_V<sceVshCommonUtil_3CA4266A>,                "sceVshCommonUtil_3CA4266A",              'i', ""   },
	{0X649C3568, &WrapI_V<sceVshCommonUtil_649C3568>,                "sceVshCommonUtil_649C3568",              'i', ""   },
	{0XA721B1D1, &WrapI_V<sceVshCommonUtil_A721B1D1>,                "sceVshCommonUtil_A721B1D1",              'i', ""   },
};

const HLEFunction scePaf[] =
{
	{0X03B20F85, &WrapI_V<scePaf_03B20F85>,                          "scePaf_03B20F85",                        'i', ""   },
	{0X30AF7A24, &WrapI_V<scePaf_30AF7A24>,                          "scePaf_30AF7A24",                        'i', ""   },
	{0X44AE2A91, &WrapI_V<scePaf_44AE2A91>,                          "scePaf_44AE2A91",                        'i', ""   },
	{0X4CA9F1A7, &WrapI_V<scePaf_4CA9F1A7>,                          "scePaf_4CA9F1A7",                        'i', ""   },
	{0X4CF09BA2, &WrapI_V<scePaf_4CF09BA2>,                          "scePaf_4CF09BA2",                        'i', ""   },
	{0X4CFAB8D4, &WrapI_V<scePaf_4CFAB8D4>,                          "scePaf_4CFAB8D4",                        'i', ""   },
	{0X4D45C2CF, &WrapI_V<scePaf_4D45C2CF>,                          "scePaf_4D45C2CF",                        'i', ""   },
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
	{0XCC3426BF, &WrapI_V<scePaf_CC3426BF>,                          "scePaf_CC3426BF",                        'i', ""   },
	{0XD590412B, &WrapI_V<scePaf_D590412B>,                          "scePaf_D590412B",                        'i', ""   },
	{0XD7DCB972, &WrapI_V<scePaf_D7DCB972>,                          "scePaf_D7DCB972",                        'i', ""   },
	{0XE00895CD, &WrapI_V<scePaf_E00895CD>,                          "scePaf_E00895CD",                        'i', ""   },
	{0XF48A9040, &WrapI_V<scePaf_F48A9040>,                          "scePaf_F48A9040",                        'i', ""   },
	{0XFC5C8A32, &WrapI_V<scePaf_FC5C8A32>,                          "scePaf_FC5C8A32",                        'i', ""   },
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
	RegisterHLEModule("scePaf", ARRAY_SIZE(scePaf), scePaf);

	// add new modules here.
}

