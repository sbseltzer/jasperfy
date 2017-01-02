//! Includes
#include "game.h"

//! Variables
orxOBJECT* pstScene;

//! Code

orxBOOL HaveCollided(orxOBJECT *pstSenderObject, orxOBJECT *pstRecipientObject, orxSTRING objectA, orxSTRING objectB, orxOBJECT **pstObjectA, orxOBJECT **pstObjectB)
{
  orxSTRING senderObjectName = (orxSTRING)orxObject_GetName(pstSenderObject);
  orxSTRING recipientObjectName = (orxSTRING)orxObject_GetName(pstRecipientObject);
  orxBOOL collided = orxTRUE;
  if ((orxString_Compare(senderObjectName, objectA) == 0) && (orxString_Compare(recipientObjectName, objectB) == 0)) {
      *pstObjectA = pstSenderObject;
      *pstObjectB = pstRecipientObject;
  } else if ((orxString_Compare(senderObjectName, objectB) == 0) && (orxString_Compare(recipientObjectName, objectA) == 0)) {
      *pstObjectB = pstSenderObject;
      *pstObjectA = pstRecipientObject;
  } else {
    collided = orxFALSE;
  }
  return collided;
}

orxSTATUS orxFASTCALL Bootstrap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Adds default config search path at executable parent directory
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../", orxFALSE);

  // Load a user settings file manually. This is done so that orxCrypt usage can be separated out (if so desired)
  orxConfig_Load("Settings.ini");

  // Done!
  return eResult;
}

orxSTATUS orxFASTCALL PhysicsEventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  if (_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD) {
    orxOBJECT *pstRecipientObject, *pstSenderObject, *pstObjectA, *pstObjectB;

    pstSenderObject = orxOBJECT(_pstEvent->hSender);
    pstRecipientObject = orxOBJECT(_pstEvent->hRecipient);

    orxSTRING senderObjectName = (orxSTRING)orxObject_GetName(pstSenderObject);
    orxSTRING recipientObjectName = (orxSTRING)orxObject_GetName(pstRecipientObject);
  }

  return eResult;
}

void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
}

orxSTATUS orxFASTCALL Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  orxCLOCK *pstClock;

  // Creates viewport
  orxViewport_CreateFromConfig("Viewport");

  // Creates pstScene
  pstScene = orxObject_CreateFromConfig("Scene");

  // Add physics event handler
  orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, PhysicsEventHandler);

  // Register Update handler
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

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
