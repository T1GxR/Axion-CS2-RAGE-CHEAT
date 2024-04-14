//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// $Header: $
// $NoKeywords: $
//
// Serialization buffer
//===========================================================================//

#pragma warning (disable : 4514)

#include "utlbuffer.h"
#include "../../../utilities/log.h"
#include "strtools.h"



//-----------------------------------------------------------------------------
// Character conversions for C strings
//-----------------------------------------------------------------------------
class CUtlCStringConversion : public CUtlCharConversion
{
public:
	CUtlCStringConversion(char nEscapeChar, const char* pDelimiter, size_t nCount, ConversionArray_t* pArray);

	// Finds a conversion for the passed-in string, returns length
	virtual char FindConversion(const char* pString, size_t* pLength);

private:
	char m_pConversion[256];
};


//-----------------------------------------------------------------------------
// Character conversions for no-escape sequence strings
//-----------------------------------------------------------------------------
class CUtlNoEscConversion : public CUtlCharConversion
{
public:
	CUtlNoEscConversion(char nEscapeChar, const char* pDelimiter, size_t nCount, ConversionArray_t* pArray) :
		CUtlCharConversion(nEscapeChar, pDelimiter, nCount, pArray) {}

	// Finds a conversion for the passed-in string, returns length
	virtual char FindConversion(const char* pString, size_t* pLength) { *pLength = 0; return 0; }
};


//-----------------------------------------------------------------------------
// List of character conversions
//-----------------------------------------------------------------------------
//BEGIN_CUSTOM_CHAR_CONVERSION(CUtlCStringConversion, s_StringCharConversion, "\"", '\\')
//{
//	'\n', "n"
//},
//	{ '\t', "t" },
//	{ '\v', "v" },
//	{ '\b', "b" },
//	{ '\r', "r" },
//	{ '\f', "f" },
//	{ '\a', "a" },
//	{ '\\', "\\" },
//	{ '\?', "\?" },
//	{ '\'', "\'" },
//	{ '\"', "\"" },
//END_CUSTOM_CHAR_CONVERSION(CUtlCStringConversion, s_StringCharConversion, "\"", '\\')
//
//CUtlCharConversion* GetCStringCharConversion()
//{
//	return &s_StringCharConversion;
//}
//
//BEGIN_CUSTOM_CHAR_CONVERSION(CUtlNoEscConversion, s_NoEscConversion, "\"", 0x7F)
//{
//	0x7F, ""
//},
//END_CUSTOM_CHAR_CONVERSION(CUtlNoEscConversion, s_NoEscConversion, "\"", 0x7F)
//
//CUtlCharConversion* GetNoEscCharConversion()
//{
//	return &s_NoEscConversion;
//}


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CUtlCStringConversion::CUtlCStringConversion(char nEscapeChar, const char* pDelimiter, size_t nCount, ConversionArray_t* pArray) :
	CUtlCharConversion(nEscapeChar, pDelimiter, nCount, pArray)
{
	memset(m_pConversion, 0x0, sizeof(m_pConversion));
	for (size_t i = 0; i < nCount; ++i)
	{
		m_pConversion[(unsigned char)(pArray[i].m_pReplacementString[0])] = pArray[i].m_nActualChar;
	}
}

// Finds a conversion for the passed-in string, returns length
char CUtlCStringConversion::FindConversion(const char* pString, size_t* pLength)
{
	char c = m_pConversion[(unsigned char)(pString[0])];
	*pLength = (c != '\0') ? 1 : 0;
	return c;
}



//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CUtlCharConversion::CUtlCharConversion(char nEscapeChar, const char* pDelimiter, size_t nCount, ConversionArray_t* pArray)
{
	m_nEscapeChar = nEscapeChar;
	m_pDelimiter = pDelimiter;
	m_nCount = nCount;
	m_nDelimiterLength = strlen(pDelimiter);
	m_nMaxConversionLength = 0;

	memset(m_pReplacements, 0, sizeof(m_pReplacements));

	for (size_t i = 0; i < nCount; ++i)
	{
		m_pList[i] = pArray[i].m_nActualChar;
		ConversionInfo_t& info = m_pReplacements[(unsigned char)(m_pList[i])];
		info.m_pReplacementString = pArray[i].m_pReplacementString;
		info.m_nLength = strlen(info.m_pReplacementString);
		if (info.m_nLength > m_nMaxConversionLength)
		{
			m_nMaxConversionLength = info.m_nLength;
		}
	}
}


//-----------------------------------------------------------------------------
// Escape character + delimiter
//-----------------------------------------------------------------------------
char CUtlCharConversion::GetEscapeChar() const
{
	return m_nEscapeChar;
}

const char* CUtlCharConversion::GetDelimiter() const
{
	return m_pDelimiter;
}

size_t CUtlCharConversion::GetDelimiterLength() const
{
	return m_nDelimiterLength;
}


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
const char* CUtlCharConversion::GetConversionString(char c) const
{
	return m_pReplacements[(unsigned char)c].m_pReplacementString;
}

size_t CUtlCharConversion::GetConversionLength(char c) const
{
	return m_pReplacements[(unsigned char)c].m_nLength;
}

size_t CUtlCharConversion::MaxConversionLength() const
{
	return m_nMaxConversionLength;
}


//-----------------------------------------------------------------------------
// Finds a conversion for the passed-in string, returns length
//-----------------------------------------------------------------------------
char CUtlCharConversion::FindConversion(const char* pString, size_t* pLength)
{
	for (size_t i = 0; i < m_nCount; ++i)
	{
		if (!strcmp(pString, m_pReplacements[(unsigned char)(m_pList[i])].m_pReplacementString))
		{
			*pLength = m_pReplacements[(unsigned char)(m_pList[i])].m_nLength;
			return m_pList[i];
		}
	}

	*pLength = 0;
	return '\0';
}

CUtlBuffer::CUtlBuffer(size_t growSize, size_t initSize, int nFlags) :
	m_Error(0), m_Memory(growSize, initSize)
{
	m_Memory.Init(growSize, initSize);

	m_Get = 0;
	m_Put = 0;
	m_nTab = 0;
	m_nOffset = 0;
	m_Flags = static_cast<unsigned char>(nFlags);
	if ((initSize != 0))
	{
		m_nMaxPut = -1;
		AddNullTermination(m_Put);
	}
	else
	{
		m_nMaxPut = 0;
	}
	SetOverflowFuncs(&CUtlBuffer::GetOverflow, &CUtlBuffer::PutOverflow);
}

CUtlBuffer::CUtlBuffer(const void* pBuffer, size_t nSize, int nFlags) :
	m_Memory(reinterpret_cast<const unsigned char*>(pBuffer), static_cast<int>(nSize)), m_Error(0)
{

	m_Get = 0;
	m_Put = 0;
	m_nTab = 0;
	m_nOffset = 0;
	m_Flags = static_cast<unsigned char>(nFlags);

	if (IsReadOnly())
	{	
		L_PRINT(LOG_INFO) << "CUTLBuffer > readonly";
		m_nMaxPut = m_Put = nSize;
	}
	else
	{
		L_PRINT(LOG_INFO) << "CUTLBuffer > m_MaxPut = -1";
		m_nMaxPut = -1;
		AddNullTermination(m_Put);
	}

	SetOverflowFuncs(&CUtlBuffer::GetOverflow, &CUtlBuffer::PutOverflow);
}

CUtlBuffer::CUtlBuffer(const CUtlBuffer& copyFrom) :
	m_Get(copyFrom.m_Get),
	m_Put(copyFrom.m_Put),
	m_Error(copyFrom.m_Error),
	m_Flags(copyFrom.m_Flags),
	m_Reserved(copyFrom.m_Reserved),
#if defined( _GAMECONSOLE )
	pad(copyFrom.pad),
#endif
	m_nTab(copyFrom.m_nTab),
	m_nMaxPut(copyFrom.m_nMaxPut),
	m_nOffset(copyFrom.m_nOffset),
	m_GetOverflowFunc(copyFrom.m_GetOverflowFunc),
	m_PutOverflowFunc(copyFrom.m_PutOverflowFunc),
	m_Byteswap(copyFrom.m_Byteswap),
	m_Memory(copyFrom.m_Memory)
{
	

}

CUtlBuffer& CUtlBuffer::operator=(const CUtlBuffer& copyFrom)
{
	if (this != &copyFrom)
	{
		m_Get = copyFrom.m_Get;
		m_Put = copyFrom.m_Put;
		m_Error = copyFrom.m_Error;
		m_Flags = copyFrom.m_Flags;
		m_Reserved = copyFrom.m_Reserved;
#if defined( _GAMECONSOLE )
		pad = copyFrom.pad;
#endif
		m_nTab = copyFrom.m_nTab;
		m_nMaxPut = copyFrom.m_nMaxPut;
		m_nOffset = copyFrom.m_nOffset;
		m_GetOverflowFunc = copyFrom.m_GetOverflowFunc;
		m_PutOverflowFunc = copyFrom.m_PutOverflowFunc;
		m_Byteswap = copyFrom.m_Byteswap;
		m_Memory = copyFrom.m_Memory; // Use the assignment operator of CUtlMemory
	}
	return *this;
}

CUtlBuffer::CUtlBuffer(CUtlBuffer&& moveFrom) // = default
	: m_Memory(CRT::Move(moveFrom.m_Memory))
	, m_Get(CRT::Move(moveFrom.m_Get))
	, m_Put(CRT::Move(moveFrom.m_Put))
	, m_Error(CRT::Move(moveFrom.m_Error))
	, m_Flags(CRT::Move(moveFrom.m_Flags))
	, m_Reserved(CRT::Move(moveFrom.m_Reserved))
#if defined( _GAMECONSOLE )
	, pad(CRT::Move(moveFrom.pad))
#endif
	, m_nTab(CRT::Move(moveFrom.m_nTab))
	, m_nMaxPut(CRT::Move(moveFrom.m_nMaxPut))
	, m_nOffset(CRT::Move(moveFrom.m_nOffset))
	, m_GetOverflowFunc(CRT::Move(moveFrom.m_GetOverflowFunc))
	, m_PutOverflowFunc(CRT::Move(moveFrom.m_PutOverflowFunc))
	, m_Byteswap(CRT::Move(moveFrom.m_Byteswap))
{}

CUtlBuffer& CUtlBuffer::operator=(CUtlBuffer&& moveFrom) // = default
{
	m_Memory = CRT::Move(moveFrom.m_Memory);
	m_Get = CRT::Move(moveFrom.m_Get);
	m_Put = CRT::Move(moveFrom.m_Put);
	m_Error = CRT::Move(moveFrom.m_Error);
	m_Flags = CRT::Move(moveFrom.m_Flags);
	m_Reserved = CRT::Move(moveFrom.m_Reserved);
#if defined( _GAMECONSOLE )
	pad = Move(moveFrom.pad);
#endif
	m_nTab = CRT::Move(moveFrom.m_nTab);
	m_nMaxPut = CRT::Move(moveFrom.m_nMaxPut);
	m_nOffset = CRT::Move(moveFrom.m_nOffset);
	m_GetOverflowFunc = CRT::Move(moveFrom.m_GetOverflowFunc);
	m_PutOverflowFunc = CRT::Move(moveFrom.m_PutOverflowFunc);
	m_Byteswap = CRT::Move(moveFrom.m_Byteswap);

	return *this;
}

//-----------------------------------------------------------------------------
// Modifies the buffer to be binary or text; Blows away the buffer and the CONTAINS_CRLF value. 
//-----------------------------------------------------------------------------
void CUtlBuffer::SetBufferType(bool bIsText, bool bContainsCRLF)
{

	if (bIsText)
	{
		m_Flags |= TEXT_BUFFER;
	}
	else
	{
		m_Flags &= ~TEXT_BUFFER;
	}
	if (bContainsCRLF)
	{
		m_Flags |= CONTAINS_CRLF;
	}
	else
	{
		m_Flags &= ~CONTAINS_CRLF;
	}
}


//-----------------------------------------------------------------------------
// Attaches the buffer to external memory....
//-----------------------------------------------------------------------------
void CUtlBuffer::SetExternalBuffer(void* pMemory, size_t nSize, size_t nInitialPut, int nFlags)
{
	m_Memory.SetExternalBuffer((unsigned char*)pMemory, nSize);

	// Reset all indices; we just changed memory
	m_Get = 0;
	m_Put = nInitialPut;
	m_nTab = 0;
	m_Error = 0;
	m_nOffset = 0;
	m_Flags = (unsigned char)nFlags;
	m_nMaxPut = -1;
	AddNullTermination(m_Put);
}

//-----------------------------------------------------------------------------
// Assumes an external buffer but manages its deletion
//-----------------------------------------------------------------------------
void CUtlBuffer::AssumeMemory(void* pMemory, size_t nSize, size_t nInitialPut, int nFlags)
{
	// Simply take the pointer but don't mark us as external
	m_Memory.AssumeMemory((unsigned char*)pMemory, nSize);

	m_Get = 0;
	m_Put = nInitialPut;
	m_nTab = 0;
	m_Error = 0;
	m_nOffset = 0;
	m_Flags = (unsigned char)nFlags;
	m_nMaxPut = -1;
	AddNullTermination(m_Put);
}


//-----------------------------------------------------------------------------
// Allows the caller to control memory
//-----------------------------------------------------------------------------
void* CUtlBuffer::DetachMemory()
{
	// Reset all indices; we just changed memory
	m_Get = 0;
	m_Put = 0;
	m_nTab = 0;
	m_Error = 0;
	m_nOffset = 0;
	return m_Memory.DetachMemory();
}


//-----------------------------------------------------------------------------
// Makes sure we've got at least this much memory
//-----------------------------------------------------------------------------
void CUtlBuffer::EnsureCapacity(size_t num)
{
	//MEM_ALLOC_CREDIT();
	// Add one extra for the null termination
	num += 1;
	if (m_Memory.IsExternallyAllocated())
	{
		if (IsGrowable() && (m_Memory.AllocationNum() < num))
		{
			m_Memory.ConvertToGrowableMemory(0);
		}
		else
		{
			num -= 1;
		}
	}

	m_Memory.EnsureCapacity(num);
}


//-----------------------------------------------------------------------------
// Base get method from which all others derive
//-----------------------------------------------------------------------------
bool CUtlBuffer::Get(void* pMem, size_t size)
{
	if (size > 0 && CheckGet(size))
	{
		memcpy(pMem, &m_Memory[m_Get - m_nOffset], size);
		m_Get += size;
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------
// This will get at least 1 byte and up to nSize bytes. 
// It will return the number of bytes actually read.
//-----------------------------------------------------------------------------
size_t CUtlBuffer::GetUpTo(void* pMem, size_t nSize)
{
	if (CheckArbitraryPeekGet(0, nSize))
	{
		memcpy(pMem, &m_Memory[m_Get - m_nOffset], nSize);
		m_Get += nSize;
		return nSize;
	}
	return 0;
}


//-----------------------------------------------------------------------------
// Eats whitespace
//-----------------------------------------------------------------------------
void CUtlBuffer::EatWhiteSpace()
{
	if (IsText() && IsValid())
	{
		while (CheckGet(sizeof(char)))
		{
			if (!isspace(*(const unsigned char*)PeekGet()))
				break;
			m_Get += sizeof(char);
		}
	}
}


//-----------------------------------------------------------------------------
// Eats C++ style comments
//-----------------------------------------------------------------------------
bool CUtlBuffer::EatCPPComment()
{
	if (IsText() && IsValid())
	{
		// If we don't have a a c++ style comment next, we're done
		const char* pPeek = (const char*)PeekGet(2 * sizeof(char), 0);
		if (!pPeek || (pPeek[0] != '/') || (pPeek[1] != '/'))
			return false;

		// Deal with c++ style comments
		m_Get += 2;

		// read complete line
		for (char c = GetChar(); IsValid(); c = GetChar())
		{
			if (c == '\n')
				break;
		}
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------
// Peeks how much whitespace to eat
//-----------------------------------------------------------------------------
size_t CUtlBuffer::PeekWhiteSpace(size_t nOffset)
{
	if (!IsText() || !IsValid())
		return 0;

	while (CheckPeekGet(nOffset, sizeof(char)))
	{
		if (!isspace(*(unsigned char*)PeekGet(nOffset)))
			break;
		nOffset += sizeof(char);
	}

	return nOffset;
}


//-----------------------------------------------------------------------------
// Peek size of sting to come, check memory bound
//-----------------------------------------------------------------------------
size_t CUtlBuffer::PeekStringLength()
{
	if (!IsValid())
		return 0;

	// Eat preceding whitespace
	size_t nOffset = 0;
	if (IsText())
	{
		nOffset = PeekWhiteSpace(nOffset);
	}

	size_t nStartingOffset = nOffset;

	do
	{
		size_t nPeekAmount = 128;

		// NOTE: Add 1 for the terminating zero!
		if (!CheckArbitraryPeekGet(nOffset, nPeekAmount))
		{
			if (nOffset == nStartingOffset)
				return 0;
			return nOffset - nStartingOffset + 1;
		}

		const char* pTest = (const char*)PeekGet(nOffset);

		if (!IsText())
		{
			for (size_t i = 0; i < nPeekAmount; ++i)
			{
				// The +1 here is so we eat the terminating 0
				if (pTest[i] == 0)
					return (i + nOffset - nStartingOffset + 1);
			}
		}
		else
		{
			for (size_t i = 0; i < nPeekAmount; ++i)
			{
				// The +1 here is so we eat the terminating 0
				if (isspace((unsigned char)pTest[i]) || (pTest[i] == 0))
					return (i + nOffset - nStartingOffset + 1);
			}
		}

		nOffset += nPeekAmount;

	} while (true);
}


//-----------------------------------------------------------------------------
// Peek size of line to come, check memory bound
//-----------------------------------------------------------------------------
size_t CUtlBuffer::PeekLineLength()
{
	if (!IsValid())
		return 0;

	size_t nOffset = 0;
	size_t nStartingOffset = nOffset;

	do
	{
		size_t nPeekAmount = 128;

		// NOTE: Add 1 for the terminating zero!
		if (!CheckArbitraryPeekGet(nOffset, nPeekAmount))
		{
			if (nOffset == nStartingOffset)
				return 0;
			return nOffset - nStartingOffset + 1;
		}

		const char* pTest = (const char*)PeekGet(nOffset);

		for (size_t i = 0; i < nPeekAmount; ++i)
		{
			// The +2 here is so we eat the terminating '\n' and 0
			if (pTest[i] == '\n' || pTest[i] == '\r')
				return (i + nOffset - nStartingOffset + 2);
			// The +1 here is so we eat the terminating 0
			if (pTest[i] == 0)
				return (i + nOffset - nStartingOffset + 1);
		}

		nOffset += nPeekAmount;

	} while (true);
}


//-----------------------------------------------------------------------------
// Does the next bytes of the buffer match a pattern?
//-----------------------------------------------------------------------------
bool CUtlBuffer::PeekStringMatch(size_t nOffset, const char* pString, size_t nLen)
{
	if (!CheckPeekGet(nOffset, nLen))
		return false;
	return !strncmp((const char*)PeekGet(nOffset), pString, nLen);
}


//-----------------------------------------------------------------------------
// This version of PeekStringLength converts \" to \\ and " to \, etc.
// It also reads a " at the beginning and end of the string
//-----------------------------------------------------------------------------
size_t CUtlBuffer::PeekDelimitedStringLength(CUtlCharConversion* pConv, bool bActualSize)
{
	if (!IsText() || !pConv)
		return PeekStringLength();

	// Eat preceding whitespace
	size_t nOffset = 0;
	if (IsText())
	{
		nOffset = PeekWhiteSpace(nOffset);
	}

	if (!PeekStringMatch(nOffset, pConv->GetDelimiter(), pConv->GetDelimiterLength()))
		return 0;

	// Try to read ending ", but don't accept \"
	size_t nActualStart = nOffset;
	nOffset += pConv->GetDelimiterLength();
	size_t nLen = 1;	// Starts at 1 for the '\0' termination

	do
	{
		if (PeekStringMatch(nOffset, pConv->GetDelimiter(), pConv->GetDelimiterLength()))
			break;

		if (!CheckPeekGet(nOffset, 1))
			break;

		char c = *(const char*)PeekGet(nOffset);
		++nLen;
		++nOffset;
		if (c == pConv->GetEscapeChar())
		{
			size_t nLength = pConv->MaxConversionLength();
			if (!CheckArbitraryPeekGet(nOffset, nLength))
				break;

			pConv->FindConversion((const char*)PeekGet(nOffset), &nLength);
			nOffset += nLength;
		}
	} while (true);

	return bActualSize ? nLen : nOffset - nActualStart + pConv->GetDelimiterLength() + 1;
}


//-----------------------------------------------------------------------------
// Reads a null-terminated string
//-----------------------------------------------------------------------------
void CUtlBuffer::GetString(char* pString, size_t nMaxChars)
{
	if (!IsValid())
	{
		*pString = 0;
		return;
	}

	if (nMaxChars <= 0)
	{
		return;
	}

	// Remember, this *includes* the null character
	// It will be 0, however, if the buffer is empty.
	size_t nLen = PeekStringLength();

	if (IsText())
	{
		EatWhiteSpace();
	}

	if (nLen <= 0)
	{
		*pString = 0;
		m_Error |= GET_OVERFLOW;
		return;
	}

	const size_t nCharsToRead = std::min(nLen, nMaxChars) - 1;

	Get(pString, nCharsToRead);
	pString[nCharsToRead] = 0;

	if (nLen > (nCharsToRead + 1))
	{
		SeekGet(SEEK_CURRENT, nLen - (nCharsToRead + 1));
	}

	// Read the terminating NULL in binary formats
	if (!IsText())
	{
		//VerifyEquals(GetChar(), 0);
	}
}


//-----------------------------------------------------------------------------
// Reads up to and including the first \n
//-----------------------------------------------------------------------------
void CUtlBuffer::GetLine(char* pLine, size_t nMaxChars)
{
	//Assert( IsText() && !ContainsCRLF() );

	if (!IsValid())
	{
		*pLine = 0;
		return;
	}

	if (nMaxChars == 0)
	{
		nMaxChars = LLONG_MAX;
	}

	// Remember, this *includes* the null character
	// It will be 0, however, if the buffer is empty.
	size_t nLen = PeekLineLength();
	if (nLen == 0)
	{
		*pLine = 0;
		m_Error |= GET_OVERFLOW;
		return;
	}

	// Strip off the terminating NULL
	if (nLen <= nMaxChars)
	{
		Get(pLine, nLen - 1);
		pLine[nLen - 1] = 0;
	}
	else
	{
		Get(pLine, nMaxChars - 1);
		pLine[nMaxChars - 1] = 0;
		SeekGet(SEEK_CURRENT, nLen - 1 - nMaxChars);
	}
}


//-----------------------------------------------------------------------------
// This version of GetString converts \ to \\ and " to \", etc.
// It also places " at the beginning and end of the string
//-----------------------------------------------------------------------------
char CUtlBuffer::GetDelimitedCharInternal(CUtlCharConversion* pConv)
{
	char c = GetChar();
	if (c == pConv->GetEscapeChar())
	{
		size_t nLength = pConv->MaxConversionLength();
		if (!CheckArbitraryPeekGet(0, nLength))
			return '\0';

		c = pConv->FindConversion((const char*)PeekGet(), &nLength);
		SeekGet(SEEK_CURRENT, nLength);
	}

	return c;
}

char CUtlBuffer::GetDelimitedChar(CUtlCharConversion* pConv)
{
	if (!IsText() || !pConv)
		return GetChar();
	return GetDelimitedCharInternal(pConv);
}

void CUtlBuffer::GetDelimitedString(CUtlCharConversion* pConv, char* pString, size_t nMaxChars)
{
	if (!IsText() || !pConv)
	{
		GetString(pString, nMaxChars);
		return;
	}

	if (!IsValid())
	{
		*pString = 0;
		return;
	}

	if (nMaxChars == 0)
	{
		nMaxChars = LLONG_MAX;
	}

	EatWhiteSpace();
	if (!PeekStringMatch(0, pConv->GetDelimiter(), pConv->GetDelimiterLength()))
		return;

	// Pull off the starting delimiter
	SeekGet(SEEK_CURRENT, pConv->GetDelimiterLength());

	size_t nRead = 0;
	while (IsValid())
	{
		if (PeekStringMatch(0, pConv->GetDelimiter(), pConv->GetDelimiterLength()))
		{
			SeekGet(SEEK_CURRENT, pConv->GetDelimiterLength());
			break;
		}

		char c = GetDelimitedCharInternal(pConv);

		if (nRead < nMaxChars)
		{
			pString[nRead] = c;
			++nRead;
		}
	}

	if (nRead >= nMaxChars)
	{
		nRead = nMaxChars - 1;
	}
	pString[nRead] = '\0';
}


//-----------------------------------------------------------------------------
// Checks if a get is ok
//-----------------------------------------------------------------------------
bool CUtlBuffer::CheckGet(size_t nSize)
{
	if (m_Error & GET_OVERFLOW)
		return false;

	if (TellMaxPut() < m_Get + nSize)
	{
		m_Error |= GET_OVERFLOW;
		return false;
	}

	if ((m_Get < m_nOffset) || (m_Memory.AllocationNum() < m_Get - m_nOffset + nSize))
	{
		if (!OnGetOverflow(nSize))
		{
			m_Error |= GET_OVERFLOW;
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// Checks if a peek get is ok
//-----------------------------------------------------------------------------
bool CUtlBuffer::CheckPeekGet(size_t nOffset, size_t nSize)
{
	if (m_Error & GET_OVERFLOW)
		return false;

	// Checking for peek can't set the overflow flag
	bool bOk = CheckGet(nOffset + nSize);
	m_Error &= ~GET_OVERFLOW;
	return bOk;
}


//-----------------------------------------------------------------------------
// Call this to peek arbitrarily long into memory. It doesn't fail unless
// it can't read *anything* new
//-----------------------------------------------------------------------------
bool CUtlBuffer::CheckArbitraryPeekGet(size_t nOffset, size_t& nIncrement)
{
	if (TellGet() + nOffset >= TellMaxPut())
	{
		nIncrement = 0;
		return false;
	}

	if (TellGet() + nOffset + nIncrement > TellMaxPut())
	{
		nIncrement = TellMaxPut() - TellGet() - nOffset;
	}

	// NOTE: CheckPeekGet could modify TellMaxPut for streaming files
	// We have to call TellMaxPut again here
	CheckPeekGet(nOffset, nIncrement);
	size_t nMaxGet = TellMaxPut() - TellGet();
	if (nMaxGet < nIncrement)
	{
		nIncrement = nMaxGet;
	}
	return (nIncrement != 0);
}


//-----------------------------------------------------------------------------
// Peek part of the butt
//-----------------------------------------------------------------------------
const void* CUtlBuffer::PeekGet(size_t nMaxSize, size_t nOffset)
{
	if (!CheckPeekGet(nOffset, nMaxSize))
		return NULL;
	return &m_Memory[m_Get + nOffset - m_nOffset];
}


//-----------------------------------------------------------------------------
// Change where I'm reading
//-----------------------------------------------------------------------------
void CUtlBuffer::SeekGet(SeekType_t type, size_t offset)
{
	switch (type)
	{
	case SEEK_HEAD:
		m_Get = offset;
		break;

	case SEEK_CURRENT:
		m_Get += offset;
		break;

	case SEEK_TAIL:
		m_Get = m_nMaxPut - offset;
		break;
	}

	if (m_Get > m_nMaxPut)
	{
		m_Error |= GET_OVERFLOW;
	}
	else
	{
		m_Error &= ~GET_OVERFLOW;
		if (m_Get < m_nOffset || m_Get >= m_nOffset + Size())
		{
			OnGetOverflow(-1);
		}
	}
}


//-----------------------------------------------------------------------------
// Parse...
//-----------------------------------------------------------------------------

#pragma warning ( disable : 4706 )

size_t CUtlBuffer::VaScanf(const char* pFmt, va_list list)
{
	if (m_Error || !IsText())
		return 0;

	size_t numScanned = 0;
	char c;
	while ((c = *pFmt++))
	{
		// Stop if we hit the end of the buffer
		if (m_Get >= TellMaxPut())
		{
			m_Error |= GET_OVERFLOW;
			break;
		}

		switch (c)
		{
		case ' ':
			// eat all whitespace
			EatWhiteSpace();
			break;

		case '%':
		{
			// Conversion character... try to convert baby!
			char type = *pFmt++;
			if (type == 0)
				return numScanned;

			switch (type)
			{
			case 'c':
			{
				char* ch = va_arg(list, char*);
				if (CheckPeekGet(0, sizeof(char)))
				{
					*ch = *(const char*)PeekGet();
					++m_Get;
				}
				else
				{
					*ch = 0;
					return numScanned;
				}
			}
			break;

			case 'h':
			{
				if (*pFmt == 'd' || *pFmt == 'i')
				{
					if (!GetTypeText(*va_arg(list, int16_t*)))
						return numScanned; // only support short ints, don't bother with hex
				}
				else if (*pFmt == 'u')
				{
					if (!GetTypeText(*va_arg(list, uint16_t*)))
						return numScanned;
				}
				else
					return numScanned;
				++pFmt;
			}
			break;

			case 'I':
			{
				if (*pFmt++ != '6' || *pFmt++ != '4')
					return numScanned; // only support "I64d" and "I64u"

				if (*pFmt == 'd')
				{
					if (!GetTypeText(*va_arg(list, int64_t*)))
						return numScanned;
				}
				else if (*pFmt == 'u')
				{
					if (!GetTypeText(*va_arg(list, uint64_t*)))
						return numScanned;
				}
				else
				{
					return numScanned;
				}

				++pFmt;
			}
			break;

			case 'i':
			case 'd':
			{
				int32_t* pArg = va_arg(list, int32_t*);
				if (!GetTypeText(*pArg))
					return numScanned;
			}
			break;

			case 'x':
			{
				uint32_t* pArg = va_arg(list, uint32_t*);
				if (!GetTypeText(*pArg, 16))
					return numScanned;
			}
			break;

			case 'u':
			{
				uint32_t* pArg = va_arg(list, uint32_t*);
				if (!GetTypeText(*pArg))
					return numScanned;
			}
			break;

			case 'l':
			{
				// we currently support %lf and %lld
				if (*pFmt == 'f')
				{
					if (!GetTypeText(*va_arg(list, double*)))
						return numScanned;
				}
				else if (*pFmt == 'l' && *++pFmt == 'd')
				{
					if (!GetTypeText(*va_arg(list, int64_t*)))
						return numScanned;
				}
				else
					return numScanned;
			}
			break;

			case 'f':
			{
				float* pArg = va_arg(list, float*);
				if (!GetTypeText(*pArg))
					return numScanned;
			}
			break;

			case 's':
			{
				char* s = va_arg(list, char*);
				GetString(s, 64); // [SECURITY EXPLOIT: Scanf %s should be deprecated as malicious data can overrun stack buffers! Here we'd assume that at least 64 bytes are available on the stack, and even if not this shouldn't give attracker much room for code execution]
			}
			break;

			default:
			{
				// unimplemented scanf type
				return numScanned;
			}
			break;
			}

			++numScanned;
		}
		break;

		default:
		{
			// Here we have to match the format string character
			// against what's in the buffer or we're done.
			if (!CheckPeekGet(0, sizeof(char)))
				return numScanned;

			if (c != *(const char*)PeekGet())
				return numScanned;

			++m_Get;
		}
		}
	}
	return numScanned;
}

#pragma warning ( default : 4706 )

size_t CUtlBuffer::Scanf(const char* pFmt, ...)
{
	va_list args;

	va_start(args, pFmt);
	size_t count = VaScanf(pFmt, args);
	va_end(args);

	return count;
}


//-----------------------------------------------------------------------------
// Advance the get index until after the particular string is found
// Do not eat whitespace before starting. Return false if it failed
//-----------------------------------------------------------------------------
bool CUtlBuffer::GetToken(const char* pToken)
{

	// Look for the token
	size_t nLen = strlen(pToken);

	// First time through on streaming, check what we already have loaded
	// if we have enough loaded to do the check
	size_t nMaxSize = Size() - (TellGet() - m_nOffset);
	if (nMaxSize <= nLen)
	{
		nMaxSize = Size();
	}
	size_t nSizeRemaining = TellMaxPut() - TellGet();

	size_t nGet = TellGet();
	while (nSizeRemaining >= nLen)
	{
		bool bOverFlow = (nSizeRemaining > nMaxSize);
		size_t nSizeToCheck = bOverFlow ? nMaxSize : nSizeRemaining;
		if (!CheckPeekGet(0, nSizeToCheck))
			break;

		const char* pBufStart = (const char*)PeekGet();
		const char* pFoundEnd = V_strnistr(pBufStart, pToken, nSizeToCheck);

		// Time to be careful: if we are in a state of overflow
		// (namely, there's more of the buffer beyond the current window)
		// we could be looking for 'foo' for example, and find 'foobar'
		// if 'foo' happens to be the last 3 characters of the current window
		size_t nOffset = (size_t)pFoundEnd - (size_t)pBufStart;
		bool bPotentialMismatch = (bOverFlow && ((size_t)nOffset == Size() - nLen));
		if (!pFoundEnd || bPotentialMismatch)
		{
			nSizeRemaining -= nSizeToCheck;
			if (!pFoundEnd && (nSizeRemaining < nLen))
				break;

			// Second time through, stream as much in as possible
			// But keep the last portion of the current buffer
			// since we couldn't check it against stuff outside the window
			nSizeRemaining += nLen;
			nMaxSize = Size();
			SeekGet(CUtlBuffer::SEEK_CURRENT, nSizeToCheck - nLen);
			continue;
		}

		// Seek past the end of the found string
		SeekGet(CUtlBuffer::SEEK_CURRENT, (size_t)(nOffset + nLen));
		return true;
	}

	// Didn't find a match, leave the get index where it was to start with
	SeekGet(CUtlBuffer::SEEK_HEAD, nGet);
	return false;
}


//-----------------------------------------------------------------------------
// (For text buffers only)
// Parse a token from the buffer:
// Grab all text that lies between a starting delimiter + ending delimiter
// (skipping whitespace that leads + trails both delimiters).
// Note the delimiter checks are case-insensitive.
// If successful, the get index is advanced and the function returns true,
// otherwise the index is not advanced and the function returns false.
//-----------------------------------------------------------------------------
bool CUtlBuffer::ParseToken(const char* pStartingDelim, const char* pEndingDelim, char* pString, size_t nMaxLen)
{
	size_t nCharsToCopy = 0;
	size_t nCurrentGet = 0;

	size_t nEndingDelimLen;

	// Starting delimiter is optional
	char emptyBuf = '\0';
	if (!pStartingDelim)
	{
		pStartingDelim = &emptyBuf;
	}

	// Ending delimiter is not
	nEndingDelimLen = strlen(pEndingDelim);

	size_t nStartGet = TellGet();
	size_t nTokenStart = -1;
	char nCurrChar;
	EatWhiteSpace();
	while (*pStartingDelim)
	{
		nCurrChar = *pStartingDelim++;
		if (!isspace((unsigned char)nCurrChar))
		{
			if (tolower(GetChar()) != tolower(nCurrChar))
				goto parseFailed;
		}
		else
		{
			EatWhiteSpace();
		}
	}

	EatWhiteSpace();
	nTokenStart = TellGet();
	if (!GetToken(pEndingDelim))
		goto parseFailed;

	nCurrentGet = TellGet();
	nCharsToCopy = ((nCurrentGet - nEndingDelimLen) - nTokenStart);
	if (nCharsToCopy >= nMaxLen)
	{
		nCharsToCopy = nMaxLen - 1;
	}

	if (nCharsToCopy > 0)
	{
		SeekGet(CUtlBuffer::SEEK_HEAD, nTokenStart);
		Get(pString, nCharsToCopy);
		if (!IsValid())
			goto parseFailed;

		// Eat trailing whitespace
		for (; nCharsToCopy > 0; --nCharsToCopy)
		{
			if (!isspace((unsigned char)pString[nCharsToCopy - 1]))
				break;
		}
	}
	pString[nCharsToCopy] = '\0';

	// Advance the Get index
	SeekGet(CUtlBuffer::SEEK_HEAD, nCurrentGet);
	return true;

parseFailed:
	// Revert the get index
	SeekGet(SEEK_HEAD, nStartGet);
	pString[0] = '\0';
	return false;
}


//-----------------------------------------------------------------------------
// Parses the next token, given a set of character breaks to stop at
//-----------------------------------------------------------------------------
size_t CUtlBuffer::ParseToken(characterset_t* pBreaks, char* pTokenBuf, size_t nMaxLen, bool bParseComments)
{
	pTokenBuf[0] = 0;

	// skip whitespace + comments
	while (true)
	{
		if (!IsValid())
			return -1;
		EatWhiteSpace();
		if (bParseComments)
		{
			if (!EatCPPComment())
				break;
		}
		else
		{
			break;
		}
	}

	char c = GetChar();

	// End of buffer
	if (c == 0)
		return -1;

	// handle quoted strings specially
	if (c == '\"')
	{
		size_t nLen = 0;
		while (IsValid())
		{
			c = GetChar();
			if (c == '\"' || !c)
			{
				pTokenBuf[nLen] = 0;
				return nLen;
			}
			pTokenBuf[nLen] = c;
			if (++nLen == nMaxLen)
			{
				pTokenBuf[nLen - 1] = 0;
				return nMaxLen;
			}
		}

		// In this case, we hit the end of the buffer before hitting the end quote
		pTokenBuf[nLen] = 0;
		return nLen;
	}

	
	// parse a regular word
	size_t nLen = 0;
	while (true)
	{
		pTokenBuf[nLen] = c;
		if (++nLen == nMaxLen)
		{
			pTokenBuf[nLen - 1] = 0;
			return nMaxLen;
		}
		c = GetChar();
		if (!IsValid())
			break;

	
	}

	pTokenBuf[nLen] = 0;
	return nLen;
}



//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------
void CUtlBuffer::Put(const void* pMem, size_t size)
{
	if (size && CheckPut(size))
	{
		memcpy(&m_Memory[m_Put - m_nOffset], pMem, size);
		m_Put += size;

		AddNullTermination(m_Put);
	}
}

//-----------------------------------------------------------------------------
// Writes a null-terminated string
//-----------------------------------------------------------------------------
void CUtlBuffer::PutString(const char* pString)
{
	L_PRINT(LOG_INFO) << "start";
	if (!(m_Flags & TEXT_BUFFER))
	{
		L_PRINT(LOG_INFO) << "no text 2";

		if (pString != nullptr)
		{
			L_PRINT(LOG_INFO) << "no text 3";

			// Not text? append a null at the end.
			size_t nLen = strlen(pString) + 1;
			L_PRINT(LOG_INFO) << "no text 4";

			Put(pString, nLen * sizeof(char));
			L_PRINT(LOG_INFO) << "no text 5";

			return;
		}
		else
		{
			L_PRINT(LOG_INFO) << "no text Jumped to Puttypebin";

			PutTypeBin<char>(0);
		}
	}
	else if (pString != nullptr)
	{
		L_PRINT(LOG_INFO) << "1";

		size_t nTabCount = (m_Flags & AUTO_TABS_DISABLED) ? 0 : m_nTab;

		if (nTabCount > 0)
		{
			L_PRINT(LOG_INFO) << "2";

			if (WasLastCharacterCR())
			{
				L_PRINT(LOG_INFO) << "3";

				PutTabs();
				L_PRINT(LOG_INFO) << "4";

			}
			L_PRINT(LOG_INFO) << "5";

			const char* pEndl = strchr(pString, '\n');
			L_PRINT(LOG_INFO) << "6";

			while (pEndl)
			{
				L_PRINT(LOG_INFO) << "7";

				size_t nSize = (size_t)pEndl - (size_t)pString + sizeof(char);
				L_PRINT(LOG_INFO) << "8";

				Put(pString, nSize);
				L_PRINT(LOG_INFO) << "9";

				pString = pEndl + 1;
				if (*pString)
				{
					L_PRINT(LOG_INFO) << "10";

					PutTabs();
					L_PRINT(LOG_INFO) << "11";

					pEndl = strchr(pString, '\n');
					L_PRINT(LOG_INFO) << "12";

				}
				else
				{
					pEndl = NULL;
				}
			}
		}
		size_t nLen = strlen(pString);
		if (nLen)
		{
			L_PRINT(LOG_INFO) << "Mem 1";
			Put(pString, nLen * sizeof(char));
			L_PRINT(LOG_INFO) << "FINISHED";
		}
	}
}


//-----------------------------------------------------------------------------
// This version of PutString converts \ to \\ and " to \", etc.
// It also places " at the beginning and end of the string
//-----------------------------------------------------------------------------
inline void CUtlBuffer::PutDelimitedCharInternal(CUtlCharConversion* pConv, char c)
{
	size_t l = pConv->GetConversionLength(c);
	if (l == 0)
	{
		PutChar(c);
	}
	else
	{
		PutChar(pConv->GetEscapeChar());
		Put(pConv->GetConversionString(c), l);
	}
}

void CUtlBuffer::PutDelimitedChar(CUtlCharConversion* pConv, char c)
{
	if (!IsText() || !pConv)
	{
		PutChar(c);
		return;
	}

	PutDelimitedCharInternal(pConv, c);
}

void CUtlBuffer::PutDelimitedString(CUtlCharConversion* pConv, const char* pString)
{
	if (!IsText() || !pConv)
	{
		PutString(pString);
		return;
	}

	if (WasLastCharacterCR())
	{
		PutTabs();
	}
	Put(pConv->GetDelimiter(), pConv->GetDelimiterLength());

	size_t nLen = pString ? strlen(pString) : 0;
	for (size_t i = 0; i < nLen; ++i)
	{
		PutDelimitedCharInternal(pConv, pString[i]);
	}

	if (WasLastCharacterCR())
	{
		PutTabs();
	}
	Put(pConv->GetDelimiter(), pConv->GetDelimiterLength());
}


void CUtlBuffer::VaPrintf(const char* pFmt, va_list list)
{
	char temp[8192];
	//size_t nLen = V_vsnprintf(temp, sizeof(temp), pFmt, list);
	//ErrorIfNot(nLen < sizeof(temp), ("CUtlBuffer::VaPrintf: String overflowed buffer [%d]\n", sizeof(temp)));
	PutString(temp);
}

void CUtlBuffer::Printf(const char* pFmt, ...)
{
	va_list args;

	va_start(args, pFmt);
	VaPrintf(pFmt, args);
	va_end(args);
}


//-----------------------------------------------------------------------------
// Calls the overflow functions
//-----------------------------------------------------------------------------
void CUtlBuffer::SetOverflowFuncs(UtlBufferOverflowFunc_t getFunc, UtlBufferOverflowFunc_t putFunc)
{
	m_GetOverflowFunc = getFunc;
	m_PutOverflowFunc = putFunc;
}


//-----------------------------------------------------------------------------
// Calls the overflow functions
//-----------------------------------------------------------------------------
bool CUtlBuffer::OnPutOverflow(size_t nSize)
{
	return (this->*m_PutOverflowFunc)(nSize);
}

bool CUtlBuffer::OnGetOverflow(size_t nSize)
{
	return (this->*m_GetOverflowFunc)(nSize);
}


//-----------------------------------------------------------------------------
// Checks if a put is ok
//-----------------------------------------------------------------------------
bool CUtlBuffer::PutOverflow(size_t nSize)
{
	//MEM_ALLOC_CREDIT();

	if (m_Memory.IsExternallyAllocated())
	{
		if (!IsGrowable())
			return false;

		m_Memory.ConvertToGrowableMemory(0);
	}

	while (Size() < m_Put - m_nOffset + nSize)
	{
		m_Memory.Grow();
	}

	return true;
}

bool CUtlBuffer::GetOverflow(size_t nSize)
{
	return false;
}

//-----------------------------------------------------------------------------
bool CUtlBuffer::CheckPut(size_t nSize)
{
	if ((m_Error & PUT_OVERFLOW) || IsReadOnly())
		return false;

	if ((m_Put < m_nOffset) || (m_Memory.AllocationNum() < m_Put - m_nOffset + nSize))
	{
		if (!OnPutOverflow(nSize))
		{
			m_Error |= PUT_OVERFLOW;
			return false;
		}
	}
	return true;
}
void CUtlBuffer::SeekPut(SeekType_t type, size_t offset)
{
	size_t nNextPut = m_Put;
	switch (type)
	{
	case SEEK_HEAD:
		nNextPut = offset;
		break;

	case SEEK_CURRENT:
		nNextPut += offset;
		break;

	case SEEK_TAIL:
		nNextPut = m_nMaxPut - offset;
		break;
	}

	// Force a write of the data
	// FIXME: We could make this more optimal potentially by writing out
	// the entire buffer if you seek outside the current range

	// NOTE: This call will write and will also seek the file to nNextPut.
	OnPutOverflow(-nNextPut - 1);
	m_Put = nNextPut;

	AddNullTermination(m_Put);
}


void CUtlBuffer::ActivateByteSwapping(bool bActivate)
{
	m_Byteswap.ActivateByteSwapping(bActivate);
}

void CUtlBuffer::SetBigEndian(bool bigEndian)
{
	m_Byteswap.SetTargetBigEndian(bigEndian);
}

bool CUtlBuffer::IsBigEndian(void)
{
	return m_Byteswap.IsTargetBigEndian();
}


//-----------------------------------------------------------------------------
// null terminate the buffer
// NOTE: Pass in nPut here even though it is just a copy of m_Put.  This is almost always called immediately 
// after modifying m_Put and this lets it stay in a register and avoid LHS on PPC.
//-----------------------------------------------------------------------------
void CUtlBuffer::AddNullTermination(size_t nPut)
{
	if (nPut > m_nMaxPut)
	{
		if (!IsReadOnly() && ((m_Error & PUT_OVERFLOW) == 0))
		{
			// Add null termination value
			if (CheckPut(1))
			{
				m_Memory[nPut - m_nOffset] = 0;
			}
			else
			{
				// Restore the overflow state, it was valid before...
				m_Error &= ~PUT_OVERFLOW;
			}
		}
		m_nMaxPut = nPut;
	}
}

//-----------------------------------------------------------------------------
// Converts a buffer from a CRLF buffer to a CR buffer (and back)
// Returns false if no conversion was necessary (and outBuf is left untouched)
// If the conversion occurs, outBuf will be cleared.
//-----------------------------------------------------------------------------
bool CUtlBuffer::ConvertCRLF(CUtlBuffer& outBuf)
{
	if (!IsText() || !outBuf.IsText())
		return false;

	if (ContainsCRLF() == outBuf.ContainsCRLF())
		return false;

	size_t nInCount = TellMaxPut();

	outBuf.Purge();
	outBuf.EnsureCapacity(nInCount);

	bool bFromCRLF = ContainsCRLF();

	// Start reading from the beginning
	size_t nGet = TellGet();
	size_t nPut = TellPut();
	size_t nGetDelta = 0;
	size_t nPutDelta = 0;

	const char* pBase = (const char*)Base();
	intptr_t nCurrGet = 0;
	while (nCurrGet < nInCount)
	{
		const char* pCurr = &pBase[nCurrGet];
		if (bFromCRLF)
		{
			const char* pNext = V_strnistr(pCurr, "\r\n", (size_t)nInCount - (size_t)nCurrGet);
			if (!pNext)
			{
				outBuf.Put(pCurr, nInCount - nCurrGet);
				break;
			}

			intptr_t nBytes = (intptr_t)pNext - (intptr_t)pCurr;
			outBuf.Put(pCurr, nBytes);
			outBuf.PutChar('\n');
			nCurrGet += nBytes + 2;
			if (nGet >= nCurrGet - 1)
			{
				--nGetDelta;
			}
			if (nPut >= nCurrGet - 1)
			{
				--nPutDelta;
			}
		}
		else
		{
			const char* pNext = V_strnchr(pCurr, '\n', (size_t)nInCount - (size_t)nCurrGet);
			if (!pNext)
			{
				outBuf.Put(pCurr, (size_t)nInCount - (size_t)nCurrGet);
				break;
			}

			intptr_t nBytes = (intptr_t)pNext - (intptr_t)pCurr;
			outBuf.Put(pCurr, (size_t)nBytes);
			outBuf.PutChar('\r');
			outBuf.PutChar('\n');
			nCurrGet += nBytes + 1;
			if (nGet >= nCurrGet)
			{
				++nGetDelta;
			}
			if (nPut >= nCurrGet)
			{
				++nPutDelta;
			}
		}
	}


	outBuf.SeekGet(SEEK_HEAD, nGet + nGetDelta);
	outBuf.SeekPut(SEEK_HEAD, nPut + nPutDelta);

	return true;
}

//-----------------------------------------------------------------------------
// Fast swap
//-----------------------------------------------------------------------------
void CUtlBuffer::Swap(CUtlBuffer& buf)
{
	V_swap(m_Get, buf.m_Get);
	V_swap(m_Put, buf.m_Put);
	V_swap(m_nMaxPut, buf.m_nMaxPut);
	V_swap(m_Error, buf.m_Error);
	m_Memory.Swap(buf.m_Memory);
}


//-----------------------------------------------------------------------------
// Fast swap w/ a CUtlMemory.
//-----------------------------------------------------------------------------
void CUtlBuffer::Swap(CUtlMemory<uint8_t>& mem)
{
	m_Get = 0;
	m_Put = mem.AllocationCount();
	m_nMaxPut = mem.AllocationCount();
	m_Error = 0;
	m_Memory.Swap(mem);
}


//---------------------------------------------------------------------------
// Implementation of CUtlInplaceBuffer
//---------------------------------------------------------------------------


bool CUtlInplaceBuffer::InplaceGetLinePtr(char** ppszInBufferPtr, size_t* pnLineLength)
{

	size_t nLineLen = PeekLineLength();
	if (nLineLen <= 1)
	{
		SeekGet(SEEK_TAIL, 0);
		return false;
	}

	--nLineLen; // because it accounts for putting a terminating null-character

	char* pszLine = (char*) const_cast<void*>(PeekGet());
	SeekGet(SEEK_CURRENT, nLineLen);

	// Set the out args
	if (ppszInBufferPtr)
		*ppszInBufferPtr = pszLine;

	if (pnLineLength)
		*pnLineLength = nLineLen;

	return true;
}

char* CUtlInplaceBuffer::InplaceGetLinePtr(void)
{
	char* pszLine = NULL;
	size_t nLineLen = 0;

	if (InplaceGetLinePtr(&pszLine, &nLineLen))
	{

		switch (pszLine[nLineLen - 1])
		{
		case '\n':
		case '\r':
			pszLine[nLineLen - 1] = 0;
			if (--nLineLen)
			{
				switch (pszLine[nLineLen - 1])
				{
				case '\n':
				case '\r':
					pszLine[nLineLen - 1] = 0;
					break;
				}
			}
			break;

		default:
			break;
		}
	}

	return pszLine;
}
/*
inline  KeyValues3*(CS_STDCALL* fnSetType)(std::int64_t, std::int64_t) = nullptr;

KeyValues3::KeyValues3()
{
}

KeyValues3::~KeyValues3()
{
}
inline 	bool(__fastcall* LoadKV3)(KeyValues3*, void*, const char*, const KV3ID_t*, const char*) = nullptr;

bool __fastcall KeyValues3::loadkv3(KeyValues3* kv3,const char* buf,   KV3ID_t kvid)
{
	const void* hTier0 = MEM::GetModuleBaseHandle(TIER0_DLL);

	LoadKV3 = reinterpret_cast<decltype(LoadKV3)>(MEM::GetExportAddress(hTier0, CS_XOR("?LoadKV3@@YA_NPEAVKeyValues3@@PEAVCUtlString@@PEBDAEBUKV3ID_t@@2@Z")));
	CS_ASSERT(LoadKV3 != nullptr);
	CS_ASSERT(kv3 != nullptr);
	CS_ASSERT(kv3->pValue != nullptr);

	return LoadKV3(kv3, nullptr, buf, &kvid, nullptr);
}
KeyValues3* KeyValues3::set_type(kv_basic_type a1, kv_basic_type a2)
{
	fnSetType = reinterpret_cast<decltype(fnSetType)>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B C4 55 48 8D A8 08 FD")));
	CS_ASSERT(fnSetType != nullptr);
	return fnSetType(a1, a2);
}
inline 	bool(__fastcall* fnLoadfromBuffer)(KeyValues3*, const char*, CUtlBuffer*, void*, const char*, bool(_stdcall*)(const char*, void*), void*, void*, const char*, bool) = nullptr;
inline 	bool(__fastcall* fnLoadfromBufferchar)(KeyValues3*, const char*, const char*, void*, const char*, bool(_stdcall*)(const char*, void*), void*, void*, const char*, bool) = nullptr;

bool KeyValues3::load_from_buffer_char(const char* buf)
{
	const void* hTier0 = MEM::GetModuleBaseHandle(TIER0_DLL);

	fnLoadfromBufferchar = reinterpret_cast<decltype(fnLoadfromBufferchar)>(MEM::GetExportAddress(hTier0, CS_XOR("?LoadFromBuffer@KeyValues@@QEAA_NPEBD0PEAVIFileSystem@@0P6A_N0PEA")));
	CS_ASSERT(fnLoadfromBufferchar != nullptr);

	return fnLoadfromBufferchar(this, 0, buf, 0, 0, 0, 0, 0, 0, 0);
}
bool KeyValues3::load_from_buffer(CUtlBuffer* buf)
{
	const void* hTier0 = MEM::GetModuleBaseHandle(TIER0_DLL);

	fnLoadfromBuffer = reinterpret_cast<decltype(fnLoadfromBuffer)>(MEM::GetExportAddress(hTier0, CS_XOR("?LoadFromBuffer@KeyValues@@QEAA_NPEBDAEAVCUtlBuffer@@PEAVIFileSystem@@0P6A_N0PEAX@ZPEAVIKeyValuesErrorSpew@@30@Z")));
	CS_ASSERT(fnLoadfromBuffer != nullptr);

	return fnLoadfromBuffer(this, 0, buf, 0, 0, 0, 0, 0, 0, 0);
}

bool KeyValues3::load_kv3(CUtlBuffer* buf, const char* material, KV3ID_t kvid)
{
	const void* hTier0 = MEM::GetModuleBaseHandle(TIER0_DLL);

	LoadKv3 = reinterpret_cast<decltype(LoadKv3)>(MEM::GetExportAddress(hTier0, CS_XOR("?LoadKV3@@YA_NPEAVKeyValues3@@PEAVCUtlString@@PEAVCUtlBuffer@@AEBUKV3ID_t@@PEBD@Z")));
	CS_ASSERT(LoadKv3 != nullptr);

	return LoadKv3(this, material, buf, &kvid, nullptr);
}*/
inline static constexpr int k_unk_key_hash = 0x31415926;
inline static constexpr std::uint64_t k_kv_id_unk_hash1 = 0x41B818518343427E;
inline static constexpr std::uint64_t k_kv_id_unk_hash2 = 0xB5F447C23C0CDF8C;
inline bool(_fastcall* LoadKv3)(c_key_values*, void*, c_ult_buffer*, kv3_id_t*, void*, void*, void*, void*, const char*) = nullptr;
inline void(__fastcall* utlbuf)(c_ult_buffer*, int, int, int) = nullptr;
inline void(__fastcall* putstring)(c_ult_buffer*, const char*) = nullptr;
inline c_key_values*(__fastcall* settype)(c_key_values*, std::uint8_t, std::uint8_t) = nullptr;

c_ult_buffer::c_ult_buffer(int a1, int size, int a3) {


	utlbuf = reinterpret_cast<decltype(utlbuf)>(MEM::FindPattern(TIER0_DLL, CS_XOR("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 41 56 48 83 EC 20 33 DB 44 89 41 08")));
	CS_ASSERT(utlbuf != nullptr);
	utlbuf(this, a1, size, a3);
}

void c_ult_buffer::put_string(std::string_view string) {
	putstring = reinterpret_cast<decltype(putstring)>(MEM::ResolveRelativeAddress(MEM::FindPattern(TIER0_DLL, CS_XOR("E8 ?? ?? ?? ?? EB 57")), 1, 0));
	CS_ASSERT(putstring != nullptr);
	putstring(this, string.data());
}

c_key_values* create_material_resource() {
	c_key_values* kv = new c_key_values[2 * sizeof(std::uintptr_t)];

	settype = reinterpret_cast<decltype(settype)>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ?? ?? ?? ?? 66 85 DB")), 0x1, 0x0));
	CS_ASSERT(settype != nullptr);
	return settype(kv, 1, 6);
}
c_key_values* c_key_values::set_type()
{
	settype = reinterpret_cast<decltype(settype)>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 57 48 83 EC 30 48 0F")));
	CS_ASSERT(settype != nullptr);
	return settype(this, 1, 6);
}
bool c_key_values::load_kv3(c_ult_buffer* buffer) {

	LoadKv3 = reinterpret_cast<decltype(LoadKv3)>(MEM::ResolveRelativeAddress(MEM::FindPattern(TIER0_DLL, CS_XOR("E8 ?? ?? ?? ?? EB 36 8B 43 10")), 0x1, 0x0));
	CS_ASSERT(LoadKv3 != nullptr);

	kv3_id_t type{ (const char*)"generic", k_kv_id_unk_hash1, k_kv_id_unk_hash2 };
	return LoadKv3(this, nullptr, buffer, &type, nullptr, nullptr, nullptr, nullptr, "");
}

void c_key_values::load_from_buffer(std::string_view string) {
	c_ult_buffer buffer(0, (int)string.size() + 10, 1);
	buffer.put_string(string.data());
	load_kv3(&buffer);
}
