//
// FhGAACencoder.h
//
// Copyright 2011 tmkk. All rights reserved.
//

#pragma once

#include "common.h"
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#include <sndfile.h>
#include <mmsystem.h>

class FhGAACEncoder
{
  public:
	AudioCoder *encoder;
	FILE *fp;
	SNDFILE *sff;
	__int64 totalFrames;
	PCMType type;
	int samplerate;
	int bitPerSample;
	int channels;

	AudioCoder* (*finishAudio3)(_TCHAR *filename, AudioCoder *coder);
	void (*prepareToFinish)(_TCHAR *filename, AudioCoder *coder);
	AudioCoder* (*createAudio3)(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile);

	FhGAACEncoder();
	~FhGAACEncoder();
	bool openFile(_TCHAR *inFile);
	bool openStream(FILE *stream);
	__int64 beginEncode(_TCHAR *outFile, encodingParameters *params);

#ifdef DL_LIBSNDFILE
  private:
	HMODULE hLibsndfile;
#ifdef UNICODE
	SNDFILE* (*fn_sf_wchar_open)(LPCWSTR, int, SF_INFO *);
#else
	SNDFILE* (*fn_sf_open)(const char *, int, SF_INFO *);
#endif
	int (*fn_sf_close)(SNDFILE *);
	int (*fn_sf_error)(SNDFILE *);
	sf_count_t (*fn_sf_readf_int)(SNDFILE *, int *, sf_count_t);
  public:
	bool loadLibsndfile()
	{
#define CHECK(exp) do {if(!(exp)) return false;} while(0)
		HMODULE h;
		CHECK(h = LoadLibrary(_T("libsndfile-1.dll")));
		hLibsndfile = h;

		FARPROC fn;
#ifdef UNICODE
		CHECK(fn = GetProcAddress(h, "sf_wchar_open"));
		fn_sf_wchar_open = (SNDFILE* (*)(LPCWSTR, int, SF_INFO *))fn;
#else
		CHECK(fn = GetProcAddress(h, "sf_open"));
		fn_sf_open = (SNDFILE* (*)(const char *, int, SF_INFO *))fn;
#endif
		CHECK(fn = GetProcAddress(h, "sf_close"));
		fn_sf_close = (int (*)(SNDFILE *))fn;
		CHECK(fn = GetProcAddress(h, "sf_error"));
		fn_sf_error = (int (*)(SNDFILE *))fn;
		CHECK(fn = GetProcAddress(h, "sf_readf_int"));
		fn_sf_readf_int = (sf_count_t (*)(SNDFILE *, int *, sf_count_t))fn;

		return true;
	}
#else
	bool loadLibsndfile(){return true;}
#define fn_sf_wchar_open sf_wchar_open
#define fn_sf_open sf_open
#define fn_sf_close sf_close
#define fn_sf_error sf_error
#define fn_sf_readf_int sf_readf_int
#endif
};
