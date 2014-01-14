# Tascam US-2400 Control Surface Extension for Cockos Reaper

## Disclaimer

1.   This is a working beta. Install and use at your own risk. It’s more or less the second half-serious thing I did in C++, I’m sure it could be done much better - any takers? 

2.   Although there is a version for 64 bit Windows now, I have no way to test it, so you’ll have to do that yourself. (As of now it works ok apart from some crashes – working on it, and thanks for testing, Nardberr!)

## Needs

*   Cockos Reaper min. v4.57 (get it [here](http://www.reaper.fm/download.php "Download Reaper"))

*   SWS Extension min. v2.4.0 (get it [here](http://www.standingwaterstudios.com/ "Standing Water Studios"))

*   To run ReaScripts, Python has to be installed on your machine, read more about it [here](http://www.reaper.fm/sdk/reascript/reascript.php "Reaper: ReaScript info"). Choose and download the appropriate version for your machine [here](http://www.python.org/getit/ "Download Python")

*   The US-2400 has to be in Native Mode, which AFAIK is a novelty of firmware 1.31, so you might have to update it. To activate Native Mode put the US-2400 to standby (so that the Power button ‘breathes’), then keep Master Sel + Chan pushed while turning it back on – if you have the right firmware and Native Mode is active Chan should blink three times. (To put it back in the MCU emulation mode do the same with Master Sel + Aux 5)

## Install

*   If you’ve never heard of GitHub – the ‘Download ZIP’-Button should be somewhere on the right, use it to download the files, extract them and proceed as follows:

*   reaper_csurf_us2400.dll goes into Programs\REAPER\Plugins – Use the 64 bit version for Windows 7 and the like, and the 32 bit for XP.

*   All scripts go into Reaper’s Scripts directory: Documents and Settings\YourUsername\Application Data\REAPER\Scripts".

*   Now comes the tedious part: You have to load all scripts into the Action List – I don’t know if there’s a way to bulk import them – if you do: good for you, and: tell me! *I found a nice hack for this: hitting the Load button in the Action List several times opens a stack of Load File dialogs, all on top of each other. That makes it a bit quicker (at least for me).*

    For the default function set, don't import the ones in the ‘Unassigned’-folder (you can use those if you wish to override the default actions). See below for implementing custom actions.

*   Only after you have done this, open Reaper Preferences / Control Surfaces and select ‘Tascam US-2400’ with its first MIDI-Port (it shows five on my PC) – otherwise the actions will only become available after you restart Reaper.

## Uninstall

*   Delete reaper_csurf_us2400.dll

*   If you want, remove the scripts and uninstall Python, but you don't have to.

That’s about it.

## Functions

### Channel Strip Mode

Due to the way many VST plugins are coded, the parameters will not appear in the same order on the US-2400 as they do in the Graphical User Interface of the plug-in – nothing I can do about that.

You can use Reapers option to switch to a **Basic UI** (the UI button on the top left of the FX window) – the parameter order in this view will match the one of the US-2400 (thanks, Nardberr!).

Or you can use the **Scribble Strip Window**.

### Scribble Strip Window

Hitting **F-Key and Shift** together opens a resizable **Scribble Strip Window**, showing track names / FX parameter names dynamically correlated with the corresponding fader / encoder row (column?) – it even indicates touch states, selected / rec armed tracks and parameter values. 

Still not better than if the US-2400 had a display, but it sure helps:

Images: [Track View (Pan and Aux Modes)](https://raw2.github.com/DavidLichtenberger/reaper-csurf-us2400/master/Tascam-US-2400-Reaper-ScribbleStripTrack.png "Scribble Strip – Track View"), 
[Channel Strip View (Chan Mode)](https://raw2.github.com/DavidLichtenberger/reaper-csurf-us2400/master/Tascam-US-2400-Reaper-ScribbleStripChan.png "Scribble Strip – Channel Strip View")

### Rotary Encoders

#### Default / Pan Mode

* **Default:** Pan
* **Flip:** Volume
* **F-Key:** Width
* **Shift:** Pan > C

#### Aux Mode

* **Default:** Send volume
* **Flip:** Send pan!
* **F-Key:** Send Volume > -inf dB
* **Shift:** Send Volume > 0 dB

#### Chan Mode

* **Default:** FX parameter
* **Flip:** Volume
* **F-Key:** FX parameter: fine 
* **Shift:** FX parameter: toggle – *switches between 0 and 1 (on and off) this works 90% of the time. For 5 way switches and the like you’ll have to flip and use faders, sorry.*

### Faders

#### Default / Pan Mode

* **Default:** Volume
* **Flip:** Pan
* **F-Key:** Volume > -inf dB
* **Shift:** Volume > 0 dB

#### Aux Mode

* **Flip:** Send volume

#### Chan Mode

* **Flip:** FX Parameter – *if it seems you just can’t manage to turn a switch with the rotaries (unfortunately it happens, see above), you can flip and use the fader as a workaround. Sucks, I know.*

### Sel Buttons – Tracks

#### Default / Pan Mode

* **Default:** Select
* **F-Key:** Switch Phase – *indicated by the diode below the indicators*
* **Shift:** Rec Arm – *indicated by the diode below the indicators blinking*

#### Chan Mode

* **Default:** Select track for channel strip

### Sel Button – Master

#### Default / Pan Mode

* **Default:** Select all tracks / none
* **F-Key:** Select master

#### Chan Mode

* **Default:** Select master for channel strip

### Solo Button

* **Default:** Solo track
* **F-Key:** Solo only this track (un-solo all others)

### Mute-Button

* **Default:** Mute track
* **F-Key:** Mute only this track (un-mute all others)
* **Shift:** Bypass all FX of this track – *trust me, you want this outside of Chan Mode*

### Clear Solo (Master)

* **Default:** Un-solo all tracks
* **F-Key:** Un-mute all tracks

### Flip

* Flip

### Chan

* Enter / exit channel strip (Chan Mode)

### Pan

* Enter default mode (Pan Mode)

### Aux-Block (1 – 6)

#### Default / Pan Mode

* **Default / 1 – 6:** Enter Aux Mode 1 – 6 (adjust sends, etc.)
* **F-Key / 1:** Move selected tracks left
* **F-Key / 2:** Move selected tracks right
* **F-Key / 3:** Duplicate selected tracks
* **F-Key / 4:** Name selected tracks
* **F-Key / 5:** Show / hide envelopes on selected tracks
* **F-Key / 6:** Delete envelopes on selected tracks
* **Shift / 1:** Wrap selected tracks in folder / unwrap selected folders
* **Shift / 2:** Show / hide children of selected folders
* **Shift / 3:** Group selected tracks
* **Shift / 4:** Insert track (if tracks selected, insert before first selected)
* **Shift / 5:** Delete selected tracks
* **Shift / 6:** Remove selected tracks from any group

*The actions on F-Key and Shift can be customized by altering the according ReaScripts.*

#### Chan Mode

* **Default / 1:** Bank shift FX parameters left (steps of 24)
* **Default / 2:** Bank shift FX parameters right (steps of 24)
* **Default / 3:** Bypass current FX
* **Default / 4:** Insert FX (also opens and closes FX Browser)
* **Default / 5:** Delete current FX
* **Default / 6:** Switch between recording FX automation (plugin parameters) and track automation (volume, pan, etc.)

*The F-Key and Shift actions here are the same as in Pan Mode, but you can make your own additions by altering the according ReaScripts.*

#### Aux Mode

* **Default / 1 – 6:** Enter Aux Mode 1 – 6
* **F-Key / 1 – 6:** Add aux send 1 – 6 to selected tracks
* **Shift / 1 – 6:** Remove aux send 1 – 6 from selected tracks

*The actions on F-Key and Shift can be customized by altering the according ReaScripts.*

### Null Button (below F-Key)

* **Default:** Zoom to fit time selection
* **F-Key:** Scroll to play position
* **Shift:** Zoom to fit track selection

*All actions can be customized by altering the according ReaScripts.*

### Jog Wheel / Scrub Button

* **Scrub Button** activates Scrub, otherwise the Jogwheel just moves the cursor silently
* **F-Key** enables faster adjustments

### Joystick

Signals from the Joystick are not assigned to any Control Surface functions. Instead, the MIDI signals get transmitted on the second US-2400 MIDI input (CH 15, Y: CC 90 / X: CC 91), so you can use those for whatever you like.

### Bank Switches (below Scrub)

#### Default (Pan Mode) and Aux Mode:

* **Default:** Bank shift left / right (steps of 8)
* **Shift:** Bank shift left / right (steps of 24)
* **F-Key:** Move left edge of time selection left / right by 1 bar 

#### Chan Mode

* **Default:** Previous / next FX in chain
* **Shift:** Bank shift left / right (steps of 24)
* **F-Key:** Move current FX up / down in chain

### In/Out Buttons (to the right of Bank Switches)

* **Default:** Time-select previous / next region (between the previous / next set of markers)
* **Shift – In:** Toggle time-selection between whole project and current range (or nothing)
* **Shift – Out:** Toggle loop time-selection
* **F-Key:** Move right edge of time selection left / right by 1 bar 

### Transport

* **Default – Rew / FFwd:** Move play cursor to next marker or other point of interest (use Jog Wheel for other adjustments) – *Remove the ReaScripts to enable default actions* 
* **Default – Stop / Play / Record:** Stop, Play, Record
* **Shift – Rew / FFwd / Stop / Play:** Set automation mode (Off / Trim, Read, Latch, Write) for selected tracks. The diodes above blink accordingly, if there are tracks with different modes, more than one will blink. 
* **Shift – Rec:** Write current automation values of envelopes in Write / Latch Mode to the complete time selection. 
* **F-Key – Rew:** Undo
* **F-Key – FFwd:** Redo
* **F-Key – Stop:** Scroll with playback
* **F-Key – Play:** Save project with dialog (Save as)
* **F-Key – Rec:** Save project (Overwrite)

All transport actions can be customized by altering, adding or removing the according ReaScripts. 

The Default and Shift actions of all buttons have hardcoded actions that get overriden if there are ReaScripts (e.g. those in the ‘Unassigned’ folder) or assigned when the ReaScripts are removed.

## Notes

*   For an overview of the functions you can also refer to the [Keymap](https://raw2.github.com/DavidLichtenberger/reaper-csurf-us2400/master/Tascam-US-2400-Reaper-Keymap.gif "Keymap – GIF") (and you can print that and stick it on your master section). 

*   You can now make your own Reascript custom actions for the Aux Block: Keys 1 to 6 with 2 qualifier keys (Shift and F-Key) in 3 Modes (Pan, Chan, Aux) – that makes 36 actions at your disposal! 

    To do that, simply edit the corresponding scripts – you may have already noticed, their names refer to the mode / qualifier / key combination that triggers them (e.g. ‘US-2400 - Aux-Mode - FKey - 6.py’). The extension is looking for those names, so **don’t rename the files, just edit the contents**!

*   The extension identifies any track, anywhere in your tracklist, that has ‘aux---1’ to ‘aux---6’ in its name as the respective Aux. You can call them anything – as long as ‘aux---X’ is in there somewhere it should work. Also, the search is not case sensitive so you can use ‘AUX’ or ‘Aux’ if you want. 

    It has to be three dashes to avoid confusion with tracknames ending in aux, though. For example, I don’t speak French, but I think some words end with ‘...aux’, so maybe there are tracknames like ‘plateaux 2’ or whatever ... well, you can use those and mark aux-busses with three dashes, no space.

*   For the SWS Grouping / Ungrouping actions to work you have to set / save the default flags once in every project (Shift+G opens the Grouping Settings, tweak yours and hit Save)

*   You can throw away the ‘Source’ folder (it’s the c++ code) if you just want to run the extension. I would prefer it of course, if - after a short test drive - you would identify the flaws, put on your hacker hat, fork the extension and fix them.

*   There is a thread about this extension [here](http://forum.cockos.com/showthread.php?t=132165 "Cockos Reaper Forums").

## Things I (or Maybe You?) Might Do At A Later Date:

*   There is a meter mode (where the encoder rings emulate VU meters) that works when you use the US-2400 as a fake MCU, but I didn’t really try to implement it in this extension ... I must admit, I don’t really see the use – after all it’s only 15 diodes per encoder?

*   <del>As of now, to avoid swamping the US-2400 faders/encoders with MIDI updates (especially in auto read mode), the extension doesn’t use the volume/pan ‘events’ of the API (there are none existing for FX param updates anyway, AFAIK). Instead it queries values and then updates faders/encoders in the central event loop (‘run’ function). This fixes the problem but also slows down response. Maybe this is just a temporary fix?</del>

    √ *This is now as good as it can be. It still uses the run loop but has a cleverer way (and a cache) to find out which faders must be updated, so as many updates as possible find their way to the faders. I tested it against the fake MCU setup and didn’t find it to be much slower anymore.*

    *BTW, the new version also compensates for higher CSurf update rates (it’s in the Preferences, default 15 Hz), so you can tweak that without fearing it might fall apart. Although I played around with it and didn’t find it to make a big difference – even in MCU mode, which (theoretically, technically) should have a higher limit, because it employs three MIDI connections instead of one. I guess the motors of the faders have their limit, too.*

*   <del>Maybe there is a way to make the US-2400 CSurf a module of it’s own so one doesn’t have to overwrite the reaper_csurf.dll?</del>

    √ *Turns out this is even easier done than said ;-)*

*   <del>It would be cool to be able to assign custom actions to the ‘action buttons’ (like Shift-/FKey-Aux) but as of now I don’t really know how to do it.</del>

    √ *Done.*   
