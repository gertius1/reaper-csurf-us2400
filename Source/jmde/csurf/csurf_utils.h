#pragma once
#include "csurf.h"

class csurf_utils
{
public:
	static const int CONFIG_FLAG_METER_MODE;
	static void parseParams(const char* str, int parms[5]);
	static MediaTrack* Cnv_ChannelIDToMediaTrack(unsigned char ch_id, int s_ch_offset);
	static WDL_String Utl_Alphanumeric(WDL_String in_str);

private:
	csurf_utils() {}
};

