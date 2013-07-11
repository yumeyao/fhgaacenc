//
// fhgaacenc.cpp
//
// Copyright 2011 tmkk. All rights reserved.
//

#include "stdafx.h"
#include "common.h"
#include "FhGAACEncoder.h"

bool getPathForWinAmp(_TCHAR *path, unsigned int length)
{
	if(!path) return false;
	bool ret = false;
	HKEY hKey;
	_TCHAR *subKey = _T("Software\\Winamp");
	DWORD size;
	if(RegOpenKeyEx(HKEY_CURRENT_USER,subKey,0,KEY_READ,&hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey,NULL,NULL,NULL,NULL,&size);
		if(length > size) {
			if(RegQueryValueEx(hKey,NULL,NULL,NULL,(LPBYTE)path,&size) == ERROR_SUCCESS)
				ret = true;
		}
		RegCloseKey(hKey);
	}
	return ret;
}

#define _stringer(x) #x
#define stringer(x) _stringer(x)
#define printUsage()\
	fputs("fhgaacenc version " stringer(VERSION) " by tmkk\n"\
		"Usage: fhgaacenc.exe [options] infile [outfile]\n"\
		"  Note: pass - as infile to encode from stdin.\n"\
		"        pass - as outfile to encode to stdout (ADTS only).\n\n"\
		"  General encoding options\n"\
		"\t--cbr <bitrate> : encode in CBR mode, bitrate=8..576\n"\
		"\t--vbr <preset>  : encode in VBR mode, preset=1..6 [default]\n"\
		"\t--profile <auto|lc|he|hev2> : choose AAC profile (only for CBR mode)\n"\
		"\t    auto : automatically choose the optimum profile\n"\
		"\t           according to the bitrate [default]\n"\
		"\t    lc   : force use LC-AAC profile\n"\
		"\t    he   : force use HE-AAC (AAC+SBR) profile\n"\
		"\t    hev2 : force use HE-AAC v2 (AAC+SBR+PS) profile\n"\
		"\t--adts : use ADTS container instead of MPEG-4\n"\
		"  Other options \n"\
		"\t--ignorelength : ignore the size of data chunk when encoding from pipe\n"\
		"\t--quiet        : don't print the progress\n"\
		,stderr)

static void replaceSlashWithBackSlash(_TCHAR *str)
{
	int i;
	for(i=_tcslen(str)-1;i>=0;i--) {
		if(str[i] == _T('/')) str[i] = _T('\\');
	}
}

static int parseArguments(int argc, _TCHAR* argv[], encodingParameters *params)
{
	int i;
	for(i=1;i<argc;i++) {
		if(!_tcscmp(argv[i],_T("--quiet"))) params->quiet = true;
		else if(!_tcscmp(argv[i],_T("--cbr"))) {
			params->mode = kModeCBR;
			if(++i<argc) {
				params->modeQuality = _tstoi(argv[i]);
				if(params->modeQuality < 8) params->modeQuality = 8;
				else if(params->modeQuality > 576) params->modeQuality = 576;
			}
		}
		else if(!_tcscmp(argv[i],_T("--vbr"))) {
			params->mode = kModeVBR;
			if(++i<argc) {
				params->modeQuality = _tstoi(argv[i]);
				if(params->modeQuality < 1) params->modeQuality = 1;
				else if(params->modeQuality > 6) params->modeQuality = 6;
			}
		}
		else if(!_tcscmp(argv[i],_T("--profile"))) {
			if(++i<argc) {
				if(!_tcsicmp(argv[i],_T("auto"))) params->profile = kProfileAuto;
				else if(!_tcsicmp(argv[i],_T("lc"))) params->profile = kProfileLC;
				else if(!_tcsicmp(argv[i],_T("he"))) params->profile = kProfileHE;
				else if(!_tcsicmp(argv[i],_T("hev2"))) params->profile = kProfileHEv2;
			}
		}
		else if(!_tcscmp(argv[i],_T("--ignorelength"))) {
			params->ignoreLength = true;
		}
		else if(!_tcscmp(argv[i],_T("--adts"))) {
			params->adtsMode = true;
		}
		else if(!_tcsncmp(argv[i],_T("--"),2)) {}
		else {
			if(!params->inFile) {
				params->inFile = argv[i];
				if(!_tcscmp(argv[i],_T("-"))) params->readFromStdin = true;
			}
			else if(!params->outFile) params->outFile = argv[i];
		}
	}
	if(!params->inFile) return -1;
	if(params->mode == kModeVBR && params->profile != kProfileAuto) {
		params->profile = kProfileAuto;
		fputs("Warning: AAC profile in VBR mode is chosen automatically.\n",stderr);
	}
	return 0;
}

#ifdef USE_IOB_FOR_STDIO
extern "C" {
	FILE* stderr;
}
#endif
int _tmain(int argc, _TCHAR* argv[])
{
	_tsetlocale(LC_ALL, _T(""));

#ifdef USE_IOB_FOR_STDIO
	stderr = _stderr;
#endif

	if(argc==1) {
		printUsage();
		return 0;
	}

	HMODULE h = LoadLibraryA(&("Plugins\\enc_fhgaac.dll"[8]));
	if(!h) {
		_TCHAR tmp[MAX_PATH+8];
		GetModuleFileName(NULL, tmp, MAX_PATH);
		_TCHAR *filenamePtr = PathFindFileName(tmp);
		memcpy(filenamePtr, _T("QTfiles"), sizeof(_T("QTfiles")));
		SetDllDirectory(tmp);
		h = LoadLibraryA(&("Plugins\\enc_fhgaac.dll"[8]));
		if (!h && getPathForWinAmp(tmp,MAX_PATH)) {
			SetDllDirectory(tmp);
			h = LoadLibraryA("Plugins\\enc_fhgaac.dll");
		}
		SetDllDirectory(NULL);
	}
	if(!h) {
		fputs("error: cannot open enc_fhgaac.dll\n",stderr);
		return 0;
	}

	FhGAACEncoder *fhgenc = new FhGAACEncoder();

	*(void **)&(fhgenc->createAudio3) = (void *)GetProcAddress(h, "CreateAudio3");
#ifdef UNICODE
	*(void **)&(fhgenc->finishAudio3) = (void *)GetProcAddress(h,"FinishAudio3W");
	*(void **)&(fhgenc->prepareToFinish) = (void *)GetProcAddress(h,"PrepareToFinishW");
#else
	*(void **)&(fhgenc->finishAudio3) = (void *)GetProcAddress(h,"FinishAudio3");
	*(void **)&(fhgenc->prepareToFinish) = (void *)GetProcAddress(h,"PrepareToFinish");
#endif

	if(!fhgenc->createAudio3 || !fhgenc->finishAudio3 || !fhgenc->prepareToFinish) {
		fputs("error: cannot get encoding functions\n",stderr);
		FreeLibrary(h);
		return 0;
	}

	encodingParameters params;
	memset(&params,0,sizeof(encodingParameters));
	params.modeQuality = 4;
	if(parseArguments(argc, argv, &params)) {
		if(argc>1)fputs("Error while parsing arguments\n",stderr);
		printUsage();
		return 0;
	}
	/*
	if(params.mode == kModeVBR && params.adtsMode) {
		fputs("Error: only CBR is supported in ADTS AAC encoder.\n",stderr);
		return 0;
	}
	*/

	if(!params.readFromStdin) {
		replaceSlashWithBackSlash(params.inFile);
	}
	if(!params.outFile) {
		_TCHAR *filenamePtr = PathFindFileName(params.inFile);
		int pathLength = _tcslen(filenamePtr);
		_TCHAR *dst = new _TCHAR[pathLength+5];
		params.outFile = dst;
		_TCHAR *src = filenamePtr;
		while (pathLength>0) {
			pathLength--;
			*dst++ = *src++;
		}
		_TCHAR *extPtr = PathFindExtension(params.outFile);
		if(params.adtsMode) memcpy(extPtr,_T(".aac"),5*sizeof(TCHAR));
		else memcpy(extPtr,_T(".m4a"),5*sizeof(TCHAR));
	}
	else replaceSlashWithBackSlash(params.outFile);

	/* open file or stream */
	if(params.readFromStdin) {
		_setmode(_fileno(stdin), _O_BINARY);
		if(!fhgenc->openStream(stdin)) goto last;
	}
	else {
		if(GetFileAttributes(params.inFile) == -1) {
			fputs("Error: input file does not exist.\n",stderr);
			goto last;
		}
		if(!fhgenc->loadLibsndfile()) {
			fputs("Error: can't load libsndfile-1.dll.\n",stderr);
			goto last;
		}
		if(!fhgenc->openFile(params.inFile)) goto last;
	}

	/* encode */
	fhgenc->beginEncode(params.outFile,&params);

last:
	FreeLibrary(h);
	return 0;
}

