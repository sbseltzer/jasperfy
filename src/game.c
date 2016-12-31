//! Includes
#include "game.h"


//! Variables
orxOBJECT* pstPlayer;
orxOBJECT* pstPlayerGun;
orxVECTOR vFlipLeft = { -2, 2, 1 };
orxVECTOR vFlipRight = { 2, 2, 1 };
orxOBJECT* pstScoreObject;
orxS16 score = 0;
orxOBJECT* pstScene;

//! Code

void AddScore(int increase)
{
  score += increase;

  orxCHAR formattedScore[6];
  orxString_Print(formattedScore, "%d", score);

  orxObject_SetTextString(pstScoreObject, formattedScore);
}

void CreateExplosionAtObject(orxOBJECT *object, orxSTRING exploderObjectName)
{
  orxLOG("explode %s", exploderObjectName);

  if (object == orxNULL)
    return;

  orxVECTOR objectVector;
  orxObject_GetWorldPosition(object, &objectVector);
  objectVector.fZ = 0.0;

  orxOBJECT *explosion = orxObject_CreateFromConfig(exploderObjectName);

  orxObject_SetPosition(explosion, &objectVector);
}

orxSTATUS orxFASTCALL Bootstrap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Adds default release config paths or do other stuffs
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../", orxFALSE);

  // Loads a specific config file
  /* orxConfig_Load("SomeSpecificConfig.ini"); */

  // Done!
  return eResult;
}

orxSTATUS orxFASTCALL PhysicsEventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  if (_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD) {
    orxOBJECT *pstRecipientObject, *pstSenderObject;

    pstSenderObject = orxOBJECT(_pstEvent->hSender);
    pstRecipientObject = orxOBJECT(_pstEvent->hRecipient);

    orxSTRING senderObjectName = (orxSTRING)orxObject_GetName(pstSenderObject);
    orxSTRING recipientObjectName = (orxSTRING)orxObject_GetName(pstRecipientObject);

    if (orxString_Compare(senderObjectName, "StarObject") == 0) {
      orxObject_SetLifeTime(pstSenderObject, 0);
      AddScore(1000);
    }
    if (orxString_Compare(recipientObjectName, "StarObject") == 0) {
      orxObject_SetLifeTime(pstRecipientObject, 0);
      AddScore(1000);
    }

    if (orxString_Compare(senderObjectName, "BulletObject") == 0){
      CreateExplosionAtObject(pstRecipientObject, "JellyExploder");
      orxObject_SetLifeTime(pstSenderObject, 0.1);
      orxObject_SetLifeTime(pstRecipientObject, 0.1);
      AddScore(250);
    }
    if (orxString_Compare(recipientObjectName, "BulletObject") == 0){
      CreateExplosionAtObject(pstSenderObject, "JellyExploder");
      orxObject_SetLifeTime(pstSenderObject, 0.1);
      orxObject_SetLifeTime(pstRecipientObject, 0.1);
      AddScore(250);
    }

    if (orxString_Compare(recipientObjectName, "PlayerObject") == 0 &&
        orxString_Compare(senderObjectName, "MonsterObject") == 0) {
      pstPlayer = orxNULL;
      CreateExplosionAtObject(pstRecipientObject, "PlayerExploder");
      orxObject_SetLifeTime(pstRecipientObject, 0);
      orxObject_Enable(pstRecipientObject, orxFALSE);
      orxObject_AddTimeLineTrack(pstScene, "PopUpGameOverTrack");
    }
    if (orxString_Compare(senderObjectName, "PlayerObject") == 0 &&
        orxString_Compare(recipientObjectName, "MonsterObject") == 0) {
      pstPlayer = orxNULL;
      CreateExplosionAtObject(pstSenderObject, "PlayerExploder");
      orxObject_SetLifeTime(pstSenderObject, 0);
      orxObject_Enable(pstSenderObject, orxFALSE);
      orxObject_AddTimeLineTrack(pstScene, "PopUpGameOverTrack");
    }
  }

  return eResult;
}

void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxFLOAT fPlayerSpeed = 600;
  orxVECTOR vWalkVelocity = {1, 0, 0};
  orxVECTOR vJumpVelocity = {0, -1, 0};
  orxVECTOR vMove;

  if (pstPlayer == orxNULL) {
    return;
  }

  orxObject_GetSpeed(pstPlayer, &vMove);
  vMove.fX = 0;
  /* orxVector_Copy(&vMove, &orxVECTOR_0); */
  /* orxVector_Mulf(&vWalkVelocity, &vWalkVelocity, fPlayerSpeed); */
  /* orxVector_Mulf(&vJumpVelocity, &vJumpVelocity, fPlayerSpeed); */

  if (orxInput_IsActive("MoveRight")) {
    orxObject_SetScale(pstPlayer, &vFlipRight);
    orxObject_SetTargetAnim(pstPlayer, "SoldierRun");
    vMove.fX += fPlayerSpeed;
  }
  if (orxInput_IsActive("MoveLeft")) {
    orxObject_SetScale(pstPlayer, &vFlipLeft);
    orxObject_SetTargetAnim(pstPlayer, "SoldierRun");
    /* orxVector_Neg(&vWalkVelocity, &vWalkVelocity); */
    vMove.fX -= fPlayerSpeed;
  }
  if (orxInput_IsActive("Jump") && orxInput_HasNewStatus("Jump")) {
    vMove.fY -= fPlayerSpeed * 2.5f;
  }

  if (orxInput_IsActive("Shoot")) {
    orxObject_Enable(pstPlayerGun, orxTRUE);
  }
  else {
    orxObject_Enable(pstPlayerGun, orxFALSE);
  }

  orxObject_SetSpeed(pstPlayer, &vMove);

}

orxSTATUS orxFASTCALL Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  orxCLOCK       *pstClock;

  // Creates viewport
  orxViewport_CreateFromConfig("Viewport");

  // Creates pstScene
  pstScene = orxObject_CreateFromConfig("Scene");

  // Create Player
  pstPlayer = orxObject_CreateFromConfig("PlayerObject");

  // Create PlayerGun
  pstPlayerGun = (orxOBJECT*)orxObject_GetChild(pstPlayer);

  // Disable gun object to keep it from spawning bullets
  orxObject_Enable(pstPlayerGun, orxFALSE);

  // Add physics event handler
  orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, PhysicsEventHandler);

  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);
 
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  pstScoreObject = orxObject_CreateFromConfig("ScoreObject");
  // Done!
  return eResult;
}



orxSTATUS orxFASTCALL Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Screenshot?
  if(orxInput_IsActive("Screenshot") && orxInput_HasNewStatus("Screenshot"))
    {
      // Captures it
      orxScreenshot_Capture();
    }
  // Quitting?
  if(orxInput_IsActive("Quit"))
    {
      // Updates result
      eResult = orxSTATUS_FAILURE;
    }

  // Done!
  return eResult;
}

void orxFASTCALL Exit()
{
  // We could delete everything we created here but orx will do it for us anyway =)
}

int main(int argc, char **argv)
{

  orxConfig_SetBootstrap(Bootstrap);

  // Executes orx
  orx_Execute(argc, argv, Init, Run, Exit);

  // Done!
  return EXIT_SUCCESS;
}


#ifdef __orxWINDOWS__

#include "windows.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

  orxConfig_SetBootstrap(Bootstrap);

  // Executes orx
  orx_WinExecute(Init, Run, Exit);

  // Done!
  return EXIT_SUCCESS;
}

#endif // __orxWINDOWS__
