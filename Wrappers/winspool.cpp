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

#include "Settings\Settings.h"
#include "Wrapper.h"
#include "Utils\Utils.h"

struct winspool_dll
{
	HMODULE dll = nullptr;
	FARPROC ADVANCEDSETUPDIALOG = jmpaddr;
	FARPROC AdvancedSetupDialog = jmpaddr;
	FARPROC ConvertAnsiDevModeToUnicodeDevmode = jmpaddr;
	FARPROC ConvertUnicodeDevModeToAnsiDevmode = jmpaddr;
	FARPROC DEVICEMODE = jmpaddr;
	FARPROC DeviceMode = jmpaddr;
	FARPROC DocumentEvent = jmpaddr;
	FARPROC PerfClose = jmpaddr;
	FARPROC PerfCollect = jmpaddr;
	FARPROC PerfOpen = jmpaddr;
	FARPROC QueryColorProfile = jmpaddr;
	FARPROC QueryRemoteFonts = jmpaddr;
	FARPROC QuerySpoolMode = jmpaddr;
	FARPROC SpoolerDevQueryPrintW = jmpaddr;
	FARPROC StartDocDlgW = jmpaddr;
	FARPROC AbortPrinter = jmpaddr;
	FARPROC AddFormA = jmpaddr;
	FARPROC AddFormW = jmpaddr;
	FARPROC AddJobA = jmpaddr;
	FARPROC AddJobW = jmpaddr;
	FARPROC AddMonitorA = jmpaddr;
	FARPROC AddMonitorW = jmpaddr;
	FARPROC AddPortA = jmpaddr;
	FARPROC AddPortExA = jmpaddr;
	FARPROC AddPortExW = jmpaddr;
	FARPROC AddPortW = jmpaddr;
	FARPROC AddPrintProcessorA = jmpaddr;
	FARPROC AddPrintProcessorW = jmpaddr;
	FARPROC AddPrintProvidorA = jmpaddr;
	FARPROC AddPrintProvidorW = jmpaddr;
	FARPROC AddPrinterA = jmpaddr;
	FARPROC AddPrinterConnection2A = jmpaddr;
	FARPROC AddPrinterConnection2W = jmpaddr;
	FARPROC AddPrinterConnectionA = jmpaddr;
	FARPROC AddPrinterConnectionW = jmpaddr;
	FARPROC AddPrinterDriverA = jmpaddr;
	FARPROC AddPrinterDriverExA = jmpaddr;
	FARPROC AddPrinterDriverExW = jmpaddr;
	FARPROC AddPrinterDriverW = jmpaddr;
	FARPROC AddPrinterW = jmpaddr;
	FARPROC AdvancedDocumentPropertiesA = jmpaddr;
	FARPROC AdvancedDocumentPropertiesW = jmpaddr;
	FARPROC ClosePrinter = jmpaddr;
	FARPROC CloseSpoolFileHandle = jmpaddr;
	FARPROC CommitSpoolData = jmpaddr;
	FARPROC ConfigurePortA = jmpaddr;
	FARPROC ConfigurePortW = jmpaddr;
	FARPROC ConnectToPrinterDlg = jmpaddr;
	FARPROC CorePrinterDriverInstalledA = jmpaddr;
	FARPROC CorePrinterDriverInstalledW = jmpaddr;
	FARPROC CreatePrintAsyncNotifyChannel = jmpaddr;
	FARPROC CreatePrinterIC = jmpaddr;
	FARPROC DEVICECAPABILITIES = jmpaddr;
	FARPROC DeleteFormA = jmpaddr;
	FARPROC DeleteFormW = jmpaddr;
	FARPROC DeleteJobNamedProperty = jmpaddr;
	FARPROC DeleteMonitorA = jmpaddr;
	FARPROC DeleteMonitorW = jmpaddr;
	FARPROC DeletePortA = jmpaddr;
	FARPROC DeletePortW = jmpaddr;
	FARPROC DeletePrintProcessorA = jmpaddr;
	FARPROC DeletePrintProcessorW = jmpaddr;
	FARPROC DeletePrintProvidorA = jmpaddr;
	FARPROC DeletePrintProvidorW = jmpaddr;
	FARPROC DeletePrinter = jmpaddr;
	FARPROC DeletePrinterConnectionA = jmpaddr;
	FARPROC DeletePrinterConnectionW = jmpaddr;
	FARPROC DeletePrinterDataA = jmpaddr;
	FARPROC DeletePrinterDataExA = jmpaddr;
	FARPROC DeletePrinterDataExW = jmpaddr;
	FARPROC DeletePrinterDataW = jmpaddr;
	FARPROC DeletePrinterDriverA = jmpaddr;
	FARPROC DeletePrinterDriverExA = jmpaddr;
	FARPROC DeletePrinterDriverExW = jmpaddr;
	FARPROC DeletePrinterDriverPackageA = jmpaddr;
	FARPROC DeletePrinterDriverPackageW = jmpaddr;
	FARPROC DeletePrinterDriverW = jmpaddr;
	FARPROC DeletePrinterIC = jmpaddr;
	FARPROC DeletePrinterKeyA = jmpaddr;
	FARPROC DeletePrinterKeyW = jmpaddr;
	FARPROC DevQueryPrint = jmpaddr;
	FARPROC DevQueryPrintEx = jmpaddr;
	//FARPROC DeviceCapabilities = jmpaddr;		// <----- Not sure what is happening here
	FARPROC DeviceCapabilitiesA = jmpaddr;
	FARPROC DeviceCapabilitiesW = jmpaddr;
	FARPROC DevicePropertySheets = jmpaddr;
	FARPROC DocumentPropertiesA = jmpaddr;
	FARPROC DocumentPropertiesW = jmpaddr;
	FARPROC DocumentPropertySheets = jmpaddr;
	FARPROC EXTDEVICEMODE = jmpaddr;
	FARPROC EndDocPrinter = jmpaddr;
	FARPROC EndPagePrinter = jmpaddr;
	FARPROC EnumFormsA = jmpaddr;
	FARPROC EnumFormsW = jmpaddr;
	FARPROC EnumJobNamedProperties = jmpaddr;
	FARPROC EnumJobsA = jmpaddr;
	FARPROC EnumJobsW = jmpaddr;
	FARPROC EnumMonitorsA = jmpaddr;
	FARPROC EnumMonitorsW = jmpaddr;
	FARPROC EnumPortsA = jmpaddr;
	FARPROC GetDefaultPrinterA = jmpaddr;
	FARPROC SetDefaultPrinterA = jmpaddr;
	FARPROC GetDefaultPrinterW = jmpaddr;
	FARPROC SetDefaultPrinterW = jmpaddr;
	FARPROC EnumPortsW = jmpaddr;
	FARPROC EnumPrintProcessorDatatypesA = jmpaddr;
	FARPROC EnumPrintProcessorDatatypesW = jmpaddr;
	FARPROC EnumPrintProcessorsA = jmpaddr;
	FARPROC EnumPrintProcessorsW = jmpaddr;
	FARPROC EnumPrinterDataA = jmpaddr;
	FARPROC EnumPrinterDataExA = jmpaddr;
	FARPROC EnumPrinterDataExW = jmpaddr;
	FARPROC EnumPrinterDataW = jmpaddr;
	FARPROC EnumPrinterDriversA = jmpaddr;
	FARPROC EnumPrinterDriversW = jmpaddr;
	FARPROC EnumPrinterKeyA = jmpaddr;
	FARPROC EnumPrinterKeyW = jmpaddr;
	FARPROC EnumPrintersA = jmpaddr;
	FARPROC EnumPrintersW = jmpaddr;
	FARPROC ExtDeviceMode = jmpaddr;
	FARPROC FindClosePrinterChangeNotification = jmpaddr;
	FARPROC FindFirstPrinterChangeNotification = jmpaddr;
	FARPROC FindNextPrinterChangeNotification = jmpaddr;
	FARPROC FlushPrinter = jmpaddr;
	FARPROC FreePrintNamedPropertyArray = jmpaddr;
	FARPROC FreePrintPropertyValue = jmpaddr;
	FARPROC FreePrinterNotifyInfo = jmpaddr;
	FARPROC GetCorePrinterDriversA = jmpaddr;
	FARPROC GetCorePrinterDriversW = jmpaddr;
	FARPROC GetFormA = jmpaddr;
	FARPROC GetFormW = jmpaddr;
	FARPROC GetJobA = jmpaddr;
	FARPROC GetJobNamedPropertyValue = jmpaddr;
	FARPROC GetJobW = jmpaddr;
	FARPROC GetPrintExecutionData = jmpaddr;
	FARPROC GetPrintOutputInfo = jmpaddr;
	FARPROC GetPrintProcessorDirectoryA = jmpaddr;
	FARPROC GetPrintProcessorDirectoryW = jmpaddr;
	FARPROC GetPrinterA = jmpaddr;
	FARPROC GetPrinterDataA = jmpaddr;
	FARPROC GetPrinterDataExA = jmpaddr;
	FARPROC GetPrinterDataExW = jmpaddr;
	FARPROC GetPrinterDataW = jmpaddr;
	FARPROC GetPrinterDriver2A = jmpaddr;
	FARPROC GetPrinterDriver2W = jmpaddr;
	FARPROC GetPrinterDriverA = jmpaddr;
	FARPROC GetPrinterDriverDirectoryA = jmpaddr;
	FARPROC GetPrinterDriverDirectoryW = jmpaddr;
	FARPROC GetPrinterDriverPackagePathA = jmpaddr;
	FARPROC GetPrinterDriverPackagePathW = jmpaddr;
	FARPROC GetPrinterDriverW = jmpaddr;
	FARPROC GetPrinterW = jmpaddr;
	FARPROC GetSpoolFileHandle = jmpaddr;
	FARPROC InstallPrinterDriverFromPackageA = jmpaddr;
	FARPROC InstallPrinterDriverFromPackageW = jmpaddr;
	FARPROC IsValidDevmodeA = jmpaddr;
	FARPROC IsValidDevmodeW = jmpaddr;
	FARPROC OpenPrinter2A = jmpaddr;
	FARPROC OpenPrinter2W = jmpaddr;
	FARPROC OpenPrinterA = jmpaddr;
	FARPROC OpenPrinterW = jmpaddr;
	FARPROC PlayGdiScriptOnPrinterIC = jmpaddr;
	FARPROC PrinterMessageBoxA = jmpaddr;
	FARPROC PrinterMessageBoxW = jmpaddr;
	FARPROC PrinterProperties = jmpaddr;
	FARPROC ReadPrinter = jmpaddr;
	FARPROC RegisterForPrintAsyncNotifications = jmpaddr;
	FARPROC ReportJobProcessingProgress = jmpaddr;
	FARPROC ResetPrinterA = jmpaddr;
	FARPROC ResetPrinterW = jmpaddr;
	FARPROC ScheduleJob = jmpaddr;
	FARPROC SeekPrinter = jmpaddr;
	FARPROC SetFormA = jmpaddr;
	FARPROC SetFormW = jmpaddr;
	FARPROC SetJobA = jmpaddr;
	FARPROC SetJobNamedProperty = jmpaddr;
	FARPROC SetJobW = jmpaddr;
	FARPROC SetPortA = jmpaddr;
	FARPROC SetPortW = jmpaddr;
	FARPROC SetPrinterA = jmpaddr;
	FARPROC SetPrinterDataA = jmpaddr;
	FARPROC SetPrinterDataExA = jmpaddr;
	FARPROC SetPrinterDataExW = jmpaddr;
	FARPROC SetPrinterDataW = jmpaddr;
	FARPROC SetPrinterW = jmpaddr;
	FARPROC SplDriverUnloadComplete = jmpaddr;
	FARPROC SpoolerPrinterEvent = jmpaddr;
	FARPROC StartDocDlgA = jmpaddr;
	FARPROC StartDocPrinterA = jmpaddr;
	FARPROC StartDocPrinterW = jmpaddr;
	FARPROC StartPagePrinter = jmpaddr;
	FARPROC UnRegisterForPrintAsyncNotifications = jmpaddr;
	FARPROC UploadPrinterDriverPackageA = jmpaddr;
	FARPROC UploadPrinterDriverPackageW = jmpaddr;
	FARPROC WaitForPrinterChange = jmpaddr;
	FARPROC WritePrinter = jmpaddr;
	FARPROC XcvDataW = jmpaddr;
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
	if (winspool.dll)
	{
		winspool.ADVANCEDSETUPDIALOG = GetFunctionAddress(winspool.dll, "ADVANCEDSETUPDIALOG", jmpaddr);
		winspool.AdvancedSetupDialog = GetFunctionAddress(winspool.dll, "AdvancedSetupDialog", jmpaddr);
		winspool.ConvertAnsiDevModeToUnicodeDevmode = GetFunctionAddress(winspool.dll, "ConvertAnsiDevModeToUnicodeDevmode", jmpaddr);
		winspool.ConvertUnicodeDevModeToAnsiDevmode = GetFunctionAddress(winspool.dll, "ConvertUnicodeDevModeToAnsiDevmode", jmpaddr);
		winspool.DEVICEMODE = GetFunctionAddress(winspool.dll, "DEVICEMODE", jmpaddr);
		winspool.DeviceMode = GetFunctionAddress(winspool.dll, "DeviceMode", jmpaddr);
		winspool.DocumentEvent = GetFunctionAddress(winspool.dll, "DocumentEvent", jmpaddr);
		winspool.PerfClose = GetFunctionAddress(winspool.dll, "PerfClose", jmpaddr);
		winspool.PerfCollect = GetFunctionAddress(winspool.dll, "PerfCollect", jmpaddr);
		winspool.PerfOpen = GetFunctionAddress(winspool.dll, "PerfOpen", jmpaddr);
		winspool.QueryColorProfile = GetFunctionAddress(winspool.dll, "QueryColorProfile", jmpaddr);
		winspool.QueryRemoteFonts = GetFunctionAddress(winspool.dll, "QueryRemoteFonts", jmpaddr);
		winspool.QuerySpoolMode = GetFunctionAddress(winspool.dll, "QuerySpoolMode", jmpaddr);
		winspool.SpoolerDevQueryPrintW = GetFunctionAddress(winspool.dll, "SpoolerDevQueryPrintW", jmpaddr);
		winspool.StartDocDlgW = GetFunctionAddress(winspool.dll, "StartDocDlgW", jmpaddr);
		winspool.AbortPrinter = GetFunctionAddress(winspool.dll, "AbortPrinter", jmpaddr);
		winspool.AddFormA = GetFunctionAddress(winspool.dll, "AddFormA", jmpaddr);
		winspool.AddFormW = GetFunctionAddress(winspool.dll, "AddFormW", jmpaddr);
		winspool.AddJobA = GetFunctionAddress(winspool.dll, "AddJobA", jmpaddr);
		winspool.AddJobW = GetFunctionAddress(winspool.dll, "AddJobW", jmpaddr);
		winspool.AddMonitorA = GetFunctionAddress(winspool.dll, "AddMonitorA", jmpaddr);
		winspool.AddMonitorW = GetFunctionAddress(winspool.dll, "AddMonitorW", jmpaddr);
		winspool.AddPortA = GetFunctionAddress(winspool.dll, "AddPortA", jmpaddr);
		winspool.AddPortExA = GetFunctionAddress(winspool.dll, "AddPortExA", jmpaddr);
		winspool.AddPortExW = GetFunctionAddress(winspool.dll, "AddPortExW", jmpaddr);
		winspool.AddPortW = GetFunctionAddress(winspool.dll, "AddPortW", jmpaddr);
		winspool.AddPrintProcessorA = GetFunctionAddress(winspool.dll, "AddPrintProcessorA", jmpaddr);
		winspool.AddPrintProcessorW = GetFunctionAddress(winspool.dll, "AddPrintProcessorW", jmpaddr);
		winspool.AddPrintProvidorA = GetFunctionAddress(winspool.dll, "AddPrintProvidorA", jmpaddr);
		winspool.AddPrintProvidorW = GetFunctionAddress(winspool.dll, "AddPrintProvidorW", jmpaddr);
		winspool.AddPrinterA = GetFunctionAddress(winspool.dll, "AddPrinterA", jmpaddr);
		winspool.AddPrinterConnection2A = GetFunctionAddress(winspool.dll, "AddPrinterConnection2A", jmpaddr);
		winspool.AddPrinterConnection2W = GetFunctionAddress(winspool.dll, "AddPrinterConnection2W", jmpaddr);
		winspool.AddPrinterConnectionA = GetFunctionAddress(winspool.dll, "AddPrinterConnectionA", jmpaddr);
		winspool.AddPrinterConnectionW = GetFunctionAddress(winspool.dll, "AddPrinterConnectionW", jmpaddr);
		winspool.AddPrinterDriverA = GetFunctionAddress(winspool.dll, "AddPrinterDriverA", jmpaddr);
		winspool.AddPrinterDriverExA = GetFunctionAddress(winspool.dll, "AddPrinterDriverExA", jmpaddr);
		winspool.AddPrinterDriverExW = GetFunctionAddress(winspool.dll, "AddPrinterDriverExW", jmpaddr);
		winspool.AddPrinterDriverW = GetFunctionAddress(winspool.dll, "AddPrinterDriverW", jmpaddr);
		winspool.AddPrinterW = GetFunctionAddress(winspool.dll, "AddPrinterW", jmpaddr);
		winspool.AdvancedDocumentPropertiesA = GetFunctionAddress(winspool.dll, "AdvancedDocumentPropertiesA", jmpaddr);
		winspool.AdvancedDocumentPropertiesW = GetFunctionAddress(winspool.dll, "AdvancedDocumentPropertiesW", jmpaddr);
		winspool.ClosePrinter = GetFunctionAddress(winspool.dll, "ClosePrinter", jmpaddr);
		winspool.CloseSpoolFileHandle = GetFunctionAddress(winspool.dll, "CloseSpoolFileHandle", jmpaddr);
		winspool.CommitSpoolData = GetFunctionAddress(winspool.dll, "CommitSpoolData", jmpaddr);
		winspool.ConfigurePortA = GetFunctionAddress(winspool.dll, "ConfigurePortA", jmpaddr);
		winspool.ConfigurePortW = GetFunctionAddress(winspool.dll, "ConfigurePortW", jmpaddr);
		winspool.ConnectToPrinterDlg = GetFunctionAddress(winspool.dll, "ConnectToPrinterDlg", jmpaddr);
		winspool.CorePrinterDriverInstalledA = GetFunctionAddress(winspool.dll, "CorePrinterDriverInstalledA", jmpaddr);
		winspool.CorePrinterDriverInstalledW = GetFunctionAddress(winspool.dll, "CorePrinterDriverInstalledW", jmpaddr);
		winspool.CreatePrintAsyncNotifyChannel = GetFunctionAddress(winspool.dll, "CreatePrintAsyncNotifyChannel", jmpaddr);
		winspool.CreatePrinterIC = GetFunctionAddress(winspool.dll, "CreatePrinterIC", jmpaddr);
		winspool.DEVICECAPABILITIES = GetFunctionAddress(winspool.dll, "DEVICECAPABILITIES", jmpaddr);
		winspool.DeleteFormA = GetFunctionAddress(winspool.dll, "DeleteFormA", jmpaddr);
		winspool.DeleteFormW = GetFunctionAddress(winspool.dll, "DeleteFormW", jmpaddr);
		winspool.DeleteJobNamedProperty = GetFunctionAddress(winspool.dll, "DeleteJobNamedProperty", jmpaddr);
		winspool.DeleteMonitorA = GetFunctionAddress(winspool.dll, "DeleteMonitorA", jmpaddr);
		winspool.DeleteMonitorW = GetFunctionAddress(winspool.dll, "DeleteMonitorW", jmpaddr);
		winspool.DeletePortA = GetFunctionAddress(winspool.dll, "DeletePortA", jmpaddr);
		winspool.DeletePortW = GetFunctionAddress(winspool.dll, "DeletePortW", jmpaddr);
		winspool.DeletePrintProcessorA = GetFunctionAddress(winspool.dll, "DeletePrintProcessorA", jmpaddr);
		winspool.DeletePrintProcessorW = GetFunctionAddress(winspool.dll, "DeletePrintProcessorW", jmpaddr);
		winspool.DeletePrintProvidorA = GetFunctionAddress(winspool.dll, "DeletePrintProvidorA", jmpaddr);
		winspool.DeletePrintProvidorW = GetFunctionAddress(winspool.dll, "DeletePrintProvidorW", jmpaddr);
		winspool.DeletePrinter = GetFunctionAddress(winspool.dll, "DeletePrinter", jmpaddr);
		winspool.DeletePrinterConnectionA = GetFunctionAddress(winspool.dll, "DeletePrinterConnectionA", jmpaddr);
		winspool.DeletePrinterConnectionW = GetFunctionAddress(winspool.dll, "DeletePrinterConnectionW", jmpaddr);
		winspool.DeletePrinterDataA = GetFunctionAddress(winspool.dll, "DeletePrinterDataA", jmpaddr);
		winspool.DeletePrinterDataExA = GetFunctionAddress(winspool.dll, "DeletePrinterDataExA", jmpaddr);
		winspool.DeletePrinterDataExW = GetFunctionAddress(winspool.dll, "DeletePrinterDataExW", jmpaddr);
		winspool.DeletePrinterDataW = GetFunctionAddress(winspool.dll, "DeletePrinterDataW", jmpaddr);
		winspool.DeletePrinterDriverA = GetFunctionAddress(winspool.dll, "DeletePrinterDriverA", jmpaddr);
		winspool.DeletePrinterDriverExA = GetFunctionAddress(winspool.dll, "DeletePrinterDriverExA", jmpaddr);
		winspool.DeletePrinterDriverExW = GetFunctionAddress(winspool.dll, "DeletePrinterDriverExW", jmpaddr);
		winspool.DeletePrinterDriverPackageA = GetFunctionAddress(winspool.dll, "DeletePrinterDriverPackageA", jmpaddr);
		winspool.DeletePrinterDriverPackageW = GetFunctionAddress(winspool.dll, "DeletePrinterDriverPackageW", jmpaddr);
		winspool.DeletePrinterDriverW = GetFunctionAddress(winspool.dll, "DeletePrinterDriverW", jmpaddr);
		winspool.DeletePrinterIC = GetFunctionAddress(winspool.dll, "DeletePrinterIC", jmpaddr);
		winspool.DeletePrinterKeyA = GetFunctionAddress(winspool.dll, "DeletePrinterKeyA", jmpaddr);
		winspool.DeletePrinterKeyW = GetFunctionAddress(winspool.dll, "DeletePrinterKeyW", jmpaddr);
		winspool.DevQueryPrint = GetFunctionAddress(winspool.dll, "DevQueryPrint", jmpaddr);
		winspool.DevQueryPrintEx = GetFunctionAddress(winspool.dll, "DevQueryPrintEx", jmpaddr);
		winspool.DeviceCapabilities = GetFunctionAddress(winspool.dll, "DeviceCapabilities", jmpaddr);
		winspool.DeviceCapabilitiesA = GetFunctionAddress(winspool.dll, "DeviceCapabilitiesA", jmpaddr);
		winspool.DeviceCapabilitiesW = GetFunctionAddress(winspool.dll, "DeviceCapabilitiesW", jmpaddr);
		winspool.DevicePropertySheets = GetFunctionAddress(winspool.dll, "DevicePropertySheets", jmpaddr);
		winspool.DocumentPropertiesA = GetFunctionAddress(winspool.dll, "DocumentPropertiesA", jmpaddr);
		winspool.DocumentPropertiesW = GetFunctionAddress(winspool.dll, "DocumentPropertiesW", jmpaddr);
		winspool.DocumentPropertySheets = GetFunctionAddress(winspool.dll, "DocumentPropertySheets", jmpaddr);
		winspool.EXTDEVICEMODE = GetFunctionAddress(winspool.dll, "EXTDEVICEMODE", jmpaddr);
		winspool.EndDocPrinter = GetFunctionAddress(winspool.dll, "EndDocPrinter", jmpaddr);
		winspool.EndPagePrinter = GetFunctionAddress(winspool.dll, "EndPagePrinter", jmpaddr);
		winspool.EnumFormsA = GetFunctionAddress(winspool.dll, "EnumFormsA", jmpaddr);
		winspool.EnumFormsW = GetFunctionAddress(winspool.dll, "EnumFormsW", jmpaddr);
		winspool.EnumJobNamedProperties = GetFunctionAddress(winspool.dll, "EnumJobNamedProperties", jmpaddr);
		winspool.EnumJobsA = GetFunctionAddress(winspool.dll, "EnumJobsA", jmpaddr);
		winspool.EnumJobsW = GetFunctionAddress(winspool.dll, "EnumJobsW", jmpaddr);
		winspool.EnumMonitorsA = GetFunctionAddress(winspool.dll, "EnumMonitorsA", jmpaddr);
		winspool.EnumMonitorsW = GetFunctionAddress(winspool.dll, "EnumMonitorsW", jmpaddr);
		winspool.EnumPortsA = GetFunctionAddress(winspool.dll, "EnumPortsA", jmpaddr);
		winspool.GetDefaultPrinterA = GetFunctionAddress(winspool.dll, "GetDefaultPrinterA", jmpaddr);
		winspool.SetDefaultPrinterA = GetFunctionAddress(winspool.dll, "SetDefaultPrinterA", jmpaddr);
		winspool.GetDefaultPrinterW = GetFunctionAddress(winspool.dll, "GetDefaultPrinterW", jmpaddr);
		winspool.SetDefaultPrinterW = GetFunctionAddress(winspool.dll, "SetDefaultPrinterW", jmpaddr);
		winspool.EnumPortsW = GetFunctionAddress(winspool.dll, "EnumPortsW", jmpaddr);
		winspool.EnumPrintProcessorDatatypesA = GetFunctionAddress(winspool.dll, "EnumPrintProcessorDatatypesA", jmpaddr);
		winspool.EnumPrintProcessorDatatypesW = GetFunctionAddress(winspool.dll, "EnumPrintProcessorDatatypesW", jmpaddr);
		winspool.EnumPrintProcessorsA = GetFunctionAddress(winspool.dll, "EnumPrintProcessorsA", jmpaddr);
		winspool.EnumPrintProcessorsW = GetFunctionAddress(winspool.dll, "EnumPrintProcessorsW", jmpaddr);
		winspool.EnumPrinterDataA = GetFunctionAddress(winspool.dll, "EnumPrinterDataA", jmpaddr);
		winspool.EnumPrinterDataExA = GetFunctionAddress(winspool.dll, "EnumPrinterDataExA", jmpaddr);
		winspool.EnumPrinterDataExW = GetFunctionAddress(winspool.dll, "EnumPrinterDataExW", jmpaddr);
		winspool.EnumPrinterDataW = GetFunctionAddress(winspool.dll, "EnumPrinterDataW", jmpaddr);
		winspool.EnumPrinterDriversA = GetFunctionAddress(winspool.dll, "EnumPrinterDriversA", jmpaddr);
		winspool.EnumPrinterDriversW = GetFunctionAddress(winspool.dll, "EnumPrinterDriversW", jmpaddr);
		winspool.EnumPrinterKeyA = GetFunctionAddress(winspool.dll, "EnumPrinterKeyA", jmpaddr);
		winspool.EnumPrinterKeyW = GetFunctionAddress(winspool.dll, "EnumPrinterKeyW", jmpaddr);
		winspool.EnumPrintersA = GetFunctionAddress(winspool.dll, "EnumPrintersA", jmpaddr);
		winspool.EnumPrintersW = GetFunctionAddress(winspool.dll, "EnumPrintersW", jmpaddr);
		winspool.ExtDeviceMode = GetFunctionAddress(winspool.dll, "ExtDeviceMode", jmpaddr);
		winspool.FindClosePrinterChangeNotification = GetFunctionAddress(winspool.dll, "FindClosePrinterChangeNotification", jmpaddr);
		winspool.FindFirstPrinterChangeNotification = GetFunctionAddress(winspool.dll, "FindFirstPrinterChangeNotification", jmpaddr);
		winspool.FindNextPrinterChangeNotification = GetFunctionAddress(winspool.dll, "FindNextPrinterChangeNotification", jmpaddr);
		winspool.FlushPrinter = GetFunctionAddress(winspool.dll, "FlushPrinter", jmpaddr);
		winspool.FreePrintNamedPropertyArray = GetFunctionAddress(winspool.dll, "FreePrintNamedPropertyArray", jmpaddr);
		winspool.FreePrintPropertyValue = GetFunctionAddress(winspool.dll, "FreePrintPropertyValue", jmpaddr);
		winspool.FreePrinterNotifyInfo = GetFunctionAddress(winspool.dll, "FreePrinterNotifyInfo", jmpaddr);
		winspool.GetCorePrinterDriversA = GetFunctionAddress(winspool.dll, "GetCorePrinterDriversA", jmpaddr);
		winspool.GetCorePrinterDriversW = GetFunctionAddress(winspool.dll, "GetCorePrinterDriversW", jmpaddr);
		winspool.GetFormA = GetFunctionAddress(winspool.dll, "GetFormA", jmpaddr);
		winspool.GetFormW = GetFunctionAddress(winspool.dll, "GetFormW", jmpaddr);
		winspool.GetJobA = GetFunctionAddress(winspool.dll, "GetJobA", jmpaddr);
		winspool.GetJobNamedPropertyValue = GetFunctionAddress(winspool.dll, "GetJobNamedPropertyValue", jmpaddr);
		winspool.GetJobW = GetFunctionAddress(winspool.dll, "GetJobW", jmpaddr);
		winspool.GetPrintExecutionData = GetFunctionAddress(winspool.dll, "GetPrintExecutionData", jmpaddr);
		winspool.GetPrintOutputInfo = GetFunctionAddress(winspool.dll, "GetPrintOutputInfo", jmpaddr);
		winspool.GetPrintProcessorDirectoryA = GetFunctionAddress(winspool.dll, "GetPrintProcessorDirectoryA", jmpaddr);
		winspool.GetPrintProcessorDirectoryW = GetFunctionAddress(winspool.dll, "GetPrintProcessorDirectoryW", jmpaddr);
		winspool.GetPrinterA = GetFunctionAddress(winspool.dll, "GetPrinterA", jmpaddr);
		winspool.GetPrinterDataA = GetFunctionAddress(winspool.dll, "GetPrinterDataA", jmpaddr);
		winspool.GetPrinterDataExA = GetFunctionAddress(winspool.dll, "GetPrinterDataExA", jmpaddr);
		winspool.GetPrinterDataExW = GetFunctionAddress(winspool.dll, "GetPrinterDataExW", jmpaddr);
		winspool.GetPrinterDataW = GetFunctionAddress(winspool.dll, "GetPrinterDataW", jmpaddr);
		winspool.GetPrinterDriver2A = GetFunctionAddress(winspool.dll, "GetPrinterDriver2A", jmpaddr);
		winspool.GetPrinterDriver2W = GetFunctionAddress(winspool.dll, "GetPrinterDriver2W", jmpaddr);
		winspool.GetPrinterDriverA = GetFunctionAddress(winspool.dll, "GetPrinterDriverA", jmpaddr);
		winspool.GetPrinterDriverDirectoryA = GetFunctionAddress(winspool.dll, "GetPrinterDriverDirectoryA", jmpaddr);
		winspool.GetPrinterDriverDirectoryW = GetFunctionAddress(winspool.dll, "GetPrinterDriverDirectoryW", jmpaddr);
		winspool.GetPrinterDriverPackagePathA = GetFunctionAddress(winspool.dll, "GetPrinterDriverPackagePathA", jmpaddr);
		winspool.GetPrinterDriverPackagePathW = GetFunctionAddress(winspool.dll, "GetPrinterDriverPackagePathW", jmpaddr);
		winspool.GetPrinterDriverW = GetFunctionAddress(winspool.dll, "GetPrinterDriverW", jmpaddr);
		winspool.GetPrinterW = GetFunctionAddress(winspool.dll, "GetPrinterW", jmpaddr);
		winspool.GetSpoolFileHandle = GetFunctionAddress(winspool.dll, "GetSpoolFileHandle", jmpaddr);
		winspool.InstallPrinterDriverFromPackageA = GetFunctionAddress(winspool.dll, "InstallPrinterDriverFromPackageA", jmpaddr);
		winspool.InstallPrinterDriverFromPackageW = GetFunctionAddress(winspool.dll, "InstallPrinterDriverFromPackageW", jmpaddr);
		winspool.IsValidDevmodeA = GetFunctionAddress(winspool.dll, "IsValidDevmodeA", jmpaddr);
		winspool.IsValidDevmodeW = GetFunctionAddress(winspool.dll, "IsValidDevmodeW", jmpaddr);
		winspool.OpenPrinter2A = GetFunctionAddress(winspool.dll, "OpenPrinter2A", jmpaddr);
		winspool.OpenPrinter2W = GetFunctionAddress(winspool.dll, "OpenPrinter2W", jmpaddr);
		winspool.OpenPrinterA = GetFunctionAddress(winspool.dll, "OpenPrinterA", jmpaddr);
		winspool.OpenPrinterW = GetFunctionAddress(winspool.dll, "OpenPrinterW", jmpaddr);
		winspool.PlayGdiScriptOnPrinterIC = GetFunctionAddress(winspool.dll, "PlayGdiScriptOnPrinterIC", jmpaddr);
		winspool.PrinterMessageBoxA = GetFunctionAddress(winspool.dll, "PrinterMessageBoxA", jmpaddr);
		winspool.PrinterMessageBoxW = GetFunctionAddress(winspool.dll, "PrinterMessageBoxW", jmpaddr);
		winspool.PrinterProperties = GetFunctionAddress(winspool.dll, "PrinterProperties", jmpaddr);
		winspool.ReadPrinter = GetFunctionAddress(winspool.dll, "ReadPrinter", jmpaddr);
		winspool.RegisterForPrintAsyncNotifications = GetFunctionAddress(winspool.dll, "RegisterForPrintAsyncNotifications", jmpaddr);
		winspool.ReportJobProcessingProgress = GetFunctionAddress(winspool.dll, "ReportJobProcessingProgress", jmpaddr);
		winspool.ResetPrinterA = GetFunctionAddress(winspool.dll, "ResetPrinterA", jmpaddr);
		winspool.ResetPrinterW = GetFunctionAddress(winspool.dll, "ResetPrinterW", jmpaddr);
		winspool.ScheduleJob = GetFunctionAddress(winspool.dll, "ScheduleJob", jmpaddr);
		winspool.SeekPrinter = GetFunctionAddress(winspool.dll, "SeekPrinter", jmpaddr);
		winspool.SetFormA = GetFunctionAddress(winspool.dll, "SetFormA", jmpaddr);
		winspool.SetFormW = GetFunctionAddress(winspool.dll, "SetFormW", jmpaddr);
		winspool.SetJobA = GetFunctionAddress(winspool.dll, "SetJobA", jmpaddr);
		winspool.SetJobNamedProperty = GetFunctionAddress(winspool.dll, "SetJobNamedProperty", jmpaddr);
		winspool.SetJobW = GetFunctionAddress(winspool.dll, "SetJobW", jmpaddr);
		winspool.SetPortA = GetFunctionAddress(winspool.dll, "SetPortA", jmpaddr);
		winspool.SetPortW = GetFunctionAddress(winspool.dll, "SetPortW", jmpaddr);
		winspool.SetPrinterA = GetFunctionAddress(winspool.dll, "SetPrinterA", jmpaddr);
		winspool.SetPrinterDataA = GetFunctionAddress(winspool.dll, "SetPrinterDataA", jmpaddr);
		winspool.SetPrinterDataExA = GetFunctionAddress(winspool.dll, "SetPrinterDataExA", jmpaddr);
		winspool.SetPrinterDataExW = GetFunctionAddress(winspool.dll, "SetPrinterDataExW", jmpaddr);
		winspool.SetPrinterDataW = GetFunctionAddress(winspool.dll, "SetPrinterDataW", jmpaddr);
		winspool.SetPrinterW = GetFunctionAddress(winspool.dll, "SetPrinterW", jmpaddr);
		winspool.SplDriverUnloadComplete = GetFunctionAddress(winspool.dll, "SplDriverUnloadComplete", jmpaddr);
		winspool.SpoolerPrinterEvent = GetFunctionAddress(winspool.dll, "SpoolerPrinterEvent", jmpaddr);
		winspool.StartDocDlgA = GetFunctionAddress(winspool.dll, "StartDocDlgA", jmpaddr);
		winspool.StartDocPrinterA = GetFunctionAddress(winspool.dll, "StartDocPrinterA", jmpaddr);
		winspool.StartDocPrinterW = GetFunctionAddress(winspool.dll, "StartDocPrinterW", jmpaddr);
		winspool.StartPagePrinter = GetFunctionAddress(winspool.dll, "StartPagePrinter", jmpaddr);
		winspool.UnRegisterForPrintAsyncNotifications = GetFunctionAddress(winspool.dll, "UnRegisterForPrintAsyncNotifications", jmpaddr);
		winspool.UploadPrinterDriverPackageA = GetFunctionAddress(winspool.dll, "UploadPrinterDriverPackageA", jmpaddr);
		winspool.UploadPrinterDriverPackageW = GetFunctionAddress(winspool.dll, "UploadPrinterDriverPackageW", jmpaddr);
		winspool.WaitForPrinterChange = GetFunctionAddress(winspool.dll, "WaitForPrinterChange", jmpaddr);
		winspool.WritePrinter = GetFunctionAddress(winspool.dll, "WritePrinter", jmpaddr);
		winspool.XcvDataW = GetFunctionAddress(winspool.dll, "XcvDataW", jmpaddr);
	}
}