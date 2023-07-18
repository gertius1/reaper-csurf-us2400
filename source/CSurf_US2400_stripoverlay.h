#pragma once
#include "us2400.h"
#include "csurf_utils.h"

class CSurf_US2400_stripoverlay
{
	private:
		HWND stp_hwnd;
		// WNDCLASSEX stp_class;

		bool stp_repaint;
		int stp_width;
		int stp_height;
		int stp_x;
		int stp_y;
		int stp_open;

		WDL_String stp_strings[49];
		int stp_colors[24];
		unsigned long stp_enc_touch;
		unsigned long stp_enc_touch_prev;
		unsigned long stp_fdr_touch;
		unsigned long stp_fdr_touch_prev;
		unsigned long stp_sel;
		unsigned long stp_rec;
		unsigned long stp_mute;
		bool stp_chan;
		bool stp_flip;

		midi_Output* m_midiout;
	
	public:
		CSurf_US2400_stripoverlay(midi_Output* midiout);
		void sendMidi(int ch, bool m_chan, int updateModeHardwareLCD);
		void UpdateDisplay(int chan_fx, int chan_par_offs, int s_touch_fdr, int s_touch_enc[24], int s_ch_offset, MediaTrack* chan_rpr_tk, bool m_chan, bool m_flip);
		void Stp_CloseWindow();
		void Stp_OpenWindow(int chan_fx, int chan_par_offs, int s_touch_fdr, int s_touch_enc[24], int s_ch_offset, MediaTrack* chan_rpr_tk, bool m_chan, bool m_flip);
		void Stp_Paint(HWND hwnd);
		void Stp_RetrieveCoords();
		void Stp_SaveCoords();
		void Stp_StoreWinCoords(HWND hwnd);
		void Stp_Update(int ch, int chan_fx, int chan_par_offs, int s_touch_fdr, int s_touch_enc[24], int s_ch_offset, MediaTrack* chan_rpr_tk, bool m_flip, bool m_chan, int updateModeHardwareLCD);
		void SetRepaint();
		bool ShouldReopen();
		void ToggleWindow(int chan_fx, int chan_par_offs, int s_touch_fdr, int s_touch_enc[24], int s_ch_offset, MediaTrack* chan_rpr_tk, bool m_chan, bool m_flip);


};

