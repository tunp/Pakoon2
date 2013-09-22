//
// Full-access sound module for all users of the game
// 
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "stdafx.h"
#include "SoundModule.h"
#include "BGame.h"
#include "FileIOHelpers.h"

bool SoundModule::m_bRunning = false;

int  SoundModule::m_chaMenuMusic = 0;
int  SoundModule::m_chaVehicleSounds = 0;
int  SoundModule::m_chaSkidSound = 0;
int  SoundModule::m_chaGameMusic1 = 0;
int  SoundModule::m_chaGameMusic2 = 0;
int  SoundModule::m_chaMessageSound = 0;
int  SoundModule::m_chaCrashSound = 0;
int  SoundModule::m_chaMenuBrowseSound = 0;
int  SoundModule::m_chaMenuScrollSound = 0;
int  SoundModule::m_chaMenuBackSound = 0;
int  SoundModule::m_chaSlalomPortSound = 0;
int  SoundModule::m_chaDisqualifiedSound = 0;
int  SoundModule::m_chaCountdown123Sound = 0;
int  SoundModule::m_chaCountdownGoSound = 0;
int  SoundModule::m_chaMultiplayerJoinSound = 0;
int  SoundModule::m_chaMultiplayerLeftSound = 0;
int  SoundModule::m_chaGoalFanfarSound = 0;
int  SoundModule::m_chaHeliSound = 0;
int  SoundModule::m_chaJetSound = 0;
int  SoundModule::m_chaIntroSound = 0;
FSOUND_SAMPLE *SoundModule::m_pMenuMusic = 0;
FSOUND_SAMPLE *SoundModule::m_pVehicleSounds = 0;
FSOUND_SAMPLE *SoundModule::m_pGameMusic1 = 0;
FSOUND_SAMPLE *SoundModule::m_pGameMusic2 = 0;
FSOUND_SAMPLE *SoundModule::m_pMessageSound = 0;
FSOUND_SAMPLE *SoundModule::m_pSkidSound = 0;
FSOUND_SAMPLE *SoundModule::m_pCrashSound = 0;
FSOUND_SAMPLE *SoundModule::m_pMenuBrowseSound = 0;
FSOUND_SAMPLE *SoundModule::m_pMenuScrollSound = 0;
FSOUND_SAMPLE *SoundModule::m_pMenuBackSound = 0;
FSOUND_SAMPLE *SoundModule::m_pSlalomPortSound = 0;
FSOUND_SAMPLE *SoundModule::m_pDisqualifiedSound = 0;
FSOUND_SAMPLE *SoundModule::m_pCountdown123Sound = 0;
FSOUND_SAMPLE *SoundModule::m_pCountdownGoSound = 0;
FSOUND_SAMPLE *SoundModule::m_pMultiplayerJoinSound = 0;
FSOUND_SAMPLE *SoundModule::m_pMultiplayerLeftSound = 0;
FSOUND_SAMPLE *SoundModule::m_pGoalFanfarSound = 0;
FSOUND_SAMPLE *SoundModule::m_pHeliSound = 0;
FSOUND_SAMPLE *SoundModule::m_pJetSound = 0;
FSOUND_SAMPLE *SoundModule::m_pIntroSound = 0;

int  SoundModule::m_nMenuMusicVolume = 128;
int  SoundModule::m_nVehicleSoundsVolume = 128;
int  SoundModule::m_nSkidSoundVolume = 0;
int  SoundModule::m_nGameMusicVolume = 160;
int  SoundModule::m_nMessageSoundVolume = 128;
int  SoundModule::m_nHeliSoundVolume = 255;
int  SoundModule::m_nJetSoundVolume = 255;

int  SoundModule::m_nSpace = 1;

int  SoundModule::m_nGameMusicFiles = 1;

SoundModule::SoundModule() {
}

void SoundModule::Initialize() {
  if(FSOUND_Init(44100, 32, 0)) {
    m_bRunning = true;
  } else {
    BGame::MyAfxMessageBox("Cannot initialize sound system (maybe some other program is using the sound device).\nSounds will not be supported.");
  }
  // FSOUND_3D_Listener_SetRolloffFactor(0.1f); // Make distant sounds louder
  // FSOUND_3D_Update();

  int nChannels = FSOUND_GetMaxChannels();

  // Play a tiny silence file to wake up the MP3 support???
  /*
  FSOUND_SAMPLE *pTmp = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\Engine2.wav", FSOUND_LOOP_OFF, 0);
  if(pTmp) {
    int nTmp = FSOUND_PlaySound(FSOUND_FREE, pTmp);
    FSOUND_SetVolume(nTmp, 255);
  }
  */

  m_nGameMusicFiles = 0;

  do {
    CString sTmp;
    CString sKey;
    sKey.Format("Music%d", m_nGameMusicFiles + 1);
    FileHelpers::GetKeyStringFromINIFile("Game",        // section name
                                         sKey,        // key name
                                         "<default>",     // default string
                                         sTmp, // destination buffer
                                         ".\\Playlist.dat");      // initialization file name
    if(sTmp.CompareNoCase("<default>") == 0) {
      break;
    } else {
      ++m_nGameMusicFiles;
    }
  } while(true);

  if(m_nGameMusicFiles == 0) {
    m_nGameMusicFiles = 1;
  }
}

void SoundModule::FreeSound(FSOUND_SAMPLE **pSound) {
  if(*pSound) {
    FSOUND_Sample_Free(*pSound);
    *pSound = 0;
  }
}

void SoundModule::Close() {
  FreeSound(&m_pMenuMusic);
  FreeSound(&m_pVehicleSounds);
  FreeSound(&m_pSkidSound);
  FreeSound(&m_pGameMusic1);
  // FreeSound(&m_pGameMusic2);
  FreeSound(&m_pMessageSound);
  FreeSound(&m_pCrashSound);
  FreeSound(&m_pMenuBrowseSound);
  FreeSound(&m_pMenuScrollSound);
  FreeSound(&m_pMenuBackSound);
  FreeSound(&m_pSlalomPortSound);
  FreeSound(&m_pDisqualifiedSound);
  FreeSound(&m_pCountdown123Sound);
  FreeSound(&m_pCountdownGoSound);
  FreeSound(&m_pMultiplayerJoinSound);
  FreeSound(&m_pMultiplayerLeftSound);
  FreeSound(&m_pGoalFanfarSound);
  FreeSound(&m_pHeliSound);
  FSOUND_Close();
  m_bRunning = false;
}

void SoundModule::SetMenuMusicVolume(int nVol) {
  m_nMenuMusicVolume = nVol;
  if(m_chaMenuMusic && m_bRunning) {
    FSOUND_SetVolume(m_chaMenuMusic, m_nMenuMusicVolume);
  }
}

void SoundModule::SetVehicleSoundsVolume(int nVol) {
  m_nVehicleSoundsVolume = nVol;
  if(m_chaVehicleSounds && m_bRunning) {
    FSOUND_SetVolume(m_chaVehicleSounds, m_nVehicleSoundsVolume);
  }
}

void SoundModule::SetSkidSoundVolume(int nVol) {
  m_nSkidSoundVolume = nVol;
  if(m_chaSkidSound && m_bRunning) {
    FSOUND_SetVolume(m_chaSkidSound, int(double(m_nSkidSoundVolume) * double(m_nVehicleSoundsVolume) / 255.0));
  }
}

void SoundModule::SetGameMusicVolume(int nVol) {
  m_nGameMusicVolume = nVol;
  if(m_chaGameMusic1 && m_bRunning) {
    FSOUND_SetVolume(m_chaGameMusic1, m_nGameMusicVolume);
  }
}

void SoundModule::SetCrashSoundVolume(int cha, int nVol) {
  int nNewVol = int(double(nVol) * double(m_nVehicleSoundsVolume) / 255.0);
  if(nNewVol > 255) {
    nNewVol = 255;
  }
  FSOUND_SetVolume(cha, nNewVol);
}


void SoundModule::SetHeliSoundVolume(int nVol) {
  m_nHeliSoundVolume = nVol;
  if(m_chaHeliSound && m_bRunning) {
    int nNewVol = int(double(m_nHeliSoundVolume) * double(m_nVehicleSoundsVolume) / 255.0);
    if(nNewVol > 255) {
      nNewVol = 255;
    }
    FSOUND_SetVolume(m_chaHeliSound, nNewVol);
  }
}

void SoundModule::SetJetSoundVolume(int nVol) {
  m_nJetSoundVolume = nVol;
  if(m_chaJetSound && m_bRunning) {
    int nNewVol = int(double(m_nJetSoundVolume) * double(m_nVehicleSoundsVolume) / 255.0);
    if(nNewVol > 255) {
      nNewVol = 255;
    }
    FSOUND_SetVolume(m_chaJetSound, nNewVol);
  }
}

void SoundModule::SetMessageSoundVolume(int nVol) {
  m_nMessageSoundVolume = nVol;
  if(m_chaMessageSound && m_bRunning) {
    FSOUND_SetVolume(m_chaMessageSound, m_nMessageSoundVolume);
  }
}


void SoundModule::SetSoundSpace(int nSpace) {
  m_nSpace = nSpace;
}

void SoundModule::SetOutput(int nOutput) {
  switch(nOutput) {
    case 0: 
      FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND);
    break;
    case 1: 
      FSOUND_SetOutput(FSOUND_OUTPUT_WINMM);
    break;
    case 2: 
      FSOUND_SetOutput(FSOUND_OUTPUT_A3D);
    break;
    case 3:  
      FSOUND_SetOutput(FSOUND_OUTPUT_NOSOUND);
    break;
  }
}

int SoundModule::GetOutput() {
  int i = FSOUND_GetOutput();
  switch(i) {
    case FSOUND_OUTPUT_DSOUND:
      return 0;
      break;
    case FSOUND_OUTPUT_WINMM:
      return 1;
      break;
    case FSOUND_OUTPUT_A3D:
      return 2;
      break;
    case FSOUND_OUTPUT_NOSOUND:
      return 3;
      break;
    default:
      return -1;
      break;
  }
}

void SoundModule::SetDriver(int nDriver) {
  FSOUND_SetDriver(nDriver);
}

int SoundModule::GetNumDrivers() {
  return FSOUND_GetNumDrivers();
}

char* SoundModule::GetDriverName(int nDriver) {
  if(nDriver >= 0) {
    return (char*) FSOUND_GetDriverName(nDriver);
  } else {
    return (char*) FSOUND_GetDriverName(FSOUND_GetDriver());
  }
}



bool bMenuMusicPaused = false;
bool bGameMusicPaused = false;

void SoundModule::StartMenuMusic() {
  if(m_bRunning) {
    if(!m_pMenuMusic) {

      CString sMenuMusicFile = _T("p!2_-_01_-_menumusic.mp3");
      CString sTmp;
      FileHelpers::GetKeyStringFromINIFile("Menu",        // section name
                                           "Music",        // key name
                                           "<default>",     // default string
                                           sTmp, // destination buffer
                                           ".\\Playlist.dat");      // initialization file name
      if(sTmp.CompareNoCase("<default>") != 0) {
        sMenuMusicFile = sTmp;
      }

      m_pMenuMusic = FSOUND_Sample_Load(FSOUND_FREE, LPCTSTR(sMenuMusicFile), FSOUND_LOOP_NORMAL | FSOUND_MPEGACCURATE, 0, 0);
    }
    if(m_pMenuMusic) {
      static bool bPlaying = false;
      if(!bPlaying) {
        m_chaMenuMusic = FSOUND_PlaySound(FSOUND_FREE, m_pMenuMusic);
        FSOUND_SetVolume(m_chaMenuMusic, m_nMenuMusicVolume);
        bPlaying = true;
        bMenuMusicPaused = false;
      }

      if(bMenuMusicPaused) {
        FSOUND_SetPaused(m_chaMenuMusic, FALSE);
        bMenuMusicPaused = false;
      }
    }
  }
}

void SoundModule::StopMenuMusic(bool bFinal) {
  if(m_bRunning) {
    if(bFinal) {
      FSOUND_StopSound(m_chaMenuMusic);
      m_chaMenuMusic = 0;
    } else {
      if(!bMenuMusicPaused) {
        FSOUND_SetPaused(m_chaMenuMusic, TRUE);
        bMenuMusicPaused = true;
      }
    }
  }
}


void SoundModule::StartSkidSound() {
  return;
  if(m_bRunning) {
    if(!m_pSkidSound) {
      m_pSkidSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\WhiteNoise.mp3", FSOUND_LOOP_NORMAL, 0, 0);
    }
    if(m_pSkidSound) {
      m_chaSkidSound = FSOUND_PlaySound(FSOUND_FREE, m_pSkidSound);
      FSOUND_SetVolume(m_chaSkidSound, m_nSkidSoundVolume);
    }
  }
}

void SoundModule::StopSkidSound() {
  return;
  if(m_bRunning) {
    FSOUND_StopSound(m_chaSkidSound);
  }
  m_chaSkidSound = 0;
}

void SoundModule::StartGameMusic() {

  static int nMusic = 0;

  if(m_bRunning) {
    CString sGameMusicFile = _T("p!2_-_02_-_ingame01.mp3");
    CString sTmp;
    CString sKey;
    sKey.Format("Music%d", nMusic + 1);
    FileHelpers::GetKeyStringFromINIFile("Game",        // section name
                                         sKey,        // key name
                                         "<default>",     // default string
                                         sTmp, // destination buffer
                                         ".\\Playlist.dat");      // initialization file name
    if(sTmp.CompareNoCase("<default>") != 0) {
      sGameMusicFile = sTmp;
    }
    if(m_pGameMusic1) {
      FreeSound(&m_pGameMusic1);
    }
    m_pGameMusic1 = FSOUND_Sample_Load(FSOUND_FREE, LPCTSTR(sGameMusicFile), FSOUND_LOOP_NORMAL, 0, 0);

    ++nMusic;
    nMusic = nMusic % m_nGameMusicFiles;

    if(m_pGameMusic1) {
      m_chaGameMusic1 = FSOUND_PlaySound(FSOUND_FREE, m_pGameMusic1);
      FSOUND_SetVolume(m_chaGameMusic1, m_nMenuMusicVolume);
    }

    if(bGameMusicPaused) {
      FSOUND_SetPaused(m_chaGameMusic1, FALSE);
      bGameMusicPaused = false;
    }
  }
}

void SoundModule::StopGameMusic(bool bFinal) {
  if(m_bRunning) {
    if(bFinal) {
      FSOUND_StopSound(m_chaGameMusic1);
      m_chaGameMusic1 = 0;
    } else {
      if(!bGameMusicPaused) {
        FSOUND_SetPaused(m_chaGameMusic1, TRUE);
        bGameMusicPaused = true;
      }
    }
  }
}

void SoundModule::SetGameMusicRPM(int nRPM) {
  return;
  static int nFluctuation = 0;
  nFluctuation += rand() % 200 - 99;
  if(abs(nFluctuation) > 999) {
    nFluctuation = 999 * nFluctuation / 1000;
  }
  // FSOUND_SetFrequency(m_chaGameMusic, 22050 + nRPM + nFluctuation);
  // double dRatio = (double(nRPM) / 44100.0);
  // FSOUND_SetVolume(m_chaGameMusic1, int((1.0 - dRatio) * double(m_nGameMusicVolume) * double(m_nVehicleSoundsVolume) / 255.0));
  // FSOUND_SetVolume(m_chaGameMusic2, int(dRatio * double(m_nGameMusicVolume) * double(m_nVehicleSoundsVolume) / 255.0));
  // FSOUND_SetFrequency(m_chaGameMusic1, 44100 + int(double(nRPM) / 1.5) + nFluctuation / 2);
  FSOUND_SetFrequency(m_chaGameMusic1, 38000 + int(double(nRPM) * 0.8) + nFluctuation / 2);
  // FSOUND_SetFrequency(m_chaGameMusic1, 22050 + nRPM + nFluctuation / 2);
  // FSOUND_SetFrequency(m_chaGameMusic2, -44100 + 44100 + nRPM + nFluctuation / 2);
}


void SoundModule::StartMessageSound() {
  if(m_bRunning) {
    if(!m_pMessageSound) {
      m_pMessageSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\IncomingMessage.wav", FSOUND_LOOP_NORMAL, 0, 0);
    }
    if(m_pMessageSound) {
      m_chaMessageSound = FSOUND_PlaySound(FSOUND_FREE, m_pMessageSound);
      SetMessageSoundVolume(128);
    }
  }
}

void SoundModule::StopMessageSound() {
  if(m_bRunning) {
    FSOUND_StopSound(m_chaMessageSound);
  }
  m_chaMessageSound = 0;
}


void SoundModule::PreCacheIntroSound() {
  if(m_bRunning) {
    if(!m_pIntroSound) {
      m_pIntroSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\AtlantisOwesMeMoney.mp3", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pIntroSound) {
      m_chaIntroSound = FSOUND_PlaySound(FSOUND_FREE, m_pIntroSound);
      FSOUND_SetVolume(m_chaIntroSound, 0);
    }
  }
}

void SoundModule::StartIntroSound() {
  if(m_bRunning) {
    if(!m_pIntroSound) {
      m_pIntroSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\AtlantisOwesMeMoney.mp3", FSOUND_LOOP_OFF | FSOUND_MPEGACCURATE, 0, 0);
    }
    if(m_pIntroSound) {
      m_chaIntroSound = FSOUND_PlaySound(FSOUND_FREE, m_pIntroSound);
      FSOUND_SetVolume(m_chaIntroSound, 255);
    }
  }
}

void SoundModule::StopIntroSound() {
  if(m_bRunning) {
    FSOUND_StopSound(m_chaIntroSound);
    FreeSound(&m_pIntroSound);
  }
  m_chaIntroSound = 0;
}


void SoundModule::StartHeliSound() {
  if(m_bRunning) {
    if(!m_pHeliSound) {
      m_pHeliSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\Huey.wav", FSOUND_LOOP_NORMAL | FSOUND_HW3D, 0, 0);
    }
    if(m_pHeliSound) {
      m_chaHeliSound = FSOUND_PlaySound(FSOUND_FREE, m_pHeliSound);
      SetHeliSoundVolume(m_nHeliSoundVolume);
    }
  }
}

void SoundModule::StopHeliSound() {
  if(m_bRunning) {
    FSOUND_StopSound(m_chaHeliSound);
  }
  m_chaHeliSound = 0;
}

void SoundModule::SetHeliSoundPhase(double dPhase, double dBladePower) {
  SetHeliSoundVolume(int(dPhase * 255.0 * dBladePower));
  FSOUND_SetFrequency(m_chaHeliSound, int(44100.0 * dPhase));
}


void SoundModule::StartJetSound() {
  if(m_bRunning) {
    if(!m_pJetSound) {
      m_pJetSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\JetMono.wav", FSOUND_LOOP_NORMAL | FSOUND_HW3D, 0, 0);
    }
    if(m_pJetSound) {
      m_chaJetSound = FSOUND_PlaySound(FSOUND_FREE, m_pJetSound);
      SetJetSoundVolume(255);
    }
  }
}

void SoundModule::StopJetSound() {
  if(m_bRunning) {
    FSOUND_StopSound(m_chaJetSound);
  }
  m_chaJetSound = 0;
}

void SoundModule::SetJetSoundPhase(double dPhase) {
  // SetJetSoundVolume(128 + int(dPhase * 127.0));
  FSOUND_SetFrequency(m_chaJetSound, 44100 + int(20000.0 * dPhase));
}



void SoundModule::PlayCrashSound(double dVolume) {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02) && 
     (dVolume > 0.1)) {
    if(dVolume > 1.0) {
      dVolume = 1.0;
    }
    clockPrev = clockNow;
    if(!m_pCrashSound) {
      m_pCrashSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\crash.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pCrashSound) {
      m_chaCrashSound = FSOUND_PlaySound(FSOUND_FREE, m_pCrashSound);
      FSOUND_SetFrequency(m_chaCrashSound, 22050 + rand() % 10000);
      SetCrashSoundVolume(m_chaCrashSound, 55 + int(dVolume * 200.0));
    }
  }
}

void SoundModule::PlayMenuBrowseSound() {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02)) {
    clockPrev = clockNow;
    if(!m_pMenuBrowseSound) {
      m_pMenuBrowseSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\menu-scroll.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pMenuBrowseSound) {
      m_chaMenuBrowseSound = FSOUND_PlaySound(FSOUND_FREE, m_pMenuBrowseSound);
      SetCrashSoundVolume(m_chaMenuBrowseSound, m_nVehicleSoundsVolume);
    }
  }
}

void SoundModule::PlayMenuScrollSound() {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02)) {
    clockPrev = clockNow;
    if(!m_pMenuScrollSound) {
      m_pMenuScrollSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\menu-change_setting.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pMenuScrollSound) {
      m_chaMenuScrollSound = FSOUND_PlaySound(FSOUND_FREE, m_pMenuScrollSound);
      SetCrashSoundVolume(m_chaMenuScrollSound, m_nVehicleSoundsVolume);
    }
  }
}

void SoundModule::PlayMenuBackSound() {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02)) {
    clockPrev = clockNow;
    if(!m_pMenuBackSound) {
      m_pMenuBackSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\menu-back.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pMenuBackSound) {
      m_chaMenuBackSound = FSOUND_PlaySound(FSOUND_FREE, m_pMenuBackSound);
      SetCrashSoundVolume(m_chaMenuBackSound, m_nVehicleSoundsVolume);
    }
  }
}

void SoundModule::PlaySlalomPortSound() {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02)) {
    clockPrev = clockNow;
    if(!m_pSlalomPortSound) {
      m_pSlalomPortSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\player-slalom_portsuccess.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pSlalomPortSound) {
      m_chaSlalomPortSound = FSOUND_PlaySound(FSOUND_FREE, m_pSlalomPortSound);
      SetCrashSoundVolume(m_chaSlalomPortSound, m_nVehicleSoundsVolume);
    }
  }
}

void SoundModule::PlayDisqualifiedSound() {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02)) {
    clockPrev = clockNow;
    if(!m_pDisqualifiedSound) {
      m_pDisqualifiedSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\player-disqualify.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pDisqualifiedSound) {
      m_chaDisqualifiedSound = FSOUND_PlaySound(FSOUND_FREE, m_pDisqualifiedSound);
      SetCrashSoundVolume(m_chaDisqualifiedSound, m_nVehicleSoundsVolume);
    }
  }
}

void SoundModule::PlayCountdown123Sound() {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02)) {
    clockPrev = clockNow;
    if(!m_pCountdown123Sound) {
      m_pCountdown123Sound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\player-countdown-1.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pCountdown123Sound) {
      m_chaCountdown123Sound = FSOUND_PlaySound(FSOUND_FREE, m_pCountdown123Sound);
      SetCrashSoundVolume(m_chaCountdown123Sound, m_nVehicleSoundsVolume);
    }
  }
}

void SoundModule::PlayCountdownGoSound() {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02)) {
    clockPrev = clockNow;
    if(!m_pCountdownGoSound) {
      m_pCountdownGoSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\player-countdown-go.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pCountdownGoSound) {
      m_chaCountdownGoSound = FSOUND_PlaySound(FSOUND_FREE, m_pCountdownGoSound);
      SetCrashSoundVolume(m_chaCountdownGoSound, m_nVehicleSoundsVolume);
    }
  }
}

void SoundModule::PlayMultiplayerJoinSound() {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02)) {
    clockPrev = clockNow;
    if(!m_pMultiplayerJoinSound) {
      m_pMultiplayerJoinSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\multiplayer-joined.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pMultiplayerJoinSound) {
      m_chaMultiplayerJoinSound = FSOUND_PlaySound(FSOUND_FREE, m_pMultiplayerJoinSound);
      SetCrashSoundVolume(m_chaMultiplayerJoinSound, m_nVehicleSoundsVolume);
    }
  }
}

void SoundModule::PlayMultiplayerLeftSound() {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02)) {
    clockPrev = clockNow;
    if(!m_pMultiplayerLeftSound) {
      m_pMultiplayerLeftSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\multiplayer-left.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pMultiplayerLeftSound) {
      m_chaMultiplayerLeftSound = FSOUND_PlaySound(FSOUND_FREE, m_pMultiplayerLeftSound);
      SetCrashSoundVolume(m_chaMultiplayerLeftSound, m_nVehicleSoundsVolume);
    }
  }
}

void SoundModule::PlayGoalFanfarSound() {
  static clock_t clockPrev = clock();
  clock_t clockNow = clock();
  if(m_bRunning && 
     ((double(clockNow - clockPrev) / double(CLOCKS_PER_SEC)) > 0.02)) {
    clockPrev = clockNow;
    if(!m_pGoalFanfarSound) {
      m_pGoalFanfarSound = FSOUND_Sample_Load(FSOUND_FREE, ".\\Sounds\\GoalFanfar.wav", FSOUND_LOOP_OFF, 0, 0);
    }
    if(m_pGoalFanfarSound) {
      m_chaGoalFanfarSound = FSOUND_PlaySound(FSOUND_FREE, m_pGoalFanfarSound);
      SetCrashSoundVolume(m_chaGoalFanfarSound, m_nVehicleSoundsVolume);
    }
  }
}




void SoundModule::Update3DSounds(BVector& rvSouLoc, 
                                 BVector& rvSouVel, 
                                 BVector& rvLisLoc, 
                                 BOrientation& roLis, 
                                 BVector& rvLisVel) {
  static float fSouLoc[3];
  static float fSouVel[3];
  static float fLisLoc[3];
  static float fLisVel[3];
  if(m_nSpace == 1) {
    // update 3D sound location
    fSouLoc[0] = float(rvSouLoc.m_dX);
    fSouLoc[1] = float(-rvSouLoc.m_dZ);
    fSouLoc[2] = float(rvSouLoc.m_dY);
    fSouVel[0] = float(rvSouVel.m_dX);
    fSouVel[1] = float(-rvSouVel.m_dZ);
    fSouVel[2] = float(rvSouVel.m_dY);

    FSOUND_3D_SetAttributes(
      m_chaGameMusic1, 
      fSouLoc, 
      fSouVel);
    /*
    FSOUND_3D_SetAttributes(
      m_chaGameMusic2, 
      fSouLoc, 
      fSouVel);
    */
    if(m_chaCrashSound != 0) {
      FSOUND_3D_SetAttributes(
        m_chaCrashSound, 
        fSouLoc, 
        fSouVel);
    }
    if(m_chaHeliSound != 0) {
      FSOUND_3D_SetAttributes(
        m_chaHeliSound, 
        fSouLoc, 
        fSouVel);
    }
    if(m_chaJetSound != 0) {
      FSOUND_3D_SetAttributes(
        m_chaJetSound, 
        fSouLoc, 
        fSouVel);
    }

    fLisLoc[0] = float(rvLisLoc.m_dX);
    fLisLoc[1] = float(-rvLisLoc.m_dZ);
    fLisLoc[2] = float(rvLisLoc.m_dY);
    fLisVel[0] = float(rvLisVel.m_dX);
    fLisVel[1] = float(-rvLisVel.m_dZ);
    fLisVel[2] = float(rvLisVel.m_dY);

    FSOUND_3D_Listener_SetAttributes(
      fLisLoc,
      fLisVel,
      float(roLis.m_vForward.m_dX),
      float(-roLis.m_vForward.m_dZ),
      float(roLis.m_vForward.m_dY),
      float(roLis.m_vUp.m_dX),
      float(-roLis.m_vUp.m_dZ),
      float(roLis.m_vUp.m_dY));

    // FSOUND_3D_Update();
  }
}



// PAKOON! Game, Source Code and Developer Package Copyright
// =========================================================
// 
// Restrictions related to PAKOON! Game and it's use
// -------------------------------------------------
// 
// You may download and play the PAKOON! game for free. You may also copy it freely to your friends and relatives as long as you 
// provide the original setup package (downloaded from www.nic.fi/~moxide) and the copyright included in it is also given. You 
// may also use the PAKOON! game for educational purposes, as long as the origin of the PAKOON! game (i.e. www.nic.fi/~moxide) 
// is mentioned and this copyright is also provided and the creator of the game (i.e. Mikko Oksalahti, email: 
// mikko.oksalahti@nic.fi) is notified of the use in advance.
// You may not sell or otherwise accept any payment for giving or offering the game to someone else. You may not offer the 
// PAKOON! game for free on any webpage, CD, DVD or other media without a written permission from the creator of the PAKOON! 
// game (i.e. Mikko Oksalahti, email: mikko.oksalahti@nic.fi).
// You may freely include a link to PAKOON! homepage (i.e. www.nic.fi/~moxide) from your own site.
// 
// 
// Restrictions related to PAKOON! Game
// Source Code and Developer Package and their use
// -----------------------------------------------
// 
// You may download and use the PAKOON! game source code for personal use. You may not use any part of the source code or the 
// developer package on any commercial or free game or other computer program intended for public distribution without a written 
// permission from the creator of the PAKOON! game (i.e. Mikko Oksalahti, email: mikko.oksalahti@nic.fi). You may use the 
// PAKOON! game source code or developer package for educational purposes, as long as the origin of the PAKOON! game (i.e. 
// www.nic.fi/~moxide) is mentioned and this copyright is also provided and the creator of the game (i.e. Mikko Oksalahti, 
// email: mikko.oksalahti@nic.fi) is notified of the use in advance.
// You may not sell or otherwise accept any payment for giving or offering the PAKOON! game source code or developer package to 
// someone else. You may not offer the PAKOON! game source code or developer package for free on any webpage, CD, DVD or other 
// media without a written permission from the creator of the PAKOON! game (i.e. Mikko Oksalahti, email: 
// mikko.oksalahti@nic.fi).
// 
// 
// Mikko Oksalahti
// Helsinki, FINLAND
// 7.10.2002