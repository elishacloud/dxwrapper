#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <D3dDdi/FormatInfo.h>

namespace
{
	struct ArgbFormatInfo : D3dDdi::FormatInfo
	{
		ArgbFormatInfo(BYTE alphaBitCount, BYTE redBitCount, BYTE greenBitCount, BYTE blueBitCount)
			: FormatInfo(alphaBitCount, redBitCount, greenBitCount, blueBitCount)
		{
			redPos = greenBitCount + blueBitCount;
			greenPos = blueBitCount;
		}
	};

	struct AbgrFormatInfo : D3dDdi::FormatInfo
	{
		AbgrFormatInfo(BYTE alphaBitCount, BYTE blueBitCount, BYTE greenBitCount, BYTE redBitCount)
			: FormatInfo(alphaBitCount, redBitCount, greenBitCount, blueBitCount)
		{
			bluePos = redBitCount + greenBitCount;
			greenPos = redBitCount;
		}
	};

	struct XrgbFormatInfo : ArgbFormatInfo
	{
		XrgbFormatInfo(BYTE unusedBitCount, BYTE redBitCount, BYTE greenBitCount, BYTE blueBitCount)
			: ArgbFormatInfo(0, redBitCount, greenBitCount, blueBitCount)
		{
			bytesPerPixel = (unusedBitCount + redBitCount + greenBitCount + blueBitCount + 7) / 8;
		}
	};

	struct XbgrFormatInfo : AbgrFormatInfo
	{
		XbgrFormatInfo(BYTE unusedBitCount, BYTE blueBitCount, BYTE greenBitCount, BYTE redBitCount)
			: AbgrFormatInfo(0, redBitCount, greenBitCount, blueBitCount)
		{
			bytesPerPixel = (unusedBitCount + redBitCount + greenBitCount + blueBitCount + 7) / 8;
		}
	};
}

namespace D3dDdi
{
	FormatInfo::FormatInfo(BYTE alphaBitCount, BYTE redBitCount, BYTE greenBitCount, BYTE blueBitCount)
		: bytesPerPixel((alphaBitCount + redBitCount + greenBitCount + blueBitCount + 7) / 8)
		, alphaBitCount(alphaBitCount)
		, alphaPos(redBitCount + greenBitCount + blueBitCount)
		, redBitCount(redBitCount)
		, redPos(0)
		, greenBitCount(greenBitCount)
		, greenPos(0)
		, blueBitCount(blueBitCount)
		, bluePos(0)
	{
	}

	D3DCOLOR colorConvert(const FormatInfo& dstFormatInfo, D3DCOLOR srcRgbaColor)
	{
		struct ArgbColor
		{
			BYTE blue;
			BYTE green;
			BYTE red;
			BYTE alpha;
		};
		
		auto& srcColor = *reinterpret_cast<ArgbColor*>(&srcRgbaColor);

		BYTE alpha = srcColor.alpha >> (8 - dstFormatInfo.alphaBitCount);
		BYTE red = srcColor.red >> (8 - dstFormatInfo.redBitCount);
		BYTE green = srcColor.green >> (8 - dstFormatInfo.greenBitCount);
		BYTE blue = srcColor.blue >> (8 - dstFormatInfo.blueBitCount);

		return (alpha << dstFormatInfo.alphaPos) |
			(red << dstFormatInfo.redPos) |
			(green << dstFormatInfo.greenPos) |
			(blue << dstFormatInfo.bluePos);
	}

	FormatInfo getFormatInfo(D3DDDIFORMAT format)
	{
		switch (format)
		{
		case D3DDDIFMT_R3G3B2:		return ArgbFormatInfo(0, 3, 3, 2);
		case D3DDDIFMT_A8:			return ArgbFormatInfo(8, 0, 0, 0);
		case D3DDDIFMT_P8:			return ArgbFormatInfo(0, 0, 0, 8);
		case D3DDDIFMT_R8:			return ArgbFormatInfo(0, 8, 0, 0);

		case D3DDDIFMT_R5G6B5:		return ArgbFormatInfo(0, 5, 6, 5);
		case D3DDDIFMT_X1R5G5B5:	return XrgbFormatInfo(1, 5, 5, 5);
		case D3DDDIFMT_A1R5G5B5:	return ArgbFormatInfo(1, 5, 5, 5);
		case D3DDDIFMT_A4R4G4B4:	return ArgbFormatInfo(4, 4, 4, 4);
		case D3DDDIFMT_A8R3G3B2:	return ArgbFormatInfo(8, 3, 3, 2);
		case D3DDDIFMT_X4R4G4B4:	return XrgbFormatInfo(4, 4, 4, 4);
		case D3DDDIFMT_A8P8:		return ArgbFormatInfo(8, 0, 0, 8);
		case D3DDDIFMT_G8R8:		return AbgrFormatInfo(0, 0, 8, 8);

		case D3DDDIFMT_R8G8B8:		return ArgbFormatInfo(0, 8, 8, 8);

		case D3DDDIFMT_A8R8G8B8:	return ArgbFormatInfo(8, 8, 8, 8);
		case D3DDDIFMT_X8R8G8B8:	return XrgbFormatInfo(8, 8, 8, 8);
		case D3DDDIFMT_A8B8G8R8:	return AbgrFormatInfo(8, 8, 8, 8);
		case D3DDDIFMT_X8B8G8R8:	return XbgrFormatInfo(8, 8, 8, 8);

		default:
			return FormatInfo();
		}
	}
}
