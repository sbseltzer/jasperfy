static orxBANK *spstSayBank;
static orxHASHTABLE *spstSayTable;

typedef struct SayHandle {
  orxLINKLIST_NODE stNode;
  orxString zName;
  orxSTRING zTrigger;
  orxSTRING zStyle;
  orxLINKLIST stNextOptions;
} SayHandle;

SayHandle *say_Create(orxSTRING _zSayName) {
  SayHandle *sayHandle = orxNULL;

  orxASSERT(_zSayName);

  sayHandle = (SayHandle *)orxBank_Allocate(spstSayBank);
  orxASSERT(sayHandle);
  sayHandle->zName = _zSayName;

  orxConfig_PushSection(_zSayName);
  sayHandle->zTrigger = orxConfig_GetString("Trigger");
  sayHandle->zStyle = orxConfig_GetString("Style");

  orxHashTable_Add((orxU64)orxString_GetID(_zSayName), (void *) sayHandle);

  const orxSTRING zNextList = orxConfig_GetString("Next");
  orxS32 numNexts = orxConfig_GetListCounter(zNextList);
  for (orxS32 i = 0; i < numNexts; i++) {
    const orxSTRING zNextSay = orxConfig_GetListString(zNextList, i);
    SayHandle *pstStored = (SayHandle *)orxHashTable_Get(spstSayTable, (orxU64) orxString_GetID(zNextSay));
    if (pstStored != orxNULL) { /* pre-existing say handle */
      orxLinkList_AddEnd(&sayHandle->stNextOptions, (orxLINKLIST_NODE *)pstStored);
    } else { /* new handle */
      orxLinkList_AddEnd(&sayHandle->stNextOptions, (orxLINKLIST_NODE *)say_Create(zNextSay));
    }
  }
  orxConfig_PopSection();

  return sayHandle;
}

orxSTATUS orxFASTCALL say_InputEventHandler(const orxEVENT *_pstEvent) {
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  if (_pstEvent->eID == orxINPUT_EVENT_ON) {
    orxINPUT_EVENT_PAYLOAD *pstPayload = (orxINPUT_EVENT_PAYLOAD *)_pstEvent->pstPayload;
  }
  if (_pstEvent->eID == orxINPUT_EVENT_OFF) {}
  return eResult;
}

orxSTATUS say_Init() {
  orxSTATUS result = orxSTATUS_SUCCESS;
  spstSayBank = orxBank_Create(32, sizeof(SayHandle), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
  orxEvent_AddHandler(orxEVENT_TYPE_INPUT, say_InputEventHandler);
  return result;
}

SayHandle* say_Pick(orxLINKLIST *list) {
  return orxNULL;
}

void say_Trigger(SayHandle* _pstHandle) {
  orxOBJECT *sayObject = orxObject_CreateFromConfig(_pstHandle->zName);
}

void say_Update() {
  if (orxTRUE) {

    say_Display()
  }
}
