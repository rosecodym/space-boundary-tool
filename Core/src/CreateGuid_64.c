// CreateGuid_64.c
// provided "as-is", no warranty and support is given to the users of this code
// ----------------------------------------------------------------------------
//
// Creating of a GUID string using the CoCreateGuid function from Microsoft
// as originally proposed by Jim Forester
// implemented previously by Jeremy Tammik using hex-encoding
//
// This version uses a number system with base 64 to obtain a string with 22 characters.
// Also provides conversion routines for the mapping of the base 85 strings (20 chars).
//
// Peter Muigg, June 1998
// Janos Maros, July 2000
//

#if defined _MSC_VER
//#pragma warning (disable: 4115)
#endif

#include    <Windows.h>

#if defined _MSC_VER
//#pragma warning (default: 4115)
#endif

#include    <StdIO.h>
#include    <String.h>

#include    "CreateGuid_64.h"

//
// For non windows platforms you probably will need
// the following definitions instead of <Windows.h>
// to use this module:
//
//
// typedef struct _GUID {          // size is 16
//    unsigned long Data1;
//    unsigned short  Data2;
//    unsigned short  Data3;
//    unsigned char Data4[8];
// } GUID;
//
// typedef long BOOL;
//
//
// #define FALSE    0
// #define TRUE     1
//
// const GUID GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
// static long CoCreateGuid( GUID   *pGuid );
//
// (CoCreateGuid should be written to produce the globally unique data.)
//

static char * getString16FromGuid( const GUID   *pGuid, char * buf, int len );

static char * getString64FromGuid( const GUID *pGuid, char * buf, int len );
static BOOL  getGuidFromString64( const char *string, GUID *pGuid );
static BOOL cv_to_64( const unsigned long number, char *code, int len );
static BOOL cv_from_64( unsigned long *pRes, const char *str );

static BOOL  getGuidFromString85( const char    *string, GUID *pGuid );
static char * getString85FromGuid( const GUID *pGuid, char * buf, int len );
static BOOL cv_to_85( const unsigned long number, char *code, int len );
static BOOL cv_from_85( unsigned long *pRes, const char *str );

static const char *cConversionTable85 =
//          1         2         3         4         5         6         7         8
//0123456789012345678901234567890123456789012345678901234567890123456789012345678901234
 "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&^|*+,-./:;<=>?~`@_";

static const char *cConversionTable =
//          1         2         3         4         5         6   
//0123456789012345678901234567890123456789012345678901234567890123
 "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_$";

// Conversion Table must start with "0", this is the digit for the additive unit


//
// Creation of the string representing the GUID, the buffer must be able
// to hold 22 characters + 1 for the terminating 0
//
char * CreateCompressedGuidString( char * buf, int len )
{
    GUID                guid;

    guid = GUID_NULL;

   //
   // Call to the function from Microsoft
   //
    CoCreateGuid (&guid);

    if (memcmp (&GUID_NULL, &guid, sizeof (GUID)) == 0) {
        return NULL;
    }
    return getString64FromGuid (&guid, buf, len);
}

//
// Mapping the base 64 string to the conventional GUID string.
//
char * String64_To_HexaGuidString( const char *string64, char * buf, int len )
{
    GUID    guid;

    if (getGuidFromString64 (string64, &guid)) {
        return getString16FromGuid (&guid, buf, len);
    }
    return NULL;
}

//
// Mapping the base 64 string to the base 85 string through GUID.
//
char * String64_To_String85( const char *string64, char * buf, int len )
{
    GUID    guid;

    if (getGuidFromString64 (string64, &guid)) {
        return getString85FromGuid (&guid, buf, len);
    }
    return NULL;
}

//
// Mapping the base 85 string to the base 64 string through GUID.
//
char * String85_To_String64( const char *string85, char * buf, int len )
{
    GUID    guid;

    if (getGuidFromString85 (string85, &guid)) {
        return getString64FromGuid (&guid, buf, len);
    }
    return NULL;
}


//
// Conversion of a GUID to a string representing the GUID, the buffer must be able
// to hold 38 characters + 1 for the terminating 0 (6 extra characters for the separation)
//
char * getString16FromGuid( const GUID  *pGuid, char * buf, int len )
{
    if (len < 39) {
        return NULL;
    }
    sprintf (buf, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                pGuid->Data1,
                pGuid->Data2,
                pGuid->Data3,
                pGuid->Data4[0],
                pGuid->Data4[1],
                pGuid->Data4[2],
                pGuid->Data4[3],
                pGuid->Data4[4],
                pGuid->Data4[5],
                pGuid->Data4[6],
                pGuid->Data4[7]);
    return buf;
}


//
// Conversion of a GUID to a string representing the GUID, the buffer must be able
// to hold 22 characters + 1 for the terminating 0
//
char * getString64FromGuid( const GUID *pGuid, char * buf, int len )
{
    unsigned long   num[6];
    char            str[6][5];
    int             i, n;

    if (len < 23) {
        return NULL;
    }

    //
    // Creation of six 32 Bit integers from the components of the GUID structure
    //
    num[0] = (unsigned long) (pGuid->Data1 / 16777216);                                                 //    16. byte  (pGuid->Data1 / 16777216) is the same as (pGuid->Data1 >> 24)
    num[1] = (unsigned long) (pGuid->Data1 % 16777216);                                                 // 15-13. bytes (pGuid->Data1 % 16777216) is the same as (pGuid->Data1 & 0xFFFFFF)
    num[2] = (unsigned long) (pGuid->Data2 * 256 + pGuid->Data3 / 256);                                 // 12-10. bytes
    num[3] = (unsigned long) ((pGuid->Data3 % 256) * 65536 + pGuid->Data4[0] * 256 + pGuid->Data4[1]);  // 09-07. bytes
    num[4] = (unsigned long) (pGuid->Data4[2] * 65536 + pGuid->Data4[3] * 256 + pGuid->Data4[4]);       // 06-04. bytes
    num[5] = (unsigned long) (pGuid->Data4[5] * 65536 + pGuid->Data4[6] * 256 + pGuid->Data4[7]);       // 03-01. bytes
    //
    // Conversion of the numbers into a system using a base of 64
    //
    buf[0]='\0';
    n = 3;
    for (i = 0; i < 6; i++) {
        if (!cv_to_64 (num[i], str[i], n)) {
            return NULL;
        }
        strcat (buf, str[i]);
        n = 5;
    }
    return buf;
}

//
// Reconstruction of the GUID structure from the coded string
//
BOOL  getGuidFromString64( const char   *string, GUID *pGuid )
{
    char            str[6][5];
    int             len, i, j, m;
    unsigned long   num[6];

    len = (int) strlen (string);
    if (len != 22)
        return FALSE;

    j = 0;
    m = 2;

    for (i = 0; i < 6; i++) {
        strncpy (str[i], &string[j], m);
        str[i][m]= '\0';
        j = j + m;
        m = 4;
    }
    for (i = 0; i < 6; i++) {
        if (!cv_from_64 (&num[i], str[i])) {
            return FALSE;
        }
    }

    pGuid->Data1= (unsigned long) (num[0] * 16777216 + num[1]);                 // 16-13. bytes
    pGuid->Data2= (unsigned short) (num[2] / 256);                              // 12-11. bytes
    pGuid->Data3= (unsigned short) ((num[2] % 256) * 256 + num[3] / 65536);     // 10-09. bytes

    pGuid->Data4[0] = (unsigned char) ((num[3] / 256) % 256);                   //    08. byte
    pGuid->Data4[1] = (unsigned char) (num[3] % 256);                           //    07. byte
    pGuid->Data4[2] = (unsigned char) (num[4] / 65536);                         //    06. byte
    pGuid->Data4[3] = (unsigned char) ((num[4] / 256) % 256);                   //    05. byte
    pGuid->Data4[4] = (unsigned char) (num[4] % 256);                           //    04. byte
    pGuid->Data4[5] = (unsigned char) (num[5] / 65536);                         //    03. byte
    pGuid->Data4[6] = (unsigned char) ((num[5] / 256) % 256);                   //    02. byte
    pGuid->Data4[7] = (unsigned char) (num[5] % 256);                           //    01. byte

    return TRUE;
}

//
// Conversion of an integer into a number with base 64
// using the coside table cConveronTable
//
BOOL cv_to_64( const unsigned long number, char *code, int len )
{
    unsigned long   act;
    int             iDigit, nDigits;
    char            result[5];

    if (len > 5)
        return FALSE;

    act = number;
    nDigits = len - 1;

    for (iDigit = 0; iDigit < nDigits; iDigit++) {
        result[nDigits - iDigit - 1] = cConversionTable[(int) (act % 64)];
        act /= 64;
    }
    result[len - 1] = '\0';

    if (act != 0)
        return FALSE;

    strcpy (code, result);
    return TRUE;
}

//
// The reverse function to calculate the number from the code
//
BOOL cv_from_64( unsigned long *pRes, const char *str )
{
    int len, i, j, index;

    len= (int) strlen (str);
    if (len > 4)
        return FALSE;

    *pRes=0;

    for (i = 0; i < len; i++) {
        index = -1;
        for (j = 0; j < 64; j++) {
            if (cConversionTable[j] == str[i]) {
               index = j;
               break;
            }
        }
        if (index == -1)
            return FALSE;
        *pRes = *pRes * 64 + index;
    }
    return TRUE;
}

//
// Conversion of a GUID to a string representing the GUID, the buffer must be able
// to hold 20 characters + 1 for the terminating 0
//
char * getString85FromGuid( const GUID *pGuid, char * buf, int len )
{
   unsigned long num[4];
   char str[4][6];
   int i;

   if (pGuid == NULL || len < 21)
      return NULL;

   //
   // Creation of four 32 Bit integer from the components of the GUID structure
   //
   num[0] = (unsigned long) pGuid->Data1;
   num[1] = (unsigned long) (pGuid->Data2  * 65536 + pGuid->Data3);
   num[2] = (unsigned long) (
                             pGuid->Data4[0] +
                             pGuid->Data4[1] * 256 +
                             pGuid->Data4[2] * 65536 +
                             pGuid->Data4[3] * 16777216
                            );
   num[3] = (unsigned long) (
                             pGuid->Data4[4] +
                             pGuid->Data4[5] * 256 +
                             pGuid->Data4[6] * 65536 +
                             pGuid->Data4[7] * 16777216
                            );
   //
   // Conversion of the numbers into a system using a base of 85
   //
   buf[0]='\0';
   for (i = 0; i < 4; i++) {
      if (!cv_to_85 (num[i], str[i], 6))
         return NULL;
      strcat (buf, str[i]);
   }
   return buf;
}

//
// Reconstruction of the GUID structure from the coded string
//
BOOL  getGuidFromString85( const char   *string, GUID *pGuid )
{
    char str[4][6];
    int len, i;
    unsigned long data, num[4];

    len = (int) strlen (string);
    if (len != 20)
        return FALSE;
    for (i = 0; i < 4; i++) {
        strncpy (str[i], &string[i*5], 5);
        str[i][5]= '\0';
    }
    for (i = 0; i < 4; i++) {
        if (!cv_from_85 (&num[i], str[i]))
            return FALSE;
    }
    pGuid->Data1= (unsigned long) num[0];
    data= num[1];
    data = data >> 16;
    pGuid->Data2= (unsigned short) data;
    pGuid->Data3= (unsigned short) (num[1] & 65535);
    data=num[2];
    for (i = 0; i < 4; i++) {
        pGuid->Data4[i]= (unsigned char) (data & 255);
        data= data >> 8;
    }
    data=num[3];
    for (i = 4; i < 8; i++) {
        pGuid->Data4[i]= (unsigned char) (data & 255);
        data= data >> 8;
    }
    return TRUE;
}

//
// Conversion of an integer into a number with base 85
// using the code table cConversionTable85
//
BOOL cv_to_85( const unsigned long number, char *code, int len )
{
    unsigned long  act;
    int  z_array[5], ndig, i, zeroes;
    char result[6];

    if (len < 6)
        return FALSE;
    ndig=0;
    act=number;
    do {
        if (ndig > 4)
            return FALSE;
        z_array[ndig]= (int) (act % 85);
        act /= 85;
        ndig++;
    } while (act > 0);
    ndig--;
    zeroes= 4 - ndig;
    for (i = ndig; i >= 0; i--) {
        result[zeroes + ndig-i]= cConversionTable85[z_array[i]];
    }
    //
    // Filling in leading zeroes
    //
    for(i = 0; i < zeroes; i++) {
        result[i]= '0';
    }
    result[5]='\0';
    strcpy (code, result);
    return TRUE;
}

//
// The reverse function to calculate the number from the code
//
BOOL cv_from_85( unsigned long *pRes, const char *str )
{
    int len, i, j, index;
    unsigned long fact;

    len= (int) strlen (str);
    if (len > 5)
    return FALSE;

    *pRes=0;
    fact=1;
    for (i = len - 1; i >= 0; i--) {
        index=-1;
        for (j = 0; j < 85; j++) {
            if (cConversionTable85[j] == str[i]) {
                index=j;
                break;
            }
        }
        if (index == -1)
            return FALSE;
        *pRes+= index * fact;
        fact *= 85;
    }
    return TRUE;
}
