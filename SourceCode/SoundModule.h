//
// All access sound module for all users of the game
// 
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "fmod.h"
#include "BaseClasses.h"

class SoundModule {
  static bool m_bRunning;

  static int m_chaMenuMusic;
  static FSOUND_SAMPLE *m_pMenuMusic;
  static int m_chaVehicleSounds;
  static FSOUND_SAMPLE *m_pVehicleSounds;
  static int m_chaSkidSound;
  static FSOUND_SAMPLE *m_pSkidSound;
  static int m_chaGameMusic1;
  static int m_chaGameMusic2;
  static FSOUND_SAMPLE *m_pGameMusic1;
  static FSOUND_SAMPLE *m_pGameMusic2;
  static int m_chaMessageSound;
  static FSOUND_SAMPLE *m_pMessageSound;

  static int m_chaCrashSound;
  static FSOUND_SAMPLE *m_pCrashSound;

  static int m_chaMenuBrowseSound;
  static FSOUND_SAMPLE *m_pMenuBrowseSound;

  static int m_chaMenuScrollSound;
  static FSOUND_SAMPLE *m_pMenuScrollSound;

  static int m_chaMenuBackSound;
  static FSOUND_SAMPLE *m_pMenuBackSound;

  static int m_chaSlalomPortSound;
  static FSOUND_SAMPLE *m_pSlalomPortSound;

  static int m_chaDisqualifiedSound;
  static FSOUND_SAMPLE *m_pDisqualifiedSound;

  static int m_chaCountdown123Sound;
  static FSOUND_SAMPLE *m_pCountdown123Sound;

  static int m_chaCountdownGoSound;
  static FSOUND_SAMPLE *m_pCountdownGoSound;

  static int m_chaMultiplayerJoinSound;
  static FSOUND_SAMPLE *m_pMultiplayerJoinSound;

  static int m_chaMultiplayerLeftSound;
  static FSOUND_SAMPLE *m_pMultiplayerLeftSound;

  static int m_chaGoalFanfarSound;
  static FSOUND_SAMPLE *m_pGoalFanfarSound;

  static int m_chaHeliSound;
  static FSOUND_SAMPLE *m_pHeliSound;
  static int m_chaJetSound;
  static FSOUND_SAMPLE *m_pJetSound;
  static int m_chaIntroSound;
  static FSOUND_SAMPLE *m_pIntroSound;

  static int m_nMenuMusicVolume;
  static int m_nVehicleSoundsVolume;
  static int m_nSkidSoundVolume;
  static int m_nGameMusicVolume;
  static int m_nMessageSoundVolume;
  static int m_nHeliSoundVolume;
  static int m_nJetSoundVolume;
  static int m_nSpace;

  static int m_nGameMusicFiles;

  static void FreeSound(FSOUND_SAMPLE **pSound);

public:

  SoundModule();

  static int  GetMenuMusicVolume() {return m_nMenuMusicVolume;}
  static void SetMenuMusicVolume(int nVol);
  static int  GetVehicleSoundsVolume() {return m_nVehicleSoundsVolume;}
  static void SetVehicleSoundsVolume(int nVol);

  static int  GetSkidSoundVolume() {return m_nSkidSoundVolume;}
  static void SetSkidSoundVolume(int nVol);

  static int  GetGameMusicVolume() {return m_nGameMusicVolume;}
  static void SetGameMusicVolume(int nVol);
  static void SetGameMusicRPM(int nRPM);

  static int  GetMessageSoundVolume() {return m_nMessageSoundVolume;}
  static void SetMessageSoundVolume(int nVol);

  static int  GetSoundSpace() {return m_nSpace;}
  static void SetSoundSpace(int nSpace);

  static void SetOutput(int nOutput);
  static int  GetOutput();
  static void SetDriver(int nDriver);

  static int   GetNumDrivers();
  static char* GetDriverName(int nDriver);

  static void StartMenuMusic();
  static void StopMenuMusic(bool bFinal = false);

  static void StartSkidSound();
  static void StopSkidSound();

  static void StartGameMusic();
  static void StopGameMusic(bool bFinal = false);

  static void StartMessageSound();
  static void StopMessageSound();

  static void PreCacheIntroSound();
  static void StartIntroSound();
  static void StopIntroSound();

  static void StartHeliSound();
  static void SetHeliSoundVolume(int nVol);
  static void SetHeliSoundPhase(double dPhase, double dBladePower);
  static void StopHeliSound();

  static void StartJetSound();
  static void SetJetSoundVolume(int nVol);
  static void SetJetSoundPhase(double dPhase);
  static void StopJetSound();

  static void PlayCrashSound(double dVolume);
  static void SetCrashSoundVolume(int cha, int nVol);

  static void PlayMenuBrowseSound();
  static void PlayMenuScrollSound();
  static void PlayMenuBackSound();
  static void PlaySlalomPortSound();
  static void PlayDisqualifiedSound();
  static void PlayCountdown123Sound();
  static void PlayCountdownGoSound();
  static void PlayMultiplayerJoinSound();
  static void PlayMultiplayerLeftSound();
  static void PlayGoalFanfarSound();

  static void Initialize();
  static void Close();

  static void Update3DSounds(BVector& rvSouLoc, 
                             BVector& rvSouVel, 
                             BVector& rvLisLoc, 
                             BOrientation& roLis, 
                             BVector& rvLisVel);
};



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