#include "csurf_us2400_helpoverlay.h"
#ifndef _WIN32
	#ifndef __APPLE__
		#include <X11/Xlib.h>
	#else 
		#include <CoreGraphics/CGDisplayConfiguration.h>
	#endif
#endif

CSurf_US2400_helpoverlay* hlpHandler;

LRESULT CALLBACK Hlp_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_PAINT:
		hlpHandler->Hlp_Paint(hwnd);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

CSurf_US2400_helpoverlay::CSurf_US2400_helpoverlay() {
	hlp_mode = 0;
	hlp_qkey = 0;
	hlp_flip = false;

	hlp_open = 0;

	hlp_margin = 15;
	hlp_grid = 90;
	hlp_box_size = 80;
	hlp_sep = 20;

	meter_mode = false;

	hlp_hwnd = NULL;

	WNDCLASSEX hlp_class;
	hlp_class.cbSize = sizeof(WNDCLASSEX);
	hlp_class.style = 0;
	hlp_class.lpfnWndProc = (WNDPROC)Hlp_WindowProc;
	hlp_class.cbClsExtra = 0;
	hlp_class.cbWndExtra = 0;
	hlp_class.hInstance = g_hInst;
	hlp_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	hlp_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	hlp_class.hbrBackground = CreateSolidBrush(RGB(60, 60, 60));
	hlp_class.lpszMenuName = NULL;
	hlp_class.lpszClassName = "hlp";
	hlp_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&hlp_class);

	hlpHandler = this;
}


CSurf_US2400_helpoverlay::~CSurf_US2400_helpoverlay() {

}

void CSurf_US2400_helpoverlay::SetQkey(int qkey) {
	hlp_qkey = qkey;
	Hlp_Update();
}

void CSurf_US2400_helpoverlay::SetFlip(bool flip) {
	hlp_flip = flip;
	Hlp_Update();
}

void CSurf_US2400_helpoverlay::SetMode(int mode) {
	hlp_mode = mode;
	Hlp_Update();
}

void CSurf_US2400_helpoverlay::SetHelpKeyString(int key, int m, int q, WDL_String name) {

}

void CSurf_US2400_helpoverlay::Hlp_FillStrs()
{
	hlp_enc_str[0][0][0] = WDL_String("Track Pan");   // pan | _ | _
	hlp_enc_str[0][0][1] = WDL_String("Track Volume");   // pan | _ | fl
	hlp_enc_str[0][1][0] = WDL_String("Track Pan -> C");   // pan | sh | _
	hlp_enc_str[0][1][1] = WDL_String("Track Volume -> 0 dB");   // pan | sh | fl
	hlp_enc_str[0][2][0] = WDL_String("Track Stereo Width");   // pan | f | _
	hlp_enc_str[0][2][1] = WDL_String("Track Volume -> -inf dB");   // pan | f | fl
	hlp_enc_str[0][3][0] = WDL_String("Track Pan");   // pan | m | _
	hlp_enc_str[0][3][1] = WDL_String("Track Volume");   // pan | m | fl
	hlp_enc_str[1][0][0] = WDL_String("FX Parameter");   // chan | _ | _
	hlp_enc_str[1][0][1] = WDL_String("Track Volume");   // chan | _ | fl
	hlp_enc_str[1][1][0] = WDL_String("FX Parameter (Toggle)");   // chan | sh | _
	hlp_enc_str[1][1][1] = WDL_String("Track Volume -> 0 dB");   // chan | sh | fl
	hlp_enc_str[1][2][0] = WDL_String("FX Parameter (Fine)");   // chan | f | _
	hlp_enc_str[1][2][1] = WDL_String("Track Volume -> -inf dB");   // chan | f | fl
	hlp_enc_str[1][3][0] = WDL_String("FX Parameter");   // chan | m | _
	hlp_enc_str[1][3][1] = WDL_String("Track Volume");   // chan | m | fl
	hlp_enc_str[2][0][0] = WDL_String("Aux Send Level");   // aux | _ | _
	hlp_enc_str[2][0][1] = WDL_String("Aux Send Pan");   // aux | _ | fl
	hlp_enc_str[2][1][0] = WDL_String("Aux Send Level -> 0 dB");   // aux | sh | _
	hlp_enc_str[2][1][1] = WDL_String("Aux Send Pan -> C");   // aux | sh | fl
	hlp_enc_str[2][2][0] = WDL_String("Aux Send Level -> -inf dB");   // aux | f | _
	hlp_enc_str[2][2][1] = WDL_String("Aux Send Pan");   // aux | f | fl
	hlp_enc_str[2][3][0] = WDL_String("Aux Send Level");   // aux | m | _
	hlp_enc_str[2][3][1] = WDL_String("Aux Send Pan");   // aux | m | fl

	hlp_tkfdr_str[0][0][0] = WDL_String("Track Volume");   // pan | _ | _
	hlp_tkfdr_str[0][0][1] = WDL_String("Track Pan");   // pan | _ | fl
	hlp_tkfdr_str[0][1][0] = WDL_String("Track Volume -> 0 dB");   // pan | sh | _
	hlp_tkfdr_str[0][1][1] = WDL_String("Track Pan -> C");   // pan | sh | fl
	hlp_tkfdr_str[0][2][0] = WDL_String("Track Volume -> - inf dB");   // pan | f | _
	hlp_tkfdr_str[0][2][1] = WDL_String("Track Stereo Width");   // pan | f | fl
	hlp_tkfdr_str[0][3][0] = WDL_String("Track Volume");   // pan | m | _
	hlp_tkfdr_str[0][3][1] = WDL_String("Track Pan");   // pan | m | fl
	hlp_tkfdr_str[1][0][0] = WDL_String("Track Volume");   // chan | _ | _
	hlp_tkfdr_str[1][0][1] = WDL_String("FX Parameter");   // chan | _ | fl
	hlp_tkfdr_str[1][1][0] = WDL_String("Track Volume -> 0 dB");   // chan | sh | _
	hlp_tkfdr_str[1][1][1] = WDL_String("FX Parameter -> max");   // chan | sh | fl
	hlp_tkfdr_str[1][2][0] = WDL_String("Track Volume -> -inf dB");   // chan | f | _
	hlp_tkfdr_str[1][2][1] = WDL_String("FX Parameter -> min");   // chan | f | fl
	hlp_tkfdr_str[1][3][0] = WDL_String("Track Volume");   // chan | m | _
	hlp_tkfdr_str[1][3][1] = WDL_String("FX Parameter");   // chan | m | fl
	hlp_tkfdr_str[2][0][0] = WDL_String("Track Volume");   // aux | _ | _
	hlp_tkfdr_str[2][0][1] = WDL_String("Aux Send Level");   // aux | _ | fl
	hlp_tkfdr_str[2][1][0] = WDL_String("Track Volume -> 0 dB");   // aux | sh | _
	hlp_tkfdr_str[2][1][1] = WDL_String("Aux Send Level -> 0 dB");   // aux | sh | fl
	hlp_tkfdr_str[2][2][0] = WDL_String("Track Volume -> - inf dB");   // aux | f | _
	hlp_tkfdr_str[2][2][1] = WDL_String("Aux Send Level -> - inf dB");   // aux | f | fl
	hlp_tkfdr_str[2][3][0] = WDL_String("Track Volume");   // aux | m | _
	hlp_tkfdr_str[2][3][1] = WDL_String("Aux Send Level");   // aux | m | fl

	hlp_tksel_str[0][0] = WDL_String("Select Track");    // pan | _
	hlp_tksel_str[0][1] = WDL_String("Arm Track for Record");    // pan | sh
	hlp_tksel_str[0][2] = WDL_String("Switch Phase");    // pan | f
	hlp_tksel_str[0][3] = WDL_String("Select Track");    // pan | m
	hlp_tksel_str[1][0] = WDL_String("Select Track for Channel Strip");    // chan | _
	hlp_tksel_str[1][1] = WDL_String("Arm Track for Record");    // chan | sh
	hlp_tksel_str[1][2] = WDL_String("Switch Phase");    // chan | f
	hlp_tksel_str[1][3] = WDL_String("Select Track for Channel Strip");    // chan | m
	hlp_tksel_str[2][0] = WDL_String("Select Track");    // aux | _
	hlp_tksel_str[2][1] = WDL_String("Remove Aux Send");    // aux | sh
	hlp_tksel_str[2][2] = WDL_String("Add Aux Send");    // aux | f
	hlp_tksel_str[2][3] = WDL_String("Select Track");    // aux | m

	hlp_tksolo_str[0] = WDL_String("Solo Track");      // _
	hlp_tksolo_str[1] = WDL_String("Solo This Track Only");      // sh
	hlp_tksolo_str[2] = WDL_String("Solo Track");      // f
	hlp_tksolo_str[3] = WDL_String("Solo Track");      // m

	hlp_tkmute_str[0] = WDL_String("Mute Track");      // _
	hlp_tkmute_str[1] = WDL_String("Mute This Track Only");      // sh
	hlp_tkmute_str[2] = WDL_String("Bypass All Track FX");      // f
	hlp_tkmute_str[3] = WDL_String("Mute Track");      // m

	hlp_mstsel_str[0] = WDL_String("Select Tracks: None / All");      // _
	hlp_mstsel_str[1] = WDL_String("Select Master");      // sh
	hlp_mstsel_str[2] = WDL_String("Select Tracks: None / All");      // f
	hlp_mstsel_str[3] = WDL_String("Select Tracks: None / All");      // m

	hlp_clsolo_str[0] = WDL_String("Clear all Solos");      // _
	hlp_clsolo_str[1] = WDL_String("Unmute Master");      // sh
	hlp_clsolo_str[2] = WDL_String("Clear all Mutes");      // f
	hlp_clsolo_str[3] = WDL_String("Clear all Solos");      // m

	hlp_mstfdr_str[0] = WDL_String("Master Volume");      // _
	hlp_mstfdr_str[1] = WDL_String("Master Volume -> 0 dB");      // sh
	hlp_mstfdr_str[2] = WDL_String("Master Volume -> - inf dB");      // f
	hlp_mstfdr_str[3] = WDL_String("Master Volume");      // m

	hlp_chan_str[0] = WDL_String("Enter Channel Strip Mode");      // pan
	hlp_chan_str[1] = WDL_String("Exit Channel Strip Mode (Enter Pan Mode)");      // chan
	hlp_chan_str[2] = WDL_String("Enter Channel Strip Mode");      // aux

	hlp_fkey_str[0] = WDL_String("");      // _
	hlp_fkey_str[1] = WDL_String("Open / Close Scribble Strip");      // sh
	hlp_fkey_str[2] = WDL_String("");      // f
	hlp_fkey_str[3] = WDL_String("");      // m

	hlp_shift_str[0] = WDL_String("");      // _
	hlp_shift_str[1] = WDL_String("");      // sh
	hlp_shift_str[2] = WDL_String("Open / Close On-Screen Help");      // f
	hlp_shift_str[3] = WDL_String("");      // m

	hlp_keys_str[0][0][0] = WDL_String("Enter Aux Mode: Aux---1"); // aux1 | pan | _
	hlp_keys_str[0][0][1] = WDL_String(""); // aux1 | pan | sh
	hlp_keys_str[0][0][2] = WDL_String(""); // aux1 | pan | f
	hlp_keys_str[0][0][3] = WDL_String(""); // aux1 | pan | m
	hlp_keys_str[0][1][0] = WDL_String("FX Parameters: Shift Bank (< 24)"); // aux1 | chan | _
	hlp_keys_str[0][1][1] = WDL_String(""); // aux1 | chan | sh
	hlp_keys_str[0][1][2] = WDL_String(""); // aux1 | chan | f
	hlp_keys_str[0][1][3] = WDL_String(""); // aux1 | chan | m
	hlp_keys_str[0][2][0] = WDL_String("Enter Aux Mode: Aux---1"); // aux1 | aux | _
	hlp_keys_str[0][2][1] = WDL_String(""); // aux1 | aux | sh
	hlp_keys_str[0][2][2] = WDL_String(""); // aux1 | aux | f
	hlp_keys_str[0][2][3] = WDL_String(""); // aux1 | aux | m

	hlp_keys_str[1][0][0] = WDL_String("Enter Aux Mode: Aux---2"); // aux2 | pan | _
	hlp_keys_str[1][0][1] = WDL_String(""); // aux2 | pan | sh
	hlp_keys_str[1][0][2] = WDL_String(""); // aux2 | pan | f
	hlp_keys_str[1][0][3] = WDL_String(""); // aux2 | pan | m
	hlp_keys_str[1][1][0] = WDL_String("FX Parameters: Shift Bank (24 >)"); // aux2 | chan | _
	hlp_keys_str[1][1][1] = WDL_String(""); // aux2 | chan | sh
	hlp_keys_str[1][1][2] = WDL_String(""); // aux2 | chan | f
	hlp_keys_str[1][1][3] = WDL_String(""); // aux2 | chan | m
	hlp_keys_str[1][2][0] = WDL_String("Enter Aux Mode: Aux---2"); // aux2 | aux | _
	hlp_keys_str[1][2][1] = WDL_String(""); // aux2 | aux | sh
	hlp_keys_str[1][2][2] = WDL_String(""); // aux2 | aux | f
	hlp_keys_str[1][2][3] = WDL_String(""); // aux2 | aux | m

	hlp_keys_str[2][0][0] = WDL_String("Enter Aux Mode: Aux---3"); // aux3 | pan | _
	hlp_keys_str[2][0][1] = WDL_String(""); // aux3 | pan | sh
	hlp_keys_str[2][0][2] = WDL_String(""); // aux3 | pan | f
	hlp_keys_str[2][0][3] = WDL_String(""); // aux3 | pan | m
	hlp_keys_str[2][1][0] = WDL_String("Current FX: Toggle Bypass "); // aux3 | chan | _
	hlp_keys_str[2][1][1] = WDL_String(""); // aux3 | chan | sh
	hlp_keys_str[2][1][2] = WDL_String(""); // aux3 | chan | f
	hlp_keys_str[2][1][3] = WDL_String(""); // aux3 | chan | m
	hlp_keys_str[2][2][0] = WDL_String("Enter Aux Mode: Aux---3"); // aux3 | aux | _
	hlp_keys_str[2][2][1] = WDL_String(""); // aux3 | aux | sh
	hlp_keys_str[2][2][2] = WDL_String(""); // aux3 | aux | f
	hlp_keys_str[2][2][3] = WDL_String(""); // aux3 | aux | m

	hlp_keys_str[3][0][0] = WDL_String("Enter Aux Mode: Aux---4"); // aux4 | pan | _
	hlp_keys_str[3][0][1] = WDL_String(""); // aux4 | pan | sh
	hlp_keys_str[3][0][2] = WDL_String(""); // aux4 | pan | f
	hlp_keys_str[3][0][3] = WDL_String(""); // aux4 | pan | m
	hlp_keys_str[3][1][0] = WDL_String("Insert FX"); // aux4 | chan | _
	hlp_keys_str[3][1][1] = WDL_String(""); // aux4 | chan | sh
	hlp_keys_str[3][1][2] = WDL_String(""); // aux4 | chan | f
	hlp_keys_str[3][1][3] = WDL_String(""); // aux4 | chan | m
	hlp_keys_str[3][2][0] = WDL_String("Enter Aux Mode: Aux---4"); // aux4 | aux | _
	hlp_keys_str[3][2][1] = WDL_String(""); // aux4 | aux | sh
	hlp_keys_str[3][2][2] = WDL_String(""); // aux4 | aux | f
	hlp_keys_str[3][2][3] = WDL_String(""); // aux4 | aux | m

	hlp_keys_str[4][0][0] = WDL_String("Enter Aux Mode: Aux---5"); // aux5 | pan | _
	hlp_keys_str[4][0][1] = WDL_String(""); // aux5 | pan | sh
	hlp_keys_str[4][0][2] = WDL_String(""); // aux5 | pan | f
	hlp_keys_str[4][0][3] = WDL_String(""); // aux5 | pan | m
	hlp_keys_str[4][1][0] = WDL_String("Delete FX"); // aux5 | chan | _
	hlp_keys_str[4][1][1] = WDL_String(""); // aux5 | chan | sh
	hlp_keys_str[4][1][2] = WDL_String(""); // aux5 | chan | f
	hlp_keys_str[4][1][3] = WDL_String(""); // aux5 | chan | m
	hlp_keys_str[4][2][0] = WDL_String("Enter Aux Mode: Aux---5"); // aux5 | aux | _
	hlp_keys_str[4][2][1] = WDL_String(""); // aux5 | aux | sh
	hlp_keys_str[4][2][2] = WDL_String(""); // aux5 | aux | f
	hlp_keys_str[4][2][3] = WDL_String(""); // aux5 | aux | m

	hlp_keys_str[5][0][0] = WDL_String("Enter Aux Mode: Aux---6"); // aux6 | pan | _
	hlp_keys_str[5][0][1] = WDL_String(""); // aux6 | pan | sh
	hlp_keys_str[5][0][2] = WDL_String(""); // aux6 | pan | f
	hlp_keys_str[5][0][3] = WDL_String(""); // aux6 | pan | m
	hlp_keys_str[5][1][0] = WDL_String("Toggle Track / FX Automation"); // aux6 | chan | _
	hlp_keys_str[5][1][1] = WDL_String(""); // aux6 | chan | sh
	hlp_keys_str[5][1][2] = WDL_String(""); // aux6 | chan | f
	hlp_keys_str[5][1][3] = WDL_String(""); // aux6 | chan | m
	hlp_keys_str[5][2][0] = WDL_String("Enter Aux Mode: Aux---6"); // aux6 | aux | _
	hlp_keys_str[5][2][1] = WDL_String(""); // aux6 | aux | sh
	hlp_keys_str[5][2][2] = WDL_String(""); // aux6 | aux | f
	hlp_keys_str[5][2][3] = WDL_String(""); // aux6 | aux | m

	hlp_keys_str[6][0][0] = WDL_String(""); // null | pan | _
	hlp_keys_str[6][0][1] = WDL_String(""); // null | pan | sh
	hlp_keys_str[6][0][2] = WDL_String(""); // null | pan | f
	hlp_keys_str[6][0][3] = WDL_String(""); // null | pan | m
	hlp_keys_str[6][1][0] = WDL_String(""); // null | chan | _
	hlp_keys_str[6][1][1] = WDL_String(""); // null | chan | sh
	hlp_keys_str[6][1][2] = WDL_String(""); // null | chan | f
	hlp_keys_str[6][1][3] = WDL_String(""); // null | chan | m
	hlp_keys_str[6][2][0] = WDL_String(""); // null | aux | _
	hlp_keys_str[6][2][1] = WDL_String(""); // null | aux | sh
	hlp_keys_str[6][2][2] = WDL_String(""); // null | aux | f
	hlp_keys_str[6][2][3] = WDL_String(""); // null | aux | m

	hlp_keys_str[7][0][0] = WDL_String("Rewind"); // rew | pan | _
	hlp_keys_str[7][0][1] = WDL_String("Automation Mode: Off / Trim"); // rew | pan | sh
	hlp_keys_str[7][0][2] = WDL_String(""); // rew | pan | f
	hlp_keys_str[7][0][3] = WDL_String(""); // rew | pan | m
	hlp_keys_str[7][1][0] = WDL_String("Rewind"); // rew | chan | _
	hlp_keys_str[7][1][1] = WDL_String("Automation Mode: Off / Trim"); // rew | chan | sh
	hlp_keys_str[7][1][2] = WDL_String(""); // rew | chan | f
	hlp_keys_str[7][1][3] = WDL_String(""); // rew | chan | m
	hlp_keys_str[7][2][0] = WDL_String("Rewind"); // rew | aux | _
	hlp_keys_str[7][2][1] = WDL_String("Automation Mode: Off / Trim"); // rew | aux | sh
	hlp_keys_str[7][2][2] = WDL_String(""); // rew | aux | f
	hlp_keys_str[7][2][3] = WDL_String(""); // rew | aux | m

	hlp_keys_str[8][0][0] = WDL_String("Fast Forward"); // fwd | pan | _
	hlp_keys_str[8][0][1] = WDL_String("Automation Mode: Read"); // fwd | pan | sh
	hlp_keys_str[8][0][2] = WDL_String(""); // fwd | pan | f
	hlp_keys_str[8][0][3] = WDL_String(""); // fwd | pan | m
	hlp_keys_str[8][1][0] = WDL_String("Fast Forward"); // fwd | chan | _
	hlp_keys_str[8][1][1] = WDL_String("Automation Mode: Read"); // fwd | chan | sh
	hlp_keys_str[8][1][2] = WDL_String(""); // fwd | chan | f
	hlp_keys_str[8][1][3] = WDL_String(""); // fwd | chan | m
	hlp_keys_str[8][2][0] = WDL_String("Fast Forward"); // fwd | aux | _
	hlp_keys_str[8][2][1] = WDL_String("Automation Mode: Read"); // fwd | aux | sh
	hlp_keys_str[8][2][2] = WDL_String(""); // fwd | aux | f
	hlp_keys_str[8][2][3] = WDL_String(""); // fwd | aux | m

	hlp_keys_str[9][0][0] = WDL_String("Stop"); // stop | pan | _
	hlp_keys_str[9][0][1] = WDL_String("Automation Mode: Latch"); // stop | pan | sh
	hlp_keys_str[9][0][2] = WDL_String(""); // stop | pan | f
	hlp_keys_str[9][0][3] = WDL_String(""); // stop | pan | m
	hlp_keys_str[9][1][0] = WDL_String("Stop"); // stop | chan | _
	hlp_keys_str[9][1][1] = WDL_String("Automation Mode: Latch"); // stop | chan | sh
	hlp_keys_str[9][1][2] = WDL_String(""); // stop | chan | f
	hlp_keys_str[9][1][3] = WDL_String(""); // stop | chan | m
	hlp_keys_str[9][2][0] = WDL_String("Stop"); // stop | aux | _
	hlp_keys_str[9][2][1] = WDL_String("Automation Mode: Latch"); // stop | aux | sh
	hlp_keys_str[9][2][2] = WDL_String(""); // stop | aux | f
	hlp_keys_str[9][2][3] = WDL_String(""); // stop | aux | m

	hlp_keys_str[10][0][0] = WDL_String("Play"); // play | pan | _
	hlp_keys_str[10][0][1] = WDL_String("Automation Mode: Write"); // play | pan | sh
	hlp_keys_str[10][0][2] = WDL_String(""); // play | pan | f
	hlp_keys_str[10][0][3] = WDL_String(""); // play | pan | m
	hlp_keys_str[10][1][0] = WDL_String("Play"); // play | chan | _
	hlp_keys_str[10][1][1] = WDL_String("Automation Mode: Write"); // play | chan | sh
	hlp_keys_str[10][1][2] = WDL_String(""); // play | chan | f
	hlp_keys_str[10][1][3] = WDL_String(""); // play | chan | m
	hlp_keys_str[10][2][0] = WDL_String("Play"); // play | aux | _
	hlp_keys_str[10][2][1] = WDL_String("Automation Mode: Write"); // play | aux | sh
	hlp_keys_str[10][2][2] = WDL_String(""); // play | aux | f
	hlp_keys_str[10][2][3] = WDL_String(""); // play | aux | m

	hlp_keys_str[11][0][0] = WDL_String("Record (Punch in when playing)"); // rec | pan | _
	hlp_keys_str[11][0][1] = WDL_String("Automation: Write Current Value to Time Selection"); // rec | pan | sh
	hlp_keys_str[11][0][2] = WDL_String(""); // rec | pan | f
	hlp_keys_str[11][0][3] = WDL_String(""); // rec | pan | m
	hlp_keys_str[11][1][0] = WDL_String("Record (Punch in when playing)"); // rec | chan | _
	hlp_keys_str[11][1][1] = WDL_String("Automation: Write Current Value to Time Selection"); // rec | chan | sh
	hlp_keys_str[11][1][2] = WDL_String(""); // rec | chan | f
	hlp_keys_str[11][1][3] = WDL_String(""); // rec | chan | m
	hlp_keys_str[11][2][0] = WDL_String("Record (Punch in when playing)"); // rec | aux | _
	hlp_keys_str[11][2][1] = WDL_String("Automation: Write Current Value to Time Selection"); // rec | aux | sh
	hlp_keys_str[11][2][2] = WDL_String(""); // rec | aux | f
	hlp_keys_str[10][2][3] = WDL_String(""); // rec | aux | m

	hlp_bank_str[0][0][0] = WDL_String("Tracks: Shift Bank (< 8)");  // bank - | pan | _
	hlp_bank_str[0][0][1] = WDL_String("Tracks: Shift Bank (< 24)");  // bank - | pan | sh
	hlp_bank_str[0][0][2] = WDL_String("Time Selection: Move Left Locator (< 1 Bar)");  // bank - | pan | f
	hlp_bank_str[0][0][3] = WDL_String("Tracks: Shift Bank (< 8)");  // bank - | pan | m
	hlp_bank_str[0][1][0] = WDL_String("Select Previous FX in Chain");  // bank - | chan | _
	hlp_bank_str[0][1][1] = WDL_String("Tracks: Shift Bank (< 24)");  // bank - | chan | sh
	hlp_bank_str[0][1][2] = WDL_String("Move FX Up in Chain");  // bank - | chan | f
	hlp_bank_str[0][1][3] = WDL_String("Select Previous FX in Chain");  // bank - | chan | m
	hlp_bank_str[0][2][0] = WDL_String("Tracks: Shift Bank (< 8)");  // bank - | aux | _
	hlp_bank_str[0][2][1] = WDL_String("Tracks: Shift Bank (< 24)");  // bank - | aux | sh
	hlp_bank_str[0][2][2] = WDL_String("Time Selection: Move Left Locator (< 1 Bar)");  // bank - | aux | f
	hlp_bank_str[0][2][3] = WDL_String("Tracks: Shift Bank (< 8)");  // bank - | aux | m

	hlp_bank_str[1][0][0] = WDL_String("Tracks: Shift Bank (8 >)");  // bank + | pan | _
	hlp_bank_str[1][0][1] = WDL_String("Tracks: Shift Bank (24 >)");  // bank + | pan | sh
	hlp_bank_str[1][0][2] = WDL_String("Time Selection: Move Left Locator (1 Bar >)");  // bank + | pan | f
	hlp_bank_str[1][0][3] = WDL_String("Tracks: Shift Bank (8 >)");  // bank + | pan | m
	hlp_bank_str[1][1][0] = WDL_String("Select Next FX in Chain");  // bank + | chan | _
	hlp_bank_str[1][1][1] = WDL_String("Tracks: Shift Bank (24 >)");  // bank + | chan | sh
	hlp_bank_str[1][1][2] = WDL_String("Move FX Down in Chain");  // bank + | chan | f
	hlp_bank_str[1][1][3] = WDL_String("Select Previous FX in Chain");  // bank + | chan | m
	hlp_bank_str[1][2][0] = WDL_String("Tracks: Shift Bank (8 >)");  // bank + | aux | _
	hlp_bank_str[1][2][1] = WDL_String("Tracks: Shift Bank (24 >)");  // bank + | aux | sh
	hlp_bank_str[1][2][2] = WDL_String("Time Selection: Move Left Locator (1 Bar >)");  // bank + | aux | f
	hlp_bank_str[1][2][3] = WDL_String("Tracks: Shift Bank (8 >)");  // bank + | aux | m

	hlp_inout_str[0][0] = WDL_String("Time Selection: Select Previous Region"); // in | _
	hlp_inout_str[0][1] = WDL_String("Time Selection: Select All / Last Selected"); // in | sh
	hlp_inout_str[0][2] = WDL_String("Time Selection: Move Right Locator (1 Bar >)"); // in | f
	hlp_inout_str[0][3] = WDL_String("Time Selection: Select Previous Region"); // in | m

	hlp_inout_str[1][0] = WDL_String("Time Selection: Select Next Region"); // out | _
	hlp_inout_str[1][1] = WDL_String("Loop Time Selection On / Off"); // out | pan| sh
	hlp_inout_str[1][2] = WDL_String("Time Selection: Move Right Locator (< 1 Bar)"); // out | f
	hlp_inout_str[1][3] = WDL_String("Time Selection: Select Next Region"); // out | m
}

void CSurf_US2400_helpoverlay::Hlp_DrawBox(const char* caption, int top, int left, COLORREF bg_col, COLORREF mode_col, COLORREF cap_col, WDL_String command, HDC* hdc_p)
{
	HPEN lgrey_ln = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	RECT rect;

	COLORREF text_col = RGB(255, 255, 255);
	SetTextColor(*hdc_p, text_col);

	int mode_border = 4;
	int text_padding = 4;

	if (mode_col != 0)
	{
		HBRUSH mode_bg = CreateSolidBrush(mode_col);
		rect.top = top - mode_border;
		rect.left = left - mode_border;
		rect.bottom = top + hlp_box_size + mode_border;
		rect.right = left + hlp_box_size + mode_border;
		FillRect(*hdc_p, &rect, mode_bg);
		DeleteObject(mode_bg);
	}

	HBRUSH bg = CreateSolidBrush(bg_col);
	rect.top = top;
	rect.left = left;
	rect.bottom = top + hlp_box_size;
	rect.right = left + hlp_box_size;
	FillRect(*hdc_p, &rect, bg);
	DeleteObject(bg);

	HBRUSH cap_bg = CreateSolidBrush(cap_col);
	rect.top = top;
	rect.left = left;
	rect.bottom = top + 20;
	rect.right = left + hlp_box_size;
	FillRect(*hdc_p, &rect, cap_bg);
	DeleteObject(cap_bg);

	SetBkColor(*hdc_p, cap_col);
	rect.top = top + text_padding;
	rect.left = left + text_padding;
	rect.bottom = top + 20 - text_padding;
	rect.right = left + hlp_box_size - text_padding;
	#ifdef _WIN32
		DrawText(*hdc_p, caption, -1, &rect, DT_CENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS);
	#else
		DrawText(*hdc_p, caption, -1, &rect, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS );
	#endif

	SelectObject(*hdc_p, lgrey_ln);
	MoveToEx(*hdc_p, rect.left, top + 20, NULL);
	LineTo(*hdc_p, rect.right, top + 20);

	SetBkColor(*hdc_p, bg_col);
	rect.top = top + 20 + text_padding;
	rect.left = left + text_padding;
	rect.bottom = top + hlp_box_size - text_padding;
	rect.right = left + hlp_box_size - text_padding;
	#ifdef _WIN32
		DrawText(*hdc_p, command.Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS);
	#else
		DrawText(*hdc_p, command.Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS);
	#endif

	DeleteObject(lgrey_ln);
}

void CSurf_US2400_helpoverlay::Hlp_Paint(HWND hwnd)
{
	RECT rect;

	HDC hdc;
	PAINTSTRUCT ps;
	hdc = BeginPaint(hwnd, &ps);

	#ifdef _WIN32
		HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	#else
		HFONT hfont = (HFONT)GetStockObject(17);
	#endif
	HFONT rfont = (HFONT)SelectObject(hdc, hfont);

	COLORREF bg_col = RGB(50, 50, 50);
	COLORREF shift_col = RGB(50, 50, 100);
	COLORREF fkey_col = RGB(100, 50, 50);
	COLORREF mkey_col = RGB(50, 100, 50);
	COLORREF pan_col = RGB(50, 100, 100);
	COLORREF chan_col = RGB(100, 100, 50);
	COLORREF aux_col = RGB(100, 50, 100);
	COLORREF flip_col = RGB(100, 100, 100);

	COLORREF flipbtn_col = flip_col;
	int flip_i = 1;
	if (!hlp_flip)
	{
		flip_col = bg_col;
		flip_i = 0;
	}

	COLORREF mode_col = pan_col;
	if (hlp_mode == 1) mode_col = chan_col;
	else if (hlp_mode == 2) mode_col = aux_col;

	COLORREF auxbtn_col = aux_col;
	COLORREF qkey_col = bg_col;
	if ((hlp_qkey > 0) || (hlp_mode == 1))
	{
		auxbtn_col = bg_col;
		if (hlp_qkey == 1) qkey_col = shift_col;
		else if (hlp_qkey == 2) qkey_col = fkey_col;
		else if (hlp_qkey == 3) qkey_col = mkey_col;
	}

	COLORREF text_col = RGB(255, 255, 255);
	SetTextColor(hdc, text_col);

	HBRUSH standard_bg = CreateSolidBrush(bg_col);
	HBRUSH mode_bg = CreateSolidBrush(mode_col);
	HBRUSH qkey_bg = CreateSolidBrush(qkey_col);

	// global output
	rect.top = hlp_margin + 0 * hlp_sep + 0 * hlp_grid;
	rect.left = hlp_margin + 2 * hlp_sep + 4 * hlp_grid;
	rect.bottom = rect.top + 50;
	rect.right = hlp_margin + 2 * hlp_sep + 7 * hlp_grid + hlp_box_size;
	FillRect(hdc, &rect, standard_bg);


	SetBkColor(hdc, bg_col);
	rect.top += 7;
	rect.left += 7;
	DrawText(hdc, "US-2400 On-Screen Help - F-Key + Shift to close", -1, &rect, 0);

	rect.top = hlp_margin + 0 * hlp_sep + 0 * hlp_grid + 25;
	rect.left = hlp_margin + 2 * hlp_sep + 4 * hlp_grid;
	rect.right = hlp_margin + 2 * hlp_sep + 6 * hlp_grid;
	FillRect(hdc, &rect, mode_bg);

	SetBkColor(hdc, mode_col);
	rect.top += 7;
	rect.left += 7;
	if (hlp_mode == 0) DrawText(hdc, "Mode: Normal (Pan)", -1, &rect, 0);
	else if (hlp_mode == 1) DrawText(hdc, "Mode: Channel Strip", -1, &rect, 0);
	else if (hlp_mode == 2) DrawText(hdc, "Mode: Aux Sends", -1, &rect, 0);

	rect.top = hlp_margin + 0 * hlp_sep + 0 * hlp_grid + 25;
	rect.left = hlp_margin + 2 * hlp_sep + 6 * hlp_grid;
	rect.right = hlp_margin + 2 * hlp_sep + 7 * hlp_grid + hlp_box_size;
	FillRect(hdc, &rect, qkey_bg);

	rect.top += 7;
	rect.left += 7;
	SetBkColor(hdc, qkey_col);
	if (hlp_qkey == 0) DrawText(hdc, "Qualifier Key: None", -1, &rect, 0);
	else if (hlp_qkey == 1) DrawText(hdc, "Qualifier Key: Shift", -1, &rect, 0);
	else if (hlp_qkey == 2) DrawText(hdc, "Qualifier Key: F-Key", -1, &rect, 0);
	else if (hlp_qkey == 3) DrawText(hdc, "Qualifier Key: M-Key", -1, &rect, 0);

	// draw fader tracks
	rect.top = hlp_margin + 1 * hlp_sep + 4 * hlp_grid;
	rect.left = hlp_margin + 0 * hlp_sep + 0 * hlp_grid + hlp_box_size / 2 - 2;
	rect.bottom = hlp_margin + 2 * hlp_sep + 6 * hlp_grid + hlp_box_size;
	rect.right = hlp_margin + 0 * hlp_sep + 0 * hlp_grid + hlp_box_size / 2 + 2;
	FillRect(hdc, &rect, standard_bg);

	rect.left = hlp_margin + 1 * hlp_sep + 1 * hlp_grid + hlp_box_size / 2 - 2;
	rect.right = hlp_margin + 1 * hlp_sep + 1 * hlp_grid + hlp_box_size / 2 + 2;
	FillRect(hdc, &rect, standard_bg);


	// draw boxes
	Hlp_DrawBox("Encoders", hlp_margin + 0 * hlp_sep + 0 * hlp_grid, hlp_margin + 0 * hlp_sep + 0 * hlp_grid, flip_col, mode_col, qkey_col, hlp_enc_str[hlp_mode][hlp_qkey][flip_i], &hdc);

	Hlp_DrawBox("Tracks: Select", hlp_margin + 1 * hlp_sep + 1 * hlp_grid, hlp_margin + 0 * hlp_sep + 0 * hlp_grid, bg_col, mode_col, qkey_col, hlp_tksel_str[hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("Tracks: Solo", hlp_margin + 1 * hlp_sep + 2 * hlp_grid, hlp_margin + 0 * hlp_sep + 0 * hlp_grid, bg_col, bg_col, qkey_col, hlp_tksolo_str[hlp_qkey], &hdc);
	Hlp_DrawBox("Tracks: Mute", hlp_margin + 1 * hlp_sep + 3 * hlp_grid, hlp_margin + 0 * hlp_sep + 0 * hlp_grid, bg_col, bg_col, qkey_col, hlp_tkmute_str[hlp_qkey], &hdc);
	Hlp_DrawBox("Track Faders", hlp_margin + 2 * hlp_sep + 4 * hlp_grid, hlp_margin + 0 * hlp_sep + 0 * hlp_grid, flip_col, mode_col, qkey_col, hlp_tkfdr_str[hlp_mode][hlp_qkey][flip_i], &hdc);

	Hlp_DrawBox("Master Select", hlp_margin + 1 * hlp_sep + 1 * hlp_grid, hlp_margin + 1 * hlp_sep + 1 * hlp_grid, bg_col, bg_col, qkey_col, hlp_mstsel_str[hlp_qkey], &hdc);
	Hlp_DrawBox("Clear Solo", hlp_margin + 1 * hlp_sep + 2 * hlp_grid, hlp_margin + 1 * hlp_sep + 1 * hlp_grid, bg_col, bg_col, qkey_col, hlp_clsolo_str[hlp_qkey], &hdc);
	Hlp_DrawBox("Flip", hlp_margin + 1 * hlp_sep + 3 * hlp_grid, hlp_margin + 1 * hlp_sep + 1 * hlp_grid, flipbtn_col, bg_col, flipbtn_col, WDL_String("Enter Flip Mode"), &hdc);
	Hlp_DrawBox("Master Fader", hlp_margin + 2 * hlp_sep + 4 * hlp_grid, hlp_margin + 1 * hlp_sep + 1 * hlp_grid, bg_col, bg_col, qkey_col, hlp_mstfdr_str[hlp_qkey], &hdc);

	Hlp_DrawBox("Chan", hlp_margin + 1 * hlp_sep + 1 * hlp_grid, hlp_margin + 2 * hlp_sep + 2 * hlp_grid, chan_col, mode_col, chan_col, hlp_chan_str[hlp_mode], &hdc);
	Hlp_DrawBox("Pan", hlp_margin + 1 * hlp_sep + 2 * hlp_grid, hlp_margin + 2 * hlp_sep + 2 * hlp_grid, pan_col, mode_col, pan_col, WDL_String("Enter Normal (Pan) Mode"), &hdc);

	Hlp_DrawBox("1", hlp_margin + 1 * hlp_sep + 1 * hlp_grid, hlp_margin + 2 * hlp_sep + 3 * hlp_grid, auxbtn_col, mode_col, qkey_col, hlp_keys_str[0][hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("2", hlp_margin + 1 * hlp_sep + 1 * hlp_grid, hlp_margin + 2 * hlp_sep + 4 * hlp_grid, auxbtn_col, mode_col, qkey_col, hlp_keys_str[1][hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("3", hlp_margin + 1 * hlp_sep + 1 * hlp_grid, hlp_margin + 2 * hlp_sep + 5 * hlp_grid, auxbtn_col, mode_col, qkey_col, hlp_keys_str[2][hlp_mode][hlp_qkey], &hdc);

	Hlp_DrawBox("4", hlp_margin + 1 * hlp_sep + 2 * hlp_grid, hlp_margin + 2 * hlp_sep + 3 * hlp_grid, auxbtn_col, mode_col, qkey_col, hlp_keys_str[3][hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("5", hlp_margin + 1 * hlp_sep + 2 * hlp_grid, hlp_margin + 2 * hlp_sep + 4 * hlp_grid, auxbtn_col, mode_col, qkey_col, hlp_keys_str[4][hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("6", hlp_margin + 1 * hlp_sep + 2 * hlp_grid, hlp_margin + 2 * hlp_sep + 5 * hlp_grid, auxbtn_col, mode_col, qkey_col, hlp_keys_str[5][hlp_mode][hlp_qkey], &hdc);

	Hlp_DrawBox("Null", hlp_margin + 1 * hlp_sep + 3 * hlp_grid, hlp_margin + 2 * hlp_sep + 6 * hlp_grid, bg_col, mode_col, qkey_col, hlp_keys_str[6][hlp_mode][hlp_qkey], &hdc);

	if (!meter_mode)
		Hlp_DrawBox("(Meter)", hlp_margin + 1 * hlp_sep + 1 * hlp_grid, hlp_margin + 2 * hlp_sep + 7 * hlp_grid, mkey_col, bg_col, qkey_col, WDL_String("M-Key"), &hdc);

	Hlp_DrawBox("F-Key", hlp_margin + 1 * hlp_sep + 2 * hlp_grid, hlp_margin + 2 * hlp_sep + 7 * hlp_grid, fkey_col, bg_col, qkey_col, hlp_fkey_str[hlp_qkey], &hdc);

	Hlp_DrawBox("Bank -", hlp_margin + 2 * hlp_sep + 5 * hlp_grid, hlp_margin + 2 * hlp_sep + 2 * hlp_grid, bg_col, mode_col, qkey_col, hlp_bank_str[0][hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("Bank +", hlp_margin + 2 * hlp_sep + 5 * hlp_grid, hlp_margin + 2 * hlp_sep + 3 * hlp_grid, bg_col, mode_col, qkey_col, hlp_bank_str[1][hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("In", hlp_margin + 2 * hlp_sep + 5 * hlp_grid, hlp_margin + 2 * hlp_sep + 4 * hlp_grid, bg_col, bg_col, qkey_col, hlp_inout_str[0][hlp_qkey], &hdc);
	Hlp_DrawBox("Out", hlp_margin + 2 * hlp_sep + 5 * hlp_grid, hlp_margin + 2 * hlp_sep + 5 * hlp_grid, bg_col, bg_col, qkey_col, hlp_inout_str[1][hlp_qkey], &hdc);
	Hlp_DrawBox("Shift", hlp_margin + 2 * hlp_sep + 5 * hlp_grid, hlp_margin + 2 * hlp_sep + 7 * hlp_grid, shift_col, bg_col, qkey_col, hlp_shift_str[hlp_qkey], &hdc);

	Hlp_DrawBox("Rewind", hlp_margin + 2 * hlp_sep + 6 * hlp_grid, hlp_margin + 2 * hlp_sep + 2 * hlp_grid, bg_col, mode_col, qkey_col, hlp_keys_str[7][hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("Fast Forward", hlp_margin + 2 * hlp_sep + 6 * hlp_grid, hlp_margin + 2 * hlp_sep + 3 * hlp_grid, bg_col, mode_col, qkey_col, hlp_keys_str[8][hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("Stop", hlp_margin + 2 * hlp_sep + 6 * hlp_grid, hlp_margin + 2 * hlp_sep + 4 * hlp_grid, bg_col, mode_col, qkey_col, hlp_keys_str[9][hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("Play", hlp_margin + 2 * hlp_sep + 6 * hlp_grid, hlp_margin + 2 * hlp_sep + 5 * hlp_grid, bg_col, mode_col, qkey_col, hlp_keys_str[10][hlp_mode][hlp_qkey], &hdc);
	Hlp_DrawBox("Record", hlp_margin + 2 * hlp_sep + 6 * hlp_grid, hlp_margin + 2 * hlp_sep + 7 * hlp_grid, bg_col, mode_col, qkey_col, hlp_keys_str[11][hlp_mode][hlp_qkey], &hdc);

	SelectObject(hdc, rfont);
	DeleteObject(hfont);
	DeleteObject(rfont);

	DeleteObject(standard_bg);
	DeleteObject(mode_bg);
	DeleteObject(qkey_bg);
}

void CSurf_US2400_helpoverlay::Hlp_ToggleWindow(bool isMeterMode)
{	
	meter_mode = isMeterMode;
	if (hlp_open == 0)
	{
		if (hlp_hwnd == NULL)
		{
			RECT scr;
			int width = 2 * hlp_margin + 2 * hlp_sep + 7 * hlp_grid + hlp_box_size;
			int height = 2 * hlp_margin + 2 * hlp_sep + 6 * hlp_grid + hlp_box_size;
			#ifdef _WIN32
				SystemParametersInfo(SPI_GETWORKAREA, 0, &scr, 0);
				int left = scr.right / 2 - width / 2;
				int top = scr.bottom / 2 - height / 2;
			#else
				#ifdef __APPLE__
					auto mainDisplayId = CGMainDisplayID();
					int left = CGDisplayPixelsWide(mainDisplayId) / 2 - width / 2;
					int top = CGDisplayPixelsHigh(mainDisplayId) / 2 - height / 2;
				#else
					Display *dpy;
					int snum;
					dpy = XOpenDisplay(0);
					snum = DefaultScreen(dpy);
					int left  = DisplayWidth(dpy, snum) / 2 - width / 2;
					int top = DisplayHeight(dpy, snum) / 2 - width / 2;
				#endif
			#endif
			#ifdef _WIN32
				hlp_hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE, "hlp", "US-2400 On-Screen Help", WS_POPUP | WS_BORDER, left, top, width, height, NULL, NULL, g_hInst, NULL);

				SetLayeredWindowAttributes(hlp_hwnd, NULL, 220, LWA_ALPHA);				
			#else
				hlp_hwnd = CreateDialog(g_hInst, MAKEINTRESOURCE(0), nullptr, (WNDPROC)Hlp_WindowProc);
				SetWindowLong(hlp_hwnd, GWL_STYLE, WS_CAPTION|WS_BORDER|WS_SYSMENU);
				SetWindowText(hlp_hwnd, "US-2400 On-Screen Help");
				SetWindowPos(hlp_hwnd, HWND_TOPMOST, left, top, width, height, SWP_SHOWWINDOW | SWP_NOCOPYBITS);
			#endif
		}

		ShowWindow(hlp_hwnd, SW_SHOW);
		#ifdef _WIN32
			UpdateLayeredWindow(hlp_hwnd, NULL, NULL, NULL, NULL, NULL, RGB(0, 0, 0), NULL, ULW_COLORKEY);
		#else
		#endif

		hlp_open = 1;
	}
	else {
		if (hlp_hwnd != NULL)
		{
			DestroyWindow(hlp_hwnd);
			hlp_hwnd = NULL;
		}

		hlp_open = 0;
	}
} // Hlp_ToggleWindow


void CSurf_US2400_helpoverlay::Hlp_Update()
{
	if (hlp_hwnd != NULL)
	{
		InvalidateRect(hlp_hwnd, NULL, true);
		UpdateWindow(hlp_hwnd);
	}
}
