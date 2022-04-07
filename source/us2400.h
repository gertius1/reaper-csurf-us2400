#ifndef _CSURF_2400_
#define _CSURF_2400_ 

#include "csurf.h"
#include "resource.h"

extern void (*CSurf_OnFwd)(int seekplay);
extern void (*CSurf_OnRew)(int seekplay);

// ADDITIONS FOR US-2400
extern void (*ShowConsoleMsg)(const char* msg);
extern double (*GetMediaTrackInfo_Value)(MediaTrack* tr, const char* parmname);
extern bool (*SetMediaTrackInfo_Value)(MediaTrack* tr, const char* parmname, double newvalue);
extern int (*CountTracks)(ReaProject* proj);
extern MediaTrack* (*GetTrack)(ReaProject* proj, int trackidx);
extern bool (*AnyTrackSolo)(ReaProject* proj);
extern MediaTrack* (*SetMixerScroll)(MediaTrack* leftmosttrack);
extern void (*Main_OnCommandEx)(int command, int flag, ReaProject* proj);
extern void (*CSurf_OnZoom)(int xdir, int ydir);
extern void (*CSurf_OnScroll)(int xdir, int ydir);
extern int (*CountSelectedTracks)(ReaProject* proj);
extern bool (*IsTrackSelected)(MediaTrack* track);
extern double (*CSurf_OnSendPanChange)(MediaTrack* trackid, int send_index, double pan, bool relative);
extern double (*CSurf_OnSendVolumeChange)(MediaTrack* trackid, int send_index, double volume, bool relative);
extern void* (*GetSetTrackSendInfo)(MediaTrack* tr, int category, int sendidx, const char* parmname, void* setNewValue);
extern bool (*TrackFX_GetEnabled)(MediaTrack* track, int fx);
extern void (*TrackFX_SetEnabled)(MediaTrack* track, int fx, bool enabled);
extern void (*TrackFX_Show)(MediaTrack* track, int index, int showFlag);
extern ReaProject* (*EnumProjects)(int idx, char* projfn, int projfnlen);
extern void (*GetSet_LoopTimeRange)(bool isSet, bool isLoop, double* start, double* end, bool allowautoseek);
extern int (*EnumProjectMarkers)(int idx, bool* isrgn, double* pos, double* rgnend, char** name, int* markrgnindexnumber);
extern void (*TimeMap_GetTimeSigAtTime)(ReaProject* proj, double time, int* timesig_num, int* timesig_denom, double* tempo);
extern double (*TimeMap2_timeToQN)(ReaProject* proj, double tpos);
extern double (*TimeMap2_QNToTime)(ReaProject* proj, double qn);
extern double (*SnapToGrid)(ReaProject* project, double time_pos);
extern void (*SetEditCurPos2)(ReaProject* proj, double time, bool moveview, bool seekplay);
extern double (*CSurf_OnWidthChange)(MediaTrack* trackid, double width, bool relative);
extern MediaTrack* (*GetSelectedTrack)(ReaProject* proj, int seltrackidx);
extern void (*TrackFX_SetOpen)(MediaTrack* track, int fx, bool open);
extern void (*SetOnlyTrackSelected)(MediaTrack* track);
extern MediaTrack* (*GetMasterTrack)(ReaProject* proj);
extern void (*DeleteTrack)(MediaTrack* tr);
extern const char* (*GetTrackState)(MediaTrack* track, int* flags);
extern int (*NamedCommandLookup)(const char* command_name);
extern int (*GetTrackNumSends)(MediaTrack* tr, int category);
extern bool (*GetTrackSendName)(MediaTrack* track, int send_index, char* buf, int buflen);
extern void (*Main_OnCommand)(int command, int flag);
extern MediaTrack* (*GetLastTouchedTrack)();
extern bool (*GetTrackSendUIVolPan)(MediaTrack* track, int send_index, double* volume, double* pan);
extern bool (*TrackFX_GetParameterStepSizes)(MediaTrack* track, int fx, int param, double* step, double* smallstep, double* largestep, bool* istoggle);
extern const char* (*kbd_getTextFromCmd)(DWORD cmd, KbdSectionInfo* section);
extern bool (*GetSetMediaTrackInfo_String)(MediaTrack* tr, const char* parmname, char* string, bool setnewvalue);
extern const char* (*GetResourcePath)();
extern bool (*TrackFX_GetFormattedParamValue)(MediaTrack* track, int fx, int param, char* buf, int buflen);
extern int (*GetTrackColor)(MediaTrack* track);
extern bool (*HasExtState)(const char* section, const char* key);
extern const char* (*GetExtState)(const char* section, const char* key);
extern void (*SetExtState)(const char* section, const char* key, const char* value, bool persist);
extern void (*Undo_BeginBlock)();
extern void (*Undo_EndBlock)(const char* descchange, int extraflags);
extern double (*Track_GetPeakHoldDB)(MediaTrack* track, int channel, bool clear);
extern char* (*GetSetObjectState)(void* obj, const char* str);
extern void (*FreeHeapPtr)(void* ptr);
extern void (*TrackList_AdjustWindows)(bool isMajor);
// ADDITIONS FOR US-2400 -- END
#endif