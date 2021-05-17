#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <array>
#include <type_traits>
#include <vector>

#include <intrin.h>

#include <DDrawCompat/v0.3.1/Common/ScopedCriticalSection.h>
#include <DDrawCompat/v0.3.1/DDraw/Blitter.h>

#pragma warning(disable : 4127)

namespace
{
	Compat31::CriticalSection g_overlappingBltCs;

#pragma pack(1)
	class UInt24
	{
	public:
		UInt24(DWORD value)
		{
			m_low16 = static_cast<WORD>(value);
			m_high8 = static_cast<BYTE>((value & 0x00FF0000) >> 16);
		}

	private:
		WORD m_low16;
		BYTE m_high8;
	};
#pragma pack()

	template <typename Elem, std::size_t... dim>
	struct MultiDimArray;

	template <typename Elem, std::size_t firstDim, std::size_t... dim>
	struct MultiDimArray<Elem, firstDim, dim...>
	{
		typedef std::array<typename MultiDimArray<Elem, dim...>::type, firstDim> type;
	};

	template <typename Elem, std::size_t dim>
	struct MultiDimArray<Elem, dim>
	{
		typedef std::array<Elem, dim> type;
	};

	void blt(BYTE* dst, DWORD dstPitch, DWORD dstWidth, DWORD dstHeight,
		const BYTE* src, DWORD srcPitch, LONG srcWidth, LONG srcHeight,
		DWORD bytesPerPixel, const DWORD* dstColorKey, const DWORD* srcColorKey);

	template <int n> __m128i _mm_cmpeq_epi(__m128i a, __m128i b);
	template <> __m128i _mm_cmpeq_epi<8>(__m128i a, __m128i b) { return _mm_cmpeq_epi8(a, b); }
	template <> __m128i _mm_cmpeq_epi<16>(__m128i a, __m128i b) { return _mm_cmpeq_epi16(a, b); }
	template <> __m128i _mm_cmpeq_epi<32>(__m128i a, __m128i b) { return _mm_cmpeq_epi32(a, b); }

	template <int n> __m128i _mm_loadu_si(const void* p);
	template <> __m128i _mm_loadu_si<8>(const void* p) { return _mm_cvtsi32_si128(*static_cast<const uint8_t*>(p)); }
	template <> __m128i _mm_loadu_si<16>(const void* p) { return _mm_cvtsi32_si128(*static_cast<const uint16_t*>(p)); }
	template <> __m128i _mm_loadu_si<32>(const void* p) { return _mm_cvtsi32_si128(*static_cast<const uint32_t*>(p)); }
	template <> __m128i _mm_loadu_si<64>(const void* p) { return _mm_loadl_epi64(static_cast<const __m128i*>(p)); }
	template <> __m128i _mm_loadu_si<128>(const void* p) { return _mm_loadu_si128(static_cast<const __m128i*>(p)); }

	template <int n> __m128i _mm_set1_epi(DWORD a);
	template <> __m128i _mm_set1_epi<8>(DWORD a) { return _mm_set1_epi8(static_cast<uint8_t>(a)); }
	template <> __m128i _mm_set1_epi<16>(DWORD a) { return _mm_set1_epi16(static_cast<uint16_t>(a)); }
	template <> __m128i _mm_set1_epi<32>(DWORD a) { return _mm_set1_epi32(a); }

	template <int n> void _mm_storeu_si(void* p, __m128i a);
	template <> void _mm_storeu_si<8>(void* p, __m128i a) { *static_cast<uint8_t*>(p) = static_cast<uint8_t>(_mm_cvtsi128_si32(a)); }
	template <> void _mm_storeu_si<16>(void* p, __m128i a) { *static_cast<uint16_t*>(p) = static_cast<uint16_t>(_mm_cvtsi128_si32(a)); }
	template <> void _mm_storeu_si<32>(void* p, __m128i a) { *static_cast<uint32_t*>(p) = _mm_cvtsi128_si32(a); }
	template <> void _mm_storeu_si<64>(void* p, __m128i a) { _mm_storel_epi64(static_cast<__m128i*>(p), a); }
	template <> void _mm_storeu_si<128>(void* p, __m128i a) { _mm_storeu_si128(static_cast<__m128i*>(p), a); }

	template <typename Pixel, int vectorSize>
	__forceinline __m128i reverseVector(__m128i vec)
	{
		if (16 == vectorSize)
		{
			vec = _mm_shuffle_epi32(vec, _MM_SHUFFLE(0, 1, 2, 3));
		}
		else if (8 == vectorSize)
		{
			vec = _mm_shuffle_epi32(vec, _MM_SHUFFLE(3, 2, 0, 1));
		}

		if (sizeof(Pixel) <= 2)
		{
			if (vectorSize > 2)
			{
				vec = _mm_shufflelo_epi16(vec, _MM_SHUFFLE(2, 3, 0, 1));
			}
			if (16 == vectorSize)
			{
				vec = _mm_shufflehi_epi16(vec, _MM_SHUFFLE(2, 3, 0, 1));
			}
		}

		if (1 == sizeof(Pixel) && vectorSize > 1)
		{
			vec = _mm_or_si128(_mm_slli_epi16(vec, 8), _mm_srli_epi16(vec, 8));
		}

		return vec;
	}

	template <int pixelsPerVector, int count>
	__forceinline void loadSrcVectorRemainder(__m128i& vec1, __m128i& vec2,
		const BYTE*& src, int& offset, int delta, std::integral_constant<int, count>)
	{
		vec1 = _mm_insert_epi16(vec1, *(src + (offset >> 16)), (pixelsPerVector - count) / 2);
		offset += delta;
		vec2 = _mm_insert_epi16(vec2, *(src + (offset >> 16)), (pixelsPerVector - count) / 2);
		offset += delta;
		loadSrcVectorRemainder<pixelsPerVector>(vec1, vec2, src, offset, delta, std::integral_constant<int, count - 2>());
	}

	template <int pixelsPerVector>
	__forceinline void loadSrcVectorRemainder(__m128i& vec1, __m128i& vec2,
		const BYTE*& src, int& offset, int delta, std::integral_constant<int, 1> /*count*/)
	{
		vec1 = _mm_insert_epi16(vec1, *(src + (offset >> 16)), (pixelsPerVector - 1) / 2);
		offset += delta;
	}

	template <int pixelsPerVector>
	__forceinline void loadSrcVectorRemainder(__m128i& /*vec1*/, __m128i& /*vec2*/,
		const BYTE*& /*src*/, int& /*offset*/, int /*delta*/, std::integral_constant<int, 0> /*count*/)
	{
	}

	template <int pixelsPerVector>
	__forceinline void loadSrcVectorRemainder(__m128i& /*vec1*/, __m128i& /*vec2*/,
		const BYTE*& /*src*/, int& /*offset*/, int /*delta*/, std::integral_constant<int, -1> /*count*/)
	{
	}

	template <int pixelsPerVector, int count>
	__forceinline void loadSrcVectorRemainder(__m128i& vec,
		const BYTE* src, int& offset, int delta, std::integral_constant<int, count>)
	{
		__m128i vec2 = _mm_loadu_si<8>(src + (offset >> 16));
		offset += delta;
		loadSrcVectorRemainder<pixelsPerVector>(vec, vec2, src, offset, delta, std::integral_constant<int, count - 1>());
		vec2 = _mm_slli_si128(vec2, 1);
		vec = _mm_or_si128(vec, vec2);
	}

	template <int pixelsPerVector, int count>
	__forceinline typename std::enable_if<0 != count>::type loadSrcVectorRemainder(__m128i& vec,
		const WORD* src, int& offset, int delta, std::integral_constant<int, count>)
	{
		vec = _mm_insert_epi16(vec, *(src + (offset >> 16)), pixelsPerVector - count);
		offset += delta;
		loadSrcVectorRemainder<pixelsPerVector>(vec, src, offset, delta, std::integral_constant<int, count - 1>());
	}

	template <int pixelsPerVector>
	__forceinline void loadSrcVectorRemainder(__m128i& /*vec*/,
		const WORD* /*src*/, int& /*offset*/, int /*delta*/, std::integral_constant<int, 0> /*count*/)
	{
	}

	template <int pixelsPerVector, int count>
	__forceinline typename std::enable_if<0 != count>::type loadSrcVectorRemainder(__m128i& vec,
		const DWORD* src, int& offset, int delta, std::integral_constant<int, count>)
	{
		__m128i pixel = _mm_loadu_si32(src + (offset >> 16));
		pixel = _mm_slli_si128(pixel, (pixelsPerVector - count) * 4);
		vec = _mm_or_si128(vec, pixel);
		offset += delta;
		loadSrcVectorRemainder<pixelsPerVector>(vec, src, offset, delta, std::integral_constant<int, count - 1>());
	}

	template <int pixelsPerVector>
	__forceinline void loadSrcVectorRemainder(__m128i& /*vec*/,
		const DWORD* /*src*/, int& /*offset*/, int /*delta*/, std::integral_constant<int, 0> /*count*/)
	{
	}

	template <int vectorSize, bool stretch, bool mirror, typename Pixel>
	__forceinline __m128i loadSrcVector(const Pixel*& src, int& offset, int delta)
	{
		const int pixelsPerVector = vectorSize / sizeof(Pixel);
		__m128i vec = _mm_loadu_si<sizeof(Pixel) * 8>(stretch ? src + (offset >> 16) : src);
		if (stretch)
		{
			offset += delta;
			loadSrcVectorRemainder<pixelsPerVector>(vec, src, offset, delta,
				std::integral_constant<int, pixelsPerVector - 1>());
		}
		else
		{
			vec = _mm_loadu_si<vectorSize * 8>(src);
			if (mirror)
			{
				vec = reverseVector<Pixel, vectorSize>(vec);
				src -= pixelsPerVector;
			}
			else
			{
				src += pixelsPerVector;
			}
		}
		return vec;
	}

	template <typename Pixel>
	__forceinline __m128i compareColorKey(__m128i vec, DWORD colorKey)
	{
		__m128i colorKeyVec = _mm_set1_epi<sizeof(Pixel) * 8>(colorKey);
		if (4 == sizeof(Pixel))
		{
			__m128i colorKeyMask = _mm_set1_epi<sizeof(Pixel) * 8>(0x00FFFFFF);
			vec = _mm_and_si128(vec, colorKeyMask);
		}
		return _mm_cmpeq_epi<sizeof(Pixel) * 8>(vec, colorKeyVec);
	}

	template <typename Pixel, bool mirror, bool useDstColorKey, bool useSrcColorKey>
	__forceinline __m128i bltVector(__m128i dst, __m128i src, DWORD dstColorKey, DWORD srcColorKey)
	{
		if (useDstColorKey && useSrcColorKey)
		{
			__m128i maskDst = compareColorKey<Pixel>(dst, dstColorKey);
			__m128i maskSrc = compareColorKey<Pixel>(src, srcColorKey);
			__m128i mask = _mm_andnot_si128(maskSrc, maskDst);
			dst = _mm_andnot_si128(mask, dst);
			src = _mm_and_si128(mask, src);
			return _mm_or_si128(dst, src);
		}
		else if (useDstColorKey)
		{
			__m128i mask = compareColorKey<Pixel>(dst, dstColorKey);
			dst = _mm_andnot_si128(mask, dst);
			src = _mm_and_si128(mask, src);
			return _mm_or_si128(dst, src);
		}
		else if (useSrcColorKey)
		{
			__m128i mask = compareColorKey<Pixel>(src, srcColorKey);
			dst = _mm_and_si128(mask, dst);
			src = _mm_andnot_si128(mask, src);
			return _mm_or_si128(dst, src);
		}
		else
		{
			return src;
		}
	}

	template <typename Pixel, int vectorSize, bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey>
	__forceinline void bltVector(Pixel*& dst, const Pixel*& src, int& offset, int delta,
		DWORD dstColorKey, DWORD srcColorKey)
	{
		__m128i s = loadSrcVector<vectorSize, stretch, mirror>(src, offset, delta);
		__m128i d = _mm_loadu_si<vectorSize * 8>(dst);
		d = bltVector<Pixel, mirror, useDstColorKey, useSrcColorKey>(d, s, dstColorKey, srcColorKey);
		_mm_storeu_si<vectorSize * 8>(dst, d);
		dst += vectorSize / sizeof(Pixel);
	}

	template <int vectorSize, bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey, typename Pixel>
	__forceinline void bltVectorRow(Pixel* dst, const Pixel* src, DWORD width, int offset, int delta,
		DWORD dstColorKey, DWORD srcColorKey)
	{
		const int pixelsPerVector = vectorSize / sizeof(Pixel);

		if (16 == vectorSize)
		{
			for (DWORD i = width / pixelsPerVector - 1; i != 0; --i)
			{
				bltVector<Pixel, 16, stretch, mirror, useDstColorKey, useSrcColorKey>(
					dst, src, offset, delta, dstColorKey, srcColorKey);
			}
		}

		if (sizeof(Pixel) < vectorSize)
		{
			const DWORD remainder = width % pixelsPerVector;
			auto src1 = src;
			auto offset1 = offset;
			__m128i s1 = loadSrcVector<vectorSize, stretch, mirror>(src1, offset1, delta);
			if (stretch)
			{
				offset += remainder * delta;
			}
			else if (mirror)
			{
				src -= remainder;
			}
			else
			{
				src += remainder;
			}
			__m128i s2 = loadSrcVector<vectorSize, stretch, mirror>(src, offset, delta);
			__m128i d1 = _mm_loadu_si<vectorSize * 8>(dst);
			__m128i d2 = _mm_loadu_si<vectorSize * 8>(dst + remainder);
			d1 = bltVector<Pixel, mirror, useDstColorKey, useSrcColorKey>(d1, s1, dstColorKey, srcColorKey);
			_mm_storeu_si<vectorSize * 8>(dst, d1);
			d2 = bltVector<Pixel, mirror, useDstColorKey, useSrcColorKey>(d2, s2, dstColorKey, srcColorKey);
			_mm_storeu_si<vectorSize * 8>(dst + remainder, d2);
		}
		else
		{
			bltVector<Pixel, vectorSize, stretch, mirror, useDstColorKey, useSrcColorKey>(
				dst, src, offset, delta, dstColorKey, srcColorKey);
		}
	}

	template <bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey>
	__forceinline void bltPixel(UInt24*& dst, const UInt24*& src, int& offset, int delta,
		DWORD dstColorKey, DWORD srcColorKey)
	{
		const UInt24* src1 = stretch ? src + (offset >> 16) : src;
		if (useDstColorKey || useSrcColorKey)
		{
			const DWORD d = *reinterpret_cast<const WORD*>(dst) | (reinterpret_cast<const BYTE*>(dst)[2] << 16);
			const DWORD s = *reinterpret_cast<const WORD*>(src1) | (reinterpret_cast<const BYTE*>(src1)[2] << 16);
			const DWORD mask = static_cast<DWORD>(-static_cast<int>(
				(!useDstColorKey || dstColorKey == d) &&
				(!useSrcColorKey || srcColorKey != s)));
			*dst = (d & ~mask) | (s & mask);
		}
		else
		{
			*dst = *src1;
		}

		++dst;
		if (stretch)
		{
			offset += delta;
		}
		else
		{
			src += mirror ? -1 : 1;
		}
	}

	template <int vectorSize, bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey>
	__forceinline void bltVectorRow(UInt24* dst, const UInt24* src, DWORD width, int offset, int delta,
		DWORD dstColorKey, DWORD srcColorKey)
	{
		if (!stretch && !mirror && !useDstColorKey && !useSrcColorKey)
		{
			bltVectorRow<vectorSize, stretch, mirror, useDstColorKey, useSrcColorKey, BYTE>(
				reinterpret_cast<BYTE*>(dst), reinterpret_cast<const BYTE*>(src),
				width * 3, offset, delta, dstColorKey, srcColorKey);
			return;
		}

		if (2 == vectorSize)
		{
			bltPixel<stretch, mirror, useDstColorKey, useSrcColorKey>(dst, src, offset, delta, dstColorKey, srcColorKey);
			return;
		}

		if (4 == vectorSize)
		{
			bltPixel<stretch, mirror, useDstColorKey, useSrcColorKey>(dst, src, offset, delta, dstColorKey, srcColorKey);
			bltPixel<stretch, mirror, useDstColorKey, useSrcColorKey>(dst, src, offset, delta, dstColorKey, srcColorKey);
			return;
		}

		for (DWORD i = width; i != 0; --i)
		{
			bltPixel<stretch, mirror, useDstColorKey, useSrcColorKey>(dst, src, offset, delta, dstColorKey, srcColorKey);
		}
	}

	template <typename Pixel, int vectorSize, bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey>
	__forceinline std::enable_if_t<vectorSize >= (int)sizeof(Pixel) || (2 == vectorSize && 3 == sizeof(Pixel))> vectorizedBlt(
		BYTE * dst, DWORD dstPitch, DWORD dstWidth, DWORD dstHeight,
		const BYTE * src, DWORD srcPitch, int offsetX, int deltaX, int offsetY, int deltaY,
		DWORD dstColorKey, DWORD srcColorKey)
	{
		if (3 != sizeof(Pixel) && !stretch && mirror)
		{
			src -= vectorSize - sizeof(Pixel);
		}

		for (DWORD i = dstHeight; i != 0; --i)
		{
			bltVectorRow<vectorSize, stretch, mirror, useDstColorKey, useSrcColorKey>(
				reinterpret_cast<Pixel*>(dst),
				reinterpret_cast<const Pixel*>(src + (offsetY >> 16) * srcPitch),
				dstWidth, offsetX, deltaX, dstColorKey, srcColorKey);
			dst += dstPitch;
			offsetY += deltaY;
		}
	}

	template <typename Pixel, int vectorSize, bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey>
	__forceinline std::enable_if_t < vectorSize < (int)sizeof(Pixel) && (2 != vectorSize || 3 != sizeof(Pixel))> vectorizedBlt(
		BYTE* /*dst*/, DWORD /*dstPitch*/, DWORD /*dstWidth*/, DWORD /*dstHeight*/,
		const BYTE* /*src*/, DWORD /*srcPitch*/, int /*offsetX*/, int /*deltaX*/, int /*offsetY*/, int /*deltaY*/,
		const DWORD /*dstColorKey*/, const DWORD /*srcColorKey*/)
	{
	}

	template <typename Pixel, int vectorSize, bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey>
	void vectorizedBltFunc(void* dst, DWORD dstPitch, DWORD dstWidth, DWORD dstHeight,
		const void* src, DWORD srcPitch, int offsetX, int deltaX, int offsetY, int deltaY,
		DWORD dstColorKey, DWORD srcColorKey)
	{
		vectorizedBlt<Pixel, vectorSize, stretch, mirror, useDstColorKey, useSrcColorKey>(
			static_cast<BYTE*>(dst), dstPitch, dstWidth, dstHeight,
			static_cast<const BYTE*>(src), srcPitch, offsetX, deltaX, offsetY, deltaY, dstColorKey, srcColorKey);
	}

	template <typename Pixel, int vectorSize, bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey>
	auto getVectorizedBltFunc()
	{
		return &vectorizedBltFunc<Pixel, vectorSize, stretch, mirror, useDstColorKey, useSrcColorKey>;
	}

	template <typename Pixel, int vectorSize, bool stretch, bool mirror, bool useDstColorKey>
	auto getVectorizedBltFunc(bool useSrcColorKey)
	{
		return useSrcColorKey
			? getVectorizedBltFunc<Pixel, vectorSize, stretch, mirror, useDstColorKey, true>()
			: getVectorizedBltFunc<Pixel, vectorSize, stretch, mirror, useDstColorKey, false>();
	}

	template <typename Pixel, int vectorSize, bool stretch, bool mirror>
	auto getVectorizedBltFunc(bool useDstColorKey, bool useSrcColorKey)
	{
		return useDstColorKey
			? getVectorizedBltFunc<Pixel, vectorSize, stretch, mirror, true>(useSrcColorKey)
			: getVectorizedBltFunc<Pixel, vectorSize, stretch, mirror, false>(useSrcColorKey);
	}

	template <typename Pixel, int vectorSize, bool stretch>
	auto getVectorizedBltFunc(bool mirror, bool useDstColorKey, bool useSrcColorKey)
	{
		return mirror
			? getVectorizedBltFunc<Pixel, vectorSize, stretch, true>(useDstColorKey, useSrcColorKey)
			: getVectorizedBltFunc<Pixel, vectorSize, stretch, false>(useDstColorKey, useSrcColorKey);
	}

	template <typename Pixel, int vectorSize>
	auto getVectorizedBltFunc(bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey)
	{
		return stretch
			? getVectorizedBltFunc<Pixel, vectorSize, true>(mirror, useDstColorKey, useSrcColorKey)
			: getVectorizedBltFunc<Pixel, vectorSize, false>(mirror, useDstColorKey, useSrcColorKey);
	}

	template <typename Pixel>
	auto getVectorizedBltFunc(DWORD width, bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey)
	{
		if (width >= 16) return getVectorizedBltFunc<Pixel, 16>(stretch, mirror, useDstColorKey, useSrcColorKey);
		if (width >= 8) return getVectorizedBltFunc<Pixel, 8>(stretch, mirror, useDstColorKey, useSrcColorKey);
		if (width >= 4) return getVectorizedBltFunc<Pixel, 4>(stretch, mirror, useDstColorKey, useSrcColorKey);
		if (width >= 2) return getVectorizedBltFunc<Pixel, 2>(stretch, mirror, useDstColorKey, useSrcColorKey);
		return getVectorizedBltFunc<Pixel, 1>(stretch, mirror, useDstColorKey, useSrcColorKey);
	}

	auto getVectorizedBltFunc(DWORD bytesPerPixel, DWORD width,
		bool stretch, bool mirror, bool useDstColorKey, bool useSrcColorKey)
	{
		switch (bytesPerPixel)
		{
		case 4: return getVectorizedBltFunc<DWORD>(width, stretch, mirror, useDstColorKey, useSrcColorKey);
		case 3: return getVectorizedBltFunc<UInt24>(width, stretch, mirror, useDstColorKey, useSrcColorKey);
		case 2: return getVectorizedBltFunc<WORD>(width, stretch, mirror, useDstColorKey, useSrcColorKey);
		default: return getVectorizedBltFunc<BYTE>(width, stretch, mirror, useDstColorKey, useSrcColorKey);
		}
	}

	auto getVectorizedBltFuncs()
	{
		typename MultiDimArray<decltype(&vectorizedBltFunc<BYTE, 1, false, false, false, false>), 4, 5, 2, 2, 2, 2>::type vectorizedBltFuncs;
		for (int bytesPerPixel = 1; bytesPerPixel <= 4; ++bytesPerPixel)
		{
			for (int width = 0; width <= 4; ++width)
			{
				for (int stretch = 0; stretch <= 1; ++stretch)
				{
					for (int mirror = 0; mirror <= 1; ++mirror)
					{
						for (int useDstColorKey = 0; useDstColorKey <= 1; ++useDstColorKey)
						{
							for (int useSrcColorKey = 0; useSrcColorKey <= 1; ++useSrcColorKey)
							{
								vectorizedBltFuncs[bytesPerPixel - 1][width][stretch][mirror][useDstColorKey][useSrcColorKey] =
									getVectorizedBltFunc(bytesPerPixel, static_cast<DWORD>(pow(2, width)),
										stretch, mirror, useDstColorKey, useSrcColorKey);
							}
						}
					}
				}
			}
		}
		return vectorizedBltFuncs;
	}

	const auto g_vectorizedBltFuncs(getVectorizedBltFuncs());

	bool doOverlappingBlt(BYTE* dst, DWORD pitch, DWORD dstWidth, DWORD dstHeight,
		const BYTE* src, LONG srcWidth, LONG srcHeight,
		DWORD bytesPerPixel, const DWORD* dstColorKey, const DWORD* srcColorKey)
	{
		const bool mirrorLeftRight = srcWidth < 0;
		const bool mirrorUpDown = srcHeight < 0;

		const DWORD absSrcWidth = mirrorLeftRight ? -srcWidth : srcWidth;
		const DWORD absSrcHeight = mirrorUpDown ? -srcHeight : srcHeight;

		RECT dstRect = { 0, 0, static_cast<LONG>(dstWidth * bytesPerPixel), static_cast<LONG>(dstHeight) };
		RECT srcRect = { 0, 0, static_cast<LONG>(absSrcWidth * bytesPerPixel), static_cast<LONG>(absSrcHeight) };

		srcRect.top = (src - dst) / static_cast<LONG>(pitch);
		srcRect.left = (src - dst) % static_cast<LONG>(pitch);
		srcRect.bottom += srcRect.top;
		srcRect.right += srcRect.left;

		LONG rowError = 0;
		if (src < dst)
		{
			rowError = (dstRect.right - srcRect.left > static_cast<LONG>(pitch)) ? 1 : 0;
		}
		else
		{
			rowError = (srcRect.right - dstRect.left > static_cast<LONG>(pitch)) ? -1 : 0;
		}
		srcRect.left += rowError * pitch;
		srcRect.right += rowError * pitch;
		srcRect.top -= rowError;
		srcRect.bottom -= rowError;

		RECT r = {};
		if (!IntersectRect(&r, &dstRect, &srcRect))
		{
			return false;
		}

		if (!mirrorLeftRight && !mirrorUpDown)
		{
			if (EqualRect(&dstRect, &srcRect))
			{
				return true;
			}

			if (dstWidth == absSrcWidth && dstHeight == absSrcHeight && !dstColorKey && !srcColorKey)
			{
				if (dst < src)
				{
					for (DWORD y = dstHeight; y != 0; --y)
					{
						std::memmove(dst, src, dstWidth * bytesPerPixel);
						dst += pitch;
						src += pitch;
					}
				}
				else
				{
					dst += (dstHeight - 1) * pitch;
					src += (srcHeight - 1) * pitch;
					for (DWORD y = dstHeight; y != 0; --y)
					{
						std::memmove(dst, src, dstWidth * bytesPerPixel);
						dst -= pitch;
						src -= pitch;
					}
				}
				return true;
			}
		}

		Compat31::ScopedCriticalSection lock(g_overlappingBltCs);
		static std::vector<BYTE> tmpSurface;
		const LONG srcByteWidth = absSrcWidth * bytesPerPixel;
		if (tmpSurface.size() < absSrcHeight * srcByteWidth)
		{
			tmpSurface.resize(absSrcHeight * srcByteWidth);
		}
		BYTE* tmp = tmpSurface.data();

		auto vectorizedBltFunc = g_vectorizedBltFuncs[0]
			[(srcByteWidth >= 2) + (srcByteWidth >= 4) + (srcByteWidth >= 8) + (srcByteWidth >= 16)][0][0][0][0];

		vectorizedBltFunc(tmp, srcByteWidth, srcByteWidth, absSrcHeight,
			src, pitch, 0x8000, 0x10000, 0x8000, 0x10000, 0, 0);

		blt(dst, pitch, dstWidth, dstHeight,
			tmp, srcByteWidth, srcWidth, srcHeight,
			bytesPerPixel, dstColorKey, srcColorKey);

		return true;
	}

	void blt(BYTE* dst, DWORD dstPitch, DWORD dstWidth, DWORD dstHeight,
		const BYTE* src, DWORD srcPitch, LONG srcWidth, LONG srcHeight,
		DWORD bytesPerPixel, const DWORD* dstColorKey, const DWORD* srcColorKey)
	{
		const bool mirrorLeftRight = srcWidth < 0;
		const bool mirrorUpDown = srcHeight < 0;
		const DWORD absSrcWidth = mirrorLeftRight ? -srcWidth : srcWidth;
		const DWORD absSrcHeight = mirrorUpDown ? -srcHeight : srcHeight;

		if (dstPitch == srcPitch)
		{
			const BYTE* dstEnd = dst + (dstHeight - 1) * dstPitch + dstWidth * bytesPerPixel;
			const BYTE* srcEnd = src + (absSrcHeight - 1) * srcPitch + absSrcWidth * bytesPerPixel;

			if (dst < src ? dstEnd > src : srcEnd > dst)
			{
				if (doOverlappingBlt(dst, dstPitch, dstWidth, dstHeight,
					src, srcWidth, srcHeight, bytesPerPixel, dstColorKey, srcColorKey))
				{
					return;
				}
			}
		}

		int deltaX = (absSrcWidth << 16) / dstWidth;
		int deltaY = (absSrcHeight << 16) / dstHeight;

		int offsetX = deltaX / 2;
		int offsetY = deltaY / 2;

		if (mirrorLeftRight)
		{
			offsetX += static_cast<int>(dstWidth - 1) * deltaX;
			deltaX = -deltaX;
		}
		if (mirrorUpDown)
		{
			offsetY += static_cast<int>(dstHeight - 1) * deltaY;
			deltaY = -deltaY;
		}

		src += (offsetY >> 16) * srcPitch + (offsetX >> 16) * bytesPerPixel;
		offsetX &= 0x0000FFFF;
		offsetY &= 0x0000FFFF;

		const DWORD dstCk = dstColorKey ? *dstColorKey & 0x00FFFFFF : 0;
		const DWORD srcCk = srcColorKey ? *srcColorKey & 0x00FFFFFF : 0;
		const DWORD dstByteWidth = dstWidth * bytesPerPixel;

		auto vectorizedBltFunc = g_vectorizedBltFuncs
			[bytesPerPixel - 1]
		[(dstByteWidth >= 2) + (dstByteWidth >= 4) + (dstByteWidth >= 8) + (dstByteWidth >= 16)]
		[dstWidth != absSrcWidth]
		[mirrorLeftRight]
		[nullptr != dstColorKey]
		[nullptr != srcColorKey];

		vectorizedBltFunc(dst, dstPitch, dstWidth, dstHeight,
			src, srcPitch, offsetX, deltaX, offsetY, deltaY, dstCk, srcCk);
	}

	template <typename Pixel>
	void colorFill(BYTE* dst, DWORD dstPitch, DWORD dstWidth, DWORD dstHeight, DWORD color)
	{
		DWORD c = 0;
		memset(&c, color, sizeof(Pixel));
		if (c == color)
		{
			for (DWORD i = dstHeight; i != 0; --i)
			{
				memset(dst, color, dstWidth * sizeof(Pixel));
				dst += dstPitch;
			}
			return;
		}

		for (DWORD i = 0; i < dstWidth; ++i)
		{
			reinterpret_cast<Pixel*>(dst)[i] = static_cast<Pixel>(color);
		}

		for (DWORD i = dstHeight - 1; i != 0; --i)
		{
			memcpy(dst + dstPitch, dst, dstWidth * sizeof(Pixel));
			dst += dstPitch;
		}
	}
}

namespace DDraw
{
	namespace Blitter
	{
		void blt(void* dst, DWORD dstPitch, DWORD dstWidth, DWORD dstHeight,
			const void* src, DWORD srcPitch, LONG srcWidth, LONG srcHeight,
			DWORD bytesPerPixel, const DWORD* dstColorKey, const DWORD* srcColorKey)
		{
			::blt(static_cast<BYTE*>(dst), dstPitch, dstWidth, dstHeight,
				static_cast<const BYTE*>(src), srcPitch, srcWidth, srcHeight,
				bytesPerPixel, dstColorKey, srcColorKey);
		}

		void colorFill(void* dst, DWORD dstPitch, DWORD dstWidth, DWORD dstHeight, DWORD bytesPerPixel, DWORD color)
		{
			switch (bytesPerPixel)
			{
			case 1: return ::colorFill<BYTE>(static_cast<BYTE*>(dst), dstPitch, dstWidth, dstHeight, color);
			case 2: return ::colorFill<WORD>(static_cast<BYTE*>(dst), dstPitch, dstWidth, dstHeight, color);
			case 3: return ::colorFill<UInt24>(static_cast<BYTE*>(dst), dstPitch, dstWidth, dstHeight, color);
			case 4: return ::colorFill<DWORD>(static_cast<BYTE*>(dst), dstPitch, dstWidth, dstHeight, color);
			}
		}
	}
}
