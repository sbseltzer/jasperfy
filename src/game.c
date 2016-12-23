//! Includes
#include "game.h"


//! Variables
orxOBJECT* objPlayer;
orxVECTOR vLeftVelocity = {-20, 0, 0};
orxVECTOR vRightVelocity = {20, 0, 0};
orxVECTOR vJumpVelocity = {0, -600, 0};

//! Code

orxSTATUS orxFASTCALL Bootstrap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Adds default release config paths or do other stuffs
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../", orxFALSE);

  // Loads config file
  orxConfig_Load("Game.ini");

  // Done!
  return eResult;
}

orxSTATUS orxFASTCALL Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Creates viewport
  orxViewport_CreateFromConfig("Viewport");

  // Creates scene
  orxObject_CreateFromConfig("Scene");

  // Create objPlayer
  objPlayer = orxObject_CreateFromConfig("Player");

  /* orxObject_CreateFromConfig("PlatformObject"); */

  // Done!
  return eResult;
}

orxSTATUS orxFASTCALL Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  if (orxInput_IsActive("MoveLeft"))
  {
    orxObject_ApplyImpulse(objPlayer, &vLeftVelocity, orxNULL);
  }
  if (orxInput_IsActive("MoveRight"))
  {
    orxObject_ApplyImpulse(objPlayer, &vRightVelocity, orxNULL);
  }
  if (orxInput_IsActive("Jump") && orxInput_HasNewStatus("Jump"))
  {
      orxObject_ApplyImpulse(objPlayer, &vJumpVelocity, orxNULL);
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
