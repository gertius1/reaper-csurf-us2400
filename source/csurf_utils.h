#pragma once
#include "us2400.h"
#include <limits>
#include <stdexcept>

#define MAX_PARAM_NUMBERS 128

class csurf_utils
{
public:
	static const int CONFIG_FLAG_METER_MODE;
	static void parseParams(const char* str, int parms[6]);
	static MediaTrack* Cnv_ChannelIDToMediaTrack(unsigned char ch_id, int s_ch_offset);
	static WDL_String Utl_Alphanumeric(WDL_String in_str);
	static int SizeTToInt(size_t data);
	static void PrepareParamMapArray(MediaTrack* tr, int fxNr);
	static int csurf_utils::TrackFX_RemapParam(int inParamNr);


private:
	static int paramNums[MAX_PARAM_NUMBERS];

	csurf_utils() {
		for (int i = 0; i < MAX_PARAM_NUMBERS; i++)
			paramNums[i] = i;
	}
	
};

