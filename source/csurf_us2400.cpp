/*
** reaper_csurf
** Tascam US-2400 support
** Cobbled together by David Lichtenberger
** 
** No license, no guarantees.
*/

#include <string>
#include <fstream>

////// PREFERENCES //////

#define DESCSTRING "Tascam US-2400 with M-Key & VU-Meters" // "Tascam US-2400 (with VU-Meters)" //

#define DEFAULT_BANK_SWITCH 24
#define SHIFT_BANK_SWITCH 8

#define CONFIG_FLAG_BANK_SWITCH 2
#define CONFIG_FLAG_NO_FLASH 4

// Meter: -inf = x dB
#define MINUSINF -90.0

// period in which M qualifier is active (in Run cycles)
#define MDELAY 50

// Encoder resolution for volume, range: 0 > 16256
#define ENCRESVOL 100
// Encoder resolution for pan, range:  -1 > +1
#define ENCRESPAN 0.01 
// Encoder resolution for fx param, in divisions of range (e.g. -1 > +1, reso 100 > stepsize 0.02)
#define ENCRESFX 100
#define ENCRESFXFINE 3000
#define ENCRESFXTOGGLE 1
// How long will encoders be considered touch, in run circles (15 Hz -> 3 circles = 200ms)
#define ENCTCHDLY 10

// Wheel resolution for fast scrub
#define SCRUBRESFAST 1
// Wheel resolution for slow scrub
#define SCRUBRESSLOW 5
// Wheel resolution for edit cursor move, fast
#define MOVERESFAST 10
// Wheel resolution for edit cursor move, slow
#define MOVERESSLOW 50

// Blink interval / ratio (1 = appr. 30 Hz / 0.03 s)
#define MYBLINKINTV 20
#define MYBLINKRATIO 1

// Execute only X Faders/Encoders at a time
#define EXLIMIT 10

// For finding sends see MyCSurf_Aux_Send)
#define AUXSTRING "aux---%d"


////// DEBUG //////

// debug macros
#define DBGS(x) ShowConsoleMsg(x);
#define DBGF(x) sprintf(debug, "%f   ", x); ShowConsoleMsg(debug);
#define DBGD(x) sprintf(debug, "%d   ", x); ShowConsoleMsg(debug);
#define DBGX(x) sprintf(debug, "%x   ", x); ShowConsoleMsg(debug);
#define DBGB(x) if(x) ShowConsoleMsg("true   "); else ShowConsoleMsg("false   ");
#define DBGN ShowConsoleMsg("\n");



// Command Lookup
#define CMD(x) NamedCommandLookup(x)

// Unnamed Commands
#define CMD_SELALLITEMS 40182
#define CMD_UNSELALLITEMS 40289
#define CMD_TIMESEL2ITEMS 40290
#define CMD_CLEARTIMESEL 40635
#define CMD_TGGLRECBEAT 40045
#define CMD_AUTOTOSEL 41160
#define CMD_FXBROWSER 40271
#define CMD_SEL2LASTTOUCH 40914

#include "csurf_us2400.h"

// for debug  
char debug[64];

class CSurf_US2400;


inline bool dblEq(double a, double b, double prec) {
if ( ((fabs(a) - fabs(b)) < prec) && ((fabs(a) - fabs(b)) > (0 - prec)) )
  {
    return true;
  } else
  {
    return false;
  }
}


// CSURF CLASS

class CSurf_US2400 : public IReaperControlSurface
{
  int m_midi_in_dev,m_midi_out_dev, m_midi_out_dev_disp;
  int m_offset, m_size;
  midi_Output *m_midiout;
  midi_Output* m_midiout_disp;
  midi_Input *m_midiin;
  int m_cfg_flags;  // config_flag_fader_touch_mode etc

  WDL_String descspace;
  char configtmp[1024];


  // cmd_ids
  int cmd_ids[4][3][12]; //[none/shift/fkey/mkey][pan/chan/aux][1-6/null/rew/ffwd/stop/play/rec]

  // buffer for fader data
  bool waitformsb;
  unsigned char lsb;

  // for myblink
  bool s_myblink;  
  int myblink_ctr;

  // for init
  bool s_initdone;
  bool s_exitdone;

  // touchstates
  unsigned long s_touch_fdr;
  int s_touch_enc[24];

  // caches
  int cache_faders[25];
  int cache_enc[24];
  int cache_meters[24];
  unsigned long cache_upd_faders;
  unsigned long cache_upd_enc;
  unsigned long cache_upd_meters;
  char cache_exec;
  bool master_sel;

  // button states
  std::map<char, bool[2]> button_states;

  // general states
  int s_ch_offset; // bank up/down
  bool s_play, s_rec, s_loop; // play states
  char s_automode; // automation modes

  // modes
  bool m_flip, m_chan, m_pan, m_scrub;
  char m_aux;

  // qualifier keys
  bool q_fkey, q_shift, q_mkey;
  int s_mkey_on;

  // for channel strip
  MediaTrack* chan_rpr_tk;
  char chan_ch;
  int chan_fx;
  int chan_fx_count;
  int chan_par_offs;
  bool chan_fx_env_arm;

  // save track sel
  MediaTrack** saved_sel;
  int saved_sel_len;

  // loop all
  bool s_loop_all;
  double s_ts_start;
  double s_ts_end;

  // on screen help
  CSurf_US2400_helpoverlay* hlpHandler;
  CSurf_US2400_stripoverlay* stpHandler;

  //////// MIDI ////////

  void MIDIin(MIDI_event_t *evt)
  {
    unsigned char ch_id;
    
    bool btn_state = false;
    if (evt->midi_message[2] == 0x7f) btn_state = true;

    // msb of fader move?

    if ( (waitformsb) && (evt->midi_message[0] == 0xb0) && (evt->midi_message[1] < 0x19) ) {

      ch_id = evt->midi_message[1];
      int value = (evt->midi_message[2] << 7) | lsb;

      OnFaderChange(ch_id, value);

      waitformsb = false;

    } else {

      // buttons / track elements
      if (evt->midi_message[0] == 0xb1)
      {
        // master buttons
        switch (evt->midi_message[1])
        {
          case 0x61 : if (btn_state) OnMasterSel(); break;
          case 0x62 : OnClrSolo(btn_state); break;
          case 0x63 : if (btn_state) OnFlip(); break;
          case 0x64 : if (btn_state) OnChan(); break;
          case 0x6b : OnMeter(); break;
          case 0x6c : if (btn_state) OnPan(); break;
          case 0x6d : OnFKey(btn_state); break;
          case 0x65 : if (btn_state) OnAux(1); break;
          case 0x66 : if (btn_state) OnAux(2); break;
          case 0x67 : if (btn_state) OnAux(3); break;
          case 0x68 : if (btn_state) OnAux(4); break;
          case 0x69 : if (btn_state) OnAux(5); break;
          case 0x6a : if (btn_state) OnAux(6); break;
          case 0x6e : OnNull(btn_state); break;
          case 0x6f : if (btn_state) OnScrub(); break;
          case 0x70 : OnBank(-1, btn_state); break;
          case 0x71 : OnBank(1, btn_state); break;
          case 0x72 : OnIn(btn_state); break;
          case 0x73 : OnOut(btn_state); break;
          case 0x74 : OnShift(btn_state); break;
          case 0x75 : if (btn_state) OnRew(); break;
          case 0x76 : if (btn_state) OnFwd(); break;
          case 0x77 : if (btn_state) OnStop(); break;
          case 0x78 : if (btn_state) OnPlay(); break;
          case 0x79 : if (btn_state) OnRec(); break;
          default :
          {
            // track elements
            if (evt->midi_message[1] < 0x60)
            {
              ch_id = evt->midi_message[1] / 4; // floor

              char ch_element;
              ch_element = evt->midi_message[1] % 4;    // modulo

              switch (ch_element)
              {
                case 0 : OnFaderTouch(ch_id, btn_state); break;
                case 1 : if (btn_state) OnTrackSel(ch_id); break;
                case 2 : if (btn_state) OnTrackSolo(ch_id); break;
                case 3 : if (btn_state) OnTrackMute(ch_id); break;
              } // switch (ch_element)
            } // if (evt->midi_message[1] < 0x60)
          } // default
        } // switch (evt->midi_message[1])

        // encoders, fader values, jog wheel
      } else if (evt->midi_message[0] == 0xb0) 
      {
        // calculate relative value for encoders and jog wheel
        signed char rel_value;
        if (evt->midi_message[2] < 0x40) rel_value = evt->midi_message[2];
        else rel_value = 0x40 - evt->midi_message[2];

        // fader (track and master): catch lsb - msb see above
        if ( (evt->midi_message[1] >= 0x20) && (evt->midi_message[1] <= 0x38) )
        {
          lsb = evt->midi_message[3];
          waitformsb = true;

          // jog wheel
        } else if (evt->midi_message[1] == 0x3C) 
        {

          OnJogWheel(rel_value);

          // encoders
        } else if ( (evt->midi_message[1] >= 0x40) && (evt->midi_message[1] <= 0x58) ) 
        {
          ch_id = evt->midi_message[1] - 0x40;

          OnEncoderChange(ch_id, rel_value);
        }

        // touch master fader
      } else if (evt->midi_message[0] == 0xb2) 
      {
        OnFaderTouch(24, btn_state);

        // joystick
      } else if (evt->midi_message[0] == 0xbe) 
      {
        // send on to input + 1 (Tascam[2])
        kbd_OnMidiEvent(evt, m_midi_in_dev + 1);
      } // (evt->midi_message[0] == 0xb1), else, else ...
    } // if ( (waitformsb) && (evt->midi_message[0] == 0xb0) && (evt->midi_message[1] < 0x19), else
  } // MIDIin()


  void MIDIOut(unsigned char s, unsigned char d1, unsigned char d2) 
  {
    if (m_midiout) m_midiout->Send(s, d1, d2, 0);
  } // void MIDIOut(char s, char d1, char d2)



  //////// EVENTS (called by MIDIin) //////




  // TRACK ELEMENTS

  void OnTrackSel(char ch_id)
  {
    MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);
    if (ch_id < 24)
    {
      if (q_fkey)
      {
        if (m_aux > 0) MyCSurf_AddSwitchAuxSend(rpr_tk, m_aux);
        else MyCSurf_SwitchPhase(rpr_tk);

      } 
      else if (q_shift)
      {
        if (m_aux > 0) MyCSurf_RemoveAuxSend(rpr_tk, m_aux);
        else CSurf_OnRecArmChange(rpr_tk, -1);

      } else if (m_chan)
      {
        MySetSurface_Chan_SelectTrack(ch_id, false);

      } else CSurf_OnSelectedChange(rpr_tk, -1);
    
    } else if (ch_id == 24) OnMasterSel();
  } // OnTrackSel


  void OnTrackSolo(char ch_id)
  {
    MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);

    if (q_fkey) MyCSurf_ToggleSolo(rpr_tk, true);
    else MyCSurf_ToggleSolo(rpr_tk, false);
  } // OnTrackSolo


  void OnTrackMute(char ch_id)
  {
    MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);

    if (q_fkey) MyCSurf_ToggleMute(rpr_tk, true);
    else if (q_shift)
    {
        if (m_aux > 0) MyCSurf_ToggleMuteAuxSend(rpr_tk, m_aux);
        else MyCSurf_Chan_ToggleAllFXBypass(ch_id);
    }
    else MyCSurf_ToggleMute(rpr_tk, false);
  } // OnTrackMute


  void OnFaderTouch(char ch_id, bool btn_state)
  {
    if (btn_state) 
    {
      s_touch_fdr = s_touch_fdr | (1 << ch_id);
      if ((q_shift) || (q_fkey)) MySetSurface_UpdateFader(ch_id);
    } else
    {
      s_touch_fdr = s_touch_fdr & (~(1 << ch_id));
    }
    
	  stpHandler->Stp_Update(ch_id, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, false);
  } // OnFaderTouch


  void OnFaderChange(char ch_id, int value)
  {
    MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);
    int para_amount;

    double d_value;

    bool istrack = false;
    bool ismaster = false;
    bool isactive = true;

    // is track or master?
    if ( (ch_id >= 0) && (ch_id <= 23) ) istrack = true; // no track fader
    else if (rpr_tk == CSurf_TrackFromID(0, false)) ismaster = true;

    // active fader? 
    if (!rpr_tk) isactive = false; // no corresponding rpr_tk
    
    if ( (m_chan) && (m_flip) ) 
    { // only chan and flip: inside para_amount?
   
      para_amount = TrackFX_GetNumParams(chan_rpr_tk, chan_fx);
      if (chan_par_offs + ch_id >= para_amount) isactive = false;
      else isactive = true; // is track doesn't matter when chan and flipped
    
    } else if ( (m_aux > 0) && (m_flip) ) 
    { // only aux #x and flip: has send #x?
      int send_id = Cnv_AuxIDToSendID(ch_id, m_aux, true);
      if (send_id == -1) isactive = false;
    }
  
    // get values
    if (ismaster || istrack)
    {
      if (ismaster)
      { // if master -> volume

        if (q_fkey) d_value = 0.0; // MINIMUM
        else if (q_shift) d_value = 1.0; // DEFAULT
        else d_value = Cnv_FaderToVol(value); 
        CSurf_OnVolumeChange(rpr_tk, d_value, false);
      
      } else if (isactive)
      { // if is active track fader

        if (m_flip)
        {
          if (m_chan) 
          { // flip & chan -> fx param

            double min, max;
            d_value = MyCSurf_TrackFX_GetParam(chan_rpr_tk, chan_fx, chan_par_offs + ch_id, &min, &max);

            if (q_fkey) d_value = min; // MINIMUM
            else if (q_shift) d_value = max; // MAXIMUM
            else d_value = Cnv_FaderToFXParam(min, max, value);
            MyCSurf_Chan_SetFXParam(chan_rpr_tk, chan_fx, chan_par_offs + ch_id, d_value);

          } else if (m_aux > 0) 
          { // flip + aux -> send Vol

            int send_id = Cnv_AuxIDToSendID(ch_id, m_aux, true);
            if (send_id != -1)
            {

              if (q_fkey) d_value = 0.0; // MINIMUM
              else if (q_shift) d_value = 1.0; // DEFAULT
              else d_value = Cnv_FaderToVol(value); 

              CSurf_OnSendVolumeChange(rpr_tk, send_id, d_value, false);
            }
          } else
          { // pan

            if (q_fkey) 
            { // flip & fkey & pan -> width
            
              if (q_shift) d_value = 1.0; // default
              else d_value = Cnv_FaderToPanWidth(value);
              CSurf_OnWidthChange(rpr_tk, d_value, false);

            } else 
            { // flip & pan mode -> pan
            
              if (q_shift) d_value = 0.0; // default
              else d_value = Cnv_FaderToPanWidth(value);
              CSurf_OnPanChange(rpr_tk, d_value, false);
            }         
          } // if (m_chan), else

        } else
        { // no flip -> volume

          // no flip / master -> track volume
          if (q_fkey) d_value = 0.0; // MINIMUM
          else if (q_shift) d_value = 1.0; // DEFAULT
          else d_value = Cnv_FaderToVol(value); 
          CSurf_OnVolumeChange(rpr_tk, d_value, false);

        } // if (m_flip), else
      } // if (ismaster), else if (isactive)
    } // if (exists)

    stpHandler->Stp_Update(ch_id, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, false);
    MySetSurface_UpdateFader(ch_id);
  } // OnFaderChange()


  void OnEncoderChange(char ch_id, signed char rel_value)
  {
    MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);
    int para_amount;

    double d_value;
    bool dot = false; // for phase switch, rec arm

    bool exists = false;
    bool isactive = true;

    // encoder exists?
    if ( (ch_id >= 0) && (ch_id <= 23) ) exists = true;

    // active encoder? 
    if (!rpr_tk) isactive = false; // no track

    if ( (m_chan) && (!m_flip) )
    { 
      para_amount = TrackFX_GetNumParams(chan_rpr_tk, chan_fx);
      if (chan_par_offs + ch_id >= para_amount) isactive = false;
      else isactive = true; // chan + fip: is track doesn't matter

    } else if (m_aux > 0) 
    { // only aux #x: has send #x?
      int send_id = Cnv_AuxIDToSendID(ch_id, m_aux, true);
      if (send_id == -1) isactive = false;
    }

    if ( (exists) && (isactive) )
    { // is encoder and there is a track, fx parameter (chan, no flip), or send (aux, noflip)
      if (m_flip)
      {
        if (m_aux > 0)
        { // aux & flip -> send pan
          int send_id = Cnv_AuxIDToSendID(ch_id, m_aux, true);
          if (send_id != -1)
          {
            if (q_shift) d_value = 0.0; // DEFAULT
            else
            {
              GetTrackSendUIVolPan(rpr_tk, send_id, NULL, &d_value);
              d_value = Cnv_EncoderToPanWidth(d_value, rel_value);
            }
            CSurf_OnSendPanChange(rpr_tk, send_id, d_value, false);
          }
        
        } else
        { // just flip -> track volume
          
          if (q_fkey) d_value = 0.0; // MINIMUM
          else if (q_shift) d_value = 1.0; // DEFAULT
          else 
          { 
            GetTrackUIVolPan(rpr_tk, &d_value, NULL);
            d_value = Cnv_EncoderToVol(d_value, rel_value); 
          }
          CSurf_OnVolumeChange(rpr_tk, d_value, false);
        }
      } else
      {
        if (m_chan)
        { // chan -> fx_param (para_offset checked above)

          double min, max, step, fine, coarse; 
          bool toggle;
          bool has_steps = false;
          d_value = MyCSurf_TrackFX_GetParam(chan_rpr_tk, chan_fx, chan_par_offs + ch_id, &min, &max);
          
          // most of the time this fails because not implemented by plugins!
          //if ( ( TrackFX_GetParameterStepSizes(chan_rpr_tk, chan_fx, chan_par_offs + ch_id, &step, &fine, &coarse, &toggle) ) )
          if(false)
          {
            if (toggle)
            {
              has_steps = true;
              
              if (rel_value > 0) d_value = 1.0;
              else d_value = 0.0;
            
            } else if (step != 0)
            {
              has_steps = true;
            
              if (q_fkey)
              {
                if (fine != 0) step = fine;
                else step = step / 10.0;

              } else if (q_shift)
              {
                if (coarse != 0) step = coarse;
                else step = step * 10.0;
              }
            }
          } 
          
          // this is the workaround
          if (!has_steps)
          {
            step = 1/(double)ENCRESFX;
            if (q_fkey) step = 1/(double)ENCRESFXFINE;
            else if (q_shift) step = 1/(double)ENCRESFXTOGGLE;
          }

          d_value = Cnv_EncoderToFXParam(d_value, min, max, step, rel_value);
          MyCSurf_Chan_SetFXParam(chan_rpr_tk, chan_fx, chan_par_offs + ch_id, d_value);
       
        } else if (m_aux > 0)
        {
          int send_id = Cnv_AuxIDToSendID(ch_id, m_aux, true);
          if (send_id != -1)
          {
            // aux & no flip -> send-level
            if (q_fkey) d_value = 0.0; // MINIMUM 
            else if (q_shift) d_value = 1.0; // DEFAULT 
            else
            {
              
              GetTrackSendUIVolPan(rpr_tk, send_id, &d_value, NULL);
              d_value = Cnv_EncoderToVol(d_value, rel_value);
            }
            CSurf_OnSendVolumeChange(rpr_tk, send_id, d_value, false);
          }
        
        } else if (m_pan)
        {
          if (q_fkey) 
          { // pan + fkey -> width
            
            
            if (q_shift) d_value = 1.0; // default
            else 
            {
              d_value = GetMediaTrackInfo_Value(rpr_tk, "D_WIDTH");
              d_value = Cnv_EncoderToPanWidth(d_value, rel_value);
            }
            CSurf_OnWidthChange(rpr_tk, d_value, false);
          
          } else
          { // pan mode -> pan

            if (q_shift) d_value = 0.0; // default
            else 
            {
              GetTrackUIVolPan(rpr_tk, NULL, &d_value);
              d_value = Cnv_EncoderToPanWidth(d_value, rel_value);
            }
            CSurf_OnPanChange(rpr_tk, d_value, false);
          }
        }
      } // if (m_flip), else

      MySetSurface_UpdateEncoder(ch_id); // because touched track doesn't get updated
      stpHandler->Stp_Update(ch_id, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, 1);

      s_touch_enc[ch_id] = ENCTCHDLY;

    } // if ( (exists) && (isactive) )
  } // OnEncoderChange


  // MASTER TRACK

  void OnMasterSel()
  {
    if (q_fkey)
    {
      MyCSurf_SelectMaster();

    } else
    {
      if (m_chan) MySetSurface_Chan_SelectTrack(24, false);
      else MyCSurf_ToggleSelectAllTracks();
    }
  } // OnMasterSel()


  void OnClrSolo(bool btn_state)
  {
    if (btn_state) {
      if (q_fkey) MyCSurf_UnmuteAllTracks();
      else if (q_shift) MyCSurf_ToggleMute(csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset), false);
      else MyCSurf_UnsoloAllTracks();
    }

    MySetSurface_UpdateButton(0x62, btn_state, false);
  } // OnClrSolo()


  void OnFlip()
  {
    MySetSurface_ToggleFlip();
  } // OnFlip()


  // MODE BUTTONS

  void OnChan()
  {
    if (m_chan) MySetSurface_ExitChanMode();
    else MySetSurface_EnterChanMode();
  } // OnChan()


  void OnPan()
  {
    MySetSurface_EnterPanMode();
  } // OnPan()


  void OnAux(char sel)
  { 
    char mode = 0;
    if (m_chan) mode = 1;
    else if (m_aux > 0) mode = 2;
    
    char qkey = 0;
    if (q_shift) qkey = 1;
    else if (q_fkey) qkey = 2;
    else if (q_mkey) qkey = 3;

    if (qkey == 0)
    {
      if (m_chan)
      {
        switch(sel)
        {
          case 1 : MySetSurface_Chan_SetFxParamOffset(1); break;
          case 2 : MySetSurface_Chan_SetFxParamOffset(-1); break;
          case 3 : MyCSurf_Chan_ToggleFXBypass(); break;
          case 4 : MyCSurf_Chan_InsertFX(); break;
          case 5 : MyCSurf_Chan_DeleteFX(); break;
          case 6 : MyCSurf_Chan_ToggleArmFXEnv(); break;
        } 

      } else MySetSurface_EnterAuxMode(sel);  
    
    } else Main_OnCommand(cmd_ids[qkey][mode][sel-1], 0);

    MySetSurface_UpdateAuxButtons();
  } // OnAux()


  void OnMeter()
  {
    // reset holds
    if ((m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE)) MySetSurface_OutputMeters(true);
    // reset button
    else {
      q_mkey = !q_mkey;
      if (q_mkey) 
      {
        s_mkey_on = MDELAY;
		    hlpHandler->SetQkey(3);
      } else hlpHandler->SetQkey(0);

      MySetSurface_UpdateButton(0x6b, q_mkey, false);
    }
  } // OnMeter


  // QUALIFIERS

  void OnFKey(bool btn_state)
  {
    if ((btn_state && q_shift) && (s_initdone))
    {
		  stpHandler->ToggleWindow(chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_chan, m_flip);
    } 
    MySetSurface_ToggleFKey(btn_state);
  } // OnFKey()


  void OnShift(bool btn_state)
  {
    if ((btn_state && q_fkey) && (s_initdone)) hlpHandler->Hlp_ToggleWindow(m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE);
    MySetSurface_ToggleShift(btn_state);
  } // OnShift()


  // TRANSPORT

  void OnRew()
  {
    char mode = 0;
    if (m_chan) mode = 1;
    else if (m_aux > 0) mode = 2;
    
    char qkey = 0;
    if (q_shift) qkey = 1;
    else if (q_fkey) qkey = 2;
    else if (q_mkey) qkey = 3;

    if ((qkey == 0) && (cmd_ids[0][mode][7] == -1)) CSurf_OnRew(1);
    else if ((qkey == 1) && (cmd_ids[1][mode][10] == -1)) MyCSurf_Auto_SetMode(0);
    else Main_OnCommand(cmd_ids[qkey][mode][7], 0);
  } // OnRew()


  void OnFwd()
  {
    char mode = 0;
    if (m_chan) mode = 1;
    else if (m_aux > 0) mode = 2;
    
    char qkey = 0;
    if (q_shift) qkey = 1;
    else if (q_fkey) qkey = 2;
    else if (q_mkey) qkey = 3;

    if ((qkey == 0) && (cmd_ids[0][mode][8] == -1)) CSurf_OnFwd(1);
    else if ((qkey == 1) && (cmd_ids[1][mode][10] == -1)) MyCSurf_Auto_SetMode(1);
    else Main_OnCommand(cmd_ids[qkey][mode][8], 0);
  } // OnFwd()


  void OnStop()
  {
    char mode = 0;
    if (m_chan) mode = 1;
    else if (m_aux > 0) mode = 2;
    
    char qkey = 0;
    if (q_shift) qkey = 1;
    else if (q_fkey) qkey = 2;
    else if (q_mkey) qkey = 3;

    if ((qkey == 0) && (cmd_ids[0][mode][9] == -1)) CSurf_OnStop();
    else if ((qkey == 1) && (cmd_ids[1][mode][10] == -1)) MyCSurf_Auto_SetMode(2);
    else Main_OnCommand(cmd_ids[qkey][mode][9], 0);
  } // OnStop()


  void OnPlay()
  {
    char mode = 0;
    if (m_chan) mode = 1;
    else if (m_aux > 0) mode = 2;
    
    char qkey = 0;
    if (q_shift) qkey = 1;
    else if (q_fkey) qkey = 2;
    else if (q_mkey) qkey = 3;

    if ((qkey == 0) && (cmd_ids[0][mode][10] == -1)) CSurf_OnPlay(); 
    else if ((qkey == 1) && (cmd_ids[1][mode][10] == -1)) MyCSurf_Auto_SetMode(3);
    else Main_OnCommand(cmd_ids[qkey][mode][10], 0);
  } // OnPlay()


  void OnRec()
  {
    char mode = 0;
    if (m_chan) mode = 1;
    else if (m_aux > 0) mode = 2;
    
    char qkey = 0;
    if (q_shift) qkey = 1;
    else if (q_fkey) qkey = 2;
    else if (q_mkey) qkey = 3;

    if ((qkey == 0) && (cmd_ids[0][mode][11] == -1)) MyCSurf_OnRec();
    else Main_OnCommand(cmd_ids[qkey][mode][11], 0);
  } // OnRec()


  // OTHER KEYS

  void OnNull(bool btn_state)
  {
    if (btn_state) 
    {
      char mode = 0;
      if (m_chan) mode = 1;
      else if (m_aux > 0) mode = 2;
      
      char qkey = 0;
      if (q_shift) qkey = 1;
      else if (q_fkey) qkey = 2;
      else if (q_mkey) qkey = 3;

      Main_OnCommand(cmd_ids[qkey][mode][6], 0);
    }

    MySetSurface_UpdateButton(0x6e, btn_state, false);
  } // OnNull()


  void OnScrub()
  {
    MySetSurface_ToggleScrub();
  } // OnScrub()


  void OnBank(signed char dir, bool btn_state)
  {
    char btn_id;
    if (dir > 0) btn_id = 0x71;
    else btn_id = 0x70;
    
    if (btn_state)
    {
      if (m_chan)
      {
        if (q_fkey) MyCSurf_Chan_MoveFX(dir);
        else if (q_shift) MySetSurface_ShiftBanks(dir, DEFAULT_BANK_SWITCH);
        else MyCSurf_Chan_SelectFX(chan_fx + dir);

      } else
      {
        if (q_fkey) MyCSurf_MoveTimeSel(dir, 0, false);
        else
        {
		  char factor = (q_shift) ? SHIFT_BANK_SWITCH : DEFAULT_BANK_SWITCH;
          MySetSurface_ShiftBanks(dir, factor);
        }
      }
      MySetSurface_UpdateButton(btn_id, true, false);
    } else
    {
      if (m_chan) MySetSurface_UpdateButton(btn_id, true, true);
      else MySetSurface_UpdateButton(btn_id, false, false);
    }
  } // OnBank


  void OnIn(bool btn_state)
  {
    if (btn_state)
    {
      if (q_fkey) MyCSurf_MoveTimeSel(0, -1, false);
      else if (q_shift) MyCSurf_Loop_ToggleAll();
      else MyCSurf_MoveTimeSel(-1, -1, true);
    }

    // keep lit if loop activated
    if (s_loop_all) btn_state = true;
    MySetSurface_UpdateButton(0x72, btn_state, false);
  } // OnIn()


  void OnOut(bool btn_state)
  {
    if (btn_state)
    {
      if (q_fkey) MyCSurf_MoveTimeSel(0, 1, false);
      else if (q_shift) MyCSurf_ToggleRepeat();
      else MyCSurf_MoveTimeSel(1, 1, true);
    }

    // keep lit if loop activated
    if (s_loop) btn_state = true;
    MySetSurface_UpdateButton(0x73, btn_state, false);
  } // OnOut()



  // SPECIAL INPUT  

  void OnJogWheel(signed char rel_value)
  {
    if (m_scrub)
    {
      if (q_fkey) MyCSurf_Scrub(rel_value, true);
      else MyCSurf_Scrub(rel_value, false);
    
    } else
    {
      if (q_fkey) MyCSurf_MoveEditCursor(rel_value, true);
      else MyCSurf_MoveEditCursor(rel_value, false);
    }
  } // OnJogWheel()



    ////// CONVERSION & HELPERS //////


  // TRACKS / CHANNELS / SENDS

  int Cnv_MediaTrackToChannelID(MediaTrack* rpr_tk)
  {
    int ch_id = CSurf_TrackToID(rpr_tk, true);

    ch_id -= s_ch_offset;
    if (ch_id == 0) ch_id = 24;
    else ch_id = ch_id - 1;

    return ch_id;
  } // Cnv_MediaTrackToChannelID


  int Cnv_AuxIDToSendID(int ch_id, char aux, bool consider_hw_sends)
  {
    //returns number of aux sends, ignores hw sends
    char search[256];
    char sendname[256];
    sprintf(search, AUXSTRING, aux);

    MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);
    int aux_sends = GetTrackNumSends(rpr_tk, 0);
    // hardware outputs count for GetTrackSendName, too
    int hw_sends = GetTrackNumSends(rpr_tk, 1);

    int all_sends = aux_sends + hw_sends;

    for (int s = 0; s < all_sends; s++)
    {
      GetTrackSendName(rpr_tk, s, sendname, 256);
          
      // lowercase name - surely there has to be a better way?
      char c = 0;
      while ((c < 256) && (sendname[c] != 0))
      {
        if ((sendname[c] >= 65) && (sendname[c] <= 90)) sendname[c] += 32;
        c++;
      }


      if (strstr(sendname, search))
      {
          if (consider_hw_sends)
              return s;
          else
              return s - hw_sends;
      }
    }
    
    return -1;
  } // Cnv_AuxIDToSendID


  // VOLUME / SEND LEVELS

  double Cnv_FaderToVol(unsigned int value) 
  {
    // theoretically, the MIDI range is from 0 to 16383,
    // but the US2400 gets a little rough on the ends, 
    // an attempt to account for that:
    double new_val = ((double)(value + 41) * 1000.0) / 16256.0;

    if (new_val < 0.0) new_val = 0.0;
    else if (new_val > 1000.0) new_val = 1000.0;

    new_val = SLIDER2DB(new_val);
    new_val = DB2VAL(new_val);

    return(new_val);
  } // Cnv_FaderToVol


  double Cnv_EncoderToVol(double old_val, signed char rel_val) 
  {
    double lin = (DB2SLIDER( VAL2DB(old_val) ) * 16383.0 / 1000.0);
    double d_rel = (double)rel_val * (double)ENCRESVOL;
    lin = lin + d_rel;

    double new_val = ((double) lin * 1000.0) / 16383.0;

    if (new_val < 0.0) new_val = 0.0;
    else if (new_val > 1000.0) new_val = 1000.0;

    new_val = DB2VAL(SLIDER2DB(new_val));

    return new_val;
  } // Cnv_EncoderToVol


  int Cnv_VolToFader(double value) 
  {
    double new_val = VAL2DB(value);
    new_val = DB2SLIDER( new_val );

    // theoretically, the MIDI range is from 0 to 16383,
    // but the US2400 gets a little rough on the ends, 
    // an attempt to account for that:
    new_val = new_val * 16256.0 / 1000.0 + 41;

    int out;

    if (new_val < 50.0) new_val = 0.0;
    else if (new_val > 16250.0) new_val = 16383.0;

    out = (int)(new_val + 0.5);

    return out;
  } // Cnv_VolToFader


  unsigned char Cnv_VolToEncoder(double value) 
  {
    double new_val = (DB2SLIDER( VAL2DB(value) ) * 14.0 / 1000.0) + 1;

    if (new_val < 1.0) new_val = 1.0;
    else if (new_val > 15.0) new_val = 15.0;

    return (char)(new_val + 0.5);
  } // Cnv_VolToEncoder


  unsigned char Cnv_PeakToEncoder(double value) 
  {
    double new_val = VAL2DB(value);

    return Cnv_DBToEncoder(new_val);
  } // Cnv_PeakToEncoder


  unsigned char Cnv_DBToEncoder(double value)
  {
    if (value > MINUSINF) value = ((MINUSINF - value) / MINUSINF * 15.0) + 0.5;
    else value = 0.0;

    if (value > 15.0) value = 15.0;

    return char(value);
  } //Cnv_DBToEncoder


  // PAN / WIDTH

  double Cnv_FaderToPanWidth(unsigned int value) 
  {
    double new_val = -1.0 + ((double)value / 16383.0 * 2.0);

    if (new_val < -1.0) new_val = -1.0;
    else if (new_val > 1.0) new_val = 1.0;

    return new_val;
  } // Cnv_FaderToPanWidth


  double Cnv_EncoderToPanWidth(double old_val, signed char rel_val) 
  { 
    double new_val = old_val + (double)rel_val * (double)ENCRESPAN;

    if (new_val < -1.0) new_val = -1.0;
    else if (new_val > 1.0) new_val = 1.0;
    return new_val;
  } // Cnv_EncoderToPanWidth


  int Cnv_PanWidthToFader(double value) 
  {
    double new_val = (1.0 + value) * 16383.0 * 0.5;

    if (new_val < 0.0) new_val = 0.0;
    else if ( new_val > 16383.0) new_val = 16383.0;

    return (int)(new_val + 0.5);
  } // Cnv_PanWidthToFader


  char Cnv_PanToEncoder(double value) 
  {
    double new_val = ((1.0 + value) * 14.0 * 0.5) + 1;

    if (new_val < 1.0) new_val = 1.0;
    else if ( new_val > 15.0) new_val = 15.0;

    return (char)(new_val + 0.5);
  } // Cnv_PanToEncoder


  char Cnv_WidthToEncoder(double value) 
  {
    double new_val = fabs(value) * 7.0 + 1;
    if (new_val < 1.0) new_val = 1.0;
    else if ( new_val > 15.0) new_val = 15.0;

    return (char)(new_val + 0.5);
  } // Cnv_WidthToEncoder


  // FX PARAM

  double Cnv_FaderToFXParam(double min, double max, unsigned int value)
  {
    double new_val = min + ((double)value / 16256.0 * (max - min));

    if (new_val < min) new_val = min;
    else if (new_val > max) new_val = max;

    return new_val;
  } // Cnv_FaderToFXParam


  double Cnv_EncoderToFXParam(double old_val, double min, double max, double step, signed char rel_val)  
  {
    /* should TrackFX_GetParamStepSizes decide to work, we can use this: 
    double d_rel = (double)rel_val * step;
    */

    // for now:
    double d_rel = (double)rel_val * (max - min) * step;

    double new_val = old_val + d_rel;

    if (new_val < min) new_val = min;
    else if (new_val > max) new_val = max;

    return new_val;
  } // Cnv_EncoderToFXParam


  int Cnv_FXParamToFader(double min, double max, double value)
  {
    double new_val = (value - min) / (max - min) * 16383.0;

    if (new_val < 0.0) new_val = 0.0;
    else if (new_val > 16383.0) new_val = 16383.0;

    return (int)(new_val + 0.5);
  } // Cnv_FXParamToFader


  char Cnv_FXParamToEncoder(double min, double max, double value)
  { 
    double new_val = (value - min) / (max - min) * 14.0 + 1;

    if (new_val < 1.0) new_val = 1.0;
    else if (new_val > 15.0) new_val = 15.0;

    return (char)(new_val + 0.5);
  } // Cnv_FXParamToEncoder


  // HELPERS

  void Utl_SaveSelection()
  {
    if (saved_sel == 0)
    {
    
      saved_sel_len = CountSelectedTracks(0);
      saved_sel = new MediaTrack*[saved_sel_len];

      ReaProject* rpr_pro = EnumProjects(-1, NULL, 0);

      for(int sel_tk = 0; sel_tk < saved_sel_len; sel_tk++)
        saved_sel[sel_tk] = GetSelectedTrack(rpr_pro, sel_tk);

      // unsel all tks
      int all_tks = CountTracks(0);
      MediaTrack* tk;
  
      for (int i = 0; i < all_tks; i++)
      {
        tk = GetTrack(0, i);
        SetTrackSelected(tk, false);
      }
    }
  } // Utl_SaveSelection


  void Utl_RestoreSelection()
  {
    if (saved_sel != 0)
    {
      // unsel all tks
      int all_tks = CountTracks(0);
      MediaTrack* tk;
  
      for (int i = 0; i < all_tks; i++)
      {
        tk = GetTrack(0, i);
        SetTrackSelected(tk, false);
      }

      for(int sel_tk = 0; sel_tk < saved_sel_len; sel_tk++)
        SetTrackSelected(saved_sel[sel_tk], true);

      delete saved_sel;
      saved_sel = 0;
    }
  } // Utl_RestoreSelection


  void Utl_GetCustomCmdIds()
  {
    const char* name;

    // go through all custom commands and parse names to get cmd ids
    for (int cmd = 50000; cmd <= 65535; cmd++)
    {
      name = kbd_getTextFromCmd(cmd, NULL);
      const char* end = strstr(name, "US-2400 -");
      if (end != NULL)
      {
        size_t len = end - name - 9;

        int qkey = -1;
        int mode = -1;
        int key = -1;

        // find qualifier key
        if (strstr(name, "- NoKey -")) qkey = 0;
        else if (strstr(name, "- Shift -")) qkey = 1;
        else if (strstr(name, "- FKey -")) qkey = 2;
        else if (strstr(name, "- MKey -")) qkey = 3;

        // find mode
        if (strstr(name, "- Pan -")) mode = 0;
        else if (strstr(name, "- Chan -")) mode = 1;
        else if (strstr(name, "- Aux -")) mode = 2;

        // find key
        if (strstr(name, "- Null")) key = 6;
        else if (strstr(name, "- Rew")) key = 7;
        else if (strstr(name, "- FFwd")) key = 8;
        else if (strstr(name, "- Stop")) key = 9;
        else if (strstr(name, "- Play")) key = 10;
        else if (strstr(name, "- Rec")) key = 11;
        else
        {
          char index_string[4];
          for (int s = 0; s < 6; s++)
          {
             sprintf(index_string, "- %d", s+1);
             if (strstr(name, index_string)) key = s;
          }
        }

        // save cmd id
        if (key != -1)
        {
          // if no mode or qkey specified enter found action for all undefined modes / qkeys
          for (int q = 0; q <= 3; q++)
          {
            for (int m = 0; m <= 2; m++)
            {
              if ( 
                ( (q == qkey) || ((qkey == -1) && (cmd_ids[q][m][key] == -1)) ) && 
                ( (m == mode) || ((mode == -1) && (cmd_ids[q][m][key] == -1)) ) 
              )
              {
                cmd_ids[q][m][key] = cmd;
				
                WDL_String nameStr = WDL_String(name);
                nameStr.DeleteSub(0, 8);
                nameStr.SetLen(csurf_utils::SizeTToInt(len));
                hlpHandler->SetHelpKeyString(key, m, q, nameStr);
              }
            }
          }
        }
      }
    }
  } // Utl_GetCustomCmdIds


  void Utl_CheckFXInsert()
  {
    int real_fx_count = TrackFX_GetCount(chan_rpr_tk);
    if (chan_fx_count != real_fx_count)
    {
     
      chan_fx_count = real_fx_count;
      chan_fx = chan_fx_count - 1;

      // update display and encoders / faders
      for (int ch = 0; ch < 24; ch++)
      {
		    stpHandler->Stp_Update(ch, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, false);
        if (!m_flip) MySetSurface_UpdateEncoder(ch);
        else MySetSurface_UpdateFader(ch);
      }

      MyCSurf_Chan_OpenFX(chan_fx);
    }
  } // Utl_CheckFXInsert


  MediaTrack* Utl_FindAux(int aux_id)
  {
    MediaTrack* tk;
    
    for (int tk_id = 0; tk_id < CountTracks(0); tk_id++)
    {
      tk = GetTrack(0, tk_id);
      const char* name = GetTrackState(tk, 0);
      
      // lowercase aux
      int c = 0;
      int chr_found = 0;
      while ((name[c] != '\0') && (c < (int)strlen(name)))
      {
        if (chr_found == 0 && ((name[c] == 'A') || (name[c] == 'a'))) chr_found = 1;
        else if (chr_found == 1 && ((name[c] == 'U') || (name[c] == 'u'))) chr_found = 2;
        else if (chr_found == 2 && ((name[c] == 'X') || (name[c] == 'x'))) chr_found = 3;
        else if ((chr_found >= 3) && (chr_found <= 5) && (name[c] == '-')) chr_found++;
        else if ((chr_found == 6) && (name[c] == (aux_id + 48))) chr_found = 7;

        if (chr_found == 7) return tk;

        c++;
      }
    }
    
    return NULL;
  }

  WDL_String Utl_Chunk_InsertLine(WDL_String chunk, WDL_String line, WDL_String before)
  {
    char* pstr = chunk.Get();
    char* ppos = strstr(pstr, before.Get());
    
    // found 'before'? insert
    if (ppos != NULL)
    {
      char* lineStr = line.Get();
      size_t lineLen = strlen(lineStr);
      size_t insertPos = ppos - pstr;
      chunk.Insert( line.Get(), csurf_utils::SizeTToInt(insertPos), csurf_utils::SizeTToInt(lineLen) );
    }

    return chunk;
  }

  WDL_String Utl_Chunk_RemoveLine(WDL_String chunk, WDL_String search)
  {
    char* pstr = chunk.Get();
    char* ppos = strstr(pstr, search.Get());
    
    if (ppos != NULL)
    {
      // search beggining of line
      char* pstart = ppos;
      while ( (*pstart != '\n') && (pstart > pstr) )
        pstart--;
      
      // search end of line
      char* pend = ppos;
      while ( (*pend != '\n') && (pend < pstr + strlen(pstr)) )
        pend++;

      chunk.DeleteSub(csurf_utils::SizeTToInt(pstart - pstr), csurf_utils::SizeTToInt(pend - pstart));
    }

    return chunk;
  }

  WDL_String Utl_Chunk_Replace(WDL_String chunk, WDL_String search, WDL_String replace)
  {
    char* pstr = chunk.Get();
    char* ppos = strstr(pstr, search.Get());

    if (ppos != NULL)
    {
      int insertPos = csurf_utils::SizeTToInt(ppos - pstr);
      int searchLen = csurf_utils::SizeTToInt(strlen(search.Get()));
      int replaceLen = csurf_utils::SizeTToInt(strlen(replace.Get()));
      chunk.DeleteSub(insertPos, searchLen);
      chunk.Insert( replace.Get(), insertPos, replaceLen);
    }

    return chunk;
  }


public:


  ////// CONSTRUCTOR / DESTRUCTOR //////

  CSurf_US2400(int indev, int outdev, int outdev_disp, int cflags, int *errStats)
  {
    ////// GLOBAL VARS //////

    m_midi_in_dev = indev;
    m_midi_out_dev = outdev;
    m_midi_out_dev_disp = outdev_disp;

    m_cfg_flags = cflags;

    m_offset = 0;
    m_size = 0;


    // reset cmd_ids
    for (char qkey = 0; qkey < 4; qkey++)
      for (char mode = 0; mode < 3; mode++)
        for (char key = 0; key < 12; key++)
          cmd_ids[qkey][mode][key] = -1;

    // for fader data
    waitformsb = false;

    // for myblink
    s_myblink = false;
    myblink_ctr = 0;

    // for init
    s_initdone = false;
    s_exitdone = false;
  
    // touchstates & cache
    s_touch_fdr = 0;
  
    for (char i = 0; i < 25; i++) 
    {
      cache_faders[i] = 0;
      if (i < 24) 
      {
        s_touch_enc[i] = 0;
        cache_enc[i] = 0;
      }
    }
  
    cache_upd_faders = 0;
    cache_upd_enc = 0;
    cache_exec = 0;
    master_sel = false;


    // general states
    s_ch_offset = 0; // bank up/down
    s_play = false; // playstates
    s_rec = false;
    s_loop = false;
    s_automode = 1; // automationmodes

    // modes
    m_flip = false;
    m_chan = false;
    m_pan = true;
    m_aux = 0;
    m_scrub = false;

    // qualifier keys
    q_fkey = false;
    q_shift = false;
    q_mkey = false;

    // for channel strip
    chan_ch = 0;
    chan_fx = 0;
    chan_par_offs = 0;
    chan_fx_env_arm = false;

    // save selection
    saved_sel = 0;
    saved_sel_len = 0;

    // loop all
    s_loop_all = false;

    // create midi hardware access
    m_midiin = m_midi_in_dev >= 0 ? CreateMIDIInput(m_midi_in_dev) : NULL;
    m_midiout = m_midi_out_dev >= 0 ? CreateThreadedMIDIOutput( CreateMIDIOutput(m_midi_out_dev, false, NULL) ) : NULL;
    m_midiout_disp = m_midi_out_dev_disp >= 0 ? CreateThreadedMIDIOutput(CreateMIDIOutput(m_midi_out_dev_disp, false, NULL)) : NULL;


    if (errStats)
    {
      if (m_midi_in_dev >=0  && !m_midiin) *errStats|=1;
      if (m_midi_out_dev >=0  && !m_midiout) *errStats|=2;
      if (m_midi_out_dev_disp >= 0 && !m_midiout_disp) *errStats |= 3;

    }

    if (m_midiin) m_midiin->start();

    hlpHandler = new CSurf_US2400_helpoverlay();
    stpHandler = new CSurf_US2400_stripoverlay(m_midiout_disp);
  } // CSurf_US2400()


  ~CSurf_US2400()
  {
    s_exitdone = MySetSurface_Exit();
    do
    { Sleep(500);  
    } while (!s_exitdone);
    
    if (saved_sel)
        delete saved_sel;

    if (m_midiout_disp)
        delete m_midiout_disp;

    if (m_midiout)
        delete m_midiout;

    if (m_midiin)
        delete m_midiin;
  } // ~CSurf_US2400()


    // FX Param Wrapper Functions (for reordering params)


  double MyCSurf_TrackFX_GetParam(MediaTrack* tr, int fx, int param, double* minval, double* maxval)
  {
      int remappedParam = csurf_utils::TrackFX_RemapParam(param);
      if (remappedParam >= 0) 
        return TrackFX_GetParam(tr, fx, abs(remappedParam), minval, maxval);
      else //negative sign means invert rotation of parameter
        return 1-TrackFX_GetParam(tr, fx, abs(remappedParam), minval, maxval);
  }//MyCSurf_TrackFX_GetParam

  bool MyCSurf_TrackFX_SetParam(MediaTrack* tr, int fx, int param, double val)
  {
      int remappedParam = csurf_utils::TrackFX_RemapParam(param);

      if (remappedParam >= 0)
        return TrackFX_SetParam(tr, fx, abs(remappedParam), val);
      else  //negative sign means invert rotation of parameter
        return TrackFX_SetParam(tr, fx, abs(remappedParam), (double)(1.0f-val));
  }//MyCSurf_TrackFX_GetParam


  ////// CUSTOM SURFACE UPDATES //////

  bool MySetSurface_Init() 
  {
    stpHandler->Stp_RetrieveCoords();

	  hlpHandler->Hlp_FillStrs(); // insert hardcoded strings into hlp_xxx_str

    Utl_GetCustomCmdIds(); // inserts custom cmd strs into hlp_keys_str
    CSurf_ResetAllCachedVolPanStates(); 
    TrackList_UpdateAllExternalSurfaces(); 

    // Initially Pan Mode
    MySetSurface_EnterPanMode();

    // Initially Scrub off
    m_scrub = true;
    MySetSurface_ToggleScrub();
    
    // update BankLEDs
    MySetSurface_UpdateBankLEDs();

    // update loop
    s_loop = !(bool)GetSetRepeat(-1);
    MyCSurf_ToggleRepeat();
    SetRepeatState(s_loop);

    // Set global auto mode to off / trim, CSurf cmds only change track modes
    SetAutomationMode(0, false);
    MySetSurface_UpdateAutoLEDs();

    return true;
  } // MySetSurface_Init


  bool MySetSurface_Exit()
  {
    stpHandler->Stp_SaveCoords();
	
    CSurf_ResetAllCachedVolPanStates();
	
    if (m_chan) MySetSurface_ExitChanMode();
	

    for (char ch_id = 0; ch_id <= 24; ch_id++)
    {
      // reset faders
      MIDIOut(0xb0, ch_id + 0x1f, 0);
      MIDIOut(0xb0, ch_id, 0);

      // reset encoders
      if (ch_id < 24) MIDIOut(0xb0, ch_id + 0x40, 0);
    }

    // reset mute/solo/select
    for (char btn_id = 0; btn_id <= 0x79; btn_id ++)
      MIDIOut(0xb1, btn_id, 0);

    // reset bank leds
    MIDIOut(0xb0, 0x5d, 0);

    stpHandler->Stp_CloseWindow();

    return true;
  } // MySetSurface_Exit


  void MySetSurface_UpdateButton(unsigned char btn_id, bool btn_state, bool blink)
  {
    //button cache
    if ((button_states[btn_id][0] == btn_state) && (button_states[btn_id][1] == blink)) {
      return;
    }
    else {
      button_states[btn_id][0] = btn_state; 
      button_states[btn_id][1] = blink;
      unsigned char btn_cmd = 0x7f; // on
      if (blink) btn_cmd = 0x01; // blink
      if (!btn_state) btn_cmd = 0x00; // off
      MIDIOut(0xb1, btn_id, btn_cmd);
    }

  } // MySetSurface_UpdateButton


  void MySetSurface_UpdateBankLEDs()
  {
    char led_id = s_ch_offset / 24;
    MIDIOut(0xb0, 0x5d, led_id);
  } // MySetSurface_UpdateBankLEDs


  void MySetSurface_UpdateAutoLEDs()
  {
    // update transport buttons
    for (char btn_id = 0; btn_id <= 3; btn_id ++)
    {
      bool on = false;

      if ( (btn_id == 3) && (s_play) ) on = true;
      if ( (s_automode & (1 << btn_id)) && (s_myblink) ) on = !on;

      MySetSurface_UpdateButton(0x75 + btn_id, on, false);
    }
  } // MySetSurface_UpdateAutoLEDs


  void MySetSurface_UpdateAuxButtons()
  {
    for (char aux_id = 1; aux_id <= 6; aux_id++)
    {
      bool on = false;
      
      if ( (q_fkey) || (q_shift) )
      { // qualifier keys

        on = false;

      } else if (m_aux == aux_id) 
      { // aux modes

        on = true;

      } else if (m_chan) 
      { // chan mode
      
        // bypass
        int amount_fx = TrackFX_GetCount(chan_rpr_tk);
        bool bypass_fx;

        // if fx doesn't exist yet (e.g. insert on first open)
        if (chan_fx >= amount_fx) bypass_fx = false;
        else bypass_fx = !(bool)TrackFX_GetEnabled(chan_rpr_tk, chan_fx);
        bool bypass_allfx = !(bool)GetMediaTrackInfo_Value(chan_rpr_tk, "I_FXEN");

        if ( (aux_id == 3) && (bypass_fx) && (!q_fkey) && (!q_shift) ) on = true;

        // write fx or tk auto?
        if ( (aux_id == 6) && (chan_fx_env_arm) && (!q_fkey) && (!q_shift) ) on = true;
      }

      if (m_chan) MySetSurface_UpdateButton(0x64 + aux_id, on, true);
      else MySetSurface_UpdateButton(0x64 + aux_id, on, false);
    }
  } // MySetSurface_UpdateAuxButtons


  void MySetSurface_UpdateFader(unsigned char ch_id)
  {

    MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);
    int para_amount;

    double d_value;
    int value;
    
    bool istrack = false;
    bool ismaster = false;
    bool isactive = true;

    // is track or master?
    if ( (ch_id >= 0) && (ch_id <= 23) ) istrack = true; // no track fader
    else if (rpr_tk == CSurf_TrackFromID(0, false)) ismaster = true;

    // active fader? 
    if (!rpr_tk) isactive = false; // no corresponding rpr_tk
    
    if ( (m_chan) && (m_flip) ) 
    { // only chan and flip: inside para_amount?
   
      para_amount = TrackFX_GetNumParams(chan_rpr_tk, chan_fx);
      if (chan_par_offs + ch_id >= para_amount) isactive = false;
      else isactive = true; // is track doesn't matter when chan and flipped
    
    } else if ( (m_aux > 0) && (m_flip) ) 
    { // only aux #x and flip: has send #x?
      
      int send_id = Cnv_AuxIDToSendID(ch_id, m_aux, true);
      if (send_id == -1) isactive = false;
    }

    if (istrack || ismaster)
    {
      // get values
      if (ismaster)
      { // if master -> volume

        GetTrackUIVolPan(rpr_tk, &d_value, NULL);
        value = Cnv_VolToFader(d_value);
      
      } else if (!isactive)
      { // if there is no track, parameter (chan & flip), send (aux & flip), reset

        value = 0; 
   
      } else
      { // if is active track fader

        if (m_flip)
        {
          if (m_chan) 
          { // flip & chan -> fx param

            double min, max;
            d_value = MyCSurf_TrackFX_GetParam(chan_rpr_tk, chan_fx, chan_par_offs + ch_id, &min, &max);
            value = Cnv_FXParamToFader(min, max, d_value);

          } else if (m_aux > 0)
          { // flip + aux -> send Vol
            int send_id = Cnv_AuxIDToSendID(ch_id, m_aux, true);
            if (send_id != -1)
            {
              d_value = GetTrackSendUIVolPan(rpr_tk, send_id, &d_value, NULL);
              value = Cnv_VolToFader(d_value);
            }
          
          } else
          { // pan
            
            if (q_fkey)
            { // flip + fkey + pan -> width

              d_value = GetMediaTrackInfo_Value(rpr_tk, "D_WIDTH");
              value = Cnv_PanWidthToFader(d_value);
            
            } else {
              // flip + pan -> pan

              GetTrackUIVolPan(rpr_tk, NULL, &d_value);
              value = Cnv_PanWidthToFader(d_value);
            }
          } // if (m_chan)
        } else
        { // no flip -> volume

          GetTrackUIVolPan(rpr_tk, &d_value, NULL);
          value = Cnv_VolToFader(d_value);

        } // if (m_flip), else
      } // if (!rpr_tk || rpr_tk == CSurf_TrackFromID(0, false)), else
      
      if (value != cache_faders[ch_id])
      {
        // new value to cache (gets executed on next run cycle)
        cache_faders[ch_id] = value;

        // set upd flag 
        cache_upd_faders = cache_upd_faders | (1 << ch_id);

      }  

    } // if (active or master)
  } // MySetSurface_UpdateFader


  void MySetSurface_ExecuteFaderUpdate(int ch_id)
  {
    if ((s_touch_fdr & (1 << ch_id)) == 0)
    {
      // send midi
      MIDIOut(0xb0, ch_id + 0x1f, (cache_faders[ch_id] & 0x7f));
      MIDIOut(0xb0, ch_id, ((cache_faders[ch_id] >> 7) & 0x7f));
      
      // remove update flag
      cache_upd_faders = cache_upd_faders & (~(1 << ch_id));  
    }
  } // MySetSurface_ExecuteFaderUpdate


  void MySetSurface_UpdateEncoder(int ch_id)
  {
    // encoder exists?
    if ( (ch_id >= 0) && (ch_id <= 23) )
    {

      MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);
      int para_amount;

      double d_value;
      unsigned char value = 0;
      bool dot = false; // for phase switch, rec arm

      bool istrack = true;
      bool exists = false;
      bool isactive = true;
      bool ispre = false;
      bool ismute = false;

      // active encoder? 
      if (!rpr_tk) 
      { // no track
        
        isactive = false; 
        istrack = false;
      }

      if ( (m_chan) && (!m_flip) )
      { 
        para_amount = TrackFX_GetNumParams(chan_rpr_tk, chan_fx);
        if (chan_par_offs + ch_id >= para_amount) isactive = false;
        else isactive = true; // chan + !flip: is track doesn't matter

      } else if ((m_aux > 0) && (rpr_tk != NULL))
      { // only aux #x: has send #x?
        int send_id = Cnv_AuxIDToSendID(ch_id, m_aux, false);
        isactive = false;
        if (send_id != -1) 
        {
          isactive = true;
          int* send_mode = (int*)GetSetTrackSendInfo(rpr_tk, 0, send_id, "I_SENDMODE", NULL);
          if (*send_mode > 0) ispre = true;
          ismute = *((bool*)GetSetTrackSendInfo(rpr_tk, 0, send_id, "B_MUTE", NULL));
        }
      }

      // get values and update
     
      if (!isactive)
      { // is not active encoder

        if (m_pan) value = 15; // reset, show something to inhlpate inactive
        else value = 0; // reset
      
      } else 
      { 
        if (m_flip)
        {
          if (m_aux > 0)
          { // flip + aux -> send Pan
            int send_id = Cnv_AuxIDToSendID(ch_id, m_aux, true);
            if (send_id != -1)
            {
              GetTrackSendUIVolPan(rpr_tk, send_id, NULL, &d_value);
              value = Cnv_PanToEncoder(d_value);
              if (m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE) value += 0x10; // pan mode
            }
          } else
          { // flip -> volume

            GetTrackUIVolPan(rpr_tk, &d_value, NULL);
            value = Cnv_VolToEncoder(d_value);
            if (m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE) value += 0x20; // bar mode
          }
        } else
        {
          if (m_chan)
          { // chan -> fx_param (para_offset checked above)

            double min, max;
            d_value = MyCSurf_TrackFX_GetParam(chan_rpr_tk, chan_fx, chan_par_offs + ch_id, &min, &max);

            value = Cnv_FXParamToEncoder(min, max, d_value);
            if (m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE) value += 0x20; // bar mode
         
          } else if (m_aux > 0)
          { // aux -> send level
            int send_id = Cnv_AuxIDToSendID(ch_id, m_aux, true);
            if (send_id != -1)
            {
              GetTrackSendUIVolPan(rpr_tk, send_id, &d_value, NULL);
              value = Cnv_VolToEncoder(d_value);
              if (m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE) value += 0x20; // bar mode;
            }
          
          } else if (m_pan)
          {
            if (q_fkey) 
            { // pan mode + fkey -> width
              
              d_value = GetMediaTrackInfo_Value(rpr_tk, "D_WIDTH");
              if (!(m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE))
              {
                value = Cnv_PanToEncoder(d_value);
              
              } else {
                value = Cnv_WidthToEncoder(d_value);
                value += 0x30; // width mode
              }

            } else
            { // pan mode -> pan

              GetTrackUIVolPan(rpr_tk, NULL, &d_value);
              value = Cnv_PanToEncoder(d_value);
              if (m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE) value += 0x10; // pan mode
            }
          }
        } // if (m_flip), else
         
      } // if !active, else

      if (istrack)
      {

        // aux mode: active & pre/post
        if (m_aux > 0)
        {
          if (isactive) dot = true;
          if ((ispre) && (s_myblink)) dot = !dot;
          if (ismute) dot = false;

        // other modes: phase / rec arm
        } else
        {
          // phase states
          if ( (bool)GetMediaTrackInfo_Value(rpr_tk, "B_PHASE") ) dot = true;
          
          // rec arms: blink
          if ( ((bool)GetMediaTrackInfo_Value(rpr_tk, "I_RECARM")) && (s_myblink) ) dot = !dot;
        }

        if (dot) value += 0x40; // set dot
      }

      if (value != cache_enc[ch_id])
      {
        // new value to cache (gets executed on next run cycle)
        cache_enc[ch_id] = value;

        // set upd flag 
        cache_upd_enc = cache_upd_enc | (1 << ch_id);
      }    
    } // if exists
  } // MySetSurface_UpdateEncoder


  void MySetSurface_ExecuteEncoderUpdate(int ch_id)
  {
    unsigned char out = cache_enc[ch_id];
    
    // send midi -> pan
    MIDIOut(0xb0, ch_id + 0x40, cache_enc[ch_id]);

    // send midi -> meter mode
    if (!(m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE))
    {
      if (out & 0x40) MIDIOut(0xb0, ch_id + 0x60, 0x60);
      MIDIOut(0xb0, ch_id + 0x60, cache_enc[ch_id] + 0x50);
    }
    
    // remove update flag
    cache_upd_enc = cache_upd_enc & (~(1 << ch_id));
  } // MySetSurface_ExecuteEncoderUpdate


  void MySetSurface_OutputMeters(bool reset)
  {
    MediaTrack* tk;
    double tk_peak_l, tk_peak_r, tk_hold;
    int peak_out, hold_out;

    // iterate through channels
    for(int ch = 0; ch < 24; ch++)
    {
      // get data
      tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch, s_ch_offset);
	  if (tk) {
		tk_peak_l = Track_GetPeakInfo(tk, 0);
		tk_peak_r = Track_GetPeakInfo(tk, 1);

		peak_out = Cnv_PeakToEncoder((tk_peak_l + tk_peak_r) / 2);

		tk_hold = (Track_GetPeakHoldDB(tk, 0, reset) + Track_GetPeakHoldDB(tk, 1, reset)) * 50;
		hold_out = Cnv_DBToEncoder(tk_hold) + 0x10;

		// over
		if ((tk_peak_l > 1.0) || (tk_peak_r > 1.0)) peak_out += 0x60;
		else peak_out += 0x40;

		if (hold_out != cache_meters[ch])
		{
			// new value to cache (gets executed on next run cycle)
			cache_meters[ch] = hold_out;

			// set upd flag 
			cache_upd_meters = cache_upd_meters | (1 << ch);
		}
		// midi out
		MIDIOut(0xb0, ch + 0x60, peak_out);
	  }
    }
  } // MySetSurface_OutputMeters


  void MySetSurface_ExecuteMeterUpdate(char ch_id)
  {
    MIDIOut(0xb0, ch_id + 0x60, cache_meters[ch_id]);

    // remove update flag
    cache_upd_meters = cache_upd_meters & (~(1 << ch_id));
  } // MySetSurface_ExecuteMeterUpdate


  void MySetSurface_UpdateTrackElement(char ch_id)
  {
    // get info
    MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);
    
    // if trreck exists
    if (rpr_tk)
    {
      bool selected = IsTrackSelected(rpr_tk);
      bool solo = (bool)GetMediaTrackInfo_Value(rpr_tk, "I_SOLO");
      bool mute = (bool)GetMediaTrackInfo_Value(rpr_tk, "B_MUTE");
      bool bypass = (bool)GetMediaTrackInfo_Value(rpr_tk, "I_FXEN");

      // blink chan sel
      selected = ( (m_chan && ch_id == chan_ch && s_myblink) != selected );

      // blink track fx bypass
      mute = ( (!bypass && s_myblink) != mute );

      MySetSurface_UpdateButton(ch_id * 4 + 1, selected, false);
      MySetSurface_UpdateButton(ch_id * 4 + 2, solo, false);
      MySetSurface_UpdateButton(ch_id * 4 + 3, mute, false);
      
    } else
    {
      // reset buttons
      MySetSurface_UpdateButton(ch_id * 4 + 1, false, false);
      MySetSurface_UpdateButton(ch_id * 4 + 2, false, false);
      MySetSurface_UpdateButton(ch_id * 4 + 3, false, false);
    }
  } // MySetSurface_UpdateTrackElement


  void MySetSurface_ToggleFlip()
  {
    m_flip = !m_flip;

    CSurf_ResetAllCachedVolPanStates();
    TrackList_UpdateAllExternalSurfaces();

    MySetSurface_UpdateButton(0x63, m_flip, true);

	  hlpHandler->SetFlip(m_flip);
  } // MySetSurface_ToggleFlip


  void MySetSurface_ToggleFKey(bool btn_state)
  {
    q_fkey = btn_state;

    MySetSurface_UpdateButton(0x6d, btn_state, false);
    MySetSurface_UpdateAuxButtons();

    // update encoders for width in pan mode
    if (m_pan) {
      for (char ch_id = 0; ch_id < 24; ch_id++) {
        if (m_flip) { MySetSurface_UpdateFader(ch_id); }
        else { MySetSurface_UpdateEncoder(ch_id); }
      }
    }

	  if (btn_state) { hlpHandler->SetQkey(2); }
    else { hlpHandler->SetQkey(0); }
  } // MySetSurface_ToggleFKey


  void MySetSurface_ToggleShift(bool btn_state)
  {
    q_shift = btn_state;

    MySetSurface_UpdateButton(0x74, btn_state, false);
    MySetSurface_UpdateAuxButtons();

    if (btn_state) hlpHandler->SetQkey(1);
    else hlpHandler->SetQkey(0);
  } // MySetSurface_ToggleShift


  void MySetSurface_ToggleScrub()
  {
    m_scrub = !m_scrub;
    MySetSurface_UpdateButton(0x6f, m_scrub, false);
  } // MySetSurface_ToggleScrub


  void MySetSurface_Chan_SetFxParamOffset(char dir)
  {
    chan_par_offs -= 24 * dir;
    if (chan_par_offs < 0) chan_par_offs = 0;

    // check parameter count
    int amount_paras = TrackFX_GetNumParams(chan_rpr_tk, chan_fx);
    if (chan_par_offs >= amount_paras) chan_par_offs -= 24;
    
    // update encoders or faders and scribble strip
    for (char ch_id = 0; ch_id <= 23; ch_id++) 
    {
      if (m_flip) MySetSurface_UpdateFader(ch_id);
      else MySetSurface_UpdateEncoder(ch_id);

      stpHandler->Stp_Update(ch_id, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, (ch_id >= 23) * 2);
    }

  } // MySetSurface_Chan_Set_FXParamOffset


  void MySetSurface_Chan_SelectTrack(unsigned char ch_id, bool force_upd)
  {
    if ( (ch_id != chan_ch) || (force_upd) )
    {
      // reset button of old channel
      if (IsTrackSelected(chan_rpr_tk)) MySetSurface_UpdateButton(chan_ch * 4 + 1, true, false);
      else MySetSurface_UpdateButton(chan_ch * 4 + 1, false, false);

      // close fx of old channel
      MyCSurf_Chan_CloseFX(chan_fx);

      // activate new channel
      MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);
      chan_ch = ch_id;
      chan_rpr_tk = rpr_tk;
      
      MySetSurface_UpdateButton(chan_ch * 4 + 1, true, true);

      // open fx              
      MyCSurf_Chan_OpenFX(chan_fx);

	  for (int enc = 0; enc < 24; enc++)
		  stpHandler->Stp_Update(enc, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, (enc >= 23) * 2);
    }

    // bugfix: deselect master
    if (!master_sel) SetTrackSelected(csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset), false);
  } // MySetSurface_Chan_SelectTrack


  void MySetSurface_ShiftBanks(char dir, char factor)
  { 
    double track_amount = (double)CountTracks(0);

    int old_ch_offset = s_ch_offset;

    // move in dir by 8 or 24 (factor)
    int oct_steps = (s_ch_offset / 8);
    if (s_ch_offset % 8 != 0) oct_steps++;
    oct_steps += dir * factor / 8;
    s_ch_offset = oct_steps * 8;

    // min / max
    // if (s_ch_offset > (CSurf_NumTracks(true) - 24)) s_ch_offset = CSurf_NumTracks(true) - 24;
    if (s_ch_offset > 168) s_ch_offset = 168;
    if (s_ch_offset < 0) s_ch_offset = 0;

    // update channel strip
    if (m_chan) chan_ch = chan_ch + old_ch_offset - s_ch_offset;

    // update mixer display
    SetMixerScroll(csurf_utils::Cnv_ChannelIDToMediaTrack(0, s_ch_offset));

    // update encoders, faders, track buttons, scribble strip
    for(char ch_id = 0; ch_id < 24; ch_id++)
    {
      MySetSurface_UpdateEncoder(ch_id);
      MySetSurface_UpdateFader(ch_id);
      MySetSurface_UpdateTrackElement(ch_id);

      stpHandler->Stp_Update(ch_id, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, (ch_id>=23)*2);
    }

    MySetSurface_UpdateBankLEDs();
  } // MySetSurface_ShiftBanks


  void MySetSurface_EnterChanMode()
  {
    if (m_pan) MySetSurface_ExitPanMode();
    if (m_aux > 0) MySetSurface_ExitAuxMode();
   
    if (!chan_fx_env_arm) MyCSurf_Chan_ToggleArmFXEnv();

    m_chan = true;
    // blink Chan Button
    MySetSurface_UpdateButton(0x64, true, true);

    chan_rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(chan_ch, s_ch_offset);

    Utl_CheckFXInsert();
    
    // blink Track Select
    MySetSurface_UpdateButton(chan_ch * 4 + 1, true, true);

    // blink para offset, bypass
    MySetSurface_UpdateAuxButtons();
      
    // blink banks
    MySetSurface_UpdateButton(0x70, true, true);
    MySetSurface_UpdateButton(0x71, true, true);

    // update scribble strip
	for (int enc = 0; enc < 24; enc++)
		stpHandler->Stp_Update(enc, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, (enc >= 23) * 2);

    // bugfix: deselect master
    if (!master_sel) SetTrackSelected(csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset), false);

	  hlpHandler->SetMode(1);
  } // MySetSurface_EnterChanMode


  void MySetSurface_ExitChanMode()
  {
    m_chan = false;

    // reset chan button
    MySetSurface_UpdateButton(0x64, false, false);

    // reset select button
	if (chan_rpr_tk != NULL) {
		if (IsTrackSelected(chan_rpr_tk)) MySetSurface_UpdateButton(chan_ch * 4 + 1, true, false);
		else MySetSurface_UpdateButton(chan_ch * 4 + 1, false, false);
	}

    // if writing fx envs, stop now
	if (chan_fx_env_arm) MyCSurf_Chan_ToggleArmFXEnv();

    // close window
	MyCSurf_Chan_CloseFX(chan_fx);

    // unblink bank buttons
	MySetSurface_UpdateButton(0x70, false, false);
	MySetSurface_UpdateButton(0x71, false, false);

	MySetSurface_EnterPanMode();

    // update scribble strip
	for (int enc = 0; enc < 24; enc++)
		stpHandler->Stp_Update(enc, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, (enc >= 23) * 2);

    // bugfix: deselect master
	if (!master_sel) SetTrackSelected(csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset), false);
  } // MySetSurface_ExitChanMode


  void MySetSurface_EnterPanMode()
  {
    if (m_chan) MySetSurface_ExitChanMode();
    if (m_aux > 0) MySetSurface_ExitAuxMode();

    m_pan = true;
    MySetSurface_UpdateButton(0x6c, true, false);

    // reset Aux
    MySetSurface_UpdateAuxButtons();

    // update encoders or faders
    for (char ch_id = 0; ch_id < 23; ch_id++) 
      if (m_flip) MySetSurface_UpdateFader(ch_id);
      else MySetSurface_UpdateEncoder(ch_id);

	  hlpHandler->SetMode(0);
  } // MySetSurface_EnterPanMode


  void MySetSurface_ExitPanMode()
  {
    m_pan = false;
    MySetSurface_UpdateButton(0x6c, false, false);
  } // MySetSurface_ExitPanMode


  void MySetSurface_EnterAuxMode(unsigned char sel)
  {
    if (m_pan) MySetSurface_ExitPanMode();

    m_aux = sel;

    // reset Aux
    MySetSurface_UpdateAuxButtons();

    // update encoders or faders
    for (char ch_id = 0; ch_id < 23; ch_id++)
    { 
      if (m_flip) MySetSurface_UpdateFader(ch_id);
      MySetSurface_UpdateEncoder(ch_id); // update encoders on flip also (> send pan)!
    }

	  hlpHandler->SetMode(2);
  } // MySetSurface_EnterAuxMode


  void MySetSurface_ExitAuxMode()
  {
    m_aux = 0;
  } // MySetSurface_ExitAuxMode

  

  // REAPER INITIATED SURFACE UPDATES

  void SetSurfaceVolume(MediaTrack* rpr_tk, double vol)
  { 
    int ch_id = Cnv_MediaTrackToChannelID(rpr_tk);

    if ( (ch_id >= 0) && (ch_id <= 24) ) {
      if (!m_flip) { MySetSurface_UpdateFader(ch_id); }
      else if (ch_id <= 23) { MySetSurface_UpdateEncoder(ch_id); }
    }
  } // SetSurfaceVolume
  

  void SetSurfacePan(MediaTrack* rpr_tk, double pan)
  {
    int ch_id = Cnv_MediaTrackToChannelID(rpr_tk);

    if ( (ch_id >= 0) && (ch_id <= 24) ) {
      if (m_flip) { MySetSurface_UpdateFader(ch_id); }
      else if (ch_id <= 23) { MySetSurface_UpdateEncoder(ch_id); }
    }
  } // SetSurfacePan
  

  void SetTrackListChange()
  {
    CSurf_ResetAllCachedVolPanStates(); // is this needed?

    
    // update Midi LCD separately in m_chan mode (special mode where tracknames are updated in m_chan)

    if (m_chan)
        for (int i = 0; i < 24; i++)
        {
            if (i < 24) stpHandler->Stp_Update(i, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, false, (i >= 23) * 2);
        }

    // reset faders, encoders, track elements, update scribble strip
    for (int i = 0; i < 25; i++)
    {
      MySetSurface_UpdateFader(i);
      MySetSurface_UpdateTrackElement(i);
      MySetSurface_UpdateEncoder(i);

      if (i < 24) stpHandler->Stp_Update(i, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, (i >= 23) * 2);
    }
  } // SetTrackListChange


  void SetSurfaceMute(MediaTrack* rpr_tk, bool mute)
  {
    int ch_id = Cnv_MediaTrackToChannelID(rpr_tk);

    if ( (ch_id >= 0) && (ch_id <= 23) ) 
    {
      if (mute) MySetSurface_UpdateButton(4 * ch_id + 3, true, false);
      else MySetSurface_UpdateButton(4 * ch_id + 3, false, false);

	  stpHandler->Stp_Update(ch_id, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, false);
    }
  } // SetSurfaceMute


  void SetSurfaceSelected(MediaTrack* rpr_tk, bool selected)
  {
    int ch_id = Cnv_MediaTrackToChannelID(rpr_tk);

    // update buttons
    if ( (ch_id >= 0) && (ch_id <= 24) ) 
    {
      MySetSurface_UpdateButton(4 * ch_id + 1, selected, false);
	  stpHandler->Stp_Update(ch_id, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, false);
    }
  } // SetSurfaceSelected


  void SetSurfaceSolo(MediaTrack* rpr_tk, bool solo)
  {
    int ch_id = Cnv_MediaTrackToChannelID(rpr_tk);

    if ( (ch_id >= 0) && (ch_id <= 23) ) 
    {
      if (solo) MySetSurface_UpdateButton(4 * ch_id + 2, true, false);
      else MySetSurface_UpdateButton(4 * ch_id + 2, false, false);
    }

    // update CLR SOLO
    if (AnyTrackSolo(0)) MySetSurface_UpdateButton(0x62, true, true);
    else MySetSurface_UpdateButton(0x62, false, false);
  } // SetSurfaceSolo


  void SetSurfaceRecArm(MediaTrack* rpr_tk, bool recarm)
  {
    int ch_id = Cnv_MediaTrackToChannelID(rpr_tk);

    if ((ch_id > 0) && (ch_id < 24))
    {
      MySetSurface_UpdateEncoder(ch_id);
	  stpHandler->Stp_Update(ch_id, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, false);
    }
  } // SetSurfaceRecArm


  bool GetTouchState(MediaTrack* rpr_tk, int isPan)
  {
    int ch_id = Cnv_MediaTrackToChannelID(rpr_tk);

    if (isPan == 0)
    {
      if ( (ch_id >= 0) && (ch_id <= 24) && ((s_touch_fdr & (1 << ch_id)) > 0) ) return true;
      else return false;

    } else
    {
      if ( (ch_id >= 0) && (ch_id <= 23) && (s_touch_enc[ch_id] > 0) ) return true;
      else return false;
    }

  } // GetTouchState


  void SetPlayState(bool play, bool pause, bool rec)
  {
    s_play = play && !pause;
    s_rec = rec;
    
    MySetSurface_UpdateAutoLEDs();
    MySetSurface_UpdateButton(0x79, s_rec, false);
  } // SetPlayState


  void SetRepeatState(bool rep)
  {
    s_loop = rep;

    // update in/out buttons
    MySetSurface_UpdateButton(0x73, s_loop, false); 
  } // SetRepeatState



  ////// SUBMIT CHANGES TO REAPER /////

  // API Event Triggers:
  // double CSurf_OnVolumeChange(MediaTrack *trackid, double volume, bool relative)
  // double CSurf_OnPanChange(MediaTrack *trackid, double pan, bool relative)
  // bool CSurf_OnMuteChange(MediaTrack *trackid, int mute)
  // bool CSurf_OnSelectedChange(MediaTrack *trackid, int selected)
  // bool CSurf_OnSoloChange(MediaTrack *trackid, int solo)
  // bool CSurf_OnFXChange(MediaTrack *trackid, int en)
  // bool CSurf_OnRecArmChange(MediaTrack *trackid, int recarm)
  // void CSurf_OnPlay()
  // void CSurf_OnStop()
  // void CSurf_OnFwd(int seekplay)
  // void CSurf_OnRew(int seekplay)
  // void CSurf_OnRecord()
  // void CSurf_GoStart()
  // void CSurf_GoEnd()
  // void CSurf_OnArrow(int whichdir, bool wantzoom)
  // void CSurf_OnTrackSelection(MediaTrack *trackid)
  // void CSurf_ResetAllCachedVolPanStates()
  // void CSurf_void ScrubAmt(double amt)


  // TRACK CONTROLS AND BEYOND

  void MyCSurf_ToggleSolo(MediaTrack* rpr_tk, bool this_only)
  {
    int solo = (int)GetMediaTrackInfo_Value(rpr_tk, "I_SOLO");
    if (this_only)
    {
      // unsolo all, (re-)solo only selected
      SoloAllTracks(0);
      solo = 2;
    } else {
      // toggle solo
      if (solo == 2) solo = 0;
      else solo = 2;
    }

    CSurf_OnSoloChange(rpr_tk, solo);
    
    /*
    bool solo_surf = true;  
    if (solo == 0) solo_surf = false;

    SetSurfaceSolo(rpr_tk, solo_surf);*/
  } // MyCSurf_ToggleSolo


  void MyCSurf_UnsoloAllTracks()
  {
    SoloAllTracks(0);
  } // MyCSurf_UnsoloAllTracks


  void MyCSurf_ToggleMute(MediaTrack* rpr_tk, bool this_only)
  {
    char mute;
    if (this_only)
    {
      // unmute all, (re-)mute only selected
      MuteAllTracks(0);
      mute = 1;
    } else
    {
      // toggle mute on selected
      mute = -1;
    }
    CSurf_OnMuteChange(rpr_tk, mute);
  } // MyCSurf_ToggleMute


  void MyCSurf_UnmuteAllTracks()
  {
    MuteAllTracks(0);
  } // MyCSurf_UnmuteAllTracks


  void MyCSurf_SwitchPhase(MediaTrack* rpr_tk)
  {
    char ch_id = Cnv_MediaTrackToChannelID(rpr_tk);

    bool phase = (bool)GetMediaTrackInfo_Value(rpr_tk, "B_PHASE");

    phase = !phase;
    SetMediaTrackInfo_Value(rpr_tk, "B_PHASE", phase);

    MySetSurface_UpdateEncoder(ch_id);
  } // MyCSurf_SwitchPhase


  void MyCSurf_AddSwitchAuxSend(MediaTrack* rpr_tk, int aux)
  {
    int tk_id = (int)GetMediaTrackInfo_Value(rpr_tk, "IP_TRACKNUMBER") - 1;
    MediaTrack* aux_tk = Utl_FindAux(aux);
   
    char* chunk = GetSetObjectState(aux_tk, "");
    WDL_String chunk_wdl = WDL_String(chunk);

    // search for existing sends
    char search[90];
    char insert[90];
    int found_mode = -1;
    for (int m = 0; m <= 3; m++)
    {
      sprintf(search, "AUXRECV %d %d", tk_id, m);
      if (strstr(chunk, search)) found_mode = m;
    }

    FreeHeapPtr(chunk);

    if (found_mode == -1)
    {
      // new line for post send
      sprintf(insert, "AUXRECV %d 0 1.00000000000000 0.00000000000000 0 0 0 0 0 -1.00000000000000 0 -1 ''\n", tk_id);
      chunk_wdl = Utl_Chunk_InsertLine(chunk_wdl, WDL_String(insert), WDL_String("MIDIOUT "));
  
    } else if (found_mode == 0)
    {
      // new line for post send
      sprintf(search, "AUXRECV %d 0", tk_id);
      sprintf(insert, "AUXRECV %d 3", tk_id);

      chunk_wdl = Utl_Chunk_Replace(chunk_wdl, WDL_String(search), WDL_String(insert));
    
    } else
    {
      // new line for post send
      sprintf(search, "AUXRECV %d %d", tk_id, found_mode);
      sprintf(insert, "AUXRECV %d 0", tk_id);

      chunk_wdl = Utl_Chunk_Replace(chunk_wdl, WDL_String(search), WDL_String(insert));     
    } 

    GetSetObjectState(aux_tk, chunk_wdl.Get());

  } // MyCSurf_AddSwitchAuxSend


  void MyCSurf_RemoveAuxSend(MediaTrack* rpr_tk, int aux)
  {
    int tk_id = (int)round(GetMediaTrackInfo_Value(rpr_tk, "IP_TRACKNUMBER") - 1);
    MediaTrack* aux_tk = Utl_FindAux(aux);
   
    char* chunk = GetSetObjectState(aux_tk, "");
    WDL_String chunk_wdl = WDL_String(chunk);
    FreeHeapPtr(chunk); 

    char search[90];
    
    // search for post send
    sprintf(search, "AUXRECV %d", tk_id);

    chunk_wdl = Utl_Chunk_RemoveLine(chunk_wdl, WDL_String(search));

    GetSetObjectState(aux_tk, chunk_wdl.Get()); 
  } // MyCSurf_RemoveAuxSend


  void MyCSurf_ToggleMuteAuxSend(MediaTrack* rpr_tk, int aux)
  {
      int tk_id = (int)round(GetMediaTrackInfo_Value(rpr_tk, "IP_TRACKNUMBER") - 1);
      MediaTrack* aux_tk = Utl_FindAux(aux);

      char* chunk = GetSetObjectState(aux_tk, "");
      WDL_String chunk_wdl = WDL_String(chunk);

      // search for existing sends
      char search[90];
      char insert[90];
      int mutePos = 5;  //mute status position in string
      char* pch;
      int muteStatus;

      int i;

      sprintf(search, "AUXRECV %d", tk_id);
      char* auxSubstr = strstr(chunk, search);
      if (auxSubstr) // track has aux
      {
          // get aux mute status
          //Splitting string into tokens
          search[0] = '\0'; //reset string for later use
          insert[0] = '\0'; //init string for use with strcat

          pch = strtok(auxSubstr, " ");

          for (i=0; i<mutePos; i++)
          {
              strcat(search, pch);
              strcat(insert, pch);
              pch = strtok(NULL, " ");
              strcat(search, " ");
              strcat(insert, " ");
          }

          muteStatus = atoi(pch);
          
          // toggle aux mute status
          if (muteStatus == 0)
          {
              strcat(search, "0");
              strcat(insert, "1");
          }
          else if (muteStatus == 1)
          {
              strcat(search, "1");
              strcat(insert, "0");
          }
          
          // write aux mute status
          chunk_wdl = Utl_Chunk_Replace(chunk_wdl, WDL_String(search), WDL_String(insert));
          GetSetObjectState(aux_tk, chunk_wdl.Get());
      }

      FreeHeapPtr(chunk);

      
  } //MyCSurf_ToggleMuteAuxSend


  void MyCSurf_SelectMaster() 
  {
    MediaTrack* rpr_master = csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset);

    master_sel = IsTrackSelected(rpr_master);
    master_sel = !master_sel;
    CSurf_OnSelectedChange(rpr_master, (int)master_sel); // ?
    SetTrackSelected(rpr_master, master_sel); // ?
    Main_OnCommand(CMD_SEL2LASTTOUCH, 0);
  } // MyCSurf_SelectMaster


  void MyCSurf_ToggleSelectAllTracks() 
  {
    int sel_tks = CountSelectedTracks(0);
    int all_tks = CountTracks(0);

    bool sel = false;
    MediaTrack* tk;

    // no track selected, master also not selected?
    if ( (sel_tks == 0) && ( !IsTrackSelected(csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset) ) ) ) sel = true;

    // set tracks sel or unsel
    for (int i = 0; i < all_tks; i++)
    {
      tk = GetTrack(0, i);
      SetTrackSelected(tk, sel);
    }

    // apply to master also
    MediaTrack* rpr_master = csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset);
    SetTrackSelected(rpr_master, sel); 
    master_sel = sel;

  } // MyCSurf_ToggleSelectAllTracks() 


  // CUSTOM TRANSPORT 

  void MyCSurf_OnRec()
  {
    if (s_play) 
    {
      s_rec = !s_rec;
      Main_OnCommand(CMD_TGGLRECBEAT, 0);
      if (s_rec) MySetSurface_UpdateButton(0x79, true, true);
      else MySetSurface_UpdateButton(0x79, false, false);
    } else 
    {
      CSurf_OnRecord();
    }
  } // MyCSurf_OnRec


  // CHANNEL STRIP

  void MyCSurf_Chan_SelectFX(int open_fx_id)
  {
    MyCSurf_Chan_CloseFX(chan_fx);
    MyCSurf_Chan_OpenFX(open_fx_id);
  } // MyCSurf_Chan_SelectFX


  void MyCSurf_Chan_OpenFX(int fx_id)
  {
    int amount_fx = TrackFX_GetCount(chan_rpr_tk);

    if (fx_id >= amount_fx) fx_id = 0;
    else if (fx_id < 0) fx_id = amount_fx - 1;

    chan_fx = fx_id;
    TrackFX_Show(chan_rpr_tk, chan_fx, 2); // hide floating window
    TrackFX_Show(chan_rpr_tk, chan_fx, 0); // show chain window
    TrackFX_SetOpen(chan_rpr_tk, chan_fx, true);

    csurf_utils::PrepareParamMapArray(chan_rpr_tk, chan_fx);

    // reset param offset
    chan_par_offs = 0;

    MySetSurface_UpdateAuxButtons();

	for (int enc = 0; enc < 24; enc++)
		stpHandler->Stp_Update(enc, chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_flip, m_chan, (enc >= 23) * 2);

    // bugfix: deselect master
    if (!master_sel) SetTrackSelected(csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset), false);
  } // MyCSurf_Chan_OpenFX
  

  void MyCSurf_Chan_CloseFX(int fx_id)
  {
    TrackFX_Show(chan_rpr_tk, fx_id, 2); // hide floating window
    TrackFX_Show(chan_rpr_tk, fx_id, 0); // hide chain window

    // bugfix: deselect master
    if (!master_sel) SetTrackSelected(csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset), false);
  } // MyCSurf_Chan_CloseFX


  void MyCSurf_Chan_DeleteFX()
  {
    Undo_BeginBlock();

    // count fx
    int before_del = TrackFX_GetCount(chan_rpr_tk);

    //isolate track for action
    Utl_SaveSelection();
    SetOnlyTrackSelected(chan_rpr_tk);
    Main_OnCommand(CMD_SEL2LASTTOUCH, 0);

    TrackFX_SetOpen(chan_rpr_tk, chan_fx, true);
    Main_OnCommand(CMD("_S&M_REMOVE_FX"), 0);
    
    Utl_RestoreSelection();
    
    if (before_del > 1)
    { 
      // if there are fx left open the previous one in chain
      chan_fx--;
      MyCSurf_Chan_OpenFX(chan_fx);
    }

    // bugfix: deselect master
    if (!master_sel) SetTrackSelected(csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset), false);

    Undo_EndBlock("Delete FX", 0);
  } // MyCSurf_Chan_DeleteFX


  void MyCSurf_Chan_InsertFX()
  {
    // isolate track for action
    Utl_SaveSelection();
    SetOnlyTrackSelected(chan_rpr_tk);
    Main_OnCommand(CMD_SEL2LASTTOUCH, 0);
    
    TrackFX_Show(chan_rpr_tk, chan_fx, 1); // show chain window
    TrackFX_SetOpen(chan_rpr_tk, chan_fx, true);
    Main_OnCommand(CMD_FXBROWSER, 0);

    Utl_RestoreSelection();

    // bugfix: deselect master
    if (!master_sel) SetTrackSelected(csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset), false);

  } // MyCSurf_Chan_InsertFX


  void MyCSurf_Chan_MoveFX(char dir)
  {
    Undo_BeginBlock();

    int amount_fx = TrackFX_GetCount(chan_rpr_tk);

    // isolate track for selection
    Utl_SaveSelection();
    SetOnlyTrackSelected(chan_rpr_tk);
    Main_OnCommand(CMD_SEL2LASTTOUCH, 0);

    TrackFX_SetOpen(chan_rpr_tk, chan_fx, true);
    if ( (dir < 0) && (chan_fx > 0) )
    {
      Main_OnCommand(CMD("_S&M_MOVE_FX_UP"), 0);
      chan_fx--;
    
    } else if ( (dir > 0) && (chan_fx < amount_fx - 1) )
    {
      Main_OnCommand(CMD("_S&M_MOVE_FX_DOWN"), 0);
      chan_fx++;  
    }

    Utl_RestoreSelection();

    // bugfix: deselect master
    if (!master_sel) SetTrackSelected(csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset), false);

    Undo_EndBlock("Move FX", 0);
  } // MyCSurf_Chan_MoveFX


  void MyCSurf_Chan_ToggleAllFXBypass(int ch_id)
  {
    MediaTrack* rpr_tk = csurf_utils::Cnv_ChannelIDToMediaTrack(ch_id, s_ch_offset);

    // get info
    bool bypass = (bool)GetMediaTrackInfo_Value(rpr_tk, "I_FXEN");
    
    // toggle bypass
    bypass = !bypass;
    CSurf_OnFXChange(rpr_tk, (int)bypass);   
  } // MyCSurf_Chan_ToggleAllFXBypass


  void MyCSurf_Chan_ToggleFXBypass()
  {
    bool bypass = (bool)TrackFX_GetEnabled(chan_rpr_tk, chan_fx);
    bypass = !bypass;
    TrackFX_SetEnabled(chan_rpr_tk, chan_fx, bypass);
    MySetSurface_UpdateAuxButtons();
  } // MyCSurf_Chan_ToggleFXBypass


  void MyCSurf_Chan_SetFXParam(MediaTrack* rpr_tk, int fx_id, int para_id, double value)
  {
    char ch_id = Cnv_MediaTrackToChannelID(rpr_tk);

    MyCSurf_TrackFX_SetParam(rpr_tk, fx_id, para_id, value);

    if (m_flip) MySetSurface_UpdateFader(ch_id);
    else MySetSurface_UpdateEncoder(ch_id);
  } // MyCSurf_Chan_SetFXParam


  void MyCSurf_Chan_ToggleArmFXEnv()
  {
    Utl_SaveSelection();
    SetOnlyTrackSelected(csurf_utils::Cnv_ChannelIDToMediaTrack(chan_ch, s_ch_offset));

    // arm all envelopes -> toggle fx disarms only them
    if (chan_fx_env_arm) Main_OnCommand(CMD("_S&M_ARMALLENVS"), 0);
    // disarm all envelopes -> toggle fx arms only them
    else Main_OnCommand(CMD("_S&M_DISARMALLENVS"), 0);
 
    // toggle arm fx envelopes
    Main_OnCommand(CMD("_S&M_TGLARMPLUGENV"), 0);

    chan_fx_env_arm = !chan_fx_env_arm;

    Utl_RestoreSelection();

    MySetSurface_UpdateAuxButtons();
  }


  // AUTOMATION

  void MyCSurf_Auto_SetMode(int mode)
  {
    // mode: 0 = off / trim, 1 = read, 2 = touch, 3 = write, 4 = latch

    // exchange touch and latch
    char set_mode = mode;
    if (mode == 2) set_mode = 4;

    int sel_tks = CountSelectedTracks(0);
    int all_tks = CountTracks(0);
    MediaTrack* tk;

    // if writing fx automation only select and change mode for current track
    if (chan_fx_env_arm)
    {
      Utl_SaveSelection();
      SetOnlyTrackSelected(chan_rpr_tk);

    // if none selected, select and change all
    } else if (sel_tks == 0)
    {
      Utl_SaveSelection();

      for (int i = 0; i < all_tks; i++)
      {
        tk = GetTrack(0, i);
        SetTrackSelected(tk, true);
      }
    }
    
    // set mode for new selection
    int set_sel = CountSelectedTracks(0);
    for (int t = 0; t < set_sel; t++)
    {
      tk = GetSelectedTrack(0, t);
      SetTrackAutomationMode(tk, set_mode);      
    }

    // restore selection
    if ((chan_fx_env_arm) || (sel_tks == 0))
      Utl_RestoreSelection();


    /* update CSurf here - because SetAutoMode() only works for global mode changes? */

    // check all tracks for auto modes
    int tk_mode;
    s_automode = 0;

    for (int t = 0; t < all_tks; t++)    
    {
      tk = GetTrack(0, t);
      tk_mode = GetTrackAutomationMode(tk);

      // switch touch and latch
      if (tk_mode == 4) tk_mode = 2;

      // set flags
      s_automode = s_automode | (1 << tk_mode);
    } 

    MySetSurface_UpdateAutoLEDs();
  } // MyCSurf_Auto_SetMode


  void MyCSurf_Auto_WriteCurrValues()
  {
    Main_OnCommand(CMD_AUTOTOSEL, 0);
  } // MyCSurf_Auto_WriteCurrValues


  // CUSTOM COMMANDS FOR TIME SELECTION / CURSOR

  void MyCSurf_ToggleRepeat()
  {
    s_loop = (bool)GetSetRepeat(2);
  } // MyCSurf_ToggleRepeat


  void MyCSurf_MoveTimeSel(signed char start_dir, signed char end_dir, bool markers)
  {
    // markers: true = move to next marker/region, false = move 1 bar
    // start_dir: move start of time sel by dir
    // end_dir: move end of time sel by dir


    // do nothing if sel all is on
    if (!s_loop_all)
    {

      // get time range
      ReaProject* rpr_pro = EnumProjects(-1, NULL, 0);
      double start_time, start_beats, end_time, end_beats;
      int start_num, start_denom, end_num, end_denom;

      // get time selection
      GetSet_LoopTimeRange(false, true, &start_time, &end_time, false);

      if (markers)
      {

        // max 100 markers / regions (couldn't get vectors to work)
        double* starts = new double[100];
        double* ends = new double[100];

        double curr_region_end = 9999999999.0;
        double last_pos = 0.0;
        bool inside_region = false;
        int idx = 0;
        int sel = 0;

        double sel_approx = 9999999999.0;

        double pos, region_end, start_diff, end_diff;
        bool is_region;
        int x = 0;
        while ( (x = EnumProjectMarkers(x, &is_region, &pos, &region_end, NULL, NULL)) && (idx <= 100) )
        {

          // did we leave a previously established region?
          if (pos > curr_region_end) 
          {
            // count region end as marker
            pos = curr_region_end;
            x--;

            // reset region flags
            inside_region = false;
            curr_region_end = 9999999999.0;
          }
          
          // add this range [last -> current marker (or region start/end)] to list
          if (
            (dblEq(pos, last_pos, 0.001) == false) &&
            (idx <= 100) && 
            ( 
              (idx == 0) || 
              (dblEq(last_pos, starts[idx-1], 0.001) == false) ||
              (dblEq(pos, ends[idx-1], 0.001) == false)
            ) )
          {
            starts[idx] = last_pos;
            ends[idx] = pos;

            // range fits current time selection?
            start_diff = fabs(fabs(starts[idx]) - fabs(start_time));
            end_diff = fabs(fabs(ends[idx]) - fabs(end_time));
            if (start_diff + end_diff < sel_approx)
            {
              sel_approx = start_diff + end_diff;
              sel = idx;

              // exact match? then select next or previous range depending on start_dir
              if (sel_approx < 0.002)
              {
                sel += start_dir;
              }
            }

            idx++;
          }

          // add region to list
          if (is_region) 
          {
            inside_region = true;
            curr_region_end = region_end;
            
            starts[idx] = pos;
            ends[idx] = region_end;

            // range fits current time selection? 
            start_diff = fabs(fabs(starts[idx]) - fabs(start_time));
            end_diff = fabs(fabs(ends[idx]) - fabs(end_time));
            if (start_diff + end_diff < sel_approx)
            {
              sel_approx = start_diff + end_diff;
              sel = idx;

              // exact match? then select next or previous range depending on start_dir
              if (sel_approx < 0.002)
              {
                sel += start_dir;
              }
            }

            idx++;
                      
          } 

          last_pos = pos;

        }

        // end reached, but still inside a region? do one last goround
        if ( (inside_region) && (idx <= 100) )
        {
          pos = curr_region_end;
          if ( 
            (dblEq(pos, last_pos, 0.001) == false) &&
            ( 
              (dblEq(last_pos, starts[idx-1], 0.001) == false) ||
              (dblEq(pos, ends[idx-1], 0.001) == false)
            ) )
          {
            starts[idx] = last_pos;
            ends[idx] = pos;

            // range fits current time selection?
            start_diff = fabs(fabs(starts[idx]) - fabs(start_time));
            end_diff = fabs(fabs(ends[idx]) - fabs(end_time));
            if (start_diff + end_diff < sel_approx)
            {
              sel_approx = start_diff + end_diff;
              sel = idx;

              // exact match? then select next or previous range depending on start_dir
              if (sel_approx < 0.002)
              {
                sel += start_dir;
              }
            }

            idx++;
          }
        }

        // clamp selection to boundaries of list, wrap around
        if (sel >= idx)
        {
          sel = 0;
        }

        if (sel < 0)
        {
          sel = idx - 1;
        }
 
        // set new time selection
        start_time = starts[sel];
        end_time = ends[sel];
        GetSet_LoopTimeRange(true, true, &start_time, &end_time, false);

        // clean up arrays
        delete [] starts;
        delete [] ends;

      } else
      {
        // start / end: get time sig and position in beats
        TimeMap_GetTimeSigAtTime(rpr_pro, start_time, &start_num, &start_denom, NULL);
        start_beats = TimeMap2_timeToQN(rpr_pro, start_time);

        TimeMap_GetTimeSigAtTime(rpr_pro, end_time, &end_num, &end_denom, NULL);
        end_beats = TimeMap2_timeToQN(rpr_pro, end_time);

        // shift by bars
        start_beats += start_dir * (4 * start_num / start_denom);
        end_beats += end_dir * (4 * end_num / end_denom);

        // reconvert to seconds
        start_time = TimeMap2_QNToTime(rpr_pro, start_beats);
        end_time = TimeMap2_QNToTime(rpr_pro, end_beats);

        // snap to grid
        SnapToGrid(rpr_pro, start_time);
        SnapToGrid(rpr_pro, end_time);

        // set time selection
        GetSet_LoopTimeRange(true, true, &start_time, &end_time, false);
      }
    }
  } // MyCSurf_MoveTimeSel


  void MyCSurf_Loop_ToggleAll()
  {
    if (!s_loop_all) {
      
      // save current sel
      GetSet_LoopTimeRange(false, true, &s_ts_start, &s_ts_end, false);
      
      // time sel all
      Main_OnCommand(CMD_SELALLITEMS, 0);
      Main_OnCommand(CMD_TIMESEL2ITEMS, 0);
      Main_OnCommand(CMD_UNSELALLITEMS, 0);  

      s_loop_all = true;
    
    } else {

      // restore time sel
      GetSet_LoopTimeRange(true, true, &s_ts_start, &s_ts_end, false);

      s_loop_all = false;
    }

    MySetSurface_UpdateButton(0x72, s_loop_all, false); 

  } // MyCSurf_Loop_ToggleAll


  void MyCSurf_RemoveTimeSel()
  {
    Main_OnCommand(CMD_CLEARTIMESEL, 0);
  } // MyCSurf_RemoveTimeSel


  void MyCSurf_Scrub(char rel_value, bool fast)
  {
    double d_value;
    if (fast) d_value = (double)rel_value / (double)SCRUBRESFAST;
    else d_value = (double)rel_value / (double)SCRUBRESSLOW;
    
    CSurf_ScrubAmt(d_value);
  } // MyCSurf_Scrub
    

  void MyCSurf_MoveEditCursor(char rel_value, bool fast)
  {
    ReaProject* rpr_pro = EnumProjects(-1, NULL, 0);
    double old_val = GetCursorPosition();
    double d_value;
    if (fast) d_value = (double)rel_value / (double)MOVERESFAST;
    else d_value = (double)rel_value / (double)MOVERESSLOW;

    SetEditCurPos2(rpr_pro, old_val + d_value, true, false);
  } // MyCSurf_MoveEditCursor



  ////// CONFIG STUFF FOR REGISTERING AND PREFERENCES //////

  const char *GetTypeString() 
  {
    return "US-2400";
  }
  

  const char *GetDescString()
  {
    descspace.Set(DESCSTRING);
    char tmp[512];
    WDL_String vers = WDL_String("1.3.0");
    WDL_String midiStr = WDL_String(0);
    char midi_in_name[64];
    midi_in_name[0]=0;
    char midi_out_name[64];
    midi_out_name[0]=0;
    char midi_out_name_disp[64];
    midi_out_name_disp[0] = 0;
    GetMIDIInputName(m_midi_in_dev,midi_in_name,sizeof(midi_in_name));
    GetMIDIOutputName(m_midi_out_dev,midi_out_name,sizeof(midi_out_name));
    GetMIDIOutputName(m_midi_out_dev_disp, midi_out_name_disp, sizeof(midi_out_name_disp));
    if(midi_in_name[0] == 0) {
      midiStr.Append("None");
    } else {
      midiStr.Append(midi_in_name);
    }
    midiStr.Append("/");
    if(midi_out_name[0] == 0) {
      midiStr.Append("None");
    } else {
      midiStr.Append(midi_out_name);
    }
    midiStr.Append("/");
    if (midi_out_name_disp[0] == 0) {
        midiStr.Append("None");
    }
    else {
        midiStr.Append(midi_out_name_disp);
    }


    sprintf(tmp," (%s) [%s]",vers.Get(),midiStr.Get());
    descspace.Append(tmp);
    return descspace.Get();     
  }
  

  const char *GetConfigString() // string of configuration data
  {
    sprintf(configtmp,"0 0 %d %d %d %d",m_midi_in_dev,m_midi_out_dev,m_midi_out_dev_disp, m_cfg_flags);      
    return configtmp;
  }



  ////// CENTRAL CSURF MIDI EVENT LOOP //////

  void Run()
  {
    // midi processing
    if ( (m_midiin) ) //&& (s_initdone) )
    {
      m_midiin->SwapBufs(timeGetTime());
      int l=0;
      MIDI_eventlist *list=m_midiin->GetReadBuf();
      MIDI_event_t *evts;
      while ((evts=list->EnumItems(&l))) MIDIin(evts);
    }


    // countdown enc touch delay
    for (char i = 0; i < 24; i++)
      if (s_touch_enc[i] > 0) s_touch_enc[i]--;


    // Execute fader/encoder updates
    char i = 0;
    char ex = 0;
    do 
    {
    
      if ((cache_upd_faders & (1 << cache_exec)) > 0) {
        MySetSurface_ExecuteFaderUpdate(cache_exec);
        ex += 2;
      }
      if ((cache_upd_enc & (1 << cache_exec)) > 0) {
        MySetSurface_ExecuteEncoderUpdate(cache_exec);
        ex++;
      }

      if ((m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE) && ((cache_upd_meters & (1 << cache_exec)) > 0)) {
        MySetSurface_ExecuteMeterUpdate(cache_exec);
        ex++;
      }
           
      // incr / wrap cache_exec
      cache_exec++;
      if (cache_exec > 24) cache_exec = 0;

      i++;
      
      // repeat loop until all channels checked or exlimit reached
    } while ((i < 25) && (ex < EXLIMIT));


    // meters
    if ((m_cfg_flags&csurf_utils::CONFIG_FLAG_METER_MODE)) MySetSurface_OutputMeters(false); // false = no reset


    // countdown m button delay, update if applicable
    if (q_mkey)
    {
      if (s_mkey_on > 0) {
        s_mkey_on--;
        
        // blink all participating buttons to indicate mkey mode
        bool mblnk = false;
        if (s_mkey_on % 2 == 0) mblnk = true;
        
        char mde = 0;
        if (m_chan) mde = 1;
        if (m_aux > 0) mde = 2;

        bool mact = false;
        
        // aux buttons / transport
        for (char b = 0; b < 6; b++)
        {
          if (cmd_ids[3][mde][b] != -1)
          {
            MySetSurface_UpdateButton(0x65 + b, mblnk, false);
            mact = true;
          }
          if ((b < 5) && (cmd_ids[3][mde][b+7] != -1))
          {
            MySetSurface_UpdateButton(0x75 + b, mblnk, false);
            mact = true;
          }
        }

        // null button
        if (cmd_ids[3][mde][6] != -1)
        {
          MySetSurface_UpdateButton(0x6e, mblnk, false);
          mact = true;
        }

        // meter button
        if (mact) MySetSurface_UpdateButton(0x6b, mblnk, false); // meter
      } 
      else
      {
        q_mkey = false;

		    hlpHandler->SetQkey(0);

        //reset buttons
        MySetSurface_UpdateAuxButtons(); // aux
        MySetSurface_UpdateAutoLEDs(); // transport
        MySetSurface_UpdateButton(0x6e, false, false); // null
        MySetSurface_UpdateButton(0x6b, false, false); // meter
      }
    }


    // blink
    if (myblink_ctr > MYBLINKINTV)
    {
      s_myblink = !s_myblink;

      // reset counter, on is shorter
      if (s_myblink) myblink_ctr = MYBLINKINTV - MYBLINKRATIO;
      else myblink_ctr = 0;
      
      for (char ch = 0; ch < 24; ch++)
      {
        // update encoders (rec arm)
        MySetSurface_UpdateEncoder(ch);

        // update track buttons
        MySetSurface_UpdateTrackElement(ch);

      }

      // blink Master if anything selected
      MediaTrack* master = csurf_utils::Cnv_ChannelIDToMediaTrack(24, s_ch_offset);
      bool on = ( ( ( (CountSelectedTracks(0) > 0) || IsTrackSelected(master) ) && s_myblink ) != IsTrackSelected(master) );
      MySetSurface_UpdateButton(97, on, false);

      // Update automation modes
      MySetSurface_UpdateAutoLEDs();

    } else
    {
      myblink_ctr++;
    }

	  stpHandler->UpdateDisplay(chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_chan, m_flip);


    // check fx count if chan mode
    if (m_chan)
    {
      Utl_CheckFXInsert();
    }

    // init
    if (!s_initdone) 
    {
      s_initdone = MySetSurface_Init();
	  if(stpHandler->ShouldReopen())
		stpHandler->Stp_OpenWindow(chan_fx, chan_par_offs, s_touch_fdr, s_touch_enc, s_ch_offset, chan_rpr_tk, m_chan, m_flip);
    }
  } // Run



}; // class CSurf_US2400



////// REGISTRATION, SETUP AND CONFIGURATION DIALOGS //////

static IReaperControlSurface *createFunc(const char *type_string, const char *configString, int *errStats)
{
  int parms[6];
  csurf_utils::parseParams(configString,parms);
  return new CSurf_US2400(parms[2], parms[3], parms[4], parms[5], errStats);
}


static WDL_DLGRET dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      {
        int parms[6];
		csurf_utils::parseParams((const char *)lParam,parms);

        int n=GetNumMIDIInputs();
        LRESULT x=SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN,CB_ADDSTRING,0,(LPARAM)"None");
        SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN,CB_SETITEMDATA,x,-1);
		if (-1 == parms[2]) SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_IN, CB_SETCURSEL, x, 0);

        x=SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT,CB_ADDSTRING,0,(LPARAM)"None");
        SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT,CB_SETITEMDATA,x,-1);
		if (-1 == parms[3]) SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT, CB_SETCURSEL, x, 0);

        x = SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_DISPLAY, CB_ADDSTRING, 0, (LPARAM)"None");
        SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_DISPLAY, CB_SETITEMDATA, x, -1);
        if (-1 == parms[4]) SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_DISPLAY, CB_SETCURSEL, x, 0);

        for (x = 0; x < n; x ++)
        {
          char buf[512];
          if (GetMIDIInputName(static_cast<int>(x),buf,sizeof(buf)))
          {
            LRESULT a=SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN,CB_ADDSTRING,0,(LPARAM)buf);
            SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN,CB_SETITEMDATA,a,x);
            if (x == parms[2]) SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN,CB_SETCURSEL,a,0);
          }
        }
        n=GetNumMIDIOutputs();
        for (x = 0; x < n; x ++)
        {
          char buf[512];
          if (GetMIDIOutputName(static_cast<int>(x),buf,sizeof(buf)))
          {
            LRESULT a=SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT,CB_ADDSTRING,0,(LPARAM)buf);
            SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT,CB_SETITEMDATA,a,x);
            if (x == parms[3]) SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT,CB_SETCURSEL,a,0);
          }
        }
        n = GetNumMIDIOutputs();
        for (x = 0; x < n; x++)
        {
          char buf[512];
          if (GetMIDIOutputName(static_cast<int>(x), buf, sizeof(buf)))
          {
              LRESULT a =        SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_DISPLAY, CB_ADDSTRING, 0, (LPARAM)buf);
                                 SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_DISPLAY, CB_SETITEMDATA, a, x);
              if (x == parms[4]) SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_DISPLAY, CB_SETCURSEL, a, 0);
          }
        }
		if (parms[5]&csurf_utils::CONFIG_FLAG_METER_MODE)
			CheckDlgButton(hwndDlg, IDC_CHECK_METERMODE, BST_CHECKED);
		if (parms[5]&CONFIG_FLAG_BANK_SWITCH)
			CheckDlgButton(hwndDlg, IDC_CHECK_BANK_SWITCH, BST_CHECKED);
		if (parms[5]&CONFIG_FLAG_NO_FLASH)
			CheckDlgButton(hwndDlg, IDC_CHECK_NO_FLASHING, BST_CHECKED);
	
      }
    break;
    case WM_USER+1024:
      if (wParam > 1 && lParam)
      {
        char tmp[512];

        int offs=0, size=9;
        LRESULT indev=-1, outdev=-1, outdev_disp = -1;
        LRESULT r=SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN,CB_GETCURSEL,0,0);
        if (r != CB_ERR) indev = SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN,CB_GETITEMDATA,r,0);
        r=SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT,CB_GETCURSEL,0,0);
        if (r != CB_ERR)  outdev = SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT,CB_GETITEMDATA,r,0);
        r = SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_DISPLAY, CB_GETCURSEL, 0, 0);
        if (r != CB_ERR)  outdev_disp = SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_DISPLAY, CB_GETITEMDATA, r, 0);

		int cflags = 0;
		if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_METERMODE))
			cflags|=csurf_utils::CONFIG_FLAG_METER_MODE;
		if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_BANK_SWITCH))
			cflags |= CONFIG_FLAG_BANK_SWITCH;
		if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_NO_FLASHING))
			cflags |= CONFIG_FLAG_NO_FLASH;


        sprintf(tmp,"0 0 %d %d %d %d",static_cast<int>(indev),static_cast<int>(outdev), static_cast<int>(outdev_disp),cflags);
        lstrcpyn((char *)lParam, tmp,static_cast<int>(wParam));       
      }
    break;
  }
  return 0;
}


static HWND configFunc(const char *type_string, HWND parent, const char *initConfigString)
{
  return CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SURFACEEDIT_US2400), parent, dlgProc, (LPARAM) initConfigString);
}

reaper_csurf_reg_t csurf_us2400_reg = 
{
  "US-2400",
  DESCSTRING,
  createFunc,
  configFunc,
};