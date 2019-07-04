#include "CSurf_US2400_stripoverlay.h"

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

CSurf_US2400_stripoverlay::CSurf_US2400_stripoverlay() {
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

	// Display
	stp_hwnd = NULL;
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

	HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
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
				DrawText(hdc, "F L I P", -1, &rect, DT_CENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS);
			}

			// draw fx name
			if (stp_chan)
			{
				rect.top = F2I(single_height + touch_height + padding);
				rect.bottom = F2I(win_height - padding);

				SetTextColor(hdc, RGB(150, 150, 150));
				DrawText(hdc, stp_strings[49].Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS);
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
		DrawText(hdc, stp_strings[ch].Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS);

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
		DrawText(hdc, stp_strings[ch + 24].Get(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS);

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
			SystemParametersInfo(SPI_GETWORKAREA, 0, &scr, 0);
			stp_width = scr.right - scr.left;
			stp_height = 80;
			stp_x = 0;
			stp_y = scr.bottom - stp_height;
		}

		stp_hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, "stp", "US-2400 Display", WS_THICKFRAME | WS_POPUP, stp_x, stp_y, stp_width, stp_height, NULL, NULL, g_hInst, NULL);
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


void CSurf_US2400_stripoverlay::Stp_Update(int ch, int chan_fx, int chan_par_offs, int s_touch_fdr, int s_touch_enc[24], int s_ch_offset, MediaTrack* chan_rpr_tk, bool m_flip, bool m_chan)
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
				tk_num_c = "";
				tk_name = "";
				stp_colors[ch] = 0;
				stp_mute = stp_mute & (~(1 << ch));
				stp_sel = stp_sel & (~(1 << ch));
				stp_rec = stp_rec & (~(1 << ch));
			}


			fx_amount = TrackFX_GetNumParams(tk, chan_fx);
			if (ch + chan_par_offs < fx_amount)
			{
				// fx param value
				TrackFX_GetFormattedParamValue(tk, chan_fx, ch + chan_par_offs, buffer, 64);
				if (strlen(buffer) == 0)
				{
					double min, max;
					double par = TrackFX_GetParam(tk, chan_fx, ch + chan_par_offs, &min, &max);
					sprintf(buffer, "%.4f", par);
				}
				par_val = WDL_String(buffer);

				// fx param name
				TrackFX_GetParamName(tk, chan_fx, ch + chan_par_offs, buffer, 64);
				par_name = WDL_String(buffer);

			}
			else
			{
				par_val = "";
				par_name = "";
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
				stp_strings[49] = WDL_String(buffer);
				stp_strings[49] = csurf_utils::Utl_Alphanumeric(stp_strings[49]);
			}

			// keep only alphanumeric, replace everything else with space
			stp_strings[ch + 24] = csurf_utils::Utl_Alphanumeric(stp_strings[ch + 24]);

			stp_repaint = true;
		}
	}
} // Stp_Update


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
				Stp_Update(ch, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan);
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

