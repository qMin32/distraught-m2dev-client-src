#pragma once
#include <string>
#include <cstring>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <utility>

#include <EterLocale/Arabic.h>

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

// Maximum text length for security/performance (prevent DoS attacks)
constexpr size_t MAX_TEXT_LENGTH = 65536; // 64KB of text
constexpr size_t MAX_CHAT_TEXT_LENGTH = 4096; // 4KB for chat messages

// Arabic shaping buffer size calculations
constexpr size_t ARABIC_SHAPING_EXPANSION_FACTOR = 2;
constexpr size_t ARABIC_SHAPING_SAFETY_MARGIN = 16;
constexpr size_t ARABIC_SHAPING_EXPANSION_FACTOR_RETRY = 4;
constexpr size_t ARABIC_SHAPING_SAFETY_MARGIN_RETRY = 64;

// ============================================================================
// DEBUG LOGGING (Only enabled in Debug builds)
// ============================================================================
#ifdef _DEBUG
	#define DEBUG_BIDI  // Enabled in debug builds for diagnostics
#endif

#ifdef DEBUG_BIDI
	#include <cstdio>
	#define BIDI_LOG(fmt, ...) printf("[BiDi] " fmt "\n", __VA_ARGS__)
	#define BIDI_LOG_SIMPLE(msg) printf("[BiDi] %s\n", msg)
#else
	#define BIDI_LOG(fmt, ...) ((void)0)
	#define BIDI_LOG_SIMPLE(msg) ((void)0)
#endif

// ============================================================================
// OPTIMIZED CHARACTER CLASSIFICATION (Lookup Tables)
// ============================================================================
// Replaces expensive GetStringTypeW() syscalls with O(1) table lookups.
// Tables are initialized once on first use (thread-safe via static init).

namespace BiDiTables
{
	// Character property flags
	enum ECharFlags : uint8_t
	{
		CF_NONE   = 0,
		CF_ALPHA  = 0x01,  // Alphabetic (Latin, Cyrillic, Greek, etc.)
		CF_DIGIT  = 0x02,  // Numeric digit (0-9, Arabic-Indic, etc.)
		CF_RTL    = 0x04,  // RTL script (Arabic, Hebrew)
		CF_ARABIC = 0x08,  // Arabic letter that needs shaping
	};

	// Main character flags table (65536 entries for BMP)
	inline const uint8_t* GetCharFlagsTable()
	{
		static uint8_t s_table[65536] = {0};
		static bool s_initialized = false;

		if (!s_initialized)
		{
			// ASCII digits
			for (int i = '0'; i <= '9'; ++i)
				s_table[i] |= CF_DIGIT;

			// ASCII letters
			for (int i = 'A'; i <= 'Z'; ++i)
				s_table[i] |= CF_ALPHA;
			for (int i = 'a'; i <= 'z'; ++i)
				s_table[i] |= CF_ALPHA;

			// Latin Extended-A/B (0x0100-0x024F)
			for (int i = 0x0100; i <= 0x024F; ++i)
				s_table[i] |= CF_ALPHA;

			// Latin Extended Additional (0x1E00-0x1EFF)
			for (int i = 0x1E00; i <= 0x1EFF; ++i)
				s_table[i] |= CF_ALPHA;

			// Greek (0x0370-0x03FF)
			for (int i = 0x0370; i <= 0x03FF; ++i)
				s_table[i] |= CF_ALPHA;

			// Cyrillic (0x0400-0x04FF)
			for (int i = 0x0400; i <= 0x04FF; ++i)
				s_table[i] |= CF_ALPHA;

			// Hebrew (0x0590-0x05FF) - RTL
			for (int i = 0x0590; i <= 0x05FF; ++i)
				s_table[i] |= CF_RTL | CF_ALPHA;

			// Arabic (0x0600-0x06FF) - RTL + needs shaping
			for (int i = 0x0600; i <= 0x06FF; ++i)
				s_table[i] |= CF_RTL | CF_ALPHA;
			// Arabic letters that need shaping (0x0621-0x064A)
			for (int i = 0x0621; i <= 0x064A; ++i)
				s_table[i] |= CF_ARABIC;

			// Arabic Supplement (0x0750-0x077F)
			for (int i = 0x0750; i <= 0x077F; ++i)
				s_table[i] |= CF_RTL | CF_ALPHA;

			// Arabic Extended-A (0x08A0-0x08FF)
			for (int i = 0x08A0; i <= 0x08FF; ++i)
				s_table[i] |= CF_RTL | CF_ALPHA;

			// Arabic-Indic digits (0x0660-0x0669)
			for (int i = 0x0660; i <= 0x0669; ++i)
				s_table[i] |= CF_DIGIT;

			// Extended Arabic-Indic digits (0x06F0-0x06F9)
			for (int i = 0x06F0; i <= 0x06F9; ++i)
				s_table[i] |= CF_DIGIT;

			// Arabic Presentation Forms-A (0xFB50-0xFDFF) - already shaped
			for (int i = 0xFB50; i <= 0xFDFF; ++i)
				s_table[i] |= CF_RTL | CF_ALPHA;

			// Arabic Presentation Forms-B (0xFE70-0xFEFF) - already shaped
			for (int i = 0xFE70; i <= 0xFEFF; ++i)
				s_table[i] |= CF_RTL | CF_ALPHA;

			// Hebrew presentation forms (0xFB1D-0xFB4F)
			for (int i = 0xFB1D; i <= 0xFB4F; ++i)
				s_table[i] |= CF_RTL | CF_ALPHA;

			// CJK (0x4E00-0x9FFF) - treat as LTR alpha
			for (int i = 0x4E00; i <= 0x9FFF; ++i)
				s_table[i] |= CF_ALPHA;

			// Hangul (0xAC00-0xD7AF)
			for (int i = 0xAC00; i <= 0xD7AF; ++i)
				s_table[i] |= CF_ALPHA;

			// RTL marks and controls
			s_table[0x200F] |= CF_RTL; // RLM
			s_table[0x061C] |= CF_RTL; // ALM
			for (int i = 0x202B; i <= 0x202E; ++i)
				s_table[i] |= CF_RTL; // RLE/RLO/PDF/LRE/LRO
			for (int i = 0x2066; i <= 0x2069; ++i)
				s_table[i] |= CF_RTL; // Isolates

			s_initialized = true;
		}

		return s_table;
	}

	// Fast O(1) character classification functions
	inline bool IsRTL(wchar_t ch) { return GetCharFlagsTable()[(uint16_t)ch] & CF_RTL; }
	inline bool IsAlpha(wchar_t ch) { return GetCharFlagsTable()[(uint16_t)ch] & CF_ALPHA; }
	inline bool IsDigit(wchar_t ch) { return GetCharFlagsTable()[(uint16_t)ch] & CF_DIGIT; }
	inline bool IsArabicLetter(wchar_t ch) { return GetCharFlagsTable()[(uint16_t)ch] & CF_ARABIC; }
	inline bool IsStrongLTR(wchar_t ch)
	{
		uint8_t flags = GetCharFlagsTable()[(uint16_t)ch];
		// Strong LTR = (Alpha OR Digit) AND NOT RTL
		return (flags & (CF_ALPHA | CF_DIGIT)) && !(flags & CF_RTL);
	}
}

// ============================================================================
// BUFFER POOLING (Avoid per-call allocations)
// ============================================================================

namespace BiDiBuffers
{
	struct TBufferPool
	{
		std::vector<wchar_t> shaped;

		void EnsureCapacity(size_t n)
		{
			size_t needed = n * 2 + 64;
			if (shaped.capacity() < needed) shaped.reserve(needed);
		}

		void Clear()
		{
			shaped.clear();
		}
	};

	inline TBufferPool& Get()
	{
		thread_local static TBufferPool s_pool;
		return s_pool;
	}
}

// ============================================================================
// UNICODE VALIDATION HELPERS
// ============================================================================

// Check if codepoint is a valid Unicode scalar value (not surrogate, not non-character)
static inline bool IsValidUnicodeScalar(wchar_t ch)
{
	// Reject surrogate pairs (UTF-16 encoding artifacts, invalid in UTF-8)
	if (ch >= 0xD800 && ch <= 0xDFFF)
		return false;

	// Reject non-characters (reserved by Unicode standard)
	if ((ch >= 0xFDD0 && ch <= 0xFDEF) || // Arabic Presentation Forms non-chars
	    (ch & 0xFFFE) == 0xFFFE)           // U+FFFE, U+FFFF, etc.
		return false;

	// Accept everything else in BMP (0x0000-0xFFFF)
	return true;
}

// Sanitize a wide string by removing invalid Unicode codepoints
static inline void SanitizeWideString(std::wstring& ws)
{
	ws.erase(std::remove_if(ws.begin(), ws.end(),
		[](wchar_t ch) { return !IsValidUnicodeScalar(ch); }),
		ws.end());
}

// ============================================================================
// OPTIMIZED UTF-8 CONVERSION
// ============================================================================
// Fast paths for ASCII-only text (very common in games).
// Falls back to Windows API for non-ASCII.

namespace Utf8Fast
{
	// Check if string is pure ASCII (no bytes >= 128)
	inline bool IsAsciiOnly(const char* s, size_t len)
	{
		// Process 8 bytes at a time for speed
		const char* end = s + len;
		const char* aligned_end = s + (len & ~7);

		while (s < aligned_end)
		{
			// Check 8 bytes at once using bitwise OR
			uint64_t chunk;
			memcpy(&chunk, s, 8);
			if (chunk & 0x8080808080808080ULL)
				return false;
			s += 8;
		}

		// Check remaining bytes
		while (s < end)
		{
			if ((unsigned char)*s >= 128)
				return false;
			++s;
		}
		return true;
	}

	// Fast ASCII-only conversion (no API calls)
	inline std::wstring AsciiToWide(const char* s, size_t len)
	{
		std::wstring out;
		out.reserve(len);
		for (size_t i = 0; i < len; ++i)
			out.push_back(static_cast<wchar_t>(static_cast<unsigned char>(s[i])));
		return out;
	}

	// Fast ASCII-only conversion (no API calls)
	inline std::string WideToAscii(const wchar_t* ws, size_t len)
	{
		std::string out;
		out.reserve(len);
		for (size_t i = 0; i < len; ++i)
		{
			wchar_t ch = ws[i];
			if (ch < 128)
				out.push_back(static_cast<char>(ch));
			else
				return ""; // Not pure ASCII, caller should use full conversion
		}
		return out;
	}
}

// UTF-8 -> UTF-16 (Windows wide)
// OPTIMIZED: Fast path for ASCII-only strings (avoids 2x API calls)
inline std::wstring Utf8ToWide(const std::string& s)
{
	if (s.empty())
		return L"";

	// Validate size limits (prevent DoS and INT_MAX overflow)
	if (s.size() > MAX_TEXT_LENGTH || s.size() > INT_MAX)
	{
		BIDI_LOG("Utf8ToWide: String too large (%zu bytes)", s.size());
		return L"";
	}

	// Fast path: ASCII-only strings (very common in games)
	if (Utf8Fast::IsAsciiOnly(s.data(), s.size()))
		return Utf8Fast::AsciiToWide(s.data(), s.size());

	// Slow path: Use Windows API for non-ASCII
	int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), nullptr, 0);
	if (wlen <= 0)
	{
		BIDI_LOG("Utf8ToWide: Invalid UTF-8 sequence (error %d)", GetLastError());
		return L"";
	}

	std::wstring out(wlen, L'\0');
	int written = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), out.data(), wlen);
	if (written <= 0 || written != wlen)
	{
		BIDI_LOG("Utf8ToWide: Second conversion failed (written=%d, expected=%d, error=%d)", written, wlen, GetLastError());
		return L"";
	}

	return out;
}

// Convenience overload for char*
// OPTIMIZED: Fast path for ASCII-only strings
inline std::wstring Utf8ToWide(const char* s)
{
	if (!s || !*s)
		return L"";

	size_t len = strlen(s);

	// Fast path: ASCII-only strings
	if (Utf8Fast::IsAsciiOnly(s, len))
		return Utf8Fast::AsciiToWide(s, len);

	// Slow path: Use Windows API
	int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, nullptr, 0);
	if (wlen <= 0)
		return L"";

	std::wstring out(wlen, L'\0');
	int written = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, out.data(), wlen);
	if (written <= 0 || written != wlen)
	{
		BIDI_LOG("Utf8ToWide(char*): Conversion failed (written=%d, expected=%d, error=%d)", written, wlen, GetLastError());
		return L"";
	}

	// Drop the terminating NUL from std::wstring length
	if (!out.empty() && out.back() == L'\0')
		out.pop_back();

	return out;
}

// UTF-16 (Windows wide) -> UTF-8
// OPTIMIZED: Fast path for ASCII-only strings
inline std::string WideToUtf8(const std::wstring& ws)
{
	if (ws.empty())
		return "";

	// Validate size limits (prevent DoS and INT_MAX overflow)
	if (ws.size() > MAX_TEXT_LENGTH || ws.size() > INT_MAX)
		return "";

	// Fast path: Check if all characters are ASCII
	bool isAscii = true;
	for (size_t i = 0; i < ws.size() && isAscii; ++i)
		isAscii = (ws[i] < 128);

	if (isAscii)
	{
		std::string out;
		out.reserve(ws.size());
		for (size_t i = 0; i < ws.size(); ++i)
			out.push_back(static_cast<char>(ws[i]));
		return out;
	}

	// Slow path: Use Windows API for non-ASCII
	int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
	if (len <= 0)
		return "";

	std::string out(len, '\0');
	int written = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, ws.data(), (int)ws.size(), out.data(), len, nullptr, nullptr);
	if (written <= 0 || written != len)
	{
		BIDI_LOG("WideToUtf8: Conversion failed (written=%d, expected=%d, error=%d)", written, len, GetLastError());
		return "";
	}
	return out;
}

// Convenience overload for wchar_t*
inline std::string WideToUtf8(const wchar_t* ws)
{
	if (!ws)
		return "";
	return WideToUtf8(std::wstring(ws));
}

// ============================================================================
// RTL & BiDi formatting for RTL UI
// ============================================================================

enum class EBidiDir { LTR, RTL };
enum class ECharDir : unsigned char { Neutral, LTR, RTL };

// Optimized: O(1) lookup table instead of GetStringTypeW() syscalls
static inline bool IsRTLCodepoint(wchar_t ch)
{
	return BiDiTables::IsRTL(ch);
}

// Optimized: O(1) lookup table instead of GetStringTypeW() syscalls
static inline bool IsStrongAlpha(wchar_t ch)
{
	return BiDiTables::IsAlpha(ch);
}

// Optimized: O(1) lookup table instead of GetStringTypeW() syscalls
static inline bool IsDigit(wchar_t ch)
{
	return BiDiTables::IsDigit(ch);
}

static inline bool IsNameTokenPunct(wchar_t ch)
{
	switch (ch)
	{
		case L'#':
		case L'@':
		case L'$':
		case L'%':
		case L'&':
		case L'*':
		case L'+':
		case L'-':
		case L'_':
		case L'=':
		case L'.':
		case L',':
		case L'/':
		case L'\\':
		case L'(':
		case L')':
		// Brackets are handled specially - see GetCharDirSmart
		// case L'[':
		// case L']':
		case L'{':
		case L'}':
		case L'<':
		case L'>':
			return true;
		default:
			return false;
	}
}

// Optimized: O(1) lookup - Check RTL first to avoid classifying Arabic as LTR
static inline bool IsStrongLTR(wchar_t ch)
{
	return BiDiTables::IsStrongLTR(ch);
}

static inline bool HasStrongLTRNeighbor(const wchar_t* s, int n, int i)
{
	// Skip neutral characters (spaces, punctuation) to find nearest strong character
	// This fixes mixed-direction text like "english + arabic"

	// Search backwards for strong character (skip neutrals/whitespace)
	for (int j = i - 1; j >= 0; --j)
	{
		wchar_t ch = s[j];

		// Skip spaces and common neutral punctuation
		if (ch == L' ' || ch == L'\t' || ch == L'\n')
			continue;

		// Found strong LTR
		if (IsStrongLTR(ch))
			return true;

		// Found strong RTL or other strong character
		if (IsRTLCodepoint(ch) || IsStrongAlpha(ch))
			break;
	}

	// Search forwards for strong character (skip neutrals/whitespace)
	for (int j = i + 1; j < n; ++j)
	{
		wchar_t ch = s[j];

		// Skip spaces and common neutral punctuation
		if (ch == L' ' || ch == L'\t' || ch == L'\n')
			continue;

		// Found strong LTR
		if (IsStrongLTR(ch))
			return true;

		// Found strong RTL or other strong character
		if (IsRTLCodepoint(ch) || IsStrongAlpha(ch))
			break;
	}

	return false;
}

static inline ECharDir GetCharDir(wchar_t ch)
{
	if (IsRTLCodepoint(ch))
		return ECharDir::RTL;

	// Use IsStrongLTR which now correctly excludes RTL
	if (IsStrongLTR(ch))
		return ECharDir::LTR;

	return ECharDir::Neutral;
}

static inline ECharDir GetCharDirSmart(const wchar_t* s, int n, int i)
{
	wchar_t ch = s[i];

	// True RTL letters/marks
	if (IsRTLCodepoint(ch))
		return ECharDir::RTL;

	// True LTR letters/digits (now correctly excludes RTL)
	if (IsStrongLTR(ch))
		return ECharDir::LTR;

	// Parentheses: always LTR to keep them with their content
	if (ch == L'(' || ch == L')')
		return ECharDir::LTR;

	// Common punctuation: treat as strong LTR to prevent jumping around in mixed text
	// This makes "Hello + اختبار" and "اختبار + Hello" both keep punctuation in place
	if (ch == L'+' || ch == L'-' || ch == L'=' || ch == L'*' || ch == L'/' ||
	    ch == L'<' || ch == L'>' || ch == L'&' || ch == L'|' || ch == L'@' || ch == L'#')
		return ECharDir::LTR;

	// Percentage sign: attach to numbers (scan nearby for digits/minus/plus)
	// Handles: "%20", "20%", "-6%", "%d%%", etc.
	if (ch == L'%')
	{
		// Look backward for digit, %, -, or +
		for (int j = i - 1; j >= 0 && (i - j) < 5; --j)
		{
			wchar_t prev = s[j];
			if (IsDigit(prev) || prev == L'%' || prev == L'-' || prev == L'+')
				return ECharDir::LTR;
			if (prev != L' ' && prev != L'\t')
				break; // Stop if we hit non-numeric character
		}
		// Look forward for digit, %, -, or +
		for (int j = i + 1; j < n && (j - i) < 5; ++j)
		{
			wchar_t next = s[j];
			if (IsDigit(next) || next == L'%' || next == L'-' || next == L'+')
				return ECharDir::LTR;
			if (next != L' ' && next != L'\t')
				break; // Stop if we hit non-numeric character
		}
		return ECharDir::Neutral;
	}

	// Minus/dash: attach to numbers (scan nearby for digits/%)
	// Handles: "-6", "5-10", "-6%%", etc.
	if (ch == L'-')
	{
		// Look backward for digit or %
		for (int j = i - 1; j >= 0 && (i - j) < 3; --j)
		{
			wchar_t prev = s[j];
			if (IsDigit(prev) || prev == L'%')
				return ECharDir::LTR;
			if (prev != L' ' && prev != L'\t')
				break;
		}
		// Look forward for digit or %
		for (int j = i + 1; j < n && (j - i) < 3; ++j)
		{
			wchar_t next = s[j];
			if (IsDigit(next) || next == L'%')
				return ECharDir::LTR;
			if (next != L' ' && next != L'\t')
				break;
		}
		return ECharDir::Neutral;
	}

	// Colon: attach to preceding text direction
	// Look backward to find strong character
	if (ch == L':')
	{
		for (int j = i - 1; j >= 0; --j)
		{
			if (s[j] == L' ' || s[j] == L'\t')
				continue; // Skip spaces
			if (IsRTLCodepoint(s[j]))
				return ECharDir::RTL; // Attach to RTL text
			if (IsStrongLTR(s[j]))
				return ECharDir::LTR; // Attach to LTR text
		}
		return ECharDir::Neutral;
	}

	// Enhancement marker: '+' followed by digit(s) should attach to preceding text
	// If preceded by RTL, treat as RTL to keep "+9" with the item name
	// Otherwise treat as LTR
	if (ch == L'+' && i + 1 < n && IsDigit(s[i + 1]))
	{
		// Look backward for the last strong character
		for (int j = i - 1; j >= 0; --j)
		{
			if (IsRTLCodepoint(s[j]))
				return ECharDir::RTL; // Attach to preceding RTL text
			if (IsStrongLTR(s[j]))
				return ECharDir::LTR; // Attach to preceding LTR text
			// Skip neutral characters
		}
		return ECharDir::LTR; // Default to LTR if no strong character found
	}

	// Brackets: always attach to the content inside them
	// This fixes hyperlinks like "[درع فولاذي أسود+9]"
	if (ch == L'[' || ch == L']')
	{
		// Opening bracket '[': look forward for strong character
		if (ch == L'[')
		{
			for (int j = i + 1; j < n; ++j)
			{
				wchar_t next = s[j];
				if (next == L']') break; // End of bracket content
				if (IsRTLCodepoint(next)) return ECharDir::RTL;
				if (IsStrongLTR(next)) return ECharDir::LTR;
			}
		}
		// Closing bracket ']': look backward for strong character
		else if (ch == L']')
		{
			for (int j = i - 1; j >= 0; --j)
			{
				wchar_t prev = s[j];
				if (prev == L'[') break; // Start of bracket content
				if (IsRTLCodepoint(prev)) return ECharDir::RTL;
				if (IsStrongLTR(prev)) return ECharDir::LTR;
			}
		}
		// If we can't determine, treat as neutral
		return ECharDir::Neutral;
	}

	// Spaces should attach to adjacent strong characters to avoid fragmentation
	// This fixes "english + arabic" by keeping " + " with "english"
	if (ch == L' ' || ch == L'\t')
	{
		if (HasStrongLTRNeighbor(s, n, i))
			return ECharDir::LTR;
		// Note: We don't check for RTL neighbor because ResolveNeutralDir handles that
	}

	// Name-token punctuation: if adjacent to LTR, treat as LTR to keep token intact
	if (IsNameTokenPunct(ch) && HasStrongLTRNeighbor(s, n, i))
		return ECharDir::LTR;

	return ECharDir::Neutral;
}

// Pre-computed strong character lookup for O(1) neutral resolution
struct TStrongDirCache
{
	std::vector<EBidiDir> nextStrong; // nextStrong[i] = direction of next strong char after position i
	EBidiDir baseDir;

	TStrongDirCache(const wchar_t* s, int n, EBidiDir base) : nextStrong(n), baseDir(base)
	{
		// Build reverse lookup: scan from end to beginning
		// Use GetCharDirSmart for context-aware character classification
		EBidiDir lastSeen = baseDir;
		for (int i = n - 1; i >= 0; --i)
		{
			ECharDir cd = GetCharDirSmart(s, n, i);
			if (cd == ECharDir::LTR)
				lastSeen = EBidiDir::LTR;
			else if (cd == ECharDir::RTL)
				lastSeen = EBidiDir::RTL;

			nextStrong[i] = lastSeen;
		}
	}

	EBidiDir GetNextStrong(int i) const
	{
		if (i + 1 < (int)nextStrong.size())
			return nextStrong[i + 1];
		return baseDir;
	}
};

static inline EBidiDir ResolveNeutralDir(const wchar_t* s, int n, int i, EBidiDir baseDir, EBidiDir lastStrong, const TStrongDirCache* cache = nullptr)
{
	// Use pre-computed cache if available (O(1) instead of O(n))
	EBidiDir nextStrong = baseDir;
	if (cache)
	{
		nextStrong = cache->GetNextStrong(i);
	}
	else
	{
		// Linear scan (slower, but works without cache)
		for (int j = i + 1; j < n; ++j)
		{
			ECharDir cd = GetCharDirSmart(s, n, j);
			if (cd == ECharDir::LTR) { nextStrong = EBidiDir::LTR; break; }
			if (cd == ECharDir::RTL) { nextStrong = EBidiDir::RTL; break; }
		}
	}

	// If both sides agree, neutral adopts that direction
	if (lastStrong == nextStrong)
		return lastStrong;

	// Handle edge cases for leading/trailing punctuation
	if (nextStrong == baseDir && lastStrong != baseDir)
		return lastStrong;

	if (lastStrong == baseDir && nextStrong != baseDir)
		return nextStrong;

	// Otherwise fall back to base direction
	return baseDir;
}

static EBidiDir DetectBaseDir_FirstStrong(const wchar_t* s, int n)
{
	if (!s || n <= 0)
		return EBidiDir::LTR;

	for (int i = 0; i < n; ++i)
	{
		const wchar_t ch = s[i];
		// Check RTL first, then alpha
		if (IsRTLCodepoint(ch))
			return EBidiDir::RTL;

		if (IsStrongAlpha(ch))
			return EBidiDir::LTR;
	}

	return EBidiDir::LTR;
}

static std::vector<wchar_t> BuildVisualBidiText_Tagless(const wchar_t* s, int n, bool forceRTL)
{
	if (!s || n <= 0)
		return {};

	// Use buffer pool to avoid per-call allocations
	BiDiBuffers::TBufferPool& buffers = BiDiBuffers::Get();
	buffers.EnsureCapacity((size_t)n);

	// 1) base direction
	EBidiDir base = forceRTL ? EBidiDir::RTL : DetectBaseDir_FirstStrong(s, n);

	// Pre-compute strong character positions for O(1) neutral resolution
	TStrongDirCache strongCache(s, n, base);

	// 2) split into runs - use a more efficient approach
	// Instead of TBidiRun with vectors, use start/end indices
	struct TRunInfo { int start; int end; EBidiDir dir; };
	thread_local static std::vector<TRunInfo> s_runs;
	s_runs.clear();
	s_runs.reserve((size_t)std::max(4, n / 50));

	EBidiDir lastStrong = base;
	EBidiDir currentRunDir = base;
	int runStart = 0;

	for (int i = 0; i < n; ++i)
	{
		EBidiDir d;
		ECharDir cd = GetCharDirSmart(s, n, i);

		if (cd == ECharDir::RTL)
		{
			d = EBidiDir::RTL;
			lastStrong = EBidiDir::RTL;
		}
		else if (cd == ECharDir::LTR)
		{
			d = EBidiDir::LTR;
			lastStrong = EBidiDir::LTR;
		}
		else
		{
			// Pass cache for O(1) lookup instead of O(n) scan
			d = ResolveNeutralDir(s, n, i, base, lastStrong, &strongCache);
		}

		// Start a new run if direction changes
		if (d != currentRunDir)
		{
			if (i > runStart)
				s_runs.push_back({runStart, i, currentRunDir});
			runStart = i;
			currentRunDir = d;
		}
	}
	// Push final run
	if (n > runStart)
		s_runs.push_back({runStart, n, currentRunDir});

	// 3) shape RTL runs using pooled buffer
	buffers.shaped.clear();

	auto shapeRun = [&](int start, int end) -> std::pair<const wchar_t*, int>
	{
		int len = end - start;
		if (len <= 0)
			return {nullptr, 0};

		// Check for potential integer overflow
		if ((size_t)len > SIZE_MAX / ARABIC_SHAPING_EXPANSION_FACTOR_RETRY - ARABIC_SHAPING_SAFETY_MARGIN_RETRY)
			return {s + start, len}; // Return unshaped

		size_t neededSize = buffers.shaped.size() + (size_t)len * ARABIC_SHAPING_EXPANSION_FACTOR + ARABIC_SHAPING_SAFETY_MARGIN;
		if (buffers.shaped.capacity() < neededSize)
			buffers.shaped.reserve(neededSize);

		size_t outStart = buffers.shaped.size();
		buffers.shaped.resize(outStart + (size_t)len * ARABIC_SHAPING_EXPANSION_FACTOR + ARABIC_SHAPING_SAFETY_MARGIN);

		int outLen = Arabic_MakeShape(const_cast<wchar_t*>(s + start), len,
		                               buffers.shaped.data() + outStart,
		                               (int)(buffers.shaped.size() - outStart));

		if (outLen <= 0)
			return {s + start, len}; // Return unshaped on failure

		buffers.shaped.resize(outStart + (size_t)outLen);
		return {buffers.shaped.data() + outStart, outLen};
	};

	// 4) produce visual order
	std::vector<wchar_t> visual;
	visual.reserve((size_t)n);

	auto emitRun = [&](const TRunInfo& run)
	{
		if (run.dir == EBidiDir::RTL)
		{
			// Shape and reverse RTL runs
			std::pair<const wchar_t*, int> shaped = shapeRun(run.start, run.end);
			const wchar_t* ptr = shaped.first;
			int len = shaped.second;
			if (ptr && len > 0)
			{
				for (int k = len - 1; k >= 0; --k)
					visual.push_back(ptr[k]);
			}
		}
		else
		{
			// LTR runs: copy directly
			visual.insert(visual.end(), s + run.start, s + run.end);
		}
	};

	if (base == EBidiDir::LTR)
	{
		for (const auto& run : s_runs)
			emitRun(run);
	}
	else
	{
		for (int i = (int)s_runs.size() - 1; i >= 0; --i)
			emitRun(s_runs[(size_t)i]);
	}

	return visual;
}

// ============================================================================
// Chat Message BiDi Processing (Separate name/message handling)
// ============================================================================

// Build visual BiDi text for chat messages with separate name and message
// This avoids fragile " : " detection and handles cases where username contains " : "
//
// RECOMMENDED USAGE:
//   Instead of: SetValue("PlayerName : Message")
//   Use this function with separated components:
//     - name: "PlayerName" (without " : ")
//     - msg: "Message" (without " : ")
//
// INTEGRATION NOTES:
//   To use this properly, you need to:
//   1. Modify the server/network code to send chat name and message separately
//   2. Or parse the chat string in PythonNetworkStreamPhaseGame.cpp BEFORE passing to GrpTextInstance
//   3. Then call this function instead of BuildVisualBidiText_Tagless
//
static inline std::vector<wchar_t> BuildVisualChatMessage(
	const wchar_t* name, int nameLen,
	const wchar_t* msg, int msgLen,
	bool forceRTL)
{
	if (!name || !msg || nameLen <= 0 || msgLen <= 0)
		return {};

	// Check if message contains RTL or hyperlink tags
	bool msgHasRTL = false;
	bool msgHasTags = false;
	for (int i = 0; i < msgLen; ++i)
	{
		if (IsRTLCodepoint(msg[i]))
			msgHasRTL = true;
		if (msg[i] == L'|')
			msgHasTags = true;
		if (msgHasRTL && msgHasTags)
			break;
	}

	// Build result based on UI direction (pre-reserve exact size)
	std::vector<wchar_t> visual;
	visual.reserve((size_t)(nameLen + msgLen + 3)); // +3 for " : "

	// Decision: UI direction determines order (for visual consistency)
	// RTL UI: "Message : Name" (message on right, consistent with RTL reading flow)
	// LTR UI: "Name : Message" (name on left, consistent with LTR reading flow)
	if (forceRTL)
	{
		// RTL UI: "Message : Name"
		// Don't apply BiDi if message has tags (hyperlinks are pre-formatted)
		if (msgHasTags)
		{
			visual.insert(visual.end(), msg, msg + msgLen);
		}
		else
		{
			// Apply BiDi to message with auto-detection (don't force RTL)
			// Let the BiDi algorithm detect base direction from first strong character
			std::vector<wchar_t> msgVisual = BuildVisualBidiText_Tagless(msg, msgLen, forceRTL);
			visual.insert(visual.end(), msgVisual.begin(), msgVisual.end());
		}
		visual.push_back(L' ');
		visual.push_back(L':');
		visual.push_back(L' ');
		visual.insert(visual.end(), name, name + nameLen); // Name on left side
	}
	else
	{
		// LTR UI: "Name : Message"
		visual.insert(visual.end(), name, name + nameLen); // Name on left side
		visual.push_back(L' ');
		visual.push_back(L':');
		visual.push_back(L' ');
		// Don't apply BiDi if message has tags (hyperlinks are pre-formatted)
		if (msgHasTags)
		{
			visual.insert(visual.end(), msg, msg + msgLen);
		}
		else
		{
			// Apply BiDi to message with auto-detection (don't force RTL)
			// Let the BiDi algorithm detect base direction from first strong character
			std::vector<wchar_t> msgVisual = BuildVisualBidiText_Tagless(msg, msgLen, forceRTL);
			visual.insert(visual.end(), msgVisual.begin(), msgVisual.end());
		}
	}

	return visual;
}

// ============================================================================
// TextTail formatting for RTL UI
// ============================================================================

enum class EPlaceDir
{
	Left, // place block to the LEFT of the cursor (cursor is a right edge)
	Right // place block to the RIGHT of the cursor (cursor is a left edge)
};

template <typename TText>
inline float TextTailBiDi(TText* t, float cursorX, float y, float z, float fxAdd, EPlaceDir dir)
{
	if (!t)
		return cursorX;

	int w = 0, h = 0;
	t->GetTextSize(&w, &h);
	const float fw = static_cast<float>(w);

	float x;
	if (dir == EPlaceDir::Left)
	{
		x = t->IsRTL() ? cursorX : (cursorX - fw);
		// advance cursor left
		cursorX = cursorX - fw - fxAdd;
	}
	else
	{
		x = t->IsRTL() ? (cursorX + fw) : cursorX;
		// advance cursor right
		cursorX = cursorX + fw + fxAdd;
	}

	// SNAP to pixel grid to avoid "broken pixels"
	x = floorf(x + 0.5f);
	y = floorf(y + 0.5f);

	t->SetPosition(x, y, z);
	t->Update();

	return cursorX;
}
