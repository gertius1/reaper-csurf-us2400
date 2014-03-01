# Tascam US-2400 Control Surface Extension for Cockos Reaper

###### Contents
* [Disclaimer](#disclaimer)
* [Needs](#needs)
* [Install](#install)
* [Uninstall](#uninstall)
* [Features](#features)
	* [Hardcoded Functions](#hardcoded-functions)
	* [Aux Mode](#aux-mode)
	* [Channel Strip Mode](#channel-strip-mode)
	* [Custom Actions](#custom-actions)
	* [M-Key or Meter Mode](#m-key-or-meter-mode)
	* [Scribble Strip Window](scrobble-strip-window)
	* [On-Screen-Help](#on-screen-help)
* [Notes](#notes)

## Disclaimer

1.   This is a working beta. Install and use at your own risk. It’s more or less the second half-serious thing I did in C++, I’m sure it could be done much better - any takers? 

2.   Although there is a version for 64 bit Windows now, I have no way to test it, so you’ll have to do that yourself. (As of now it works ok apart from some crashes – working on it, and thanks for testing, Nardberr and mim!)

## Needs

*   Cockos Reaper min. v4.57 (get it [here](http://www.reaper.fm/download.php "Download Reaper"))

*   SWS Extension min. v2.4.0 (get it [here](http://www.standingwaterstudios.com/ "Standing Water Studios"))

*   To run ReaScripts, Python has to be installed on your machine, read more about it [here](http://www.reaper.fm/sdk/reascript/reascript.php "Reaper: ReaScript info"). Choose and download the appropriate version for your machine [here](http://www.python.org/getit/ "Download Python")

*   The US-2400 has to be in Native Mode, which AFAIK is a novelty of firmware 1.31, so you might have to update it. To activate Native Mode put the US-2400 to standby (so that the Power button ‘breathes’), then keep Master Sel + Chan pushed while turning it back on – if you have the right firmware and Native Mode is active Chan should blink three times. (To put it back in the MCU emulation mode do the same with Master Sel + Aux 5)

## Install

*   If you’ve never heard of GitHub – the ‘Download ZIP’-Button should be somewhere on the right, use it to download the files, extract them and proceed as follows:

*   reaper_csurf_us2400.dll goes into Programs\Reaper\Plugins. Use the 64 bit version for Windows 7 and the like, and the 32 bit for XP. 

	If you want to use the Meter Mode (encoder rings as VU Meters) choose the file that ends with ‘_metermode’, if you want to use the Meter-Button as an additional qualifier key (like Shift or F-Key) instead choose the file with ‘_mkey’ in the name. Read more about it below, under ‘M-Key or Meter Mode’

*   I recommend putting the ReaScript (.py) files for custom actions (see below) in Reaper’s Scripts directory: Documents and Settings\YourUsername\Application Data\Reaper\Scripts but you can put them anywhere.

* 	The ‘Split’ actions require some custom Jesusonic-Plugins (mainly for routing stuff), those are in the according folder (Custom Actions/Splits), they go here: Documents and Settings\YourUsername\Application Data\Reaper\Effects.

*   Now comes the tedious part: You have to load all scripts into the Action List – I don’t know if there’s a way to bulk import them – if you do: good for you, and: tell me! *I found a nice hack for this: hitting the Load button in the Action List several times opens a stack of Load File dialogs, all on top of each other. That makes it a bit quicker (at least for me).*

*   Only after you have done this, open Reaper Preferences / Control Surfaces and select ‘Tascam US-2400’ with its first MIDI-Port (it shows five on my PC) – otherwise the actions will only become available after you restart Reaper.

*   You can throw away the ‘Source’ folder (it’s the c++ code) if you just want to run the extension. I would prefer it of course, if - after a short test drive - you would identify the flaws, put on your hacker hat, fork the extension and fix them.

## Uninstall

*   Delete reaper_csurf_us2400.dll

*   If you want, remove the scripts/JS plug-ins and uninstall Python, but you don’t have to.

That’s about it.

## Features

### Hardcoded Functions

#### Rotary Encoders

##### Default / Pan Mode

* **Default:** Pan
* **Flip:** Volume
* **F-Key:** Width
* **Shift:** Pan > C

##### Aux Mode

* **Default:** Send volume
* **Flip:** Send pan!
* **F-Key:** Send Volume > -inf dB
* **Shift:** Send Volume > 0 dB

##### Chan Mode

* **Default:** FX parameter
* **Flip:** Volume
* **F-Key:** FX parameter: fine 
* **Shift:** FX parameter: toggle / coarse – *switches between 0 and 1 (on and off) this works 90% of the time. For 5 way switches and the like you’ll have to flip and use faders, sorry.* 

	*There are rumors about plug ins that report optimal parameter step sizes to Reaper. If you’re lucky enough to happen upon one of those, the standard step size should be automatically set to that value. If applicable, toggle should be automatically set, too. Shift should activate a coarser step size (10 times the default step size) in this case. I’m saying ‘should’ because I haven’t found a plug in to test this with yet. If you found one that’s freeware, tell me.*

#### Faders

##### Default / Pan Mode

* **Default:** Volume
* **Flip:** Pan
* **F-Key:** Volume > -inf dB
* **Shift:** Volume > 0 dB

##### Aux Mode

* **Flip:** Send volume

##### Chan Mode

* **Flip:** FX Parameter – *if it seems you just can’t manage to turn a switch with the rotaries (unfortunately it happens, see above), you can flip and use the fader as a workaround. Sucks, I know.*

#### Sel Buttons – Tracks

##### Default / Pan Mode

* **Default:** Select
* **F-Key:** Switch Phase – *indicated by the light below the encoders*
* **Shift:** Rec Arm – *indicated by the light below the encoders blinking*

##### Aux Mode
* **F-Key:** Add Aux Send to track
* **Shift:** Remove Aux Send from track

*Attention users of previous versions, this is new in this version – I find it to be more intuitive. Also: more room for custom actions*

##### Chan Mode

* **Default:** Select track for channel strip

#### Sel Button – Master

##### Default / Pan Mode

* **Default:** Deselect / Select all tracks
* **F-Key:** Select master

##### Chan Mode

* **Default:** Select master for channel strip

#### Solo Button

* **Default:** Solo track
* **F-Key:** Solo only this track (un-solo all others)

#### Mute-Button

* **Default:** Mute track
* **F-Key:** Mute only this track (un-mute all others)
* **Shift:** Bypass all FX of this track – *trust me, you want this outside of Chan Mode*

#### Clear Solo (Master)

* **Default:** Un-solo all tracks
* **F-Key:** Un-mute all tracks

#### Flip

* Flip

#### Chan

* Enter / exit channel strip (Chan Mode)

#### Pan

* Enter default mode (Pan Mode)

#### Aux-Block (1 – 6)

##### Default / Pan Mode

* **Default / 1 – 6:** Enter Aux Mode 1 – 6 (adjust sends, etc.)

##### Chan Mode

* **Default / 1:** Bank shift FX parameters left (steps of 24)
* **Default / 2:** Bank shift FX parameters right (steps of 24)
* **Default / 3:** Bypass current FX
* **Default / 4:** Insert FX (also opens and closes FX Browser)
* **Default / 5:** Delete current FX
* **Default / 6:** Switch between recording FX automation (plugin parameters) and track automation (volume, pan, etc.)

##### Aux Mode

* **Default / 1 – 6:** Enter Aux Mode 1 – 6

#### Null Button (below F-Key)

*Can be assigned to custom actions*

#### Jog Wheel / Scrub Button

* **Scrub Button** activates Scrub, otherwise the Jogwheel just moves the cursor silently
* **F-Key** enables faster adjustments

#### Joystick

Signals from the Joystick are not assigned to any Control Surface functions. Instead, the MIDI signals get transmitted on the second US-2400 MIDI input (CH 15, Y: CC 90 / X: CC 91), so you can use those for whatever you like.

#### Bank Switches (below Scrub)

##### Default (Pan Mode) and Aux Mode:

* **Default:** Bank shift left / right (steps of 8)
* **Shift:** Bank shift left / right (steps of 24)
* **F-Key:** Move left edge of time selection left / right by 1 bar 

##### Chan Mode

* **Default:** Previous / next FX in chain
* **Shift:** Bank shift left / right (steps of 24)
* **F-Key:** Move current FX up / down in chain

#### In/Out Buttons (to the right of Bank Switches)

* **Default:** Time-select previous / next region (between the previous / next set of markers)
* **Shift – In:** Toggle time-selection between whole project and current range (or nothing)
* **Shift – Out:** Toggle loop time-selection
* **F-Key:** Move right edge of time selection left / right by 1 bar 

#### Transport

*Do what you expect them to do :-)*

### Aux Mode

The extension identifies any track, anywhere in your tracklist, that has ‘aux---1’ to ‘aux---6’ in its name as the respective Aux. You can call them anything – as long as ‘aux---X’ is in there somewhere it should work. Also, the search is not case sensitive so you can use ‘AUX’ or ‘Aux’ if you want. 

It has to be three dashes to avoid confusion with tracknames ending in ‘aux’, though. For example, I don’t speak French, but I think some words end with ‘...aux’, so maybe there are tracknames like ‘plateaux 2’ or whatever ... well, you can use those and mark aux-busses with three dashes, no space.

The US-2400 supports up to 6 aux busses, you can access the settings to each one entering aux mode 1 to 6 by pressing Aux buttons 1 to 6. In Aux Mode you can add and remove sends by pressing FKey/Shift and Select of the according track, and adjust the levels with the rotary encoder. Using flip in Aux Mode gives you Aux Levels on the faders and Aux Pans on the encoders!

### Channel Strip Mode

When entering Channel Strip Mode (Chan Mode for short) you can select the FX chain of one track using the Track Select buttons – your existing track selection stays active, a blinking Track Select button indicates the selected FX chain. 

In this mode the rotary encoders are used for FX parameters. The function auf Aux and Bank buttons changes, so you can step through your parameters and through you FX chain, insert new FX, delete FX and move them up and down the chain using the Aux buttons in this mode (see details below).

Due to the way many VST plugins are coded, the parameters will not appear in the same order on the US-2400 as they do in the Graphical User Interface of the plug-in – nothing I can do about that.

You can use Reapers option to switch to a basic UI (the UI button on the top left of the FX window) – the parameter order in this view will match the one of the US-2400 (thanks, Nardberr!).

Or you can use the **Scribble Strip Window** (see below).

### Custom Actions

Many of the buttons are connected with a trigger that just fires ReaScripts. The scripts get assigned to the according buttons on program start by parts of their filenames. You can give your ReaScript any name you like, I recommend a short description of what it does, because the filename is the text that shows up in Reaper’s action dialog and in the US-2400 On-Screen Help (see below).

**Important:** The filename has to contain a kind of signature at the end that the csurf plugin uses to identify and assign the action – it looks like this:

‘(US-2400 - Chan - FKey - 3)’

... which is pretty self-explanatory I guess, but here are the details nonetheless:

*	**‘US-2400’** – this identifies it as an action the CSurf plug-in is supposed to load.
*	**‘Pan’ / ‘Chan’ / ‘Aux’** – assigns an action to a certain mode (i.e. the action is only available in that mode). You can leave this bit out, then the action will be triggered regardless of current mode.
* 	**‘NoKey’ / ‘Shift’ / ‘FKey’ / ‘MKey’** – the qualifier key this action is assigned to, obviously. You can leave this out, too, for the action to be assigned to all qualifier combinations. Use ‘NoKey’ if you want the action assigned to the button without qualifier exclusively. ‘MKey’ is only available if you haven’t installed the meter mode version (see above) of course.
*	Lastly, the button itself – available are the 6 Aux Buttons (**‘1’** to **‘6’**), the **‘Null’** button, and the transport section (**‘Rew’, ‘FFwd’, ‘Stop’, ‘Play’, ‘Rec’**).

**The right form is important for assignment to work:** Separate the above bits by space, dash, space (‘ - ’) and the identification is case sensitive (e.g. ‘Ffwd’ won’t work).

**The narrower assignment overwrites the wider one.** – Example: Let’s say you have an action ‘(US-2400 - 3)’. This should be triggered by the 3rd Aux button, with every qualifier in every mode. But if you also have an action ‘(US-2400 - FKey - 3)’ this action overrides the other action (only for FKey/3 of course). ‘(US-2400 - Chan - FKey - 3’ would overide both previous ones.

**All buttons except ‘Null’ have hardcoded actions, when no qualifier is pressed:** Aux 1 to 6 with no qualifier enters Aux Mode, transport does the obvious things, but you can override those by loading an action using the according signature (e.g. override Play with ‘(US-2400 - Play’)!

#### Action sets in the install package

##### Automation

This set includes:

* Remove envelopes from selected tracks (FKey - 6)
* Show or hide envelopes (FKey - 5)

##### General

This set includes:

* Undo (FKey - Rew)
* Redo (FKey - FFwd)
* Save (FKey - Play)
* Open Render Dialog (FKey - Rec)

##### Time / Zoom / Scroll

This set includes:
* Toggle scrolling with play position (FKey - Stop)
* Scroll to play position – not edit cursor! (FKey - Null)
* Zoom time selection (NoKey - Null)
* Zoom track selection (Shift - Null) – *this seems to be buggy, working on it!*
* Jump to next point of interest – end of loop, marker, etc. (NoKey - FFwd)
* Jump to previous point of interest – beginning of loop, marker, etc. (NoKey - Rew) - *These last two actions Overrride the hardcoded FFwd / Rew. I find them to be much more useful, I use the scrub wheel to get around if I ever have to, mostly I’m just jumping from point to point. But you can of course put them somewhere else by renaming the file, or simply not install them.

##### Track actions

This set includes:
* Move selected tracks left (FKey - 1)
* Move selected tracks right (FKey - 2)
* Duplicate selected tracks (FKey - 3)
* Rename selected tracks – uses SWS’ very handy dialog (FKey - 4)
* Insert empty track before first selected track (Shift - 4)
* Delete selected tracks (Shift - 5)
* Wrap selected tracks in folders / remove selected folder, unwrap containing tracks first (Shift - 1)
* Open or close selected folders – in mixer view (Shift - 2)
* Group selected tracks (Shift - 3)
* Ungroup selected tracks (Shift - 6) – *For the SWS Grouping / Ungrouping actions to work you have to set / save the default flags once in every project: Shift + G (on your keyboard, not the US-2400) opens the Grouping Settings, tweak yours and hit Save.

##### FX Favorites

Edit these files to load your favorite plugins: Open the .py file in a text editor and insert the name of the plug in you want to load with that action in line 4 behind ‘fx_name = ’, in quotes (e.g. ‘fx_name = "ReaComp"’).

Templates are included for actions triggered by every Aux Button with all qualifiers in Chan Mode (Chan - FKey/MKey/Shift - 1/2/3/4/5/6).

##### Splits

These actions are not really US-2400 related, but I found them so useful, I wanted to have them on the US-2400 – and since I made it, why should I exclude it? You can always play around with them and just throw the scripts out if you don’t need this stuff.

Basically they split the signal of a single track into several sub tracks (like an mono/stereo pair for example). Those get send to a folder that automatically adjusts the sum volume of the sub tracks to stay at 0 dB, regardless what you do with the volumes of the sub tracks.

Included flavors:
* Split first selected track LR – left/right (MKey - 1) – *Much quicker than duplicating and setting item props! You can always work with all mono files like the oldschool of course, then this action makes no sense to you at all, probably ...*
* Split first selected track MS – mono/stereo (MKey - 2)
* Split first selected track LFHF – frequency crossover lo/hi freq (MKey - 3) – *Set frequencies in the ‘(LH-Split)’ plug-in that gets inserted automatically in the first sub track.*
* Split first selected track LMH – frequency crossover lo/mid/hi freq (MKey - 4) – *Set frequencies in the ‘(LMH-Split)’ plug-in that gets inserted automatically in the first sub track.*
* Split first selected track LMMF – 4-way frequency crossover, lo/lo mids/hi mids/hi freq (MKey - 5) – *Set frequencies in the ‘(LMMH-Split)’ plug-in that gets inserted automatically in the first sub track.*
* Add parallel processor (MKey - 6) – *Go wild with NY compression! The automatic volume compensation in the folder track makes sure you judge the sound and not the volume*
* Unsplit first selected split track (MKey - Null) – *Removes a split and returns to the single track you had before. Any FX you inserted in the sub tracks will be lost!*

**Important:** I didn’t have time yet to fully test drive those actions, so use with caution. I think you can’t use them on folders as of now. Nesting splits should work though (like using an LMH on the stereo signal of an MS split).

### M-Key or Meter Mode

I finally found out how to adress the Meter Mode of the US-2400, you can employ it by installing the .dll with ‘_meter-mode’ in the filename and then hitting the Meter Key on the US-2400.

But I admit that I find this a quite useless feature (15 lights for a VU signal, that’s more like a consumer tape deck ...) – I could use an additional qualifier key instead (more custom actions, yay!) so I made a version for that as well: If you use the .dll with ‘_m-key’ in the filename hitting Meter makes another command set available (like Shift or F-Key). 

Unfortunately this button doesn’t send an signal for releasing the button (unlike every other button, weirdly), which poses a problem in using it as a qualifier: If you can’t check for a button-up you have now way of knowing if it’s still pressed when another button goes down (which is of course how qualifier keys work). 

The workaround is as follows: when you press the M-Key it stays activated for a short time (half a second or so). The M-Key and all buttons with assigned M actions flash rapidly during that time. This should not make a real difference, just so you know.

### Scribble Strip Window

Hitting **Shift and F-Key** together (in that order) opens a resizable **Scribble Strip Window**, showing track names / FX parameter names dynamically correlated with the corresponding fader / encoder row (column?) – it even indicates touch states, selected / rec armed tracks and parameter values. 

Still not better than if the US-2400 had a display, but it sure helps.

Images:
[Track View (Pan and Aux Modes)](https://raw2.github.com/DavidLichtenberger/reaper-csurf-us2400/master/Tascam-US-2400-Reaper-ScribbleStripTrack.png "Scribble Strip – Track View"), 
[Channel Strip View (Chan Mode)](https://raw2.github.com/DavidLichtenberger/reaper-csurf-us2400/master/Tascam-US-2400-Reaper-ScribbleStripChan.png "Scribble Strip – Channel Strip View")

### On-Screen-Help

In previous versions I included a sheet you could print out and stick on the Master section but now, with a third qualifier key (M-Key) the custom actions got a bit much – I was running out of room. 

So I made an On-Screen Help Window that shows all button functions (hardcoced ones as well as custom actions that were loaded). You can access it by hitting **F-Key and Shift** together and in that order (reversed order brings up Channel Strip).

Images:
[Pan Mode – no qualifier](https://raw2.github.com/DavidLichtenberger/reaper-csurf-us2400/master/Tascam-US-2400-Reaper-OnScreenHelpPan.png "On Screen Help – Pan Mode, no qualifier"), 
[Chan Mode – no qualifier](https://raw2.github.com/DavidLichtenberger/reaper-csurf-us2400/master/Tascam-US-2400-Reaper-OnScreenHelpChan.png "On Screen Help – Chan Mode, no qualifier"), 
[Chan Mode – F-Key pressed](https://raw2.github.com/DavidLichtenberger/reaper-csurf-us2400/master/Tascam-US-2400-Reaper-OnScreenHelpChan.png "On Screen Help – Chan Mode, F-Key pressed")

## Notes

*   There is a thread about this extension [here](http://forum.cockos.com/showthread.php?t=132165 "Cockos Reaper Forums").

*   When I have some time, I’ll update the ‘Issues’ section with known bugs and stuff. As of now it’s nothing major.
