/**
* Copyright (C) 2017 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "cfg.h"
#include "wrapper.h"

bool winspoolFlag = false;

struct winspool_dll
{
	HMODULE dll;
	FARPROC ADVANCEDSETUPDIALOG;
	FARPROC AdvancedSetupDialog;
	FARPROC ConvertAnsiDevModeToUnicodeDevmode;
	FARPROC ConvertUnicodeDevModeToAnsiDevmode;
	FARPROC DEVICEMODE;
	FARPROC DeviceMode;
	FARPROC DocumentEvent;
	FARPROC PerfClose;
	FARPROC PerfCollect;
	FARPROC PerfOpen;
	FARPROC QueryColorProfile;
	FARPROC QueryRemoteFonts;
	FARPROC QuerySpoolMode;
	FARPROC SpoolerDevQueryPrintW;
	FARPROC StartDocDlgW;
	FARPROC AbortPrinter;
	FARPROC AddFormA;
	FARPROC AddFormW;
	FARPROC AddJobA;
	FARPROC AddJobW;
	FARPROC AddMonitorA;
	FARPROC AddMonitorW;
	FARPROC AddPortA;
	FARPROC AddPortExA;
	FARPROC AddPortExW;
	FARPROC AddPortW;
	FARPROC AddPrintProcessorA;
	FARPROC AddPrintProcessorW;
	FARPROC AddPrintProvidorA;
	FARPROC AddPrintProvidorW;
	FARPROC AddPrinterA;
	FARPROC AddPrinterConnection2A;
	FARPROC AddPrinterConnection2W;
	FARPROC AddPrinterConnectionA;
	FARPROC AddPrinterConnectionW;
	FARPROC AddPrinterDriverA;
	FARPROC AddPrinterDriverExA;
	FARPROC AddPrinterDriverExW;
	FARPROC AddPrinterDriverW;
	FARPROC AddPrinterW;
	FARPROC AdvancedDocumentPropertiesA;
	FARPROC AdvancedDocumentPropertiesW;
	FARPROC ClosePrinter;
	FARPROC CloseSpoolFileHandle;
	FARPROC CommitSpoolData;
	FARPROC ConfigurePortA;
	FARPROC ConfigurePortW;
	FARPROC ConnectToPrinterDlg;
	FARPROC CorePrinterDriverInstalledA;
	FARPROC CorePrinterDriverInstalledW;
	FARPROC CreatePrintAsyncNotifyChannel;
	FARPROC CreatePrinterIC;
	FARPROC DEVICECAPABILITIES;
	FARPROC DeleteFormA;
	FARPROC DeleteFormW;
	FARPROC DeleteJobNamedProperty;
	FARPROC DeleteMonitorA;
	FARPROC DeleteMonitorW;
	FARPROC DeletePortA;
	FARPROC DeletePortW;
	FARPROC DeletePrintProcessorA;
	FARPROC DeletePrintProcessorW;
	FARPROC DeletePrintProvidorA;
	FARPROC DeletePrintProvidorW;
	FARPROC DeletePrinter;
	FARPROC DeletePrinterConnectionA;
	FARPROC DeletePrinterConnectionW;
	FARPROC DeletePrinterDataA;
	FARPROC DeletePrinterDataExA;
	FARPROC DeletePrinterDataExW;
	FARPROC DeletePrinterDataW;
	FARPROC DeletePrinterDriverA;
	FARPROC DeletePrinterDriverExA;
	FARPROC DeletePrinterDriverExW;
	FARPROC DeletePrinterDriverPackageA;
	FARPROC DeletePrinterDriverPackageW;
	FARPROC DeletePrinterDriverW;
	FARPROC DeletePrinterIC;
	FARPROC DeletePrinterKeyA;
	FARPROC DeletePrinterKeyW;
	FARPROC DevQueryPrint;
	FARPROC DevQueryPrintEx;
	//FARPROC DeviceCapabilities;		// <----- Not sure what is happening here
	FARPROC DeviceCapabilitiesA;
	FARPROC DeviceCapabilitiesW;
	FARPROC DevicePropertySheets;
	FARPROC DocumentPropertiesA;
	FARPROC DocumentPropertiesW;
	FARPROC DocumentPropertySheets;
	FARPROC EXTDEVICEMODE;
	FARPROC EndDocPrinter;
	FARPROC EndPagePrinter;
	FARPROC EnumFormsA;
	FARPROC EnumFormsW;
	FARPROC EnumJobNamedProperties;
	FARPROC EnumJobsA;
	FARPROC EnumJobsW;
	FARPROC EnumMonitorsA;
	FARPROC EnumMonitorsW;
	FARPROC EnumPortsA;
	FARPROC GetDefaultPrinterA;
	FARPROC SetDefaultPrinterA;
	FARPROC GetDefaultPrinterW;
	FARPROC SetDefaultPrinterW;
	FARPROC EnumPortsW;
	FARPROC EnumPrintProcessorDatatypesA;
	FARPROC EnumPrintProcessorDatatypesW;
	FARPROC EnumPrintProcessorsA;
	FARPROC EnumPrintProcessorsW;
	FARPROC EnumPrinterDataA;
	FARPROC EnumPrinterDataExA;
	FARPROC EnumPrinterDataExW;
	FARPROC EnumPrinterDataW;
	FARPROC EnumPrinterDriversA;
	FARPROC EnumPrinterDriversW;
	FARPROC EnumPrinterKeyA;
	FARPROC EnumPrinterKeyW;
	FARPROC EnumPrintersA;
	FARPROC EnumPrintersW;
	FARPROC ExtDeviceMode;
	FARPROC FindClosePrinterChangeNotification;
	FARPROC FindFirstPrinterChangeNotification;
	FARPROC FindNextPrinterChangeNotification;
	FARPROC FlushPrinter;
	FARPROC FreePrintNamedPropertyArray;
	FARPROC FreePrintPropertyValue;
	FARPROC FreePrinterNotifyInfo;
	FARPROC GetCorePrinterDriversA;
	FARPROC GetCorePrinterDriversW;
	FARPROC GetFormA;
	FARPROC GetFormW;
	FARPROC GetJobA;
	FARPROC GetJobNamedPropertyValue;
	FARPROC GetJobW;
	FARPROC GetPrintExecutionData;
	FARPROC GetPrintOutputInfo;
	FARPROC GetPrintProcessorDirectoryA;
	FARPROC GetPrintProcessorDirectoryW;
	FARPROC GetPrinterA;
	FARPROC GetPrinterDataA;
	FARPROC GetPrinterDataExA;
	FARPROC GetPrinterDataExW;
	FARPROC GetPrinterDataW;
	FARPROC GetPrinterDriver2A;
	FARPROC GetPrinterDriver2W;
	FARPROC GetPrinterDriverA;
	FARPROC GetPrinterDriverDirectoryA;
	FARPROC GetPrinterDriverDirectoryW;
	FARPROC GetPrinterDriverPackagePathA;
	FARPROC GetPrinterDriverPackagePathW;
	FARPROC GetPrinterDriverW;
	FARPROC GetPrinterW;
	FARPROC GetSpoolFileHandle;
	FARPROC InstallPrinterDriverFromPackageA;
	FARPROC InstallPrinterDriverFromPackageW;
	FARPROC IsValidDevmodeA;
	FARPROC IsValidDevmodeW;
	FARPROC OpenPrinter2A;
	FARPROC OpenPrinter2W;
	FARPROC OpenPrinterA;
	FARPROC OpenPrinterW;
	FARPROC PlayGdiScriptOnPrinterIC;
	FARPROC PrinterMessageBoxA;
	FARPROC PrinterMessageBoxW;
	FARPROC PrinterProperties;
	FARPROC ReadPrinter;
	FARPROC RegisterForPrintAsyncNotifications;
	FARPROC ReportJobProcessingProgress;
	FARPROC ResetPrinterA;
	FARPROC ResetPrinterW;
	FARPROC ScheduleJob;
	FARPROC SeekPrinter;
	FARPROC SetFormA;
	FARPROC SetFormW;
	FARPROC SetJobA;
	FARPROC SetJobNamedProperty;
	FARPROC SetJobW;
	FARPROC SetPortA;
	FARPROC SetPortW;
	FARPROC SetPrinterA;
	FARPROC SetPrinterDataA;
	FARPROC SetPrinterDataExA;
	FARPROC SetPrinterDataExW;
	FARPROC SetPrinterDataW;
	FARPROC SetPrinterW;
	FARPROC SplDriverUnloadComplete;
	FARPROC SpoolerPrinterEvent;
	FARPROC StartDocDlgA;
	FARPROC StartDocPrinterA;
	FARPROC StartDocPrinterW;
	FARPROC StartPagePrinter;
	FARPROC UnRegisterForPrintAsyncNotifications;
	FARPROC UploadPrinterDriverPackageA;
	FARPROC UploadPrinterDriverPackageW;
	FARPROC WaitForPrinterChange;
	FARPROC WritePrinter;
	FARPROC XcvDataW;
} winspool;

__declspec(naked) void FakeADVANCEDSETUPDIALOG() { _asm { jmp[winspool.ADVANCEDSETUPDIALOG] } }
__declspec(naked) void FakeAdvancedSetupDialog() { _asm { jmp[winspool.AdvancedSetupDialog] } }
__declspec(naked) void FakeConvertAnsiDevModeToUnicodeDevmode() { _asm { jmp[winspool.ConvertAnsiDevModeToUnicodeDevmode] } }
__declspec(naked) void FakeConvertUnicodeDevModeToAnsiDevmode() { _asm { jmp[winspool.ConvertUnicodeDevModeToAnsiDevmode] } }
__declspec(naked) void FakeDEVICEMODE() { _asm { jmp[winspool.DEVICEMODE] } }
__declspec(naked) void FakeDeviceMode() { _asm { jmp[winspool.DeviceMode] } }
__declspec(naked) void FakeDocumentEvent() { _asm { jmp[winspool.DocumentEvent] } }
__declspec(naked) void FakePerfClose() { _asm { jmp[winspool.PerfClose] } }
__declspec(naked) void FakePerfCollect() { _asm { jmp[winspool.PerfCollect] } }
__declspec(naked) void FakePerfOpen() { _asm { jmp[winspool.PerfOpen] } }
__declspec(naked) void FakeQueryColorProfile() { _asm { jmp[winspool.QueryColorProfile] } }
__declspec(naked) void FakeQueryRemoteFonts() { _asm { jmp[winspool.QueryRemoteFonts] } }
__declspec(naked) void FakeQuerySpoolMode() { _asm { jmp[winspool.QuerySpoolMode] } }
__declspec(naked) void FakeSpoolerDevQueryPrintW() { _asm { jmp[winspool.SpoolerDevQueryPrintW] } }
__declspec(naked) void FakeStartDocDlgW() { _asm { jmp[winspool.StartDocDlgW] } }
__declspec(naked) void FakeAbortPrinter() { _asm { jmp[winspool.AbortPrinter] } }
__declspec(naked) void FakeAddFormA() { _asm { jmp[winspool.AddFormA] } }
__declspec(naked) void FakeAddFormW() { _asm { jmp[winspool.AddFormW] } }
__declspec(naked) void FakeAddJobA() { _asm { jmp[winspool.AddJobA] } }
__declspec(naked) void FakeAddJobW() { _asm { jmp[winspool.AddJobW] } }
__declspec(naked) void FakeAddMonitorA() { _asm { jmp[winspool.AddMonitorA] } }
__declspec(naked) void FakeAddMonitorW() { _asm { jmp[winspool.AddMonitorW] } }
__declspec(naked) void FakeAddPortA() { _asm { jmp[winspool.AddPortA] } }
__declspec(naked) void FakeAddPortExA() { _asm { jmp[winspool.AddPortExA] } }
__declspec(naked) void FakeAddPortExW() { _asm { jmp[winspool.AddPortExW] } }
__declspec(naked) void FakeAddPortW() { _asm { jmp[winspool.AddPortW] } }
__declspec(naked) void FakeAddPrintProcessorA() { _asm { jmp[winspool.AddPrintProcessorA] } }
__declspec(naked) void FakeAddPrintProcessorW() { _asm { jmp[winspool.AddPrintProcessorW] } }
__declspec(naked) void FakeAddPrintProvidorA() { _asm { jmp[winspool.AddPrintProvidorA] } }
__declspec(naked) void FakeAddPrintProvidorW() { _asm { jmp[winspool.AddPrintProvidorW] } }
__declspec(naked) void FakeAddPrinterA() { _asm { jmp[winspool.AddPrinterA] } }
__declspec(naked) void FakeAddPrinterConnection2A() { _asm { jmp[winspool.AddPrinterConnection2A] } }
__declspec(naked) void FakeAddPrinterConnection2W() { _asm { jmp[winspool.AddPrinterConnection2W] } }
__declspec(naked) void FakeAddPrinterConnectionA() { _asm { jmp[winspool.AddPrinterConnectionA] } }
__declspec(naked) void FakeAddPrinterConnectionW() { _asm { jmp[winspool.AddPrinterConnectionW] } }
__declspec(naked) void FakeAddPrinterDriverA() { _asm { jmp[winspool.AddPrinterDriverA] } }
__declspec(naked) void FakeAddPrinterDriverExA() { _asm { jmp[winspool.AddPrinterDriverExA] } }
__declspec(naked) void FakeAddPrinterDriverExW() { _asm { jmp[winspool.AddPrinterDriverExW] } }
__declspec(naked) void FakeAddPrinterDriverW() { _asm { jmp[winspool.AddPrinterDriverW] } }
__declspec(naked) void FakeAddPrinterW() { _asm { jmp[winspool.AddPrinterW] } }
__declspec(naked) void FakeAdvancedDocumentPropertiesA() { _asm { jmp[winspool.AdvancedDocumentPropertiesA] } }
__declspec(naked) void FakeAdvancedDocumentPropertiesW() { _asm { jmp[winspool.AdvancedDocumentPropertiesW] } }
__declspec(naked) void FakeClosePrinter() { _asm { jmp[winspool.ClosePrinter] } }
__declspec(naked) void FakeCloseSpoolFileHandle() { _asm { jmp[winspool.CloseSpoolFileHandle] } }
__declspec(naked) void FakeCommitSpoolData() { _asm { jmp[winspool.CommitSpoolData] } }
__declspec(naked) void FakeConfigurePortA() { _asm { jmp[winspool.ConfigurePortA] } }
__declspec(naked) void FakeConfigurePortW() { _asm { jmp[winspool.ConfigurePortW] } }
__declspec(naked) void FakeConnectToPrinterDlg() { _asm { jmp[winspool.ConnectToPrinterDlg] } }
__declspec(naked) void FakeCorePrinterDriverInstalledA() { _asm { jmp[winspool.CorePrinterDriverInstalledA] } }
__declspec(naked) void FakeCorePrinterDriverInstalledW() { _asm { jmp[winspool.CorePrinterDriverInstalledW] } }
__declspec(naked) void FakeCreatePrintAsyncNotifyChannel() { _asm { jmp[winspool.CreatePrintAsyncNotifyChannel] } }
__declspec(naked) void FakeCreatePrinterIC() { _asm { jmp[winspool.CreatePrinterIC] } }
__declspec(naked) void FakeDEVICECAPABILITIES() { _asm { jmp[winspool.DEVICECAPABILITIES] } }
__declspec(naked) void FakeDeleteFormA() { _asm { jmp[winspool.DeleteFormA] } }
__declspec(naked) void FakeDeleteFormW() { _asm { jmp[winspool.DeleteFormW] } }
__declspec(naked) void FakeDeleteJobNamedProperty() { _asm { jmp[winspool.DeleteJobNamedProperty] } }
__declspec(naked) void FakeDeleteMonitorA() { _asm { jmp[winspool.DeleteMonitorA] } }
__declspec(naked) void FakeDeleteMonitorW() { _asm { jmp[winspool.DeleteMonitorW] } }
__declspec(naked) void FakeDeletePortA() { _asm { jmp[winspool.DeletePortA] } }
__declspec(naked) void FakeDeletePortW() { _asm { jmp[winspool.DeletePortW] } }
__declspec(naked) void FakeDeletePrintProcessorA() { _asm { jmp[winspool.DeletePrintProcessorA] } }
__declspec(naked) void FakeDeletePrintProcessorW() { _asm { jmp[winspool.DeletePrintProcessorW] } }
__declspec(naked) void FakeDeletePrintProvidorA() { _asm { jmp[winspool.DeletePrintProvidorA] } }
__declspec(naked) void FakeDeletePrintProvidorW() { _asm { jmp[winspool.DeletePrintProvidorW] } }
__declspec(naked) void FakeDeletePrinter() { _asm { jmp[winspool.DeletePrinter] } }
__declspec(naked) void FakeDeletePrinterConnectionA() { _asm { jmp[winspool.DeletePrinterConnectionA] } }
__declspec(naked) void FakeDeletePrinterConnectionW() { _asm { jmp[winspool.DeletePrinterConnectionW] } }
__declspec(naked) void FakeDeletePrinterDataA() { _asm { jmp[winspool.DeletePrinterDataA] } }
__declspec(naked) void FakeDeletePrinterDataExA() { _asm { jmp[winspool.DeletePrinterDataExA] } }
__declspec(naked) void FakeDeletePrinterDataExW() { _asm { jmp[winspool.DeletePrinterDataExW] } }
__declspec(naked) void FakeDeletePrinterDataW() { _asm { jmp[winspool.DeletePrinterDataW] } }
__declspec(naked) void FakeDeletePrinterDriverA() { _asm { jmp[winspool.DeletePrinterDriverA] } }
__declspec(naked) void FakeDeletePrinterDriverExA() { _asm { jmp[winspool.DeletePrinterDriverExA] } }
__declspec(naked) void FakeDeletePrinterDriverExW() { _asm { jmp[winspool.DeletePrinterDriverExW] } }
__declspec(naked) void FakeDeletePrinterDriverPackageA() { _asm { jmp[winspool.DeletePrinterDriverPackageA] } }
__declspec(naked) void FakeDeletePrinterDriverPackageW() { _asm { jmp[winspool.DeletePrinterDriverPackageW] } }
__declspec(naked) void FakeDeletePrinterDriverW() { _asm { jmp[winspool.DeletePrinterDriverW] } }
__declspec(naked) void FakeDeletePrinterIC() { _asm { jmp[winspool.DeletePrinterIC] } }
__declspec(naked) void FakeDeletePrinterKeyA() { _asm { jmp[winspool.DeletePrinterKeyA] } }
__declspec(naked) void FakeDeletePrinterKeyW() { _asm { jmp[winspool.DeletePrinterKeyW] } }
__declspec(naked) void FakeDevQueryPrint() { _asm { jmp[winspool.DevQueryPrint] } }
__declspec(naked) void FakeDevQueryPrintEx() { _asm { jmp[winspool.DevQueryPrintEx] } }
__declspec(naked) void FakeDeviceCapabilities() { _asm { jmp[winspool.DeviceCapabilities] } }
__declspec(naked) void FakeDeviceCapabilitiesA() { _asm { jmp[winspool.DeviceCapabilitiesA] } }
__declspec(naked) void FakeDeviceCapabilitiesW() { _asm { jmp[winspool.DeviceCapabilitiesW] } }
__declspec(naked) void FakeDevicePropertySheets() { _asm { jmp[winspool.DevicePropertySheets] } }
__declspec(naked) void FakeDocumentPropertiesA() { _asm { jmp[winspool.DocumentPropertiesA] } }
__declspec(naked) void FakeDocumentPropertiesW() { _asm { jmp[winspool.DocumentPropertiesW] } }
__declspec(naked) void FakeDocumentPropertySheets() { _asm { jmp[winspool.DocumentPropertySheets] } }
__declspec(naked) void FakeEXTDEVICEMODE() { _asm { jmp[winspool.EXTDEVICEMODE] } }
__declspec(naked) void FakeEndDocPrinter() { _asm { jmp[winspool.EndDocPrinter] } }
__declspec(naked) void FakeEndPagePrinter() { _asm { jmp[winspool.EndPagePrinter] } }
__declspec(naked) void FakeEnumFormsA() { _asm { jmp[winspool.EnumFormsA] } }
__declspec(naked) void FakeEnumFormsW() { _asm { jmp[winspool.EnumFormsW] } }
__declspec(naked) void FakeEnumJobNamedProperties() { _asm { jmp[winspool.EnumJobNamedProperties] } }
__declspec(naked) void FakeEnumJobsA() { _asm { jmp[winspool.EnumJobsA] } }
__declspec(naked) void FakeEnumJobsW() { _asm { jmp[winspool.EnumJobsW] } }
__declspec(naked) void FakeEnumMonitorsA() { _asm { jmp[winspool.EnumMonitorsA] } }
__declspec(naked) void FakeEnumMonitorsW() { _asm { jmp[winspool.EnumMonitorsW] } }
__declspec(naked) void FakeEnumPortsA() { _asm { jmp[winspool.EnumPortsA] } }
__declspec(naked) void FakeGetDefaultPrinterA() { _asm { jmp[winspool.GetDefaultPrinterA] } }
__declspec(naked) void FakeSetDefaultPrinterA() { _asm { jmp[winspool.SetDefaultPrinterA] } }
__declspec(naked) void FakeGetDefaultPrinterW() { _asm { jmp[winspool.GetDefaultPrinterW] } }
__declspec(naked) void FakeSetDefaultPrinterW() { _asm { jmp[winspool.SetDefaultPrinterW] } }
__declspec(naked) void FakeEnumPortsW() { _asm { jmp[winspool.EnumPortsW] } }
__declspec(naked) void FakeEnumPrintProcessorDatatypesA() { _asm { jmp[winspool.EnumPrintProcessorDatatypesA] } }
__declspec(naked) void FakeEnumPrintProcessorDatatypesW() { _asm { jmp[winspool.EnumPrintProcessorDatatypesW] } }
__declspec(naked) void FakeEnumPrintProcessorsA() { _asm { jmp[winspool.EnumPrintProcessorsA] } }
__declspec(naked) void FakeEnumPrintProcessorsW() { _asm { jmp[winspool.EnumPrintProcessorsW] } }
__declspec(naked) void FakeEnumPrinterDataA() { _asm { jmp[winspool.EnumPrinterDataA] } }
__declspec(naked) void FakeEnumPrinterDataExA() { _asm { jmp[winspool.EnumPrinterDataExA] } }
__declspec(naked) void FakeEnumPrinterDataExW() { _asm { jmp[winspool.EnumPrinterDataExW] } }
__declspec(naked) void FakeEnumPrinterDataW() { _asm { jmp[winspool.EnumPrinterDataW] } }
__declspec(naked) void FakeEnumPrinterDriversA() { _asm { jmp[winspool.EnumPrinterDriversA] } }
__declspec(naked) void FakeEnumPrinterDriversW() { _asm { jmp[winspool.EnumPrinterDriversW] } }
__declspec(naked) void FakeEnumPrinterKeyA() { _asm { jmp[winspool.EnumPrinterKeyA] } }
__declspec(naked) void FakeEnumPrinterKeyW() { _asm { jmp[winspool.EnumPrinterKeyW] } }
__declspec(naked) void FakeEnumPrintersA() { _asm { jmp[winspool.EnumPrintersA] } }
__declspec(naked) void FakeEnumPrintersW() { _asm { jmp[winspool.EnumPrintersW] } }
__declspec(naked) void FakeExtDeviceMode() { _asm { jmp[winspool.ExtDeviceMode] } }
__declspec(naked) void FakeFindClosePrinterChangeNotification() { _asm { jmp[winspool.FindClosePrinterChangeNotification] } }
__declspec(naked) void FakeFindFirstPrinterChangeNotification() { _asm { jmp[winspool.FindFirstPrinterChangeNotification] } }
__declspec(naked) void FakeFindNextPrinterChangeNotification() { _asm { jmp[winspool.FindNextPrinterChangeNotification] } }
__declspec(naked) void FakeFlushPrinter() { _asm { jmp[winspool.FlushPrinter] } }
__declspec(naked) void FakeFreePrintNamedPropertyArray() { _asm { jmp[winspool.FreePrintNamedPropertyArray] } }
__declspec(naked) void FakeFreePrintPropertyValue() { _asm { jmp[winspool.FreePrintPropertyValue] } }
__declspec(naked) void FakeFreePrinterNotifyInfo() { _asm { jmp[winspool.FreePrinterNotifyInfo] } }
__declspec(naked) void FakeGetCorePrinterDriversA() { _asm { jmp[winspool.GetCorePrinterDriversA] } }
__declspec(naked) void FakeGetCorePrinterDriversW() { _asm { jmp[winspool.GetCorePrinterDriversW] } }
__declspec(naked) void FakeGetFormA() { _asm { jmp[winspool.GetFormA] } }
__declspec(naked) void FakeGetFormW() { _asm { jmp[winspool.GetFormW] } }
__declspec(naked) void FakeGetJobA() { _asm { jmp[winspool.GetJobA] } }
__declspec(naked) void FakeGetJobNamedPropertyValue() { _asm { jmp[winspool.GetJobNamedPropertyValue] } }
__declspec(naked) void FakeGetJobW() { _asm { jmp[winspool.GetJobW] } }
__declspec(naked) void FakeGetPrintExecutionData() { _asm { jmp[winspool.GetPrintExecutionData] } }
__declspec(naked) void FakeGetPrintOutputInfo() { _asm { jmp[winspool.GetPrintOutputInfo] } }
__declspec(naked) void FakeGetPrintProcessorDirectoryA() { _asm { jmp[winspool.GetPrintProcessorDirectoryA] } }
__declspec(naked) void FakeGetPrintProcessorDirectoryW() { _asm { jmp[winspool.GetPrintProcessorDirectoryW] } }
__declspec(naked) void FakeGetPrinterA() { _asm { jmp[winspool.GetPrinterA] } }
__declspec(naked) void FakeGetPrinterDataA() { _asm { jmp[winspool.GetPrinterDataA] } }
__declspec(naked) void FakeGetPrinterDataExA() { _asm { jmp[winspool.GetPrinterDataExA] } }
__declspec(naked) void FakeGetPrinterDataExW() { _asm { jmp[winspool.GetPrinterDataExW] } }
__declspec(naked) void FakeGetPrinterDataW() { _asm { jmp[winspool.GetPrinterDataW] } }
__declspec(naked) void FakeGetPrinterDriver2A() { _asm { jmp[winspool.GetPrinterDriver2A] } }
__declspec(naked) void FakeGetPrinterDriver2W() { _asm { jmp[winspool.GetPrinterDriver2W] } }
__declspec(naked) void FakeGetPrinterDriverA() { _asm { jmp[winspool.GetPrinterDriverA] } }
__declspec(naked) void FakeGetPrinterDriverDirectoryA() { _asm { jmp[winspool.GetPrinterDriverDirectoryA] } }
__declspec(naked) void FakeGetPrinterDriverDirectoryW() { _asm { jmp[winspool.GetPrinterDriverDirectoryW] } }
__declspec(naked) void FakeGetPrinterDriverPackagePathA() { _asm { jmp[winspool.GetPrinterDriverPackagePathA] } }
__declspec(naked) void FakeGetPrinterDriverPackagePathW() { _asm { jmp[winspool.GetPrinterDriverPackagePathW] } }
__declspec(naked) void FakeGetPrinterDriverW() { _asm { jmp[winspool.GetPrinterDriverW] } }
__declspec(naked) void FakeGetPrinterW() { _asm { jmp[winspool.GetPrinterW] } }
__declspec(naked) void FakeGetSpoolFileHandle() { _asm { jmp[winspool.GetSpoolFileHandle] } }
__declspec(naked) void FakeInstallPrinterDriverFromPackageA() { _asm { jmp[winspool.InstallPrinterDriverFromPackageA] } }
__declspec(naked) void FakeInstallPrinterDriverFromPackageW() { _asm { jmp[winspool.InstallPrinterDriverFromPackageW] } }
__declspec(naked) void FakeIsValidDevmodeA() { _asm { jmp[winspool.IsValidDevmodeA] } }
__declspec(naked) void FakeIsValidDevmodeW() { _asm { jmp[winspool.IsValidDevmodeW] } }
__declspec(naked) void FakeOpenPrinter2A() { _asm { jmp[winspool.OpenPrinter2A] } }
__declspec(naked) void FakeOpenPrinter2W() { _asm { jmp[winspool.OpenPrinter2W] } }
__declspec(naked) void FakeOpenPrinterA() { _asm { jmp[winspool.OpenPrinterA] } }
__declspec(naked) void FakeOpenPrinterW() { _asm { jmp[winspool.OpenPrinterW] } }
__declspec(naked) void FakePlayGdiScriptOnPrinterIC() { _asm { jmp[winspool.PlayGdiScriptOnPrinterIC] } }
__declspec(naked) void FakePrinterMessageBoxA() { _asm { jmp[winspool.PrinterMessageBoxA] } }
__declspec(naked) void FakePrinterMessageBoxW() { _asm { jmp[winspool.PrinterMessageBoxW] } }
__declspec(naked) void FakePrinterProperties() { _asm { jmp[winspool.PrinterProperties] } }
__declspec(naked) void FakeReadPrinter() { _asm { jmp[winspool.ReadPrinter] } }
__declspec(naked) void FakeRegisterForPrintAsyncNotifications() { _asm { jmp[winspool.RegisterForPrintAsyncNotifications] } }
__declspec(naked) void FakeReportJobProcessingProgress() { _asm { jmp[winspool.ReportJobProcessingProgress] } }
__declspec(naked) void FakeResetPrinterA() { _asm { jmp[winspool.ResetPrinterA] } }
__declspec(naked) void FakeResetPrinterW() { _asm { jmp[winspool.ResetPrinterW] } }
__declspec(naked) void FakeScheduleJob() { _asm { jmp[winspool.ScheduleJob] } }
__declspec(naked) void FakeSeekPrinter() { _asm { jmp[winspool.SeekPrinter] } }
__declspec(naked) void FakeSetFormA() { _asm { jmp[winspool.SetFormA] } }
__declspec(naked) void FakeSetFormW() { _asm { jmp[winspool.SetFormW] } }
__declspec(naked) void FakeSetJobA() { _asm { jmp[winspool.SetJobA] } }
__declspec(naked) void FakeSetJobNamedProperty() { _asm { jmp[winspool.SetJobNamedProperty] } }
__declspec(naked) void FakeSetJobW() { _asm { jmp[winspool.SetJobW] } }
__declspec(naked) void FakeSetPortA() { _asm { jmp[winspool.SetPortA] } }
__declspec(naked) void FakeSetPortW() { _asm { jmp[winspool.SetPortW] } }
__declspec(naked) void FakeSetPrinterA() { _asm { jmp[winspool.SetPrinterA] } }
__declspec(naked) void FakeSetPrinterDataA() { _asm { jmp[winspool.SetPrinterDataA] } }
__declspec(naked) void FakeSetPrinterDataExA() { _asm { jmp[winspool.SetPrinterDataExA] } }
__declspec(naked) void FakeSetPrinterDataExW() { _asm { jmp[winspool.SetPrinterDataExW] } }
__declspec(naked) void FakeSetPrinterDataW() { _asm { jmp[winspool.SetPrinterDataW] } }
__declspec(naked) void FakeSetPrinterW() { _asm { jmp[winspool.SetPrinterW] } }
__declspec(naked) void FakeSplDriverUnloadComplete() { _asm { jmp[winspool.SplDriverUnloadComplete] } }
__declspec(naked) void FakeSpoolerPrinterEvent() { _asm { jmp[winspool.SpoolerPrinterEvent] } }
__declspec(naked) void FakeStartDocDlgA() { _asm { jmp[winspool.StartDocDlgA] } }
__declspec(naked) void FakeStartDocPrinterA() { _asm { jmp[winspool.StartDocPrinterA] } }
__declspec(naked) void FakeStartDocPrinterW() { _asm { jmp[winspool.StartDocPrinterW] } }
__declspec(naked) void FakeStartPagePrinter() { _asm { jmp[winspool.StartPagePrinter] } }
__declspec(naked) void FakeUnRegisterForPrintAsyncNotifications() { _asm { jmp[winspool.UnRegisterForPrintAsyncNotifications] } }
__declspec(naked) void FakeUploadPrinterDriverPackageA() { _asm { jmp[winspool.UploadPrinterDriverPackageA] } }
__declspec(naked) void FakeUploadPrinterDriverPackageW() { _asm { jmp[winspool.UploadPrinterDriverPackageW] } }
__declspec(naked) void FakeWaitForPrinterChange() { _asm { jmp[winspool.WaitForPrinterChange] } }
__declspec(naked) void FakeWritePrinter() { _asm { jmp[winspool.WritePrinter] } }
__declspec(naked) void FakeXcvDataW() { _asm { jmp[winspool.XcvDataW] } }

void LoadWinspool()
{
	// Load real dll
	winspool.dll = LoadDll(dtype.winspool);
	// Load dll functions
	winspoolFlag = true;
	winspool.ADVANCEDSETUPDIALOG = GetProcAddress(winspool.dll, "ADVANCEDSETUPDIALOG");
	winspool.AdvancedSetupDialog = GetProcAddress(winspool.dll, "AdvancedSetupDialog");
	winspool.ConvertAnsiDevModeToUnicodeDevmode = GetProcAddress(winspool.dll, "ConvertAnsiDevModeToUnicodeDevmode");
	winspool.ConvertUnicodeDevModeToAnsiDevmode = GetProcAddress(winspool.dll, "ConvertUnicodeDevModeToAnsiDevmode");
	winspool.DEVICEMODE = GetProcAddress(winspool.dll, "DEVICEMODE");
	winspool.DeviceMode = GetProcAddress(winspool.dll, "DeviceMode");
	winspool.DocumentEvent = GetProcAddress(winspool.dll, "DocumentEvent");
	winspool.PerfClose = GetProcAddress(winspool.dll, "PerfClose");
	winspool.PerfCollect = GetProcAddress(winspool.dll, "PerfCollect");
	winspool.PerfOpen = GetProcAddress(winspool.dll, "PerfOpen");
	winspool.QueryColorProfile = GetProcAddress(winspool.dll, "QueryColorProfile");
	winspool.QueryRemoteFonts = GetProcAddress(winspool.dll, "QueryRemoteFonts");
	winspool.QuerySpoolMode = GetProcAddress(winspool.dll, "QuerySpoolMode");
	winspool.SpoolerDevQueryPrintW = GetProcAddress(winspool.dll, "SpoolerDevQueryPrintW");
	winspool.StartDocDlgW = GetProcAddress(winspool.dll, "StartDocDlgW");
	winspool.AbortPrinter = GetProcAddress(winspool.dll, "AbortPrinter");
	winspool.AddFormA = GetProcAddress(winspool.dll, "AddFormA");
	winspool.AddFormW = GetProcAddress(winspool.dll, "AddFormW");
	winspool.AddJobA = GetProcAddress(winspool.dll, "AddJobA");
	winspool.AddJobW = GetProcAddress(winspool.dll, "AddJobW");
	winspool.AddMonitorA = GetProcAddress(winspool.dll, "AddMonitorA");
	winspool.AddMonitorW = GetProcAddress(winspool.dll, "AddMonitorW");
	winspool.AddPortA = GetProcAddress(winspool.dll, "AddPortA");
	winspool.AddPortExA = GetProcAddress(winspool.dll, "AddPortExA");
	winspool.AddPortExW = GetProcAddress(winspool.dll, "AddPortExW");
	winspool.AddPortW = GetProcAddress(winspool.dll, "AddPortW");
	winspool.AddPrintProcessorA = GetProcAddress(winspool.dll, "AddPrintProcessorA");
	winspool.AddPrintProcessorW = GetProcAddress(winspool.dll, "AddPrintProcessorW");
	winspool.AddPrintProvidorA = GetProcAddress(winspool.dll, "AddPrintProvidorA");
	winspool.AddPrintProvidorW = GetProcAddress(winspool.dll, "AddPrintProvidorW");
	winspool.AddPrinterA = GetProcAddress(winspool.dll, "AddPrinterA");
	winspool.AddPrinterConnection2A = GetProcAddress(winspool.dll, "AddPrinterConnection2A");
	winspool.AddPrinterConnection2W = GetProcAddress(winspool.dll, "AddPrinterConnection2W");
	winspool.AddPrinterConnectionA = GetProcAddress(winspool.dll, "AddPrinterConnectionA");
	winspool.AddPrinterConnectionW = GetProcAddress(winspool.dll, "AddPrinterConnectionW");
	winspool.AddPrinterDriverA = GetProcAddress(winspool.dll, "AddPrinterDriverA");
	winspool.AddPrinterDriverExA = GetProcAddress(winspool.dll, "AddPrinterDriverExA");
	winspool.AddPrinterDriverExW = GetProcAddress(winspool.dll, "AddPrinterDriverExW");
	winspool.AddPrinterDriverW = GetProcAddress(winspool.dll, "AddPrinterDriverW");
	winspool.AddPrinterW = GetProcAddress(winspool.dll, "AddPrinterW");
	winspool.AdvancedDocumentPropertiesA = GetProcAddress(winspool.dll, "AdvancedDocumentPropertiesA");
	winspool.AdvancedDocumentPropertiesW = GetProcAddress(winspool.dll, "AdvancedDocumentPropertiesW");
	winspool.ClosePrinter = GetProcAddress(winspool.dll, "ClosePrinter");
	winspool.CloseSpoolFileHandle = GetProcAddress(winspool.dll, "CloseSpoolFileHandle");
	winspool.CommitSpoolData = GetProcAddress(winspool.dll, "CommitSpoolData");
	winspool.ConfigurePortA = GetProcAddress(winspool.dll, "ConfigurePortA");
	winspool.ConfigurePortW = GetProcAddress(winspool.dll, "ConfigurePortW");
	winspool.ConnectToPrinterDlg = GetProcAddress(winspool.dll, "ConnectToPrinterDlg");
	winspool.CorePrinterDriverInstalledA = GetProcAddress(winspool.dll, "CorePrinterDriverInstalledA");
	winspool.CorePrinterDriverInstalledW = GetProcAddress(winspool.dll, "CorePrinterDriverInstalledW");
	winspool.CreatePrintAsyncNotifyChannel = GetProcAddress(winspool.dll, "CreatePrintAsyncNotifyChannel");
	winspool.CreatePrinterIC = GetProcAddress(winspool.dll, "CreatePrinterIC");
	winspool.DEVICECAPABILITIES = GetProcAddress(winspool.dll, "DEVICECAPABILITIES");
	winspool.DeleteFormA = GetProcAddress(winspool.dll, "DeleteFormA");
	winspool.DeleteFormW = GetProcAddress(winspool.dll, "DeleteFormW");
	winspool.DeleteJobNamedProperty = GetProcAddress(winspool.dll, "DeleteJobNamedProperty");
	winspool.DeleteMonitorA = GetProcAddress(winspool.dll, "DeleteMonitorA");
	winspool.DeleteMonitorW = GetProcAddress(winspool.dll, "DeleteMonitorW");
	winspool.DeletePortA = GetProcAddress(winspool.dll, "DeletePortA");
	winspool.DeletePortW = GetProcAddress(winspool.dll, "DeletePortW");
	winspool.DeletePrintProcessorA = GetProcAddress(winspool.dll, "DeletePrintProcessorA");
	winspool.DeletePrintProcessorW = GetProcAddress(winspool.dll, "DeletePrintProcessorW");
	winspool.DeletePrintProvidorA = GetProcAddress(winspool.dll, "DeletePrintProvidorA");
	winspool.DeletePrintProvidorW = GetProcAddress(winspool.dll, "DeletePrintProvidorW");
	winspool.DeletePrinter = GetProcAddress(winspool.dll, "DeletePrinter");
	winspool.DeletePrinterConnectionA = GetProcAddress(winspool.dll, "DeletePrinterConnectionA");
	winspool.DeletePrinterConnectionW = GetProcAddress(winspool.dll, "DeletePrinterConnectionW");
	winspool.DeletePrinterDataA = GetProcAddress(winspool.dll, "DeletePrinterDataA");
	winspool.DeletePrinterDataExA = GetProcAddress(winspool.dll, "DeletePrinterDataExA");
	winspool.DeletePrinterDataExW = GetProcAddress(winspool.dll, "DeletePrinterDataExW");
	winspool.DeletePrinterDataW = GetProcAddress(winspool.dll, "DeletePrinterDataW");
	winspool.DeletePrinterDriverA = GetProcAddress(winspool.dll, "DeletePrinterDriverA");
	winspool.DeletePrinterDriverExA = GetProcAddress(winspool.dll, "DeletePrinterDriverExA");
	winspool.DeletePrinterDriverExW = GetProcAddress(winspool.dll, "DeletePrinterDriverExW");
	winspool.DeletePrinterDriverPackageA = GetProcAddress(winspool.dll, "DeletePrinterDriverPackageA");
	winspool.DeletePrinterDriverPackageW = GetProcAddress(winspool.dll, "DeletePrinterDriverPackageW");
	winspool.DeletePrinterDriverW = GetProcAddress(winspool.dll, "DeletePrinterDriverW");
	winspool.DeletePrinterIC = GetProcAddress(winspool.dll, "DeletePrinterIC");
	winspool.DeletePrinterKeyA = GetProcAddress(winspool.dll, "DeletePrinterKeyA");
	winspool.DeletePrinterKeyW = GetProcAddress(winspool.dll, "DeletePrinterKeyW");
	winspool.DevQueryPrint = GetProcAddress(winspool.dll, "DevQueryPrint");
	winspool.DevQueryPrintEx = GetProcAddress(winspool.dll, "DevQueryPrintEx");
	winspool.DeviceCapabilities = GetProcAddress(winspool.dll, "DeviceCapabilities");
	winspool.DeviceCapabilitiesA = GetProcAddress(winspool.dll, "DeviceCapabilitiesA");
	winspool.DeviceCapabilitiesW = GetProcAddress(winspool.dll, "DeviceCapabilitiesW");
	winspool.DevicePropertySheets = GetProcAddress(winspool.dll, "DevicePropertySheets");
	winspool.DocumentPropertiesA = GetProcAddress(winspool.dll, "DocumentPropertiesA");
	winspool.DocumentPropertiesW = GetProcAddress(winspool.dll, "DocumentPropertiesW");
	winspool.DocumentPropertySheets = GetProcAddress(winspool.dll, "DocumentPropertySheets");
	winspool.EXTDEVICEMODE = GetProcAddress(winspool.dll, "EXTDEVICEMODE");
	winspool.EndDocPrinter = GetProcAddress(winspool.dll, "EndDocPrinter");
	winspool.EndPagePrinter = GetProcAddress(winspool.dll, "EndPagePrinter");
	winspool.EnumFormsA = GetProcAddress(winspool.dll, "EnumFormsA");
	winspool.EnumFormsW = GetProcAddress(winspool.dll, "EnumFormsW");
	winspool.EnumJobNamedProperties = GetProcAddress(winspool.dll, "EnumJobNamedProperties");
	winspool.EnumJobsA = GetProcAddress(winspool.dll, "EnumJobsA");
	winspool.EnumJobsW = GetProcAddress(winspool.dll, "EnumJobsW");
	winspool.EnumMonitorsA = GetProcAddress(winspool.dll, "EnumMonitorsA");
	winspool.EnumMonitorsW = GetProcAddress(winspool.dll, "EnumMonitorsW");
	winspool.EnumPortsA = GetProcAddress(winspool.dll, "EnumPortsA");
	winspool.GetDefaultPrinterA = GetProcAddress(winspool.dll, "GetDefaultPrinterA");
	winspool.SetDefaultPrinterA = GetProcAddress(winspool.dll, "SetDefaultPrinterA");
	winspool.GetDefaultPrinterW = GetProcAddress(winspool.dll, "GetDefaultPrinterW");
	winspool.SetDefaultPrinterW = GetProcAddress(winspool.dll, "SetDefaultPrinterW");
	winspool.EnumPortsW = GetProcAddress(winspool.dll, "EnumPortsW");
	winspool.EnumPrintProcessorDatatypesA = GetProcAddress(winspool.dll, "EnumPrintProcessorDatatypesA");
	winspool.EnumPrintProcessorDatatypesW = GetProcAddress(winspool.dll, "EnumPrintProcessorDatatypesW");
	winspool.EnumPrintProcessorsA = GetProcAddress(winspool.dll, "EnumPrintProcessorsA");
	winspool.EnumPrintProcessorsW = GetProcAddress(winspool.dll, "EnumPrintProcessorsW");
	winspool.EnumPrinterDataA = GetProcAddress(winspool.dll, "EnumPrinterDataA");
	winspool.EnumPrinterDataExA = GetProcAddress(winspool.dll, "EnumPrinterDataExA");
	winspool.EnumPrinterDataExW = GetProcAddress(winspool.dll, "EnumPrinterDataExW");
	winspool.EnumPrinterDataW = GetProcAddress(winspool.dll, "EnumPrinterDataW");
	winspool.EnumPrinterDriversA = GetProcAddress(winspool.dll, "EnumPrinterDriversA");
	winspool.EnumPrinterDriversW = GetProcAddress(winspool.dll, "EnumPrinterDriversW");
	winspool.EnumPrinterKeyA = GetProcAddress(winspool.dll, "EnumPrinterKeyA");
	winspool.EnumPrinterKeyW = GetProcAddress(winspool.dll, "EnumPrinterKeyW");
	winspool.EnumPrintersA = GetProcAddress(winspool.dll, "EnumPrintersA");
	winspool.EnumPrintersW = GetProcAddress(winspool.dll, "EnumPrintersW");
	winspool.ExtDeviceMode = GetProcAddress(winspool.dll, "ExtDeviceMode");
	winspool.FindClosePrinterChangeNotification = GetProcAddress(winspool.dll, "FindClosePrinterChangeNotification");
	winspool.FindFirstPrinterChangeNotification = GetProcAddress(winspool.dll, "FindFirstPrinterChangeNotification");
	winspool.FindNextPrinterChangeNotification = GetProcAddress(winspool.dll, "FindNextPrinterChangeNotification");
	winspool.FlushPrinter = GetProcAddress(winspool.dll, "FlushPrinter");
	winspool.FreePrintNamedPropertyArray = GetProcAddress(winspool.dll, "FreePrintNamedPropertyArray");
	winspool.FreePrintPropertyValue = GetProcAddress(winspool.dll, "FreePrintPropertyValue");
	winspool.FreePrinterNotifyInfo = GetProcAddress(winspool.dll, "FreePrinterNotifyInfo");
	winspool.GetCorePrinterDriversA = GetProcAddress(winspool.dll, "GetCorePrinterDriversA");
	winspool.GetCorePrinterDriversW = GetProcAddress(winspool.dll, "GetCorePrinterDriversW");
	winspool.GetFormA = GetProcAddress(winspool.dll, "GetFormA");
	winspool.GetFormW = GetProcAddress(winspool.dll, "GetFormW");
	winspool.GetJobA = GetProcAddress(winspool.dll, "GetJobA");
	winspool.GetJobNamedPropertyValue = GetProcAddress(winspool.dll, "GetJobNamedPropertyValue");
	winspool.GetJobW = GetProcAddress(winspool.dll, "GetJobW");
	winspool.GetPrintExecutionData = GetProcAddress(winspool.dll, "GetPrintExecutionData");
	winspool.GetPrintOutputInfo = GetProcAddress(winspool.dll, "GetPrintOutputInfo");
	winspool.GetPrintProcessorDirectoryA = GetProcAddress(winspool.dll, "GetPrintProcessorDirectoryA");
	winspool.GetPrintProcessorDirectoryW = GetProcAddress(winspool.dll, "GetPrintProcessorDirectoryW");
	winspool.GetPrinterA = GetProcAddress(winspool.dll, "GetPrinterA");
	winspool.GetPrinterDataA = GetProcAddress(winspool.dll, "GetPrinterDataA");
	winspool.GetPrinterDataExA = GetProcAddress(winspool.dll, "GetPrinterDataExA");
	winspool.GetPrinterDataExW = GetProcAddress(winspool.dll, "GetPrinterDataExW");
	winspool.GetPrinterDataW = GetProcAddress(winspool.dll, "GetPrinterDataW");
	winspool.GetPrinterDriver2A = GetProcAddress(winspool.dll, "GetPrinterDriver2A");
	winspool.GetPrinterDriver2W = GetProcAddress(winspool.dll, "GetPrinterDriver2W");
	winspool.GetPrinterDriverA = GetProcAddress(winspool.dll, "GetPrinterDriverA");
	winspool.GetPrinterDriverDirectoryA = GetProcAddress(winspool.dll, "GetPrinterDriverDirectoryA");
	winspool.GetPrinterDriverDirectoryW = GetProcAddress(winspool.dll, "GetPrinterDriverDirectoryW");
	winspool.GetPrinterDriverPackagePathA = GetProcAddress(winspool.dll, "GetPrinterDriverPackagePathA");
	winspool.GetPrinterDriverPackagePathW = GetProcAddress(winspool.dll, "GetPrinterDriverPackagePathW");
	winspool.GetPrinterDriverW = GetProcAddress(winspool.dll, "GetPrinterDriverW");
	winspool.GetPrinterW = GetProcAddress(winspool.dll, "GetPrinterW");
	winspool.GetSpoolFileHandle = GetProcAddress(winspool.dll, "GetSpoolFileHandle");
	winspool.InstallPrinterDriverFromPackageA = GetProcAddress(winspool.dll, "InstallPrinterDriverFromPackageA");
	winspool.InstallPrinterDriverFromPackageW = GetProcAddress(winspool.dll, "InstallPrinterDriverFromPackageW");
	winspool.IsValidDevmodeA = GetProcAddress(winspool.dll, "IsValidDevmodeA");
	winspool.IsValidDevmodeW = GetProcAddress(winspool.dll, "IsValidDevmodeW");
	winspool.OpenPrinter2A = GetProcAddress(winspool.dll, "OpenPrinter2A");
	winspool.OpenPrinter2W = GetProcAddress(winspool.dll, "OpenPrinter2W");
	winspool.OpenPrinterA = GetProcAddress(winspool.dll, "OpenPrinterA");
	winspool.OpenPrinterW = GetProcAddress(winspool.dll, "OpenPrinterW");
	winspool.PlayGdiScriptOnPrinterIC = GetProcAddress(winspool.dll, "PlayGdiScriptOnPrinterIC");
	winspool.PrinterMessageBoxA = GetProcAddress(winspool.dll, "PrinterMessageBoxA");
	winspool.PrinterMessageBoxW = GetProcAddress(winspool.dll, "PrinterMessageBoxW");
	winspool.PrinterProperties = GetProcAddress(winspool.dll, "PrinterProperties");
	winspool.ReadPrinter = GetProcAddress(winspool.dll, "ReadPrinter");
	winspool.RegisterForPrintAsyncNotifications = GetProcAddress(winspool.dll, "RegisterForPrintAsyncNotifications");
	winspool.ReportJobProcessingProgress = GetProcAddress(winspool.dll, "ReportJobProcessingProgress");
	winspool.ResetPrinterA = GetProcAddress(winspool.dll, "ResetPrinterA");
	winspool.ResetPrinterW = GetProcAddress(winspool.dll, "ResetPrinterW");
	winspool.ScheduleJob = GetProcAddress(winspool.dll, "ScheduleJob");
	winspool.SeekPrinter = GetProcAddress(winspool.dll, "SeekPrinter");
	winspool.SetFormA = GetProcAddress(winspool.dll, "SetFormA");
	winspool.SetFormW = GetProcAddress(winspool.dll, "SetFormW");
	winspool.SetJobA = GetProcAddress(winspool.dll, "SetJobA");
	winspool.SetJobNamedProperty = GetProcAddress(winspool.dll, "SetJobNamedProperty");
	winspool.SetJobW = GetProcAddress(winspool.dll, "SetJobW");
	winspool.SetPortA = GetProcAddress(winspool.dll, "SetPortA");
	winspool.SetPortW = GetProcAddress(winspool.dll, "SetPortW");
	winspool.SetPrinterA = GetProcAddress(winspool.dll, "SetPrinterA");
	winspool.SetPrinterDataA = GetProcAddress(winspool.dll, "SetPrinterDataA");
	winspool.SetPrinterDataExA = GetProcAddress(winspool.dll, "SetPrinterDataExA");
	winspool.SetPrinterDataExW = GetProcAddress(winspool.dll, "SetPrinterDataExW");
	winspool.SetPrinterDataW = GetProcAddress(winspool.dll, "SetPrinterDataW");
	winspool.SetPrinterW = GetProcAddress(winspool.dll, "SetPrinterW");
	winspool.SplDriverUnloadComplete = GetProcAddress(winspool.dll, "SplDriverUnloadComplete");
	winspool.SpoolerPrinterEvent = GetProcAddress(winspool.dll, "SpoolerPrinterEvent");
	winspool.StartDocDlgA = GetProcAddress(winspool.dll, "StartDocDlgA");
	winspool.StartDocPrinterA = GetProcAddress(winspool.dll, "StartDocPrinterA");
	winspool.StartDocPrinterW = GetProcAddress(winspool.dll, "StartDocPrinterW");
	winspool.StartPagePrinter = GetProcAddress(winspool.dll, "StartPagePrinter");
	winspool.UnRegisterForPrintAsyncNotifications = GetProcAddress(winspool.dll, "UnRegisterForPrintAsyncNotifications");
	winspool.UploadPrinterDriverPackageA = GetProcAddress(winspool.dll, "UploadPrinterDriverPackageA");
	winspool.UploadPrinterDriverPackageW = GetProcAddress(winspool.dll, "UploadPrinterDriverPackageW");
	winspool.WaitForPrinterChange = GetProcAddress(winspool.dll, "WaitForPrinterChange");
	winspool.WritePrinter = GetProcAddress(winspool.dll, "WritePrinter");
	winspool.XcvDataW = GetProcAddress(winspool.dll, "XcvDataW");
}

void FreeWinspoolLibrary()
{
	if (winspoolFlag) FreeLibrary(winspool.dll);
}