//
// BTerrain.h
// 
// Classes for the terrain model of Pakoon! v1.0
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "BObject.h" 
#include "stdio.h"

#include <string>

using namespace std;

const int    cnBlockTiles    = 64;
const double cdBlockTileSize = 5.0; // was 6.0
const double cdWorldHemisphereRadius = 3000.0;
const int    cnRequiredRenderPasses = 1;
const int    cnMaxObjectsInOneBlock = 10; 



class BTerrainTile {
public:
  BVector m_vNormal1;
  BVector m_vNormal2;
  double  m_dFriction;
};

class BTerrain;

//*****************************************************************************
class BTerrainBlock {

  // BTerrainBlock represents a 2D height map in world coordinates. 
  // Coordinate (x, y) is inside this block if the following is true:
  //    (x >= m_vCorner.m_dX) && (x <= m_vCorner.m_dX + m_dSize) &&
  //    (y >= m_vCorner.m_dY) && (y <= m_vCorner.m_dY + m_dSize)
  // In this case the height for the coordinate (x, y) can be found from:
  //    ***888 CHECK!!!***

public:
  BTerrain     *m_pTerrain;           // Pointer to "Parent"
  BVector       m_vCenter;            // Center of the block in world coordinates
  BVector       m_vCorner;            // World coordinates of the upperleft corner
  double        m_dSize;              // Size of the block (Y and Y size, in world units; 1=1m)
  double        m_dTileSize;          // Size of one tile in world coordinates
  int           m_nStep;              // Step between display list res coordinates in the array
  int           m_nSize;              // Number of coordinates in X/Y direction
  int           m_nMaxRes;            // The best resolution available (i.e. current calculated resolution)
  int           m_nDisplayListRes;    // Resolution for which the current display list is calculated
  bool          m_bDisplayListValid;  // Has a valid display list?
  int           m_nDisplayLists;      // Number of active display lists
  int           m_nDisplayListBase;   // Base of the display lists, for all passes (May be invalid. See m_bDisplayListValid above)
  bool          m_bVisible;           // Is currently visible?
  double       *m_pdHeights;          // An array holding the height for each coordinate in the block
  BTerrainTile *m_pTiles;             // An array holding the normals etc. for each tile
  int           m_nHeights;           // Number of height values in the array
  int           m_nTriangles;         // Number of triangles in the display list
  int           m_nObjects;           // number of objects in this block's area
  BObject      *m_objectArray[cnMaxObjectsInOneBlock]; // Objects in this block's area

public:

  // constructor and destructor
  BTerrainBlock();
  ~BTerrainBlock();

  // linked ring
  BTerrainBlock *m_pNext;       // Points to next block in the ring
  BTerrainBlock *m_pPrev;       // Points to prev block in the ring

  // public methods
  void    SetTextureCoordinate(BVector vPoint);
  // Sets the correct texture coordinate for the given point. Uses multitexturing if supported.
  void    LoadTerrainBlockCache(FILE *fp);    
  // Loads the cached terrain data from file to speed up rendering.
  void    SaveTerrainBlockCache(FILE *fp);    
  // Saves the current terrain data to a cache file to speed up rendering.
  void    SetCenter(BVector v);               
  // Sets the center of the block in world coordinates
  BVector GetCenter();                        
  // Returns the center of the block in world coordinates
  double  HeightAt(int nX, int nY, bool bIgnore = false, double dTileSize = 1.0);           
  // Return height at the given location
  BVector GetNormalAtTile(int nX, int nY, int nRes); 
  // Returns the normal at the given resolution-specific point
  bool    PointIsInsideBlock(BVector vLoc, double dMargin = 0.0);   
  // True if the given point is inside the block
  bool    IsVisible(BVector vOffset, BVector vFrom, BVector vLookingTowards, double dAngleOfView); 
  // True if part of the block is visible from the given location
  bool    IsAtLeastOfRes(int nRes);           
  // True if block has data for at least of given resolution
  void    CalculateWithRes(int nRes, BVector vOffset); 
  // Calculates data for the given resolution (invalidates display list)
  bool    HasValidDisplayList(int nRes);      
  // True if block has display list for the given resolution
  int     GetDisplayRes();                    
  // Returns the current resolution of the active display lists
  void    CreateDisplayListsOfRes(int nRes, bool bOverride, bool bWireframe, bool bNormals);  
  // Creates display lists for the given resolution
  void    CreateTileData(int nRes);
  // Creates tile information (normals and friction for collision detection)
  int     Render(int nPass);                  
  // Renders the given pass display list (different textures are rendered in different passes). Returns the number of triangles rendered.
  void    Destroy();                          
  // Cleans up the block for deletion
  void    DestroyDisplayLists();              
  // Cleans up the display lists of the block
};





//*****************************************************************************
class BBlockRing {
  BTerrainBlock *m_pFirstBlock;    // Pointer to the first block (i.e. center block) in the block list. Static so that TerrainBlock objetcs can access it also.
  int m_nBlocks;
public:
  BBlockRing();
  ~BBlockRing();
  int            GetNBlocks() {return m_nBlocks;}
  BTerrainBlock *GetHead();
  BTerrainBlock *GetTail();
  BTerrainBlock *SetHead(BTerrainBlock *pNewHead);
  BTerrainBlock *Remove(BTerrainBlock *pBlock);
  BTerrainBlock *AddTail(BTerrainBlock *pNewBlock);
  BTerrainBlock *AddFront(BTerrainBlock *pNewBlock);
  void           RemoveAll();
};





//*****************************************************************************
class BTerrain {
private:
  static int            m_nRes1; // Rendering resolutions for different distances
  static int            m_nRes2;
  static int            m_nRes3;
  static int            m_nRes4;
  static int            m_nRes5;
  static int            m_nRes6;

  BTerrainBlock        *m_pPrevCenter;    // Pointer to the previous center block (used for checking if a new area is entered)
public:
  BVector               m_vOffset;        // Offset for getting the drawing around origin
  BBlockRing            m_ringVisible;    // Ring of visible terrain blocks
  BVector               m_vCameraLoc;     // Current location for the camera     
  BVector               m_vCameraLookDir; // Current looking direction for the camera     
  double                m_dBlockSize;     // Size of individual blocks in world units

  string               m_sSceneName;
  static GLfloat        m_fGroundTextureScaler1;
  static GLfloat        m_fGroundTextureScaler2;

  static bool m_bOverride;

  // constructor and destructor
  BTerrain();
  ~BTerrain();

  // public methods etc.

  enum RendRes{RENDER_SLOW_MACHINE, RENDER_MINIMUM, RENDER_LOW, RENDER_MEDIUM, RENDER_HIGH, RENDER_MAXIMUM};
  void SetRenderResolution(RendRes renderResolution);
  // Selects the given resolution for rendering
  void PreProcessVisualization();
  // Creates texture objetcs.
  void LoadTerrainCache(string sFilenamePrefix);
  // Creates initial terrain cache
  static void    GetColorForHeight(double dHeight, double &rdR, double &rdG, double &rdB); 
  // Returns the terrain color for the given height
  void           MakeTerrainValid(BVector vLocCar, BVector vLocCamera, BVector vLookingTowards, bool bCreateDLs, bool bWireframe, bool bNormals); 
  // Calculates terrain blocks and validates their display lists (if necessary)
  int            Render(int nSkyDetail, BVector vCamLoc, BVector vLookingTo, BTerrainBlock *pOnlyThisBlock = 0);
  // Draws the terrain. Returns the number of triangles rendered
  BTerrainBlock *FindCenterBlock(BVector vLoc);           
  // Returns the (center) block based on the given location (if none exists, one is created)
  BVector        GetCenterForRelLoc(BVector vLoc, int nRelXFromLoc, int nRelYFromLoc); 
  // Returns the world coordinates for the relative block
  BTerrainBlock *ExistsBlockForLocation(int nRelXFromFirst, int nRelYFromFirst, BBlockRing *pRing); 
  // True if there's a block at the given relative location. Returns the block
  BTerrainBlock *CreateBlockAt(BVector vLoc, int nRelXFromLoc, int nRelYFromLoc); 
  // Creates a non-valid block at the given relative location. Returns the created block
  BTerrainBlock *CreateBlockAtAbs(BVector vLoc, BBlockRing *pRing);
  // Creates a non-valid block at the given absolute location to the given ring. Returns the created block
  BTerrainBlock *DeleteVisibleBlock(BTerrainBlock *pBlock);
  // Removes the block from the visible ting and moves it to the cache ring. Returns next visible block
  static int     GetResBasedOnDistance(double dDistance); 
  // Returns the configured resolution for the given distance. Static so that TerrainBlock objects can access it also.

  void           RenderAlphaCircle(BVector vCenter, double dTileSize, double dRadius);

  void           StartUsingScene(string sSceneName, BVector vOrigin, double dGroundTextureScaler1, double dGroundTextureScaler2);
  void           CreateTerrainDisplayLists();
  void           StopUsingScene();
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
