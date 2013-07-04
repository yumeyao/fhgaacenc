//
// common.h
//
// Copyright 2011 tmkk. All rights reserved.
//

#pragma once

#define VERSION 20120624

#define DL_LIBSNDFILE
#define USE_FREE_FOR_DELETE

#ifdef _MSC_VER
#define fseeko _fseeki64
#define ftello _ftelli64
#endif

#if _MSC_FULL_VER > 13009037
#pragma intrinsic(_byteswap_ulong)
#pragma intrinsic(_byteswap_ushort)
#define SWAP32(n) _byteswap_ulong(n)
#define SWAP16(n) _byteswap_ushort(n)
#else
#define SWAP32(n) ((((n)>>24)&0xff) | (((n)>>8)&0xff00) | (((n)<<8)&0xff0000) | (((n)<<24)&0xff000000))
#define SWAP16(n) ((((n)>>8)&0xff) | (((n)<<8)&0xff00))
#endif

#ifdef __cplusplus
#ifdef USE_FREE_FOR_DELETE
__forceinline void __cdecl operator delete( void *pv ) {
	free(pv);
}
#endif
#endif

typedef enum
{
	kModeVBR = 0,
	kModeCBR
} codecMode;

typedef enum
{
	kProfileAuto = 0,
	kProfileLC,
	kProfileHE,
	kProfileHEv2
} codecProfile;

typedef struct
{
	_TCHAR *inFile;
	_TCHAR *outFile;
	codecMode mode;
	int modeQuality;
	codecProfile profile;
	bool quiet;
	bool readFromStdin;
	bool ignoreLength;
	bool adtsMode;
} encodingParameters;

typedef enum
{
	kPCMTypeSignedInt = 0,
	kPCMTypeUnsignedInt,
	kPCMTypeFloat
} PCMType;

class AudioCoder
{
  public:
    AudioCoder() { }
    virtual int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)=0; //returns bytes in out
    virtual int hoge(void);
    virtual ~AudioCoder() { };
};
