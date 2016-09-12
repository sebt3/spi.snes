// Snes9x includes
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h>
#include "snes9x.h"
//#include "port.h"
#include "memmap.h"
#include "dma.h"
#include "apu/apu.h"
#include "fxinst.h"
#include "fxemu.h"
#include "sdd1.h"
#include "srtc.h"
#include "snapshot.h"
#include "display.h"
#include "controls.h"
#include "conffile.h"
#include "apu/apu.h"
#include "memmap.h"
#include "scenes.h"
// Interface include
#include "rgroup.h"

#include <alsa/asoundlib.h>


void S9xSyncSpeedFinish (void);

menu_Texture *render;
SDL_AudioSpec *audiospec;
Eina_Bool rom_loaded;

// Emu interface //////////////////////////////////////////////////////////////////////
static 	int bufSize = 256;

#ifdef USE_SDL_AUDIO
static void sdl_audio_callback (void *userdata, Uint8 *stream, int len) {
	SDL_LockAudio ();
	//printf("%d - %d\n", len, S9xGetSampleCount());
	S9xMixSamples (stream, len >> 1);
	SDL_UnlockAudio ();
}

static void samples_available (void *data) {
	SDL_LockAudio ();
	S9xFinalizeSamples ();
	SDL_UnlockAudio ();
}
#else

static snd_pcm_t *pcm;
static int sound_buffer_size;
static uint8 *sound_buffer;

static void samples_available (void *data) {
    snd_pcm_sframes_t frames_written, frames;
    int bytes;

    S9xFinalizeSamples ();

    frames = snd_pcm_avail_update (pcm);
    if (frames < 0)
    {
        frames = snd_pcm_recover (pcm, frames, 1);
    }

    frames = MIN (frames, S9xGetSampleCount () >> (Settings.Stereo ? 1 : 0));

    bytes = snd_pcm_frames_to_bytes (pcm, frames);
    if (bytes <= 0)
    {
        return;
    }

    if (sound_buffer_size < bytes || sound_buffer == NULL)
    {
        sound_buffer = (uint8 *) realloc (sound_buffer, bytes);
        sound_buffer_size = bytes;
    }

    S9xMixSamples (sound_buffer, frames << (Settings.Stereo ? 1 : 0));

    frames_written = 0;

    while (frames_written < frames)
    {
        int result;

        result = snd_pcm_writei (pcm,
                                 sound_buffer +
                                  snd_pcm_frames_to_bytes (pcm, frames_written),
                                 frames - frames_written);

        if (result < 0)
        {
            result = snd_pcm_recover (pcm, result, 1);

            if (result < 0)
            {
                break;
            }
        }
        else
        {
            frames_written += result;
        }
    }

    return;
}

void EMU_startAudio() {
    int err;
    snd_pcm_sw_params_t *sw_params;
    snd_pcm_uframes_t alsa_buffer_size, alsa_period_size;

    if ((err = snd_pcm_open (&pcm,
                             "default",
                             SND_PCM_STREAM_PLAYBACK,
                             SND_PCM_NONBLOCK) < 0))
        goto fail;

    if ((err = snd_pcm_set_params (pcm,
                                   Settings.SixteenBitSound ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_U8,
                                   SND_PCM_ACCESS_RW_INTERLEAVED,
                                   Settings.Stereo ? 2 : 1,
                                   Settings.SoundPlaybackRate,
                                   1 /* Allow software resampling */,
                                   bufSize * 1000))
         < 0)
    {
        goto close_fail;
    }

    snd_pcm_sw_params_alloca (&sw_params);
    snd_pcm_sw_params_current (pcm, sw_params);
    snd_pcm_get_params (pcm, &alsa_buffer_size, &alsa_period_size);
    /* Don't start until we're [nearly] full */
    snd_pcm_sw_params_set_start_threshold (pcm,
                                           sw_params,
                                           (alsa_buffer_size / alsa_period_size) * alsa_period_size);
    /* Transfer in blocks of period-size */
    snd_pcm_sw_params_set_avail_min (pcm, sw_params, alsa_period_size);
    err = snd_pcm_sw_params (pcm, sw_params);
    if (err < 0)
        goto close_fail;

    return;

close_fail:
    snd_pcm_drain (pcm);
    snd_pcm_close (pcm);
    pcm = NULL;

fail:

    return;
}
#endif

static void	EMU_update_conf() {
	ZeroMemory(&Settings, sizeof(Settings));
	S9xLoadConfigFiles (NULL, 0);
	Settings.MouseMaster = TRUE;
	Settings.SuperScopeMaster = TRUE;
	Settings.JustifierMaster = TRUE;
	Settings.MultiPlayer5Master = TRUE;
	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;
	Settings.SixteenBitSound = TRUE;
	Settings.Stereo = TRUE;
	Settings.SoundSync = FALSE;
	Settings.SoundPlaybackRate = 32000;
	Settings.SoundInputRate = 32000;
	Settings.SupportHiRes = TRUE;
	Settings.Transparency = TRUE;
	Settings.AutoDisplayMessages = TRUE;
	Settings.InitialInfoStringTimeout = 120;
	Settings.HDMATimingHack = 100;
	Settings.BlockInvalidVRAMAccessMaster = TRUE;
	Settings.StopEmulation = TRUE;
	Settings.WrongMovieStateProtection = TRUE;
	Settings.DumpStreamsMaxFrames = -1;
	Settings.StretchScreenshots = 1;
	Settings.SnapshotScreenshots = TRUE;
	Settings.SkipFrames = AUTO_FRAMERATE;
	Settings.TurboSkipFrames = 15;
	Settings.CartAName[0] = 0;
	Settings.CartBName[0] = 0;
	Settings.DisableGameSpecificHacks = FALSE;
}

void		EMU_Init() {
	
	rom_loaded = EINA_FALSE;
	EMU_update_conf();

	// Core emulation Init
	if (!Memory.Init() || !S9xInitAPU()) {
		fprintf(stderr, "Snes9x: Memory allocation failure - not enough RAM/virtual memory available.\nExiting...\n");
		Memory.Deinit();
		S9xDeinitAPU();
		exit(1);
	}

	// Control Init
	S9xMapButton(SDLK_RIGHT,	S9xGetCommandT("Joypad1 Right"), FALSE);
	S9xMapButton(SDLK_LEFT,		S9xGetCommandT("Joypad1 Left"), FALSE);
	S9xMapButton(SDLK_DOWN,		S9xGetCommandT("Joypad1 Down"), FALSE);
	S9xMapButton(SDLK_UP,		S9xGetCommandT("Joypad1 Up"), FALSE);
	S9xMapButton(SDLK_RETURN,	S9xGetCommandT("Joypad1 Start"), FALSE);
#ifndef PANDORA
	S9xMapButton(SDLK_a,		S9xGetCommandT("Joypad1 X"), FALSE);
	S9xMapButton(SDLK_s,		S9xGetCommandT("Joypad1 A"), FALSE);
	S9xMapButton(SDLK_d,		S9xGetCommandT("Joypad1 B"), FALSE);
	S9xMapButton(SDLK_f,		S9xGetCommandT("Joypad1 Y"), FALSE);
	S9xMapButton(SDLK_g,		S9xGetCommandT("Joypad1 L"), FALSE);
	S9xMapButton(SDLK_h,		S9xGetCommandT("Joypad1 R"), FALSE);
	S9xMapButton(SDLK_j,		S9xGetCommandT("Joypad1 Select"), FALSE);
	S9xMapButton(SDLK_k,		S9xGetCommandT("Joypad1 Start"), FALSE);
#else
/* Matching pandora physical names
	S9xMapButton(SDLK_HOME,		S9xGetCommandT("Joypad1 A"), FALSE);
	S9xMapButton(SDLK_END,		S9xGetCommandT("Joypad1 B"), FALSE);
	S9xMapButton(SDLK_PAGEDOWN,	S9xGetCommandT("Joypad1 X"), FALSE);
	S9xMapButton(SDLK_PAGEUP,	S9xGetCommandT("Joypad1 Y"), FALSE);*/
	S9xMapButton(SDLK_END,		S9xGetCommandT("Joypad1 A"), FALSE);
	S9xMapButton(SDLK_PAGEDOWN,	S9xGetCommandT("Joypad1 B"), FALSE);
	S9xMapButton(SDLK_PAGEUP,	S9xGetCommandT("Joypad1 X"), FALSE);
	S9xMapButton(SDLK_HOME,		S9xGetCommandT("Joypad1 Y"), FALSE);
	S9xMapButton(SDLK_RSHIFT,	S9xGetCommandT("Joypad1 L"), FALSE);
	S9xMapButton(SDLK_RCTRL,	S9xGetCommandT("Joypad1 R"), FALSE);
	S9xMapButton(SDLK_LCTRL,	S9xGetCommandT("Joypad1 Select"), FALSE);
	S9xMapButton(SDLK_LALT,		S9xGetCommandT("Joypad1 Start"), FALSE);
#endif

	// GFX init
	S9xSetRenderPixelFormat(RGB565);
	render = menu_Texture_newStream("render", SNES_WIDTH, SNES_HEIGHT, MAX_SNES_WIDTH);
	GFX.Screen = render->sourceData;
	GFX.Pitch  = 1024;
	S9xGraphicsInit();

	//Sound init
#ifdef USE_SDL_AUDIO
	SDL_InitSubSystem (SDL_INIT_AUDIO);
	audiospec = (SDL_AudioSpec *) malloc (sizeof (SDL_AudioSpec));
	audiospec->freq = Settings.SoundPlaybackRate;
	audiospec->channels = Settings.Stereo ? 2 : 1;
	audiospec->format = Settings.SixteenBitSound ? AUDIO_S16SYS : AUDIO_U8;
	audiospec->samples = (bufSize * audiospec->freq / 1000) >> 1;
	audiospec->callback = sdl_audio_callback;
	if (SDL_OpenAudio (audiospec, NULL) < 0) {
		printf ("Opening audio device failed\n");
		free (audiospec);
		audiospec = NULL;
	}
	SDL_PauseAudio (0);
#else
	EMU_startAudio();
#endif
	S9xSetSamplesAvailableCallback (samples_available, NULL);
	S9xInitSound(bufSize, 0);
	S9xSetSoundMute(FALSE);

}

void		EMU_LoadRom(char *filename) {
	EMU_pause();
	EMU_onQuit();
	EMU_update_conf();
	bool8	loaded = Memory.LoadROM(filename);
	if (!loaded && filename[0]) {
		char	s[PATH_MAX + 1];
		char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

		_splitpath(filename, drive, dir, fname, ext);
		snprintf(s, PATH_MAX + 1, "%s%s%s", S9xGetDirectory(ROM_DIR), SLASH_STR, fname);
		if (ext[0] && (strlen(s) <= PATH_MAX - 1 - strlen(ext))) {
			strcat(s, ".");
			strcat(s, ext);
		}
		loaded = Memory.LoadROM(s);
	}
	if (loaded) rom_loaded = EINA_TRUE;
	//printf("Loaded \"%s\" %s: %s\n", Memory.ROMName, TITLE, VERSION);

	Memory.LoadSRAM(S9xGetFilename(".srm", SRAM_DIR));
	//S9xLoadCheatFile (S9xGetFilename (".cht", CHEAT_DIR));
	// TODO: Load & Apply cheats
}

void		EMU_saveState(int slot) {
	char def[PATH_MAX];
	char filename[PATH_MAX];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char ext[_MAX_EXT];

	_splitpath (Memory.ROMFilename, drive, dir, def, ext);

	sprintf (filename, "%s%s%s.%03d",
		S9xGetDirectory (SNAPSHOT_DIR), SLASH_STR, def,
		slot);
	printf ("saving %s\n", filename);

	if (!S9xFreezeGame (filename))
		printf("load failed\n");
}

void		EMU_loadState(int slot) {
	char def[PATH_MAX];
	char filename[PATH_MAX];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char ext[_MAX_EXT];

	_splitpath (Memory.ROMFilename, drive, dir, def, ext);

	sprintf (filename, "%s%s%s.%03d",
		S9xGetDirectory (SNAPSHOT_DIR), SLASH_STR, def,
		slot);
	printf ("Loading %s\n", filename);

	if (!S9xUnfreezeGame(filename))
		printf("load failed\n");
}


void		EMU_pause() {
	S9xSyncSpeedFinish();
	Settings.Paused = TRUE;
	S9xSetSoundMute(TRUE);
}

void		EMU_play() {
	if (!EMU_getCRC()) return;
	Settings.StopEmulation = FALSE;
	Settings.Paused = FALSE;
	Settings.ForcedPause = FALSE;
	S9xSetSoundMute(FALSE);
}

uint32_t	EMU_getCRC() {
	return Memory.ROMCRC32;
}

char *		EMU_getRomName() {
	return Memory.ROMName;
}


void		EMU_deInit() {
	Memory.Deinit();
	S9xDeinitAPU();
}

void		EMU_onQuit(void) {
	if (rom_loaded)
		S9xAutoSaveSRAM();
}

extern Uint32 fpsmc;
static Eina_Bool haveLocked = EINA_FALSE;
void EMU_unlock() {
	if (haveLocked) {
		haveLocked = EINA_FALSE;
		SDL_UnlockMutex(rendering_mutex);
	}
}

void		EMU_runFrame() {
	S9xSyncSpeedFinish ();
	if (Memory.ROMCRC32 && !Settings.StopEmulation && !Settings.Paused) {
		S9xMainLoop();
	} else if (!Memory.ROMCRC32) {
		EMU_pause();
	}
	if (haveLocked)
		EMU_unlock();
	else
		fpsmc++;
}

void		EMU_processEvent(SDL_Event *e) {
	if (Settings.Paused)
		S9xSetSoundMute(TRUE);
	switch(e->type) {
	case SDL_KEYUP:
	case SDL_KEYDOWN:
		S9xReportButton(e->key.keysym.sym, e->type==SDL_KEYDOWN);
	break;
	default:
	break;
	}
	if (!Settings.Paused)
		S9xSetSoundMute(FALSE);
}

// Snes9x interface ///////////////////////////////////////////////////////////////////


void S9xParsePortConfig (ConfigFile &conf, int pass) {
	
}

void S9xExtraUsage (void) {
	
}

void S9xParseArg (char **argv, int &i, int argc) {
	
}

bool8 S9xContinueUpdate (int width, int height) {
	return (TRUE);
}


void S9xMessage (int type, int number, const char *message) {
//When Snes9x wants to display an error, information or warning message, it calls this function. Check in messages.h for the types and individual message numbers that Snes9x currently passes as parameters.
//The idea is display the message string so the user can see it, but you choose not to display anything at all, or change the message based on the message number or message type.
//Eventually all debug output will also go via this function, trace information already does.
	//printf ("SNES9X: %s\n", message);
}


void S9xAutoSaveSRAM (void) {
//If Settings.AutoSaveDelay is not zero, Snes9x calls this function when the contents of the S-RAM has been changed. Simply call Memory.SaveSRAM function from this function.
	Memory.SaveSRAM (S9xGetFilename (".srm", SRAM_DIR));
	//S9xSaveCheatFile (S9xGetFilename (".cht", CHEAT_DIR));
}


void S9xSetPalette (void) {
//Called when the SNES color palette has changed. Use this function if your system should change its color palette to match the SNES's. Otherwise let it empty.
}

int                   syncing = 0;
static struct timeval next_frame_time = { 0, 0 };
static struct timeval now;

#undef TIMER_DIFF
#define TIMER_DIFF(a, b) ((((a).tv_sec - (b).tv_sec) * 1000000) + (a).tv_usec - (b).tv_usec)
/* Finishes syncing by using more accurate system sleep functions*/
void
S9xSyncSpeedFinish (void)
{
    if (!syncing)
        return;

    gettimeofday (&now, NULL);

    if (Settings.SoundSync)
    {
        while (!S9xSyncSound ())
        {
            usleep (100);

            gettimeofday (&next_frame_time, NULL);

            /* If we can't sync sound within a second, we're probably deadlocked */
            if (TIMER_DIFF (next_frame_time, now) > 1000000)
            {
                /* Flush out our sample buffer and give up. */
                S9xClearSamples ();

                break;
            }
        }

        next_frame_time = now;
        return;
    }

    if (TIMER_DIFF (next_frame_time, now) < -500000)
    {
        next_frame_time = now;
    }

    while (timercmp (&next_frame_time, &now, >))
    {
        int time_left = TIMER_DIFF (next_frame_time, now);

        if (time_left > 500000)
        {
            next_frame_time = now;
            break;
        }

        usleep (time_left);

        gettimeofday (&now, NULL);
    }

    next_frame_time.tv_usec += Settings.FrameTime;

    if (next_frame_time.tv_usec >= 1000000)
    {
        next_frame_time.tv_sec += next_frame_time.tv_usec / 1000000;
        next_frame_time.tv_usec %= 1000000;
    }

    syncing = 0;

    return;
}

void S9xSyncSpeed (void) {
//Called at the end of S9xMainLoop function, when emulating one frame has been done. You should adjust the frame rate in this function.
/*	if (Settings.SoundSync)
	{
		while (!S9xSyncSound())
			usleep(1);
	}

	if (Settings.DumpStreams)
		return;
	if (Settings.HighSpeedSeek > 0)
		Settings.HighSpeedSeek--;

	if (Settings.TurboMode)
	{
		if ((++IPPU.FrameSkip >= Settings.TurboSkipFrames) && !Settings.HighSpeedSeek)
		{
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		}
		else
		{
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		}

		return;
	}

	static struct timeval	next1 = { 0, 0 };
	struct timeval			now;

	gettimeofday(&now, NULL);
	// If there is no known "next" frame, initialize it now.
	if (next1.tv_sec == 0)
	{
		next1 = now;
		next1.tv_usec++;
	}

	// If we're on AUTO_FRAMERATE, we'll display frames always only if there's excess time.
	// Otherwise we'll display the defined amount of frames.
	unsigned	limit = (Settings.SkipFrames == AUTO_FRAMERATE) ? (timercmp(&next1, &now, <) ? 3 : 1) : Settings.SkipFrames;

	IPPU.RenderThisFrame = (++IPPU.SkippedFrames >= limit) ? TRUE : FALSE;
	//IPPU.RenderThisFrame = TRUE;

	if (IPPU.RenderThisFrame)
		IPPU.SkippedFrames = 0;
	else
	{
		// If we were behind the schedule, check how much it is.
		if (timercmp(&next1, &now, <))
		{
			unsigned	lag = (now.tv_sec - next1.tv_sec) * 1000000 + now.tv_usec - next1.tv_usec;
			if (lag >= 500000)
			{
				// More than a half-second behind means probably pause.
				// The next line prevents the magic fast-forward effect.
				next1 = now;
			}
		}
	}

	// Delay until we're completed this frame.
	// Can't use setitimer because the sound code already could be using it. We don't actually need it either.
	while (timercmp(&next1, &now, >))
	{
		// If we're ahead of time, sleep a while.
		unsigned	timeleft = (next1.tv_sec - now.tv_sec) * 1000000 + next1.tv_usec - now.tv_usec;
		if (timeleft > 500000) {
			next1 = now;
			break;
		}

		usleep(timeleft);

		gettimeofday(&now, NULL);
		// Continue with a while-loop because usleep() could be interrupted by a signal.
	}

	// Calculate the timestamp of the next frame.
	next1.tv_usec += Settings.FrameTime;
	if (next1.tv_usec >= 1000000)
	{
		next1.tv_sec += next1.tv_usec / 1000000;
		next1.tv_usec %= 1000000;
	}*/

#if 1
    unsigned int limit;
    int          lag;

#ifdef NETPLAY_SUPPORT
    if (S9xNetplaySyncSpeed ())
        return;
#endif

    if (Settings.HighSpeedSeek > 0)
    {
        Settings.HighSpeedSeek--;
        IPPU.RenderThisFrame = FALSE;
        IPPU.SkippedFrames = 0;

        gettimeofday (&now, NULL);
        next_frame_time = now;

        syncing = 0;

        return;
    }

    else if (Settings.TurboMode)
    {
        if ((++IPPU.FrameSkip >= Settings.TurboSkipFrames)
            && !Settings.HighSpeedSeek)
        {
            IPPU.FrameSkip = 0;
            IPPU.SkippedFrames = 0;
            IPPU.RenderThisFrame = TRUE;
        }
        else
        {
            IPPU.SkippedFrames++;
            IPPU.RenderThisFrame = FALSE;
        }

        return;
    }

    gettimeofday (&now, NULL);

    if (next_frame_time.tv_sec == 0)
    {
        next_frame_time = now;
        ++next_frame_time.tv_usec;
    }

    if (Settings.SkipFrames == AUTO_FRAMERATE && !Settings.SoundSync)
    {
        lag = TIMER_DIFF (now, next_frame_time);

        /* We compensate for the frame time by a frame in case it's just a CPU
         * discrepancy. We can recover lost time in the next frame anyway. */
        if (lag > (int) (Settings.FrameTime))
        {
            if (lag > (int) Settings.FrameTime * 10)
            {
                /* Running way too slowly */
                next_frame_time = now;
                IPPU.RenderThisFrame = 1;
                IPPU.SkippedFrames = 0;
            }
            else
            {
                IPPU.RenderThisFrame = 0;
                IPPU.SkippedFrames++;
            }
        }

        else
        {
            IPPU.RenderThisFrame = 1;
            IPPU.SkippedFrames = 0;
        }
    }
    else
    {
        limit = Settings.SoundSync ? 1 : Settings.SkipFrames + 1;

        IPPU.SkippedFrames++;
        IPPU.RenderThisFrame = 0;

        if (IPPU.SkippedFrames >= limit)
        {
            IPPU.RenderThisFrame = 1;
            IPPU.SkippedFrames = 0;
        }
    }

    syncing = 1;

    return;
#endif
}

void S9xLoadSDD1Data (void) {
//Called when the S-DD1 game is being loaded. If the user wants to use the on-the-fly S-DD1 decoding, simply set Settings.SDD1Pack to true and return. Otherwise, set Settings.SDD1Pack to false, allocates two memory spaces, read the graphics pack files (sdd1gfx.idx and sdd1gfx.dat) into the spaces, and set each Memory.SDD1XXX value correctly.
}

const char * S9xStringInput (const char *message) {
	printf("S9xStringInput- %s: ", message);
	fflush(stdout);

	return (NULL);
}

void S9xExit (void) {
//Called when some fatal error situation arises or when the “q” debugger command is used.

	printf("FATAL Error ?\n");
}

bool8 S9xInitUpdate (void) {
//Called just before Snes9x begins to render an SNES screen. Use this function if you should prepare before drawing, otherwise let it empty.
	if (!haveLocked)
		SDL_LockMutex(rendering_mutex);
	haveLocked = EINA_TRUE;
	return (TRUE);
}

bool8 S9xDeinitUpdate (int width, int height) {
//Called once a complete SNES screen has been rendered into the GFX.Screen memory buffer, now is your chance to copy the SNES rendered screen to the host computer's screen memory. The problem is that you have to cope with different sized SNES rendered screens: 256*224, 256*239, 512*224, 512*239, 512*448 and 512*478.
	if (render->orig_w != width || render->orig_h != height) {
		EMU_resize(width, height);
	}
	EMU_unlock();
	render->isUpdated = EINA_TRUE;
	return (TRUE);
}
// Input management ///////////////////////////////////////////////////////////////////

void S9xHandlePortCommand (s9xcommand_t cmd, int16 data1, int16 data2) {
}

bool S9xPollButton (uint32 id, bool *pressed) {
	return (TRUE);
}

bool S9xPollAxis (uint32 id, int16 *value) {
	return (TRUE);
}

bool S9xPollPointer (uint32 id, int16 *x, int16 *y) {
	return (TRUE);
}


// Sound processing ///////////////////////////////////////////////////////////////////

bool8 S9xOpenSoundDevice (void) {
	return (TRUE);
}

bool8 S9xOpenSoundDevice (int mode, bool8 stereo, int buffer_size) {
//S9xInitSound function calls this function to actually open the host sound device. The three parameters are the just the same as you passed in S9xInitSound function.
	return FALSE;
}


void S9xGenerateSound (void) {
//Use this function if you defined CHECK_SOUND in port.h.
}

void S9xToggleSoundChannel (int c) {
//If you port can match Snes9x's built-in SoundChannelXXX command (see controls.cpp), you may choose to use this function. Otherwise return NULL. Basically, turn on/off the sound channel c (0-7), and turn on all channels if c is 8.
}


// Path management ////////////////////////////////////////////////////////////////////

const char * S9xSelectFilename (const char *def, const char *dir1, const char *ext1, const char *title) {
	return (NULL);
}


bool8 S9xOpenSnapshotFile (const char *filename, bool8 read_only, STREAM *file) {
//This function opens a freeze-game file. STREAM is defined as a gzFile if ZLIB is defined else it's defined as FILE *. The read_only parameter is set to true when reading a freeze-game file and false when writing a freeze-game file. Open the file filepath and return its pointer file.
	char	s[PATH_MAX + 1];
	char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(filename, drive, dir, fname, ext);

	if (*drive || *dir == SLASH_CHAR || (strlen(dir) > 1 && *dir == '.' && *(dir + 1) == SLASH_CHAR)) {
		strncpy(s, filename, PATH_MAX + 1);
		s[PATH_MAX] = 0;
	} else
		snprintf(s, PATH_MAX + 1, "%s%s%s", S9xGetDirectory(SNAPSHOT_DIR), SLASH_STR, fname);

	if (!*ext && strlen(s) <= PATH_MAX - 4)
		strcat(s, ".frz");

	if ((*file = OPEN_STREAM(s, read_only ? "rb" : "wb")))
		return (TRUE);

	return (FALSE);
}

void S9xCloseSnapshotFile (STREAM file) {
//This function closes the freeze-game file opened by S9xOpenSnapshotFile function.
	CLOSE_STREAM(file);
}

const char *S9xGetFilename (const char *ex, enum s9x_getdirtype dirtype) {
//When Snes9x needs to know the name of the cheat/ips file and so on, this function is called. Check extension and dirtype, and return the appropriate filename. The current ports return the ROM file path with the given extension.
	static char	s[PATH_MAX + 1];
	char		drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
//	snprintf(s, PATH_MAX + 1, "%s%s%s%s", S9xGetDirectory(dirtype), SLASH_STR, fname, ex);
	snprintf(s, PATH_MAX + 1, "%s%s%s%s", dir, SLASH_STR, fname, ex);

	return (s);
}

const char *S9xGetFilenameInc (const char *ex, enum s9x_getdirtype dirtype) {
//Almost the same as S9xGetFilename function, but used for saving SPC files etc. So you have to take care not to delete the previously saved file, by increasing the number of the filename; romname.000.spc, romname.001.spc, ...
	static char	s[PATH_MAX + 1];
	char		drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	unsigned int	i = 0;
	const char		*d;
	struct stat		buf;

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	d = S9xGetDirectory(dirtype);

	do
		snprintf(s, PATH_MAX + 1, "%s%s%s.%03d%s", d, SLASH_STR, fname, i++, ex);
	while (stat(s, &buf) == 0 && i < 1000);

	return (s);
}

const char *S9xGetDirectory (enum s9x_getdirtype dirtype) {
//Called when Snes9x wants to know the directory dirtype.
	static char	s[PATH_MAX + 1];

	strncpy(s, Memory.ROMFilename, PATH_MAX + 1);
	s[PATH_MAX] = 0;

	for (int i = strlen(s); i >= 0; i--) {
		if (s[i] == SLASH_CHAR) {
				s[i] = 0;
				break;
		}
	}
	return (s);
}

const char *S9xChooseFilename (bool8 read_only) {
//If you port can match Snes9x's built-in LoadFreezeFile/SaveFreezeFile command (see controls.cpp), you may choose to use this function. Otherwise return NULL.
	char	s[PATH_MAX + 1];
	char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	const char	*filename;
	char		title[64];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	snprintf(s, PATH_MAX + 1, "%s.frz", fname);
	sprintf(title, "%s snapshot filename", read_only ? "Select load" : "Choose save");

	S9xSetSoundMute(TRUE);
	filename = S9xSelectFilename(s, S9xGetDirectory(SNAPSHOT_DIR), "frz", title);
	S9xSetSoundMute(FALSE);

	return (filename);
}

const char *S9xChooseMovieFilename (bool8 read_only) {
//If you port can match Snes9x's built-in BeginRecordingMovie/LoadMovie command (see controls.cpp), you may choose to use this function. Otherwise return NULL.
	char	s[PATH_MAX + 1];
	char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	const char	*filename;
	char		title[64];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	snprintf(s, PATH_MAX + 1, "%s.smv", fname);
	sprintf(title, "Choose movie %s filename", read_only ? "playback" : "record");

	S9xSetSoundMute(TRUE);
	filename = S9xSelectFilename(s, S9xGetDirectory(HOME_DIR), "smv", title);
	S9xSetSoundMute(FALSE);

	return (filename);
}

void _splitpath (const char *path, char *drive, char *dir, char *fname, char *ext) {
	*drive = 0;

	const char	*slash = strrchr(path, SLASH_CHAR),
				*dot   = strrchr(path, '.');

	if (dot && slash && dot < slash)
		dot = NULL;

	if (!slash) {
		*dir = 0;

		strcpy(fname, path);

		if (dot) {
			fname[dot - path] = 0;
			strcpy(ext, dot + 1);
		} else
			*ext = 0;
	} else 	{
		strcpy(dir, path);
		dir[slash - path] = 0;

		strcpy(fname, slash + 1);

		if (dot) {
			fname[dot - slash - 1] = 0;
			strcpy(ext, dot + 1);
		} else
			*ext = 0;
	}
}

void _makepath (char *path, const char *, const char *dir, const char *fname, const char *ext) {
	if (dir && *dir) {
		strcpy(path, dir);
		strcat(path, SLASH_STR);
	} else
		*path = 0;

	strcat(path, fname);

	if (ext && *ext) {
		strcat(path, ".");
		strcat(path, ext);
	}
}

const char *S9xBasename (bool8 read_only) {
//Deprecated, used in snaporig.cpp and snapshot.cpp though. Return NULL.
	return NULL;
}


const char * S9xBasename (const char *f) {
	const char	*p;

	if ((p = strrchr(f, '/')) != NULL || (p = strrchr(f, '\\')) != NULL)
		return (p + 1);

	return (f);
}
