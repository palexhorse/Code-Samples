/// \file main.cpp 

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <process.h>

#include <iostream>

using std::cout;
using std::endl;

#include "defines.h"
#include "abort.h"
#include "gamerenderer.h"
#include "imagefilenamelist.h"
#include "debug.h"
#include "timer.h"
#include "sprite.h"
#include "object.h"
#include "Gamepad.h"
#include "Sound.h"

//globals
BOOL menu = false;
BOOL g_bActiveApp;  ///< TRUE if this is the active application
HWND g_HwndApp; ///< Application window handle.
HINSTANCE g_hInstance; ///< Application instance handle.
char g_szGameName[256]; ///< Name of this game
char g_szShaderModel[256]; ///< The shader model version used
CImageFileNameList g_cImageFileName; ///< List of image file names.
CTimer g_cTimer; ///< The game timer.

C3DSprite* g_pPlaneSprite = nullptr; ///< Pointer to the plane sprite.
C3DSprite* g_pPlaneRSprite = nullptr; ///< Pointer to the Red plane sprite.
C3DSprite* g_pPlaneBSprite = nullptr; ///< Pointer to the Blue plane sprite.
C3DSprite* g_pPlaneYSprite = nullptr; ///< Pointer to the Yellow plane sprite.
C3DSprite* g_pPlaneExitSprite = nullptr;

CGameObject* g_pPlane = nullptr; ///< Pointer to the plane object.
C3DSprite* g_pMenuSprite = nullptr; 
C3DSprite* g_pEndSprite = nullptr;
C3DSprite* g_pPlatformSprite = nullptr; ///< Pointers to Platform sprites
C3DSprite* g_pPlatformRSprite = nullptr;
C3DSprite* g_pPlatformBSprite = nullptr;
C3DSprite* g_pPlatformYSprite = nullptr;

C3DSprite* g_pExitSprite = nullptr;

CSoundManager* g_pSoundManager;

const int PLAT_WIDTH = 506;
const int PLAT_HT = 25;

const float PLAT_XOFFSET = -200.0f;
const float PLAT_BLOCKSIZE = 32.0f;
const float HALF_PLAT_BLOCKSIZE = PLAT_BLOCKSIZE / 2.0f;

const float HAMSTER_HT = 100.0f;
const float HALF_HAMSTER_HT = HAMSTER_HT / 2.0f;
BOOL g_bPlatform[PLAT_HT][PLAT_WIDTH];

int level = 0;
int wall_index = 0;
int floor_index = 1;
int played = 0;								 
//graphics settings
int g_nPlatcolor[PLAT_HT][PLAT_WIDTH];
int g_nDeath = 0;
int g_ncolorchange;
int g_nTempColor;
int g_nScreenWidth; ///< Screen width.
int g_nScreenHeight; ///< Screen height.
Gamepad g_gamepad; ///< The Xbox controller.
BOOL g_bWireFrame = FALSE; ///< TRUE for wireframe rendering.
Vector3 exitCoord;

//XML settings
tinyxml2::XMLDocument g_xmlDocument; ///< TinyXML document for settings.
XMLElement* g_xmlSettings = nullptr; ///< TinyXML element for settings tag.

CGameRenderer GameRenderer; ///< The game renderer.
							//functions in Window.cpp
void InitGraphics();

HWND CreateDefaultWindow(char* name, HINSTANCE hInstance, int nCmdShow);

/// \brief Initialize XML settings.
///
/// Open an XML file and prepare to read settings from it. Settings
/// tag is loaded to XML element g_xmlSettings for later processing. Abort if it
/// cannot load the file or cannot find settings tag in loaded file.

void InitXMLSettings() {
	//open and load XML file
	const char* xmlFileName = "gamesettings.xml"; //Settings file name.
	if (g_xmlDocument.LoadFile(xmlFileName) != 0)
		ABORT("Cannot load settings file %s.", xmlFileName);

	//get settings tag
	g_xmlSettings = g_xmlDocument.FirstChildElement("settings"); //settings tag
	if (g_xmlSettings == nullptr) //abort if tag not found
		ABORT("Cannot find <settings> tag in %s.", xmlFileName);
} //InitXMLSettings

  /// \brief Load game settings.
  ///
  /// Load and parse game settings from a TinyXML element g_xmlSettings.

void LoadGameSettings() {
	if (!g_xmlSettings)return; //bail and fail

							   //get game name
	XMLElement* ist = g_xmlSettings->FirstChildElement("game");
	if (ist) {
		size_t len = strlen(ist->Attribute("name")); //length of name string
		strncpy_s(g_szGameName, len + 1, ist->Attribute("name"), len);
	} //if

	  //get renderer settings
	XMLElement* renderSettings =
		g_xmlSettings->FirstChildElement("renderer"); //renderer tag
	if (renderSettings) { //read renderer tag attributes
		g_nScreenWidth = renderSettings->IntAttribute("width");
		g_nScreenHeight = renderSettings->IntAttribute("height");

		size_t len = strlen(renderSettings->Attribute("shadermodel")); //length shader model string
		strncpy_s(g_szShaderModel, len + 1, renderSettings->Attribute("shadermodel"), len);
	} //if

	  //get image file names
	g_cImageFileName.GetImageFileNames(g_xmlSettings);
} //LoadGameSettings

void LoadPlatforms(int level) {
	FILE* platfile = NULL;
	
	// Read in platform layouts when loading each level
	if (level == 0 || level ==1)
		fopen_s(&platfile, "Platforms\\platforms1.txt", "rt");
	else if (level ==2)
		fopen_s(&platfile, "Platforms\\platforms2.txt", "rt");
	else if (level ==3)s
		fopen_s(&platfile, "Platforms\\platforms3.txt", "rt");
	else if (level == 4)
		fopen_s(&platfile, "Platforms\\platforms4.txt", "rt");

	char tempGet;

	if (platfile) {
		for (int i = 0; i<PLAT_HT; i++) {

			for (int j = 0; j < PLAT_WIDTH; j++) {
				tempGet = fgetc(platfile);
				if (tempGet == 'G') {// REGULAR GREEN
					g_nPlatcolor[i][j] = 0;
					g_bPlatform[i][j] = TRUE;
				}
				if (tempGet == 'R') {//RED
					g_nPlatcolor[i][j] = 1;
					g_bPlatform[i][j] = TRUE;
				}
				if (tempGet == 'B') {//BLUE
					g_nPlatcolor[i][j] = 2;
					g_bPlatform[i][j] = TRUE;
				}
				if (tempGet == 'Y') {//YELLOW
					g_nPlatcolor[i][j] = 3;
					g_bPlatform[i][j] = TRUE;
				}
				if (tempGet == 'Q') {//EXIT
					g_nPlatcolor[i][j] = 9;
					g_bPlatform[i][j] = TRUE;
				}
			}
			fgetc(platfile); //eoln
		} //for
		fclose(platfile);
	} //if
	else
		ABORT("Platform file not found.");
} //LoadPlatforms

void DrawPlatforms() {
	int exitDrawn = 0;
	float y = g_nScreenHeight - HALF_PLAT_BLOCKSIZE;
	for (int i = 0; i<PLAT_HT; i++) {
		float x = PLAT_XOFFSET;
		for (int j = 0; j<PLAT_WIDTH; j++) {
			if (g_bPlatform[i][j]) {
				// REG Platform
				if (level == 0) {
					g_pMenuSprite->Draw(Vector3(500, 600, 850.0f));
				}
				if (level == 4) {
					g_pEndSprite->Draw(Vector3(14000, 600, 850.0f));
				}
				if (g_nPlatcolor[i][j] == 0)
					g_pPlatformSprite->Draw(Vector3(x, y, 800.0f));
				// colored Platforms
				else if (g_nPlatcolor[i][j] == 1)
					g_pPlatformRSprite->Draw(Vector3(x, y, 800.0f));
				else if (g_nPlatcolor[i][j] == 2)
					g_pPlatformBSprite->Draw(Vector3(x, y, 800.0f));
				else if (g_nPlatcolor[i][j] == 3)
					g_pPlatformYSprite->Draw(Vector3(x, y, 800.0f));

				//EXIT
				else if (g_nPlatcolor[i][j] == 9)
					if (exitDrawn == 0) {
						g_pExitSprite->Draw(Vector3(x, y - 75, 849.0f));
						exitCoord = Vector3(x, y - 75, 850);
						exitDrawn = 1;
					}
			}// if
			x += PLAT_BLOCKSIZE;
		} //for
		y -= PLAT_BLOCKSIZE;
	} //for
} //DrawPlatforms
BOOL inRange(const int index, const int limit) {
	return index >= 0 && index < limit;
} //inRange

void CreateObjects() {
	g_pPlane = new CGameObject(
		Vector3((float)g_nScreenWidth / 2.0f + 150.0f, 300.0f, 800.0f),
		Vector3(0.0f, 0.0f, 0), g_pPlaneSprite);
} //CreateObjects


void Reset() {
	g_nDeath++;
	
	// delete player
	delete g_pPlane;
	// reinitialize player
	CGameObject* g_pPlane = nullptr; ///< Pointer to the plane objects
	CreateObjects();
}

void NextLevel() {
	g_nDeath = 0;
	level++;
	if ((level % 2) == 0) {
		wall_index = 12;
		floor_index = 13;
	}
	else {
		wall_index = 0;
		floor_index = 1;
	}
	Vector3 TempVelocity = g_pPlane->m_vVelocity;
	Vector3 TempPosition = g_pPlane->m_vPos;
	// clear textures, music, 
	GameRenderer.Release(); //release textures
	delete g_pPlane; //delete the plane object
	delete g_pPlaneSprite; //delete the plane sprites
	delete g_pPlaneRSprite;
	delete g_pPlaneBSprite;
	delete g_pPlaneYSprite;
	delete g_pPlatformSprite; // delete platform sprites
	delete g_pPlatformRSprite;
	delete g_pPlatformBSprite;
	delete g_pPlatformYSprite;
	delete g_pExitSprite;
	if (level == 1) // if exiting start menu
	{
		delete g_pMenuSprite;
	}

	// clear platforms
	for (int i = 0; i < PLAT_HT; i++) {
		for (int j = 0; j < PLAT_WIDTH; j++) {
			g_nPlatcolor[i][j] = NULL;
			g_bPlatform[i][j] = FALSE;
		}
	}

	if ((level % 2) == 0)
		g_pSoundManager->stop(0); // stop track 1
	else
		g_pSoundManager->stop(4);


	/// PRINT TRANSITION TEXT
	HDC hdc = GetDC(g_HwndApp); //device context for screen
	RECT rc; //screen rectangle

    //draw black rectangle to screen
	GetClientRect(g_HwndApp, &rc); //get screen rectangle into rc
	HBRUSH hBrush = CreateSolidBrush(RGB(0, 85, 87));
	FillRect(hdc, &rc, (hBrush)); //fill rc black
	DeleteObject(hBrush);

	//draw white text
	SetTextColor(hdc, RGB(255, 255, 255)); //white text 
	SetBkMode(hdc, TRANSPARENT); //transparent background
	HFONT hFont = CreateFont(-MulDiv(30, GetDeviceCaps(hdc, LOGPIXELSY), 72),
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Bauhaus 93"); //truly a scary number of zeros;
	if (hFont)SelectObject(hdc, hFont);
	DrawText(hdc, "Next Level Begin", -1, &rc, //draw text...
		DT_CENTER | DT_VCENTER | DT_SINGLELINE); //...centered on screen
												 //clean up and exit
	if (hFont){
		DeleteObject(hFont);
		Sleep(1000);
	}
	ReleaseDC(g_HwndApp, hdc);



	if ((level % 2) == 0) //levels 2, 4
		g_pSoundManager->loop(4); // play track 2
	else // levels 0, 1, 3
		g_pSoundManager->loop(0); // play track 1

	InitGraphics(); //initialize graphics
	CGameObject* g_pPlane = nullptr;
	g_pPlaneSprite = new C3DSprite(); //player sprites
	g_pPlaneRSprite = new C3DSprite();
	g_pPlaneBSprite = new C3DSprite();
	g_pPlaneYSprite = new C3DSprite();
	g_pPlaneExitSprite = new C3DSprite();
	g_pExitSprite = new C3DSprite(); // Exit Sprite


	GameRenderer.LoadTextures(wall_index, floor_index); //load images

	if (!g_pPlaneSprite->Load(g_cImageFileName[3])) //plane sprite
		ABORT("Plane image %s not found.", g_cImageFileName[3]);
	if (!g_pPlaneRSprite->Load(g_cImageFileName[8])) //red
		ABORT("Plane image %s not found.", g_cImageFileName[8]);
	if (!g_pPlaneBSprite->Load(g_cImageFileName[9])) //blue
		ABORT("Plane image %s not found.", g_cImageFileName[9]);
	if (!g_pPlaneYSprite->Load(g_cImageFileName[10])) //yellow
		ABORT("Plane image %s not found.", g_cImageFileName[10]);
	if (!g_pPlaneExitSprite->Load(g_cImageFileName[14]))
		ABORT("Plane image %s not found.", g_cImageFileName[14]);

	// Platform/Exit layout sprites
	g_pPlatformSprite = new C3DSprite();
	if (!g_pPlatformSprite->Load(g_cImageFileName[4]))
		ABORT("Platform image %s not found.", g_cImageFileName[4]);
	g_pPlatformRSprite = new C3DSprite();
	if (!g_pPlatformRSprite->Load(g_cImageFileName[5]))
		ABORT("Platform image %s not found.", g_cImageFileName[5]);
	g_pPlatformBSprite = new C3DSprite();
	if (!g_pPlatformBSprite->Load(g_cImageFileName[6]))
		ABORT("Platform image %s not found.", g_cImageFileName[6]);
	g_pPlatformYSprite = new C3DSprite();
	if (!g_pPlatformYSprite->Load(g_cImageFileName[7]))
		ABORT("Platform image %s not found.", g_cImageFileName[7]);
	if (level < 4) {
		if (!g_pExitSprite->Load(g_cImageFileName[15]))
			ABORT("Exit image %s not found.", g_cImageFileName[15]);
	}
	else if (level == 4) {
		if (!g_pExitSprite->Load(g_cImageFileName[11]))
			ABORT("Exit image %s not found.", g_cImageFileName[11]);
	}
	if (level == 4) {
		if (!g_pEndSprite->Load(g_cImageFileName[17]))
			ABORT("Exit image %s not found.", g_cImageFileName[17]);
	}
	LoadPlatforms(level);
	CreateObjects(); //create game objects
}

void EndGame() {
		g_pPlane->colorchange(g_pPlaneExitSprite);
		
		if(played == 0)
			g_pSoundManager->play(5);
		played = 1;
		g_pExitSprite->Release();
}

BOOL isOnPlatformOrGround(float x, float& y) {
	int i = int(round((g_nScreenHeight + (HALF_HAMSTER_HT)-y) / PLAT_BLOCKSIZE));
	int j = int(round((x - PLAT_XOFFSET) / PLAT_BLOCKSIZE));

	//on a platform 

	if (inRange(j, PLAT_WIDTH) && inRange(i, PLAT_HT) && g_bPlatform[i][j] && ((g_nPlatcolor[i][j] == 0) || (g_nPlatcolor[i][j] == g_ncolorchange))) {
		y = (round(y / PLAT_BLOCKSIZE)*PLAT_BLOCKSIZE)-8; //correct height 
		return TRUE;
	} //if

	else {
		return FALSE;
	}
} //isOnPlatformOrGround

BOOL isUnderPlatform(float x, float& y) {
	//int tempColor;
	int i = int(round((g_nScreenHeight - HALF_HAMSTER_HT - y) / PLAT_BLOCKSIZE));
	int j = int(round((x - PLAT_XOFFSET) / PLAT_BLOCKSIZE));
	if (g_pPlane->m_vPos.y < 140) {
		g_pSoundManager->play(3);
		g_ncolorchange = 0;
		g_nTempColor = 0;
		Reset();				//resets character when he falls off
	}

	if (inRange(j, PLAT_WIDTH) && inRange(i, PLAT_HT) && g_bPlatform[i][j]) {
		return TRUE;
		
	} //if
	
	else return FALSE;
	
} //isUnderPlatform



  /// \brief Create game objects. 
  ///
  /// Create a plane object, we'll add more objects in later demos.



  /// \brief Keyboard handler.
  ///
  /// Handler for keyboard messages from the Windows API. Takes the appropriate
  /// action when the user presses a key on the keyboard.
  /// \param keystroke Virtual key code for the key pressed
  /// \return TRUE if the game is to exit

void GamepadHandler() {

	// Enable gamepad support
	g_gamepad.SetWindow(g_HwndApp);
	//g_gamepad.SetRepeatIntervalMsAll(100); //miliseconds to repeat
	// g_gamepad.CheckConnection();
	// Update gamepad state
	g_gamepad.Refresh();

	g_gamepad.ClearMappings();
	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_DPAD_LEFT, VK_LEFT);
	g_gamepad.AddAnalogKeyMapping(Gamepad::AnalogButtons::LeftStickLeft, 1, VK_LEFT);
	//g_gamepad.AddAnalogKeyMapping(Gamepad::AnalogButtons::RightStickLeft, 1, VK_LEFT);
	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_RIGHT_THUMB, 0x39);

	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_DPAD_RIGHT, VK_RIGHT);
	g_gamepad.AddAnalogKeyMapping(Gamepad::AnalogButtons::LeftStickRight, 1, VK_RIGHT);
	//g_gamepad.AddAnalogKeyMapping(Gamepad::AnalogButtons::RightStickRight, 1, VK_RIGHT);
	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_LEFT_THUMB, VK_TAB);



	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_DPAD_UP, VK_UP);
	//g_gamepad.AddAnalogKeyMapping(Gamepad::AnalogButtons::LeftStickUp, 1, VK_UP);
	//g_gamepad.AddAnalogKeyMapping(Gamepad::AnalogButtons::RightStickUp, 1, VK_UP);

	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_DPAD_DOWN, VK_DOWN);
	g_gamepad.AddAnalogKeyMapping(Gamepad::AnalogButtons::LeftStickDown, 1, VK_DOWN);
	//g_gamepad.AddAnalogKeyMapping(Gamepad::AnalogButtons::RightStickDown, 1, VK_DOWN);

	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_A, VK_SPACE);
	g_gamepad.AddAnalogKeyMapping(Gamepad::AnalogButtons::RightTrigger, 1, VK_SPACE);
	g_gamepad.AddAnalogKeyMapping(Gamepad::AnalogButtons::LeftTrigger, 1, 0x34);

	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_B, 0x31);
	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_X, 0x32);
	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_Y, 0x33);

	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_BACK, VK_ESCAPE);
	g_gamepad.AddKeyMapping(XINPUT_GAMEPAD_START, VK_RETURN);

	// End Gamepad Code
}

BOOL KeyboardHandler(WPARAM keystroke) {
		switch (keystroke) {

			case VK_ESCAPE: //exit game
				return TRUE; //exit keyboard handler
				break;

			case VK_F1: //flip camera mode
				GameRenderer.FlipCameraMode();
				break;

			case VK_F2: //toggle wireframe mode
				g_bWireFrame = !g_bWireFrame;
				GameRenderer.SetWireFrameMode(g_bWireFrame);
				break;

			case VK_UP:
			case VK_SPACE:
				if (g_pPlane->m_vVelocity.y==0)
					g_pSoundManager->play(1);
				g_pPlane->jump();
				break;

			case VK_LEFT:
				g_pSoundManager->play(2);
				g_pPlane->accelerate(Vector3(-200.0f, 0, 0));
				break;

			case VK_RIGHT:
				g_pSoundManager->play(2);
				g_pPlane->accelerate(Vector3(200.0f, 0, 0));
				break;

			case VK_RETURN:
				if (level == 0)
					NextLevel();
				break;

			case 0x31: // 1 key -> Red
				g_ncolorchange = 1;
				g_nTempColor = 1;
				g_pPlane->colorchange(g_pPlaneRSprite);
				break;

			case 0x32: // 2 key -> Blue
				g_ncolorchange = 2;
				g_nTempColor = 2;
				g_pPlane->colorchange(g_pPlaneBSprite);
				break;

			case 0x33: // 3 key -> Yellow
				g_ncolorchange = 3;
				g_nTempColor = 3;
				g_pPlane->colorchange(g_pPlaneYSprite);
				break;

			case 0x34: // 4 key -> White
				g_ncolorchange = 0;
				g_nTempColor = 0;
				g_pPlane->colorchange(g_pPlaneSprite);
				break;

			case 0x39: // 9 key Warp to End
				if (level > 0) {
					g_pPlane->colorchange(g_pPlaneSprite);
					//Warp
					g_pPlane->m_vPos = Vector3(13900.0f, 800.0f, 800.0f);
				}
				break;

			case 0x38:
				NextLevel();
				break;

			case VK_TAB:
				g_pPlane->colorchange(g_pPlaneSprite);
				g_ncolorchange = 0;
				g_nTempColor = 0;
				Reset();
				//RESET LEVEL
				break;

		} //switch

	return FALSE; //normal exit
} //KeyboardHandler

  /// \brief Window procedure.
  ///
  /// Handler for messages from the Windows API. 
  /// \param hwnd Window handle
  /// \param message Message code
  /// \param wParam Parameter for message 
  /// \param lParam Second parameter for message
  /// \return 0 if message is handled

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) { //handle message
	case WM_ACTIVATEAPP: g_bActiveApp = (BOOL)wParam; break; //iconize

	case WM_KEYDOWN: //keyboard hit
			if (KeyboardHandler(wParam))DestroyWindow(hwnd);
		break;

	case WM_KEYUP:
		if (wParam == VK_UP || wParam == VK_SPACE || wParam == VK_RETURN ||wParam == 0x31 || wParam == 0x32 || wParam == 0x33 || wParam == 0x34)
			break;
		if (wParam == VK_LEFT || wParam == VK_RIGHT) {
			g_pPlane->idle();
			break;
		}
		else
			break;

	case WM_DESTROY: //on exit
		GameRenderer.Release(); //release textures
		delete g_pPlane; //delete the plane object
		delete g_pPlaneSprite; //delete the plane sprites
		delete g_pPlaneRSprite;
		delete g_pPlaneBSprite;
		delete g_pPlaneYSprite;

		delete g_pPlatformSprite; // delete platform sprites
		delete g_pPlatformRSprite;
		delete g_pPlatformBSprite;
		delete g_pPlatformYSprite;
		delete g_pExitSprite;

		SAFE_DELETE(g_pSoundManager);
		PostQuitMessage(0); //this is the last thing to do on exit
		break;

	default: //default window procedure
		return DefWindowProc(hwnd, message, wParam, lParam);
	} //switch(message)
	return 0;
} //WindowProc

  /// \brief Winmain.  
  ///         
  /// Main entry point for this application. 
  /// \param hInst Handle to the current instance of this application.
  /// \param hPrevInst Handle to previous instance, deprecated.
  /// \param lpCmdLine Command line string, unused. 
  /// \param nShow Specifies how the window is to be shown.
  /// \return TRUE if application terminates correctly.

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShow) {
	MSG msg; //current message
	HWND hwnd; //handle to fullscreen window

	g_hInstance = hInst;
	g_cTimer.start(); //start game timer
	InitXMLSettings(); //initialize XML settings reader
	LoadGameSettings();

	//create fullscreen window
	hwnd = CreateDefaultWindow(g_szGameName, hInst, nShow);
	if (!hwnd)return FALSE; //bail if problem creating window
	g_HwndApp = hwnd; //save window handle

	GamepadHandler(); // Bind gamepad controls

#ifdef DEBUG_ON
	g_cDebugManager.Initialize();
#endif //DEBUG_ON

	// Load Sound Effects / BG Music index
	g_pSoundManager = new CSoundManager(6);
	g_pSoundManager->Load("Sounds\\Supervenience.wav", 1);// contributed by Cactus Soda
	g_pSoundManager->Load("Sounds\\jump.wav", 1);// credit to dklon @ opengameart.org
	g_pSoundManager->Load("Sounds\\sfx_step_grass_fast.wav", 1);
	g_pSoundManager->Load("Sounds\\lose.wav", 1);
	g_pSoundManager->Load("Sounds\\Ephemeris.wav", 1);
	g_pSoundManager->Load("Sounds\\Rise02.wav", 1);

	// set volume for bg music
	g_pSoundManager->volume(.3, 0, 0);//volume based on percentage
	g_pSoundManager->volume(.3, 0, 4);
	g_pSoundManager->volume(2, 0, 5);
	g_pSoundManager->loop(0);

	
	InitGraphics(); //initialize graphics
	
	g_pMenuSprite = new C3DSprite();
	g_pEndSprite = new C3DSprite();
	g_pPlaneSprite = new C3DSprite(); //player sprites
	g_pPlaneRSprite = new C3DSprite();
	g_pPlaneBSprite = new C3DSprite();
	g_pPlaneYSprite = new C3DSprite();
	g_pExitSprite = new C3DSprite(); // Exit Sprite
	
		GameRenderer.LoadTextures(wall_index, floor_index); //load images

		if (!g_pPlaneSprite->Load(g_cImageFileName[3])) //plane sprite
			ABORT("Plane image %s not found.", g_cImageFileName[3]);
		if (!g_pPlaneRSprite->Load(g_cImageFileName[8])) //red
			ABORT("Plane image %s not found.", g_cImageFileName[8]);
		if (!g_pPlaneBSprite->Load(g_cImageFileName[9])) //blue
			ABORT("Plane image %s not found.", g_cImageFileName[9]);
		if (!g_pPlaneYSprite->Load(g_cImageFileName[10])) //yellow
			ABORT("Plane image %s not found.", g_cImageFileName[10]);
		CreateObjects(); //create game objects


		// Platform/Exit layout sprites
		g_pPlatformSprite = new C3DSprite();
		if (!g_pPlatformSprite->Load(g_cImageFileName[4]))
			ABORT("Platform image %s not found.", g_cImageFileName[4]);
		g_pPlatformRSprite = new C3DSprite();
		if (!g_pPlatformRSprite->Load(g_cImageFileName[5]))
			ABORT("Platform image %s not found.", g_cImageFileName[5]);
		g_pPlatformBSprite = new C3DSprite();
		if (!g_pPlatformBSprite->Load(g_cImageFileName[6]))
			ABORT("Platform image %s not found.", g_cImageFileName[6]);
		g_pPlatformYSprite = new C3DSprite();
		if (!g_pPlatformYSprite->Load(g_cImageFileName[7]))
			ABORT("Platform image %s not found.", g_cImageFileName[7]);
		if (!g_pExitSprite->Load(g_cImageFileName[15]))
			ABORT("Exit image %s not found.", g_cImageFileName[15]);
		if (!g_pMenuSprite->Load(g_cImageFileName[16]))
			ABORT("Menu image %s not found.", g_cImageFileName[16]);
		if (!g_pEndSprite->Load(g_cImageFileName[17]))
			ABORT("End image %s not found.", g_cImageFileName[17]);
	
		LoadPlatforms(level);

		//message loop
		while (TRUE)
			if (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE)) { //if message waiting
				if (!GetMessage(&msg, nullptr, 0, 0))return (int)msg.wParam; //get it
				TranslateMessage(&msg); DispatchMessage(&msg); //translate it
			} //if
			else {
				g_gamepad.Refresh();
				if (g_bActiveApp) {
					g_cTimer.beginframe(); //notify timer that frame has begun
					GameRenderer.ProcessFrame();
				} //if

				else
					WaitMessage();
			}

} //WinMain
 
