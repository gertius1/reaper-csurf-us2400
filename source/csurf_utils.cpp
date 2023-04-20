#include "csurf_utils.h"

#include <string>
#include <fstream>

const int csurf_utils::CONFIG_FLAG_METER_MODE = 1;
int csurf_utils::paramNums[MAX_PARAM_NUMBERS];


void csurf_utils::parseParams(const char* str, int parms[5])
{
	parms[0] = 0;
	parms[1] = 9;
	parms[2] = parms[3] = -1;
	parms[4] = 0;

	const char* p = str;
	if (p)
	{
		int x = 0;
		while (x < 5)
		{
			while (*p == ' ') p++;
			if ((*p < '0' || *p > '9') && *p != '-') break;
			parms[x++] = atoi(p);
			while (*p && *p != ' ') p++;
		}
	}
}

MediaTrack* csurf_utils::Cnv_ChannelIDToMediaTrack(unsigned char ch_id, int s_ch_offset)
{
	if (ch_id == 24)
	{
		ch_id = 0; // master = 0
	}
	else
	{
		ch_id += s_ch_offset + 1;
	}

	MediaTrack* rpr_tk = CSurf_TrackFromID(ch_id, true);

	return rpr_tk;
} // Cnv_ChannelIDToMediaTrack

WDL_String csurf_utils::Utl_Alphanumeric(WDL_String in_str)
{
	char* str_buf = in_str.Get();
	bool replace = false;
	// replace everything other than A-Z, a-z, 0-9 with a space
	for (int i = 0; i < (int)strlen(str_buf); i++)
	{
		replace = true;
		if (str_buf[i] == '\n') replace = false;
		if ((str_buf[i] >= '0') && (str_buf[i] <= '9')) replace = false;
		if ((str_buf[i] >= 'A') && (str_buf[i] <= 'Z')) replace = false;
		if ((str_buf[i] >= 'a') && (str_buf[i] <= 'z')) replace = false;

		if (replace) str_buf[i] = ' ';
	}

	WDL_String out_str = WDL_String(str_buf);
	return out_str;
} // Utl_Alphanumeric

int csurf_utils::SizeTToInt(size_t data)
{
    if (data > (std::numeric_limits<unsigned int>::max)())
        throw std::logic_error("Invalid cast.");
    return static_cast<int>(data);
}

void csurf_utils::PrepareParamMapArray(MediaTrack* tr, int fxNr)
{
	for (int i = 0; i < MAX_PARAM_NUMBERS; i++)
		paramNums[i] = i;

#ifdef _WIN32
	const int FXNAME_SIZE = 64;
	char fxNameBuf[FXNAME_SIZE];

	char path[2048];
	GetModuleFileNameA(g_hInst, path, 2048);
	std::string::size_type pos = std::string(path).find_last_of("\\/");


	TrackFX_GetFXName(tr, fxNr, fxNameBuf, FXNAME_SIZE);
	int trackFXNumParams = TrackFX_GetNumParams(tr, fxNr);

	//if file with fx name exists
	std::string fxName(fxNameBuf);
	fxName = fxName.substr(fxName.find(" ") + 1, std::string::npos);
	fxName = fxName.append(".txt");
	fxName.insert(0, "paramMap_");
	fxName.insert(0, "\\");
	fxName.insert(0, std::string(path).substr(0, pos));

	std::ifstream inFile;
	inFile.open(fxName.c_str());

	int i = 0;
	if (inFile.is_open())
	{
		while (true) {
			inFile >> paramNums[i];
			if (paramNums[i] >= trackFXNumParams-1)
				paramNums[i] = trackFXNumParams-1;

			if (inFile.eof()) break;
			if (i >= MAX_PARAM_NUMBERS) break; //prevent array overflow
			i++;
		}

		inFile.close(); // CLose input file
	}
	
#endif
}


int csurf_utils::TrackFX_RemapParam(int inParamNr)
{
	return paramNums[inParamNr];
}