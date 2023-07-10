#include "CSurf_US2400_stripoverlay.h"
#ifndef _WIN32
	#ifndef __APPLE__
		#include <X11/Xlib.h>
	#else 
		#include <CoreGraphics/CGDisplayConfiguration.h>
	#endif
#endif

// float to int macro
#define F2I(x) (int)((x) + 0.5)

static CSurf_US2400_stripoverlay* stpHandler;

LRESULT CALLBACK Stp_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
		stpHandler->Stp_Paint(hwnd);
		break;

	case WM_SIZE:
		stpHandler->SetRepaint();
		stpHandler->Stp_StoreWinCoords(hwnd);
		break;

	case WM_MOVE:
		stpHandler->SetRepaint();
		stpHandler->Stp_StoreWinCoords(hwnd);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

CSurf_US2400_stripoverlay::CSurf_US2400_stripoverlay(midi_Output* midiout) {
	m_midiout = midiout;
	stp_repaint = false;
	stp_width = -1;
	stp_height = -1;
	stp_x = -1;
	stp_y = -1;
	stp_open = 0;

	stp_enc_touch = 0;
	stp_enc_touch_prev = 0;
	stp_fdr_touch = 0;
	stp_fdr_touch_prev = 0;
	stp_sel = 0;
	stp_rec = 0;
	stp_mute = 0;
	stp_chan = false;
	stp_flip = false;

	for (int i = 0; i < 24; i++)
		stp_colors[i] = 0;

	// Display
	stp_hwnd = NULL;

#ifdef _WIN32
	WNDCLASSEX stp_class;
	stp_class.cbSize = sizeof(WNDCLASSEX);
	stp_class.style = 0;
	stp_class.lpfnWndProc = (WNDPROC)Stp_WindowProc;
	stp_class.cbClsExtra = 0;
	stp_class.cbWndExtra = 0;
	stp_class.hInstance = g_hInst;
	stp_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	stp_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	stp_class.hbrBackground = CreateSolidBrush(RGB(60, 60, 60));
	stp_class.lpszMenuName = NULL;
	stp_class.lpszClassName = "stp";
	stp_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&stp_class);
#endif
	stpHandler = this;
}

void CSurf_US2400_stripoverlay::SetRepaint() { stp_repaint = true; }

void CSurf_US2400_stripoverlay::Stp_Paint(HWND hwnd)
{
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int win_width = rect.right - rect.left;
	int win_height = rect.bottom - rect.top;

	HDC hdc;
	PAINTSTRUCT ps;
	hdc = BeginPaint(hwnd, &ps);
	#ifdef _WIN32
		HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	#else
		HFONT hfont = (HFONT)GetStockObject(17);
	#endif
	HFONT rfont = (HFONT)SelectObject(hdc, hfont);

	COLORREF bg_col = RGB(60, 60, 60);
	COLORREF separator_col = RGB(90, 90, 90);

	HBRUSH separator_bg = CreateSolidBrush(separator_col);
	HBRUSH touch_bg = CreateSolidBrush(RGB(240, 120, 120));

	HPEN white_ln = CreatePen(PS_SOLID, 1, RGB(250, 250, 250));
	HPEN lgrey_ln = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
	HPEN grey_ln = CreatePen(PS_SOLID, 1, RGB(90, 90, 90));

	double box_width = (win_width - 26.0) / 26.0;

	double touch_height = 3.0;
	double single_height = 16.0;
	double padding = 2.0;

	double x = 0;

	for (char ch = 0; ch < 24; ch++)
	{
		// draw separators
		if ((ch % 8 == 0) && (ch != 0))
		{
			rect.top = 0;
			rect.left = F2I(x);
			rect.right = F2I(x + box_width + 1);
			rect.bottom = F2I(win_height);
			FillRect(hdc, &rect, separator_bg);

			x += box_width;

			// draw info
			SetBkColor(hdc, separator_col);

			// draw flip
			if (stp_flip) {

				rect.top = F2I(padding);
				rect.bottom = F2I(single_height - padding);

				SetTextColor(hdc, RGB(240, 60, 60));
				#ifdef _WIN32
					DrawText(hdc, "F L I P", -1, &rect, DT_CENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS);
				#else
					DrawText(hdc, "F L I P", -1, &rect, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS );
				#endif
			}

			// draw fx name
			if (stp_chan)
			{
				rect.top = F2I(single_height + touch_height + padding);
				rect.bottom = F2I(win_height - padding);

				SetTextColor(hdc, RGB(150, 150, 150));
				#ifdef _WIN32
					DrawText(hdc, stp_strings[48].Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS);
				#else
					DrawText(hdc, stp_strings[48].Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS );
				#endif
			}
		}

		// draw and set background if applicable
		if (stp_colors[ch] != 0) {

			int r = GetRValue(stp_colors[ch]) / 4 + GetRValue(bg_col) / 2;
			int g = GetGValue(stp_colors[ch]) / 4 + GetGValue(bg_col) / 2;
			int b = GetBValue(stp_colors[ch]) / 4 + GetBValue(bg_col) / 2;

			HBRUSH tkbg = CreateSolidBrush(RGB(r, g, b));

			rect.top = 0;
			rect.left = F2I(x);
			rect.right = F2I(x + box_width);
			rect.bottom = F2I(win_height);
			FillRect(hdc, &rect, tkbg);

			DeleteObject(tkbg);

			SetBkColor(hdc, RGB(r, g, b));

		}
		else SetBkColor(hdc, bg_col);

		rect.left = F2I(x + padding);
		rect.right = F2I(x + box_width - padding);

		// draw text A
		SetTextColor(hdc, RGB(150, 150, 150));
		if ((!stp_chan && (stp_sel & (1 << ch))) || ((stp_chan) && (stp_enc_touch & (1 << ch))))
			SetTextColor(hdc, RGB(250, 250, 250));
		else if (!stp_chan && (stp_rec & (1 << ch)))
			SetTextColor(hdc, RGB(250, 60, 60));

		rect.top = F2I(padding);
		rect.bottom = F2I(single_height - padding);
		#ifdef _WIN32
			DrawText(hdc, stp_strings[ch].Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS);
		#else
			DrawText(hdc, stp_strings[ch].Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS);
		#endif

		// draw touch
		rect.top = F2I(single_height);
		rect.bottom = F2I(single_height + touch_height);

		if (((stp_fdr_touch & (1 << ch)) != 0) || ((stp_enc_touch & (1 << ch)) != 0))
		{
			FillRect(hdc, &rect, touch_bg);

		}
		else
		{
			SelectObject(hdc, lgrey_ln);
			int y = F2I(rect.top + ((rect.bottom - rect.top) / 2));
			MoveToEx(hdc, rect.left, y, NULL);
			LineTo(hdc, rect.right, y);
		}

		// draw text B
		SetTextColor(hdc, RGB(250, 250, 250));
		if (!stp_chan && (stp_mute & (1 << ch))) SetTextColor(hdc, RGB(150, 150, 150));

		rect.top = F2I(single_height + touch_height + padding);
		rect.bottom = F2I(win_height - padding);
		#ifdef _WIN32
			DrawText(hdc, stp_strings[ch + 24].Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS);
		#else
			DrawText(hdc, stp_strings[ch + 24].Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS);
		#endif

		// dividers
		bool draw = false;
		if ((ch + 1) % 2 != 0)
		{
			draw = true;
			SelectObject(hdc, grey_ln);

		}
		else if ((ch + 1) % 4 != 0)
		{
			draw = true;
			SelectObject(hdc, lgrey_ln);

		}
		else if ((ch + 1) % 8 != 0)
		{
			draw = true;
			SelectObject(hdc, white_ln);
		}

		x += box_width;
		if (draw)
		{
			MoveToEx(hdc, F2I(x), 0, NULL);
			LineTo(hdc, F2I(x), F2I(win_height));
		}

		x += 1.0;
	}

	SelectObject(hdc, rfont);
	DeleteObject(hfont);
	DeleteObject(rfont);

	DeleteObject(separator_bg);
	DeleteObject(touch_bg);

	DeleteObject(white_ln);
	DeleteObject(lgrey_ln);
	DeleteObject(grey_ln);

	EndPaint(hwnd, &ps);
}

void CSurf_US2400_stripoverlay::Stp_StoreWinCoords(HWND hwnd)
{
	RECT rect;
	GetWindowRect(hwnd, &rect);
	stp_x = rect.left;
	stp_y = rect.top;
	stp_width = rect.right - rect.left;
	stp_height = rect.bottom - rect.top;
}

void CSurf_US2400_stripoverlay::Stp_OpenWindow(int chan_fx, int chan_par_offs, int s_touch_fdr, int s_touch_enc[24], int s_ch_offset, MediaTrack* chan_rpr_tk, bool m_chan, bool m_flip)
{
	if (stp_hwnd == NULL)
	{
		if (stp_width == -1 || stp_height == -1)
		{
			RECT scr;
			stp_height = 80;
			stp_x = 0;		 

			#ifdef _WIN32
				SystemParametersInfo(SPI_GETWORKAREA, 0, &scr, 0);
				stp_width = scr.right - scr.left;
				stp_y = scr.bottom - stp_height;
			#else
				#ifdef __APPLE__
					auto mainDisplayId = CGMainDisplayID();
					stp_width = CGDisplayPixelsWide(mainDisplayId);
					auto height = CGDisplayPixelsHigh(mainDisplayId);
					stp_y = height - stp_height;
				#else
					Display *dpy;
					int width, height, snum;
					dpy = XOpenDisplay(0);
					snum = DefaultScreen(dpy);
					width = DisplayWidth(dpy, snum);
					height = DisplayHeight(dpy, snum);

					stp_width = width - 0;
					stp_y = height - stp_height;
				#endif
			#endif
		}
		#ifdef _WIN32
			stp_hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, "stp", "US-2400 Display", WS_THICKFRAME | WS_POPUP, stp_x, stp_y, stp_width, stp_height, NULL, NULL, g_hInst, NULL);
		#else
			stp_hwnd = CreateDialog(g_hInst, MAKEINTRESOURCE(0), nullptr, (WNDPROC)Stp_WindowProc);
			SetWindowLong(stp_hwnd, GWL_STYLE, WS_CAPTION|WS_THICKFRAME|WS_SYSMENU);
			SetWindowText(stp_hwnd, "US-2400 Display");
			SetWindowPos(stp_hwnd, HWND_TOPMOST, stp_x, stp_y, stp_width, stp_height, SWP_SHOWWINDOW | SWP_NOCOPYBITS);
		#endif
	}

	UpdateDisplay(chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_chan, m_flip);

	ShowWindow(stp_hwnd, SW_SHOW);
	UpdateWindow(stp_hwnd);

	stp_open = 1;
} // Stp_OpenWindow

bool CSurf_US2400_stripoverlay::ShouldReopen() {
	return stp_open == 1;
}

void CSurf_US2400_stripoverlay::ToggleWindow(int chan_fx, int chan_par_offs, int s_touch_fdr, int s_touch_enc[24], int s_ch_offset, MediaTrack* chan_rpr_tk, bool m_chan, bool m_flip) {
	if (stp_hwnd == NULL) this->Stp_OpenWindow(chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_chan, m_flip);
	else this->Stp_CloseWindow();
}


void CSurf_US2400_stripoverlay::Stp_CloseWindow()
{
	if (stp_hwnd != NULL)
	{
		DestroyWindow(stp_hwnd);
		stp_hwnd = NULL;
	}

	stp_open = 0;
} // Stp_CloseWindow


void CSurf_US2400_stripoverlay::Stp_Update(int ch, int chan_fx, int chan_par_offs, int s_touch_fdr, int s_touch_enc[24], int s_ch_offset, MediaTrack* chan_rpr_tk, bool m_flip, bool m_chan, int updateModeHardwareLCD)
{
	if (stp_hwnd != NULL)
	{
		if ((ch >= 0) && (ch < 24))
		{
			MediaTrack* tk;
			int tk_num, fx_amount;
			char buffer[64];
			WDL_String tk_name;
			WDL_String tk_num_c;
			WDL_String par_name;
			WDL_String par_val;
			bool sel = false;
			bool rec = false;
			bool muted = false;

			// get info

			if (m_chan)
				tk = chan_rpr_tk;
			else
				tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch, s_ch_offset);


			if (tk != NULL)
			{
				// track number
				tk_num = (int)GetMediaTrackInfo_Value(tk, "IP_TRACKNUMBER");
				sprintf(buffer, "%d", tk_num);
				tk_num_c = WDL_String(buffer);

				// track name
				GetSetMediaTrackInfo_String(tk, "P_NAME", buffer, false);
				buffer[63] = '\0';
				tk_name = WDL_String(buffer);

				// track mute, selected, rec arm
				int flags;
				GetTrackState(tk, &flags);

				// muted
				if ((bool)(flags & 8)) stp_mute = stp_mute | (1 << ch);
				else stp_mute = stp_mute & (~(1 << ch));

				// selected
				if ((bool)(flags & 2)) stp_sel = stp_sel | (1 << ch);
				else stp_sel = stp_sel & (~(1 << ch));

				// armed
				if ((bool)(flags & 64)) stp_rec = stp_rec | (1 << ch);
				else stp_rec = stp_rec & (~(1 << ch));

			}
			else
			{
				tk_num_c = WDL_String(0);
				tk_name = WDL_String(0);
				stp_colors[ch] = 0;
				stp_mute = stp_mute & (~(1 << ch));
				stp_sel = stp_sel & (~(1 << ch));
				stp_rec = stp_rec & (~(1 << ch));
			}


			fx_amount = TrackFX_GetNumParams(tk, chan_fx);
			if (ch + chan_par_offs < fx_amount)
			{
				// fx param value
				int remappedParam = abs(csurf_utils::TrackFX_RemapParam(ch + chan_par_offs));
				TrackFX_GetFormattedParamValue(tk, chan_fx, remappedParam, buffer, 64);
				if (strlen(buffer) == 0)
				{
					double min, max;
					double par = TrackFX_GetParam(tk, chan_fx, remappedParam, &min, &max);
					sprintf(buffer, "%.4f", par);
				}
				par_val = WDL_String(buffer);

				// fx param name
				TrackFX_GetParamName(tk, chan_fx, remappedParam, buffer, 64);
				par_name = WDL_String(buffer);

			}
			else
			{
				par_val = WDL_String(0);
				par_name = WDL_String(0);
			}

			// transmit

			stp_chan = false;
			stp_colors[ch] = GetTrackColor(tk);
			stp_strings[ch + 24] = tk_name;
			stp_strings[ch] = tk_num_c;
			stp_flip = m_flip;

			if (m_chan)
			{
				stp_chan = true;
				stp_strings[ch] = WDL_String(par_val);

				if ((!m_flip && ((s_touch_fdr & (1 << ch)) == 0)) || (m_flip && (s_touch_enc[ch] == 0)))
				{
					stp_colors[ch] = 0;
					stp_strings[ch + 24] = par_name;
				}

				TrackFX_GetFXName(chan_rpr_tk, chan_fx, buffer, 64);
				stp_strings[48] = WDL_String(buffer);
				stp_strings[48] = csurf_utils::Utl_Alphanumeric(stp_strings[48]);
			}

			// keep only alphanumeric, replace everything else with space
			stp_strings[ch + 24] = csurf_utils::Utl_Alphanumeric(stp_strings[ch + 24]);

			stp_repaint = true;

			if (m_midiout)
				if (updateModeHardwareLCD)
					sendMidi(ch, m_chan, updateModeHardwareLCD);
		}
	}
} // Stp_Update

void CSurf_US2400_stripoverlay::sendMidi(int ch, bool m_chan, int updateModeHardwareLCD)
{

	// Multiple Messages protocol; collect first into a big message buffer, then split up into display messages
	// 10 Chars Small Font
	const char nrOfLCDs = 18;
	const char nrOfDigitsOnLCD = 10;
	const char nrOfDigitsOnLCDLargeFont = 5;
	const char spacerSize = 2;
	char indentSizeTop = 0;
	if (m_chan)
		indentSizeTop = 1;
	else
		indentSizeTop = 2;
	const char indentSizeBottom = 1;
	const char displayLineSize = 16;
	const char displayLineSizeLargeFont = 8;
	const int singleMessageSize = 39; //Sysex Header 6/Footer 1 Byte and message content 32 Byte
	const int sysexHeaderSize = 6;
	const int sysexFooterSize = 1;
	const int nrOf_stp_strings = 49;

	bool updateFlags[nrOfLCDs]; 
	for (int i = 0; i < nrOfLCDs; i++)
		updateFlags[i] = false;

	const int totalMessageSize = nrOf_stp_strings * displayLineSize + sysexHeaderSize + sysexFooterSize;

	unsigned char bigBuf[totalMessageSize]; //collect everything here

	char lineBuf[100];
	memset(lineBuf, 0, 100);


	memset(bigBuf, 0, totalMessageSize);

	for (int i = 0; i < nrOf_stp_strings-1; i++)
	{
	
		for (int j = 0; j < nrOfDigitsOnLCD; j++)
			lineBuf[j] = 0;
		
		strcpy(lineBuf, stp_strings[i].Get());
		
		for (int j = 0; j < nrOfDigitsOnLCD; j++)
		{
			if (lineBuf[j] > 0x7F)
				lineBuf[j] = 0x20;
			if (lineBuf[j] == 0x00) //replace 0x00 to not terminate string on output
				lineBuf[j] = 0x20;
		}
		

		int offset = indentSizeBottom;
		if (i < 24)
			offset = indentSizeTop; //start first line indented for channel numbers

		
		if (!m_chan && i < 24) //print large track numbers in PAN mode
			memcpy(bigBuf + i * (nrOfDigitsOnLCDLargeFont + 1) + offset, lineBuf, nrOfDigitsOnLCDLargeFont - offset); //only nrOfDigitsOnLCD digits available
		else
		memcpy(bigBuf + i * (nrOfDigitsOnLCD + spacerSize) + offset, lineBuf, nrOfDigitsOnLCD - offset); //only nrOfDigitsOnLCD digits available
		memcpy(bigBuf + i * (nrOfDigitsOnLCD + spacerSize) + nrOfDigitsOnLCD, "||", 2); //spacer
	}

	// Limit data to SysEx range
	for (int i = 0; i < totalMessageSize-1; i++)
	{
		if (bigBuf[i] > 0x7F)
			bigBuf[i] = 0x20;
		if (bigBuf[i] == 0x00) //replace 0x00 to not terminate string on output
			bigBuf[i] = 0x20;
	}


	unsigned char messBuf[singleMessageSize];
	messBuf[0] = 0xF0;
	messBuf[1] = 0x00;
	messBuf[2] = 0x00;
	messBuf[3] = 0x66;
	//messBuf[4] = 0x17;
	messBuf[5] = m_chan;
	messBuf[singleMessageSize-1] = 0xF7;

	if (m_midiout) {

		

		if (updateModeHardwareLCD == 0) //no update of hardware LCD
		{
			return;
		}
		else if (updateModeHardwareLCD == 1) //update of specific hardware LCDs
		{
			for (int i = 0; i < nrOfLCDs; i++)
				updateFlags[i] = false;
			//determine, which LCDs need to be updated
			switch (ch % 4)
			{
			case 0:
				updateFlags[(ch / 4) * 3 + ch % 4] = true;
				break;
			case 1:
				updateFlags[(ch / 4) * 3 + ch % 4 - 1] = true;
				updateFlags[(ch / 4) * 3 + ch % 4] = true;
				break;
			case 2:
				updateFlags[(ch / 4) * 3 + ch % 4 - 1] = true;
				updateFlags[(ch / 4) * 3 + ch % 4] = true;
				break;
			case 3:
				updateFlags[(ch / 4) * 3 + ch % 4 - 1] = true;
				break;
			default:
				break;
			}
		}
		else if (updateModeHardwareLCD == 2)  //update of all hardware LCDs
		{
			for (int i = 0; i < nrOfLCDs; i++)
				updateFlags[i] = true;
		}

		//now that the LCDs that need to be updated have been determined, cycle through them all, and skip the ones that are already up-to-date
		for (int i = 0; i < nrOfLCDs; i++) //one Midi message per display
		{
			if (updateFlags[i] == false)
				continue;

			messBuf[4] = i; //display Nr
			if (!m_chan && i < 24) //print large track numbers in PAN mode
				memcpy(messBuf + sysexHeaderSize, bigBuf + i * displayLineSizeLargeFont, displayLineSize);//1st line
			else
				memcpy(messBuf + sysexHeaderSize, bigBuf + i * displayLineSize, displayLineSize);//1st line
			memcpy(messBuf + sysexHeaderSize + displayLineSize, bigBuf + i * displayLineSize + nrOfLCDs * displayLineSize, displayLineSize);//2nd line

			MIDI_event_t msg;
			msg.frame_offset = -1;
			msg.size = singleMessageSize;
			memcpy(msg.midi_message, messBuf, sizeof(messBuf));
			m_midiout->SendMsg(&msg, -1);

			updateFlags[i] = false;
		}

		//display 19: VST name in m_chan mode; 32 chars = 2*displayLineSize
		{
			messBuf[4] = 18; //display Nr
			if (m_chan)
				strcpy(lineBuf, stp_strings[48].Get());
			else
				memset(lineBuf, 0x20, 2* displayLineSize);

			for (int j = 0; j < 2 * displayLineSize; j++)
			{
				if (lineBuf[j] > 0x7F)
					lineBuf[j] = 0x20;
				if (lineBuf[j] == 0x00) //replace 0x00 to not terminate string on output
					lineBuf[j] = 0x20;
			}
			memcpy(messBuf + sysexHeaderSize, lineBuf, 2*displayLineSize);//1st line

			MIDI_event_t msg;
			msg.frame_offset = -1;
			msg.size = singleMessageSize;
			memcpy(msg.midi_message, messBuf, sizeof(messBuf));
			m_midiout->SendMsg(&msg, -1);
		}
	}


	/*
	/// WORKING ///
	// Multiple Messages protocol; collect first into a big message buffer, then split up into display messages
	// 5 Chars Big Font
	const char nrOfDigitsOnLCD = 5;
	const char displayLineSize = 8;
	const int totalMessageSize = 300;
	const int singleMessageSize = 22; //Sysex Header/Footer 6 Byte and message content 16 Byte


	unsigned char bigBuf[300]; //collect everything here

	for (int i = 0; i < 49; i++)
	{
		int offset = 0;
		if (i < 24)
			offset = 2; //start first line indented for channel numbers

		memcpy(bigBuf + i * (nrOfDigitsOnLCD + 1) + offset, stp_strings[i].Get(), nrOfDigitsOnLCD - offset); //only 5 digits available
		memcpy(bigBuf + i * (nrOfDigitsOnLCD + 1) + nrOfDigitsOnLCD, "|", 1); //spacer
	}

	// Limit data to SysEx range
	for (int i = 0; i < 299; i++)
	{
		if (bigBuf[i] > 0x7F)
			bigBuf[i] = 0x20;
		if (bigBuf[i] == 0x00) //replace 0x00 to not terminate string on output
			bigBuf[i] = 0x20;
	}
	

	unsigned char messBuf[singleMessageSize];
	messBuf[0] = 0xF0;
	messBuf[1] = 0x00;
	messBuf[2] = 0x00;
	messBuf[3] = 0x66;
	//messBuf[4] = 0x17;
	messBuf[21] = 0xF7;

	if (m_midiout) {

		for (int i = 0; i < 18; i++) //one Midi message per display line
		{
			messBuf[4] = i; //display Nr
			memcpy(messBuf+5, bigBuf+i* displayLineSize, displayLineSize);//1st line
			memcpy(messBuf + 5 + displayLineSize, bigBuf + i * displayLineSize + 18* displayLineSize, displayLineSize);//2nd line

			MIDI_event_t msg;
			msg.frame_offset = -1;
			msg.size = singleMessageSize;
			memcpy(msg.midi_message, messBuf, sizeof(messBuf));
			m_midiout->SendMsg(&msg, -1);
		}
	}

	*/
}


void CSurf_US2400_stripoverlay::UpdateDisplay(int chan_fx, int chan_par_offs, int s_touch_fdr, int s_touch_enc[24], int s_ch_offset, MediaTrack* chan_rpr_tk, bool m_chan, bool m_flip) {
	// update Strip Display
	if (stp_hwnd != NULL)
	{

		stp_fdr_touch = s_touch_fdr;
		stp_enc_touch = 0;
		for (char ch = 0; ch < 24; ch++)
		{
			if (m_chan && (s_touch_enc[ch] > 0)) stp_enc_touch = stp_enc_touch | (1 << ch);

			if ((stp_enc_touch & (1 << ch)) != (stp_enc_touch_prev & (1 << ch))
				|| (stp_fdr_touch & (1 << ch)) != (stp_fdr_touch_prev & (1 << ch)))
				Stp_Update(ch, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, false);
		}

		stp_enc_touch_prev = stp_enc_touch;
		stp_fdr_touch_prev = stp_fdr_touch;

		if (stp_repaint)
		{
			InvalidateRect(stp_hwnd, NULL, true);
			UpdateWindow(stp_hwnd);
			stp_repaint = false;
		}
	}
}

void CSurf_US2400_stripoverlay::Stp_RetrieveCoords()
{
	if (HasExtState("US2400", "stp_x")) stp_x = atoi(GetExtState("US2400", "stp_x"));
	else stp_x = -1;

	if (HasExtState("US2400", "stp_y")) stp_y = atoi(GetExtState("US2400", "stp_y"));
	else stp_y = -1;

	if (HasExtState("US2400", "stp_height")) stp_height = atoi(GetExtState("US2400", "stp_height"));
	else stp_height = -1;

	if (HasExtState("US2400", "stp_width")) stp_width = atoi(GetExtState("US2400", "stp_width"));
	else stp_width = -1;

	if (HasExtState("US2400", "stp_open")) stp_open = atoi(GetExtState("US2400", "stp_open"));
	else stp_open = 0;
} // Stp_RetrieveCoords


void CSurf_US2400_stripoverlay::Stp_SaveCoords()
{
	char buffer[32];

	sprintf(buffer, "%d", stp_x);
	SetExtState("US2400", "stp_x", buffer, true);

	sprintf(buffer, "%d", stp_y);
	SetExtState("US2400", "stp_y", buffer, true);

	sprintf(buffer, "%d", stp_width);
	SetExtState("US2400", "stp_width", buffer, true);

	sprintf(buffer, "%d", stp_height);
	SetExtState("US2400", "stp_height", buffer, true);

	sprintf(buffer, "%d", stp_open);
	SetExtState("US2400", "stp_open", buffer, true);
} // Stp_SaveCoords

