/* The implementation of swprintf() is broken on FreeBSD and sometimes fails if
 * LC_TYPE is not set to UTF-8.  This happens when characters, which cannot be
 * represented by the current LC_CTYPE, are printed. */
#if defined __FreeBSD__
#	define BROKEN_SWPRINTF
#endif

#if defined BROKEN_SWPRINTF
#	include <locale.h>
#endif

#include <exception>
#include <new>

#include "Button_System.h"
#include "Debug.h"
#include "FileMan.h"
#include "Font.h"
#include "Gameloop.h"
#include "Input.h"
#include "Intro.h"
#include "JA2_Splash.h"
#include "MemMan.h"
#include "Random.h"
#include "SGP.h"
#include "SoundMan.h"
#include "VObject.h"
#include "Video.h"
#include "VSurface.h"
#include <SDL.h>


#ifdef JA2
extern BOOLEAN gfPauseDueToPlayerGamePause;
#endif


static BOOLEAN gfApplicationActive;
BOOLEAN gfProgramIsRunning;
static BOOLEAN gfGameInitialized = FALSE;

CHAR8		gzCommandLine[100];		// Command line given

#if 0 // XXX TODO
INT32 FAR PASCAL WindowProcedure(HWND hWindow, UINT16 Message, WPARAM wParam, LPARAM lParam)
{
	static BOOLEAN fRestore = FALSE;

	switch(Message)
  {
    case WM_ACTIVATEAPP:
      switch(wParam)
      {
        case TRUE: // We are restarting DirectDraw
					if (fRestore)
          {
#ifdef JA2
	          RestoreVideoManager();

						// unpause the JA2 Global clock
            if ( !gfPauseDueToPlayerGamePause )
            {
						  PauseTime( FALSE );
            }
#endif
            gfApplicationActive = TRUE;
          }
          break;
        case FALSE: // We are suspending direct draw
#ifdef JA2
						// pause the JA2 Global clock
						PauseTime( TRUE );
						SuspendVideoManager();
#endif

          gfApplicationActive = FALSE;
          fRestore = TRUE;
          break;
      }
      break;

    case WM_CREATE:
			break;

    case WM_DESTROY:
			ShutdownStandardGamingPlatform();
      ShowCursor(TRUE);
      PostQuitMessage(0);
      break;

		case WM_SETFOCUS:
      RestoreCursorClipRect( );
			break;

		case WM_KILLFOCUS:
			// Set a flag to restore surfaces once a WM_ACTIVEATEAPP is received
			fRestore = TRUE;
			break;

    default
    : return DefWindowProc(hWindow, Message, wParam, lParam);
  }
  return 0L;
}
#endif


static void SGPExit(void);


static BOOLEAN InitializeStandardGamingPlatform(void)
{
	// now required by all (even JA2) in order to call ShutdownSGP
	atexit(SGPExit);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_EnableUNICODE(SDL_ENABLE);

#ifdef SGP_DEBUG
	// Initialize the Debug Manager - success doesn't matter
	InitializeDebugManager();
#endif

  // this one needs to go ahead of all others (except Debug), for MemDebugCounter to work right...
	FastDebugMsg("Initializing Memory Manager");
	// Initialize the Memory Manager
	if (!InitializeMemoryManager())
	{ // We were unable to initialize the memory manager
		FastDebugMsg("FAILED : Initializing Memory Manager");
		return FALSE;
	}

	FastDebugMsg("Initializing File Manager");
	// Initialize the File Manager
	if (!InitializeFileManager())
	{ // We were unable to initialize the file manager
		FastDebugMsg("FAILED : Initializing File Manager");
		return FALSE;
	}

	FastDebugMsg("Initializing Video Manager");
	// Initialize DirectDraw (DirectX 2)
	if (!InitializeVideoManager())
	{ // We were unable to initialize the video manager
		FastDebugMsg("FAILED : Initializing Video Manager");
		return FALSE;
	}

	// Initialize Video Object Manager
	FastDebugMsg("Initializing Video Object Manager");
	if ( !InitializeVideoObjectManager( ) )
	{
		FastDebugMsg("FAILED : Initializing Video Object Manager");
		return FALSE;
	}

	// Initialize Video Surface Manager
	FastDebugMsg("Initializing Video Surface Manager");
	if ( !InitializeVideoSurfaceManager( ) )
	{
		FastDebugMsg("FAILED : Initializing Video Surface Manager");
		return FALSE;
	}

#ifdef JA2
	if (!InitJA2SplashScreen()) return FALSE;
#endif

	// Initialize Font Manager
	FastDebugMsg("Initializing the Font Manager");
	// Init the manager and copy the TransTable stuff into it.
	if (!InitializeFontManager())
	{
		FastDebugMsg("FAILED : Initializing Font Manager");
		return FALSE;
	}

	FastDebugMsg("Initializing Sound Manager");
#ifndef UTIL
	InitializeSoundManager();
#endif

	FastDebugMsg("Initializing Random");
  // Initialize random number generator
  InitializeRandom(); // no Shutdown

	FastDebugMsg("Initializing Game Manager");
	// Initialize the Game
	if (!InitializeGame())
	{ // We were unable to initialize the game
		FastDebugMsg("FAILED : Initializing Game Manager");
		return FALSE;
	}

	gfGameInitialized = TRUE;

	return TRUE;
}


static void ShutdownStandardGamingPlatform(void)
{
	// Shut down the different components of the SGP

	// TEST
	SoundServiceStreams();

	if (gfGameInitialized)
	{
		ShutdownGame();
	}

	ShutdownButtonSystem();
	MSYS_Shutdown();

#ifndef UTIL
  ShutdownSoundManager();
#endif

  ShutdownFontManager();

#ifdef SGP_VIDEO_DEBUGGING
	PerformVideoInfoDumpIntoFile( "SGPVideoShutdownDump.txt", FALSE );
#endif

	ShutdownVideoSurfaceManager();
  ShutdownVideoObjectManager();
  ShutdownVideoManager();

#ifdef EXTREME_MEMORY_DEBUGGING
	DumpMemoryInfoIntoFile( "ExtremeMemoryDump.txt", FALSE );
#endif

  ShutdownMemoryManager();  // must go last, for MemDebugCounter to work right...
}


static void MainLoop()
{
	gfApplicationActive = TRUE;
	gfProgramIsRunning  = TRUE;

  while (gfProgramIsRunning)
  {
		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_ACTIVEEVENT:
					if (event.active.state & SDL_APPACTIVE)
					{
						gfApplicationActive = (event.active.gain != 0);
						break;
					}
					break;

				case SDL_KEYDOWN: KeyDown(&event.key.keysym); break;
				case SDL_KEYUP:   KeyUp(  &event.key.keysym); break;

				case SDL_MOUSEBUTTONDOWN: MouseButtonDown(&event.button); break;
				case SDL_MOUSEBUTTONUP:   MouseButtonUp(&event.button);   break;

				case SDL_MOUSEMOTION:
					gusMouseXPos = event.motion.x;
					gusMouseYPos = event.motion.y;
					break;

				case SDL_QUIT:
					gfProgramIsRunning = FALSE;
					break;
			}
		}
		else
		{
			if (gfApplicationActive)
			{
				GameLoop();
				SDL_Delay(1); // XXX HACK0001
			}
			else
			{
				SDL_WaitEvent(NULL);
			}
		}
  }
}


static BOOLEAN ParseParameters(char* const argv[]);


int main(int argc, char* argv[])
{
	try
	{
#if defined BROKEN_SWPRINTF
		if (setlocale(LC_CTYPE, "UTF-8") == NULL)
		{
			fprintf(stderr, "WARNING: Failed to set LC_CTYPE to UTF-8. Some strings might get garbled.\n");
		}
#endif

		if (!ParseParameters(argv)) return EXIT_FAILURE;
		if (argc > 1 && argv[1] != NULL) strlcpy(gzCommandLine, argv[1], lengthof(gzCommandLine));

		if (!InitializeStandardGamingPlatform()) return EXIT_FAILURE;

#if defined JA2 && defined ENGLISH
		SetIntroType(INTRO_SPLASH);
#endif

		FastDebugMsg("Running Game");

		/* At this point the SGP is set up, which means all I/O, Memory, tools, etc.
		 * are available. All we need to do is attend to the gaming mechanics
		 * themselves */
		MainLoop();

		FastDebugMsg("Exiting Game");

		// SGPExit() will be called next through the atexit() mechanism
		return EXIT_SUCCESS;
	}
	catch (const std::bad_alloc&)
	{
		fprintf(stderr, "ERROR: out of memory\n");
	}
	catch (const std::exception& e)
	{
		fprintf(stderr, "ERROR: caught unhandled exception: \"%s\"\n", e.what());
	}
	catch (...)
	{
		fprintf(stderr, "ERROR: caught unhandled unknown exception\n");
	}

	return EXIT_FAILURE;
}


static void SGPExit(void)
{
	static BOOLEAN fAlreadyExiting = FALSE;

	// helps prevent heap crashes when multiple assertions occur and call us
	if ( fAlreadyExiting )
	{
		return;
	}

	fAlreadyExiting = TRUE;
	gfProgramIsRunning = FALSE;

	ShutdownStandardGamingPlatform();
}


static BOOLEAN ParseParameters(char* const argv[])
{
	const char* const name = *argv;
	if (name == NULL) return TRUE; // argv does not even contain the program name

	BOOLEAN success = TRUE;
	while (*++argv != NULL)
	{
		if (strcmp(*argv, "-fullscreen") == 0)
		{
			VideoSetFullScreen(TRUE);
		}
		else if (strcmp(*argv, "-nosound") == 0)
		{
			SoundEnableSound(FALSE);
		}
		else if (strcmp(*argv, "-window") == 0)
		{
			VideoSetFullScreen(FALSE);
		}
		else
		{
			if (strcmp(*argv, "-help") != 0)
			{
				fprintf(stderr, "Unknown switch \"%s\"\n", *argv);
			}
			success = FALSE;
		}
	}

	if (!success)
	{
		fprintf(stderr,
			"Usage: %s [options]\n"
			"  -fullscreen  Start the game in fullscreen mode\n"
			"  -help        Display this information\n"
			"  -nosound     Turn the sound and music off\n"
			"  -window      Start the game in a window\n",
			name
		);
	}
	return success;
}
