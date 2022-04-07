#pragma once
#include "us2400.h"
#include <limits>
#include <stdexcept>

class csurf_utils
{
public:
	static const int CONFIG_FLAG_METER_MODE;
	static void parseParams(const char* str, int parms[5]);
	static MediaTrack* Cnv_ChannelIDToMediaTrack(unsigned char ch_id, int s_ch_offset);
	static WDL_String Utl_Alphanumeric(WDL_String in_str);
	static int SizeTToInt(size_t data);

private:
	csurf_utils() {}
};

