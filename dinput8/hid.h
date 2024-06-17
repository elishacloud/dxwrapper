#pragma once

// From hidusage.h
///////////////////////////////////////////////////////////////////////
typedef USHORT USAGE, *PUSAGE;

// From hidpi.h
///////////////////////////////////////////////////////////////////////
typedef struct _HIDP_PREPARSED_DATA* PHIDP_PREPARSED_DATA;

typedef struct _HIDP_CAPS
{
	USAGE    Usage;
	USAGE    UsagePage;
	USHORT   InputReportByteLength;
	USHORT   OutputReportByteLength;
	USHORT   FeatureReportByteLength;
	USHORT   Reserved[17];

	USHORT   NumberLinkCollectionNodes;

	USHORT   NumberInputButtonCaps;
	USHORT   NumberInputValueCaps;
	USHORT   NumberInputDataIndices;

	USHORT   NumberOutputButtonCaps;
	USHORT   NumberOutputValueCaps;
	USHORT   NumberOutputDataIndices;

	USHORT   NumberFeatureButtonCaps;
	USHORT   NumberFeatureValueCaps;
	USHORT   NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;

typedef struct _HIDP_BUTTON_CAPS
{
	USAGE    UsagePage;
	UCHAR    ReportID;
	BOOLEAN  IsAlias;

	USHORT   BitField;
	USHORT   LinkCollection;   // A unique internal index pointer

	USAGE    LinkUsage;
	USAGE    LinkUsagePage;

	BOOLEAN  IsRange;
	BOOLEAN  IsStringRange;
	BOOLEAN  IsDesignatorRange;
	BOOLEAN  IsAbsolute;

	ULONG    Reserved[10];
	union {
		struct {
			USAGE    UsageMin, UsageMax;
			USHORT   StringMin, StringMax;
			USHORT   DesignatorMin, DesignatorMax;
			USHORT   DataIndexMin, DataIndexMax;
		} Range;
		struct {
			USAGE    Usage, Reserved1;
			USHORT   StringIndex, Reserved2;
			USHORT   DesignatorIndex, Reserved3;
			USHORT   DataIndex, Reserved4;
		} NotRange;
	};

} HIDP_BUTTON_CAPS, * PHIDP_BUTTON_CAPS;

typedef enum _HIDP_REPORT_TYPE
{
	HidP_Input,
	HidP_Output,
	HidP_Feature
} HIDP_REPORT_TYPE;


#ifndef FACILITY_HID_ERROR_CODE
#define FACILITY_HID_ERROR_CODE 0x11
#endif

#define HIDP_ERROR_CODES(SEV, CODE) ((NTSTATUS) (((SEV) << 28) | (FACILITY_HID_ERROR_CODE << 16) | (CODE)))
#define HIDP_STATUS_SUCCESS                  (HIDP_ERROR_CODES(0x0,0))
#define HIDP_STATUS_NULL                     (HIDP_ERROR_CODES(0x8,1))
#define HIDP_STATUS_INVALID_PREPARSED_DATA   (HIDP_ERROR_CODES(0xC,1))
#define HIDP_STATUS_INVALID_REPORT_TYPE      (HIDP_ERROR_CODES(0xC,2))
#define HIDP_STATUS_INVALID_REPORT_LENGTH    (HIDP_ERROR_CODES(0xC,3))
#define HIDP_STATUS_USAGE_NOT_FOUND          (HIDP_ERROR_CODES(0xC,4))
#define HIDP_STATUS_VALUE_OUT_OF_RANGE       (HIDP_ERROR_CODES(0xC,5))
#define HIDP_STATUS_BAD_LOG_PHY_VALUES       (HIDP_ERROR_CODES(0xC,6))
#define HIDP_STATUS_BUFFER_TOO_SMALL         (HIDP_ERROR_CODES(0xC,7))
#define HIDP_STATUS_INTERNAL_ERROR           (HIDP_ERROR_CODES(0xC,8))
#define HIDP_STATUS_I8042_TRANS_UNKNOWN      (HIDP_ERROR_CODES(0xC,9))
#define HIDP_STATUS_INCOMPATIBLE_REPORT_ID   (HIDP_ERROR_CODES(0xC,0xA))
#define HIDP_STATUS_NOT_VALUE_ARRAY          (HIDP_ERROR_CODES(0xC,0xB))
#define HIDP_STATUS_IS_VALUE_ARRAY           (HIDP_ERROR_CODES(0xC,0xC))
#define HIDP_STATUS_DATA_INDEX_NOT_FOUND     (HIDP_ERROR_CODES(0xC,0xD))
#define HIDP_STATUS_DATA_INDEX_OUT_OF_RANGE  (HIDP_ERROR_CODES(0xC,0xE))
#define HIDP_STATUS_BUTTON_NOT_PRESSED       (HIDP_ERROR_CODES(0xC,0xF))
#define HIDP_STATUS_REPORT_DOES_NOT_EXIST    (HIDP_ERROR_CODES(0xC,0x10))
#define HIDP_STATUS_NOT_IMPLEMENTED          (HIDP_ERROR_CODES(0xC,0x20))



// hid.dll
/////////////////////////////////////////////////////////////////////
extern bool hidDllLoaded;

typedef long NTSTATUS;

typedef NTSTATUS __stdcall HIDP_GETCAPS_FUNC(PHIDP_PREPARSED_DATA PreparsedData, PHIDP_CAPS Capabilities);
extern HIDP_GETCAPS_FUNC* HidP_GetCaps;

typedef NTSTATUS __stdcall HIDP_GETBUTTONCAPS_FUNC(HIDP_REPORT_TYPE ReportType, PHIDP_BUTTON_CAPS ButtonCaps, PUSHORT ButtonCapsLength, PHIDP_PREPARSED_DATA PreparsedData);
extern HIDP_GETBUTTONCAPS_FUNC* HidP_GetButtonCaps;

typedef NTSTATUS __stdcall HIDP_GETUSAGES_FUNC(HIDP_REPORT_TYPE ReportType, USAGE UsagePage, USHORT LinkCollection, PUSAGE UsageList, PULONG UsageLength, PHIDP_PREPARSED_DATA PreparsedData, PCHAR Report, ULONG ReportLength);
extern HIDP_GETUSAGES_FUNC* HidP_GetUsages;

void LoadHidLibrary();
