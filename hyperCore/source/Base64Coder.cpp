#include "stdafx.h"
#include <cstdlib>
#include <assert.h>
#include <memory.h>

namespace HyperCore
{

// Note that we can't use the = sign as a padding character,
// since when parsing the line in our resource collection
// loading code, we strip all = signs... Instead,
// use something else we don't strip...
static const char STANDARD_BASE64_PAD_CHAR = '=';
static const char OWN_BASE64_PAD_CHAR = '*';

const char toBase64Map[] = {
	'A', // 0
	'B', // 1
	'C', // 2
	'D', // 3
	'E', // 4
	'F', // 5
	'G', // 6
	'H', // 7
	'I', // 8
	'J', // 9
	'K', // 10
	'L', // 11
	'M', // 12
	'N', // 13
	'O', // 14
	'P', // 15
	'Q', // 16
	'R', // 17
	'S', // 18
	'T', // 19
	'U', // 20
	'V', // 21
	'W', // 22
	'X', // 23
	'Y', // 24
	'Z', // 25
	'a', // 26
	'b', // 27
	'c', // 28
	'd', // 29
	'e', // 30
	'f', // 31
	'g', // 32
	'h', // 33
	'i', // 34
	'j', // 35
	'k', // 36
	'l', // 37
	'm', // 38
	'n', // 39
	'o', // 40
	'p', // 41
	'q', // 42
	'r', // 43
	's', // 44
	't', // 45
	'u', // 46
	'v', // 47
	'w', // 48
	'x', // 49
	'y', // 50
	'z', // 51
	'0', // 52
	'1', // 53
	'2', // 54
	'3', // 55
	'4', // 56
	'5', // 57
	'6', // 58
	'7', // 59
	'8', // 60
	'9', // 61
	'+', // 62
	'/', // 63
};

unsigned char fromBase64Map[] = {
	0, // push everything up by one since lookup is 1-base and not 0-based
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40
	0,0,62,0,0,0,63,52,53,54,55,56,57,58,59,60,61,0,0,0,0/*pad*/,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15, // 80
	16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49, // 120
	50,51,
};

// Convert 3 bytes of 8 bits to 4 bytes of 6 bits.
#define ToB64Index1( byte1 )			( byte1 >> 2 ) // 6 msb byte1 -> 6 lsb out
#define ToB64Index2( byte1, byte2 )	( ( ( byte1 & 0x3 ) << 4 ) | ( ( byte2 & 0xF0 ) >> 4 ) ) // 2 lsb byte1 -> 2msb out. 4 msb byte2 -> 4 lsb out
#define ToB64Index3( byte2, byte3 )	( ( ( byte2 & 0xF ) << 2 ) | ( ( byte3 & 0xC0 ) >> 6 ) ) // 4 lsb byte2 -> 4 msb out. 2 msb byte3 -> 2 lsb out
#define ToB64Index4( byte3 )			( byte3 & 0x3F ) // 6 lowest bits of byte3

// Convert 4 bytes of 6 bits to 3 bytes of 8 bits
#define FromB64Byte1( byte1, byte2 )	( ( byte1 << 2 ) | ( byte2 >> 4 ) ) // 6 lsb of byte1 -> 6 msb out. 2 msb byte2 -> 2 lsb out
#define FromB64Byte2( byte2, byte3 )	( ( ( byte2 & 0xF ) << 4 ) | ( ( byte3 & 0x3C ) >> 2 ) ) // 4 lsb byte2 -> 4 msb out. 4 msb byte3 - 4 lsb out
#define FromB64Byte3( byte3, byte4 )	( ( ( byte3 & 0x3 ) << 6 ) | ( byte4 & 0x3F ) ) // 2 lsb byte3 -> 2 msb out. 6 lsb byte4 -> 6 lsb out

unsigned int	GetBase64Size( unsigned int dataSize ) { return ( ( dataSize + 2 ) / 3 ) * 4; }
unsigned int	GetDataSize( unsigned int base64Size ) 
{ 
	_ASSERT( base64Size / 4.0f == static_cast< float > ( base64Size / 4 ) );
	return ( base64Size / 4 ) * 3;
}

/*****************************************************************************/
char* Base64Coder::encode(const unsigned char* data, unsigned int dataSize, bool bUseStandardPaddingChar) 
{
	// free previously encoded data
	//Reset();

	const char cPadChar = bUseStandardPaddingChar ? STANDARD_BASE64_PAD_CHAR : OWN_BASE64_PAD_CHAR;

	// allocate string on heap that will contain our encoded data
	unsigned int	encodedSize = GetBase64Size( dataSize );
	_ASSERT( GetDataSize( encodedSize ) - dataSize <= 2 ); // we expect up to two additional padding characters
	char* encodedPtr = new char[ encodedSize + 1 ]; // + 1 because of null terminator
	char* pResEncoded = encodedPtr;

	// start encoding
	while ( dataSize > 2 ) {
		// encoded byte 1
		*encodedPtr++ = toBase64Map[ ToB64Index1( *data ) ];
		// encoded byte 2
		*encodedPtr++ = toBase64Map[ ToB64Index2( *data, *( data + 1 ) ) ];
		// encoded byte 3
		*encodedPtr++ = toBase64Map[ ToB64Index3( *( data + 1 ), *( data + 2 ) ) ];
		// encoded byte 4
		*encodedPtr++ = toBase64Map[ ToB64Index4( *( data + 2 ) ) ];

		data += 3;
		dataSize -= 3;
	}

	// write the rest and pad as necessary
	if ( dataSize > 0 ) {
		// encoded byte 1
		*encodedPtr++ = toBase64Map[ ToB64Index1( *data ) ];
		if ( dataSize > 1 ) {
			// encoded byte 2
			*encodedPtr++ = toBase64Map[ ToB64Index2( *data, *( data + 1 ) ) ];
			// encoded byte 3
			*encodedPtr++ = toBase64Map[ ToB64Index3( *( data + 1 ), 0 ) ];
		} else {
			// encoded byte 2
			*encodedPtr++ = toBase64Map[ ToB64Index2( *data, 0 ) ];
			// encoded byte 3
			*encodedPtr++ = cPadChar;
		}
		// encoded byte 4
		*encodedPtr++ = cPadChar;
	}

	// add null terminator to encoded data string
	*encodedPtr++ = '\0';
#ifdef _DEBUG
	unsigned int encodedSize2 = strlen(pResEncoded);
#endif

	return pResEncoded;
}
/*****************************************************************************/
char* Base64Coder::encode(const BinaryData* pData, bool bUseStandardPaddingChar)
{
	if(!pData)
		return "";
	return encode((unsigned char*)pData->getData(), pData->getLength(), bUseStandardPaddingChar);
}
/*****************************************************************************/
unsigned char* Base64Coder::decode(const char* encoded, unsigned int iSizeIn, unsigned int& dataSizeOut, bool bUseStandardPaddingChar)
{
	if (!encoded) 
		ASSERT_RETURN_NULL;

	long long encodedSize = iSizeIn;
#ifdef _DEBUG
	unsigned int encodedSizeDb = strlen(encoded);
#endif
	if(encodedSize <= 0)
		encodedSize = strlen(encoded);
	//unsigned int encodedSize = strlen(encoded); //  + 1;
	_ASSERT(encodedSize % 4 == 0 ); // the base 64 string should be a multiple of 4

	const char cPadChar = bUseStandardPaddingChar ? STANDARD_BASE64_PAD_CHAR : OWN_BASE64_PAD_CHAR;

	// Note that we need to take into account the padding in the form of = signs
	// in the original data. Otherwise, we may return larger size than necessary.
	unsigned int origEncodedSize = encodedSize;
	int iEqualsCount = 0;
	for(origEncodedSize--; encoded[origEncodedSize] == cPadChar; origEncodedSize--, iEqualsCount++);

	// Hack: At some point, we may have lost padded = signs due to our parsing in resColl. Pretend they're
	// there to calculate the decoded size.
	long long lEncSizeForDecodeComputation = encodedSize;
	while(lEncSizeForDecodeComputation % 4 != 0)
		lEncSizeForDecodeComputation++;

	unsigned int decodedSize = GetDataSize( lEncSizeForDecodeComputation ); // null terminator should not be part of encodedSize
	unsigned char* decodedPtr = new unsigned char[ decodedSize ];
	unsigned char* decoded = decodedPtr;

	while ( encodedSize > 0 ) 
	{
		// decoded byte 1
		*decodedPtr++ = FromB64Byte1( fromBase64Map[ *encoded ], fromBase64Map[ *( encoded + 1 ) ] );
		// decoded byte 2
		*decodedPtr++ = FromB64Byte2( fromBase64Map[ *( encoded + 1 ) ], fromBase64Map[ *( encoded + 2 ) ] );
		// decoded byte 3
		*decodedPtr++ = FromB64Byte3( fromBase64Map[ *( encoded + 2 ) ], fromBase64Map[ *( encoded + 3 ) ] );

		// Otherwise we wrap around, since encodedSize is unsigned.
		// Not anymore, but this is still a bad thing.
		_ASSERT(encodedSize >= 4);

		encoded += 4;
		encodedSize -= 4;
	}

	//data = decoded;
	dataSizeOut = decodedSize - iEqualsCount;

	return decoded;
}
/*****************************************************************************/
};