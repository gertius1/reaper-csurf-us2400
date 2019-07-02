#pragma once
#include "csurf.h"
#include "csurf_utils.h"
#include <map>

class CSurf_US2400_helpoverlay {
	private:
		WDL_String hlp_enc_str[3][4][2];   // pan/chan/aux | _/sh/f/m | _/fl

		WDL_String hlp_tksel_str[3][4];    // pan/chan/aux | _/sh/f/m
		WDL_String hlp_tksolo_str[4];      // _/sh/f/m
		WDL_String hlp_tkmute_str[4];      // _/sh/f/m
		WDL_String hlp_tkfdr_str[3][4][2]; // pan/chan/aux | _/sh/f/m | _/fl

		WDL_String hlp_mstsel_str[4];      // _/sh/f/m
		WDL_String hlp_clsolo_str[4];      // _/sh/f/m
		WDL_String hlp_mstfdr_str[4];      // _/sh/f/m

		WDL_String hlp_chan_str[3];        // pan/chan/aux
		WDL_String hlp_fkey_str[4];        // _/sh/f/m
		WDL_String hlp_shift_str[4];       // _/sh/f/m

		WDL_String hlp_keys_str[12][3][4]; // aux1-6/null/req/fwd/stop/play/rec | pan/chan/aux | _/sh/f/m
		WDL_String hlp_bank_str[2][3][4];  // bank -/+ | pan/chan/aux | _/sh/f/m
		WDL_String hlp_inout_str[2][4];	   // in/out | _/sh/f/m


		int hlp_mode;
		int hlp_qkey;
		bool hlp_flip;
		int hlp_open;
		int hlp_margin;
		int hlp_grid;
		int hlp_box_size;
		int hlp_sep;

		HWND hlp_hwnd;
		WNDCLASSEX hlp_class;

		void Hlp_DrawBox(const char* caption, int top, int left, COLORREF bg_col, COLORREF mode_col, COLORREF cap_col, WDL_String command, HDC* hdc_p);


	public:
		CSurf_US2400_helpoverlay();
		~CSurf_US2400_helpoverlay();
		void Hlp_FillStrs();
		void Hlp_ToggleWindow();
		void Hlp_Update();
		void Hlp_Paint(HWND hwnd, bool meter_mode);
		void SetQkey(int qkey);
		void SetFlip(bool flip);
		void SetMode(int mode);
		void SetHelpKeyString(int key, int m, int q, WDL_String name);
};