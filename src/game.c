//! Includes
#include "game.h"


//! Variables
orxOBJECT* pstPlayer;
orxOBJECT* pstPlayerGun;
orxVECTOR vLeftVelocity = {-20, 0, 0};
orxVECTOR vRightVelocity = {20, 0, 0};
orxVECTOR vJumpVelocity = {0, -600, 0};
orxVECTOR vFlipLeft = { -2, 2, 1 };
orxVECTOR vFlipRight = { 2, 2, 1 };

//! Code

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
    }
    if (orxString_Compare(recipientObjectName, "StarObject") == 0) {
      orxObject_SetLifeTime(pstRecipientObject, 0);
    }
  }

  return eResult;
}

orxSTATUS orxFASTCALL Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Creates viewport
  orxViewport_CreateFromConfig("Viewport");

  // Creates scene
  orxObject_CreateFromConfig("Scene");

  // Create Player
  pstPlayer = orxObject_CreateFromConfig("Player");
 
  // Create PlayerGun
  pstPlayerGun = (orxOBJECT*)orxObject_GetChild(pstPlayer);

  // Disable gun object to keep it from spawning bullets
  orxObject_Enable(pstPlayerGun, orxFALSE);

  // Add physics event handler
  orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, PhysicsEventHandler);

  // Done!
  return eResult;
}

orxSTATUS orxFASTCALL Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  if (orxInput_IsActive("MoveLeft"))
  {
    orxObject_SetScale(pstPlayer, &vFlipLeft);
    orxObject_ApplyImpulse(pstPlayer, &vLeftVelocity, orxNULL);
    orxObject_SetTargetAnim(pstPlayer, "SoldierRun");
  }
  else if (orxInput_IsActive("MoveRight"))
  {
    orxObject_SetScale(pstPlayer, &vFlipRight);
    orxObject_ApplyImpulse(pstPlayer, &vRightVelocity, orxNULL);
    orxObject_SetTargetAnim(pstPlayer, "SoldierRun");
  }
  else
  {
    orxObject_SetTargetAnim(pstPlayer, orxNULL);
  }
  if (orxInput_IsActive("Shoot"))
  {
    orxObject_Enable(pstPlayerGun, orxTRUE);
  }
  else
  {
    orxObject_Enable(pstPlayerGun, orxFALSE);
  }
  if (orxInput_IsActive("Jump") && orxInput_HasNewStatus("Jump"))
  {
    orxObject_ApplyImpulse(pstPlayer, &vJumpVelocity, orxNULL);
  }
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
