static orxBANK *spstSayStateBank;
static orxBANK *spstSayHandleBank;
static orxHASHTABLE *spstSayTable;

/**
 * SayHandles form a DAG
 * 
 */
typedef struct SayHandle {
  orxLINKLIST_NODE stNode;
  orxSTRING zName;
  orxSTRING zText;
  orxLINKLIST stNextOptions;
} SayHandle;

typedef struct SayState {
  const SayHandle *pstStartHandle;
  SayHandle *pstCurrentHandle;
  SayHandle *pstPreviousHandle;
  SayHandle *pstNextHandle;
  orxOBJECT *pstSayObject; /* Object to set text on */
} SayState;

SayHandle* say_GetHandle(orxSTRING _zSayName);

SayHandle *say_CreateHandle(orxSTRING _zSayName) {
  SayHandle *pstSayHandle = (SayHandle *)orxHashTable_Get(spstSayTable, (orxU64) orxString_GetID(zNextSayName));

  if (pstSayHandle != orxNULL) {
    return pstSayHandle;
  }

  orxASSERT(_zSayName);

  pstSayHandle = (SayHandle *)orxBank_Allocate(spstSayHandleBank);
  orxASSERT(pstSayHandle);
  pstSayHandle->zName = _zSayName;

  orxConfig_PushSection(_zSayName);

  pstSayHandle->zText = orxConfig_GetString("Text");

  orxHashTable_Add((orxU64)orxString_GetID(_zSayName), (void *) pstSayHandle);

  const orxSTRING zNextList = orxConfig_GetString("Next");
  orxS32 numNexts = orxConfig_GetListCounter(zNextList);
  for (orxS32 i = 0; i < numNexts; i++) {
    const orxSTRING zNextSayName = orxConfig_GetListString(zNextList, i);
    SayHandle *pstNextSay = say_GetHandle(zNextSayName);
    if (pstNextSay == orxNULL) {
      orxLOG("Warning: SayHandle %s not found - skipping...", zNextSayName);
      continue;
    }
    orxLinkList_AddEnd(&pstSayHandle->stNextOptions, (orxLINKLIST_NODE *)pstNextSay);
  }
  orxConfig_PopSection();

  return pstSayHandle;
}

SayHandle* say_GetHandle(orxSTRING _zSayName) {
  SayHandle *pstSayHandle = (SayHandle *)orxHashTable_Get(spstSayTable, (orxU64) orxString_GetID(zNextSayName));
  if (pstSayHandle != orxNULL) {
    return pstSayHandle;
  }
  return say_CreateHandle(_zSayName);
}
SayState* say_CreateState(orxSTRING _zStateName) {
  SayState *pstSayState = orxNULL;

  orxASSERT(_zStateName);

  pstSayState = (SayHandle *)orxBank_Allocate(spstSayStateBank);
  orxASSERT(pstSayState);

  orxConfig_PushSection(_zStateName);
  orxSTRING zSayHandleName = orxConfig_GetString("SayRoot");
  orxASSERT(zSayHandleName);
  pstSayState->pstStartHandle = say_GetHandle(zSayHandleName);
  orxASSERT(pstSayState->pstStartHandle);
  pstSayState->pstCurrentHandle = pstSayState->pstStartHandle;
  orxConfig_PopSection();

  pstSayState->pstNextHandle = orxNULL;
  pstSayState->pstPreviousHandle = orxNULL;

  return pstSayState;
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
  spstSayHandleBank = orxBank_Create(32, sizeof(SayHandle), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
  spstSayStateBank = orxBank_Create(32, sizeof(SayState), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
  orxEvent_AddHandler(orxEVENT_TYPE_INPUT, say_InputEventHandler);
  return result;
}

SayHandle* say_PickByName(orxLINKLIST *_pstList, orxSTRING _zName) {
  orxLINKLIST_NODE *pstCurrentOption = orxLinkList_GetFirst(*_pstList);
  while (pstCurrentOption) {
    SayHandle *pstOptionHandle = (SayHandle *) pstCurrentOption;
    if (orxString_GetID(pstOptionHandle->zName) == orxString_GetID(_zName))
      break;
    pstCurrentOption = orxLinkList_GetNext(pstCurrentOption);
  }
  return (SayHandle *) pstCurrentOption;
}

void say_ResolveNext(SayState* _pstState) {
  _pstState->pstPreviousHandle = _pstState->_pstCurrentHandle;
  _pstState->pstCurrentHandle = _pstState->pstNextHandle;
  _pstState->pstNextHandle = orxNULL;
  orxObject_SetTextString(_pstState->pstSayObject, _pstState->pstCurrentHandle->zText);
}

void init() {
  say_Init();
  pstSayState = say_CreateState("SayText");
  pstSomeButton = orxObject_CreateFromConfig("SomeButton");
}

SayHandle* say_GetNextHandleByName(SayHandle *_pstHandle, const orxSTRING _zName) {
  return say_PickByName(&(_pstHandle->stNextOptions), _zName);
}
void chooseNextHandle(orxObject *_pstObject, SayState *_pstSayState) {
  const orxU32 u32NameID = orxString_GetID(orxObject_GetName(_pstObject));
  switch(u32NameID) {
  case orxString_GetID("Yes"):
    _pstSayState->pstCurrentHandle = say_GetNextHandleByName("TextB");
    break;
  case orxString_GetID("No"):
    _pstSayState->pstCurrentHandle = say_GetNextHandleByName("TextC");
    break;
  default:
  }
}
void oninput() {
  if(orxInput_IsActive("Click") && orxInput_HasNewStatus("Click")) {
		orxVECTOR mousePosition = { 0,0,0 };
		orxMouse_GetPosition(&mousePosition);
    orxOBJECT *obj = orxObject_Pick(mousePosition, orxString_GetID("Clickable"));
    obj = (obj != orxNULL) ? obj : orxObject_Pick(mousePosition, orxString_GetID("UIButton"));
    if (orxObject_GetGroupID(obj) == orxString_GetID("UIButton")) {
      chooseNextHandle(obj, pstSayState);
    } else {
      /* Creates state.pstSayObject and sets its text according to the state.pstCurrentHandle */
      say_Show(pstSayState);
    }
	}
  if(orxInput_IsActive("Up") && orxInput_HasNewStatus("Up")) {
  }
  if(orxInput_IsActive("Down") && orxInput_HasNewStatus("Down")) {
  }
  if(orxInput_IsActive("Cancel") && orxInput_HasNewStatus("Cancel")) {
    say_Reset(pstSayState);
    say_Hide(pstSayState);
  }
}
void update() {
  if (pstSayState->pstPicked == orxNULL) {
    SayHandle *pstCurrentOption = orxLinkList_GetFirst(pstSayState->stNextOptions);
    while (pstCurrentOption) {
      if (orxString_GetID(pstCurrentOption->zName) == orxString_GetID("TextA"))
      pstCurrentOption = (SayHandle *)orxLinkList_GetNext(pstCurrentOption);
    }
  }
}
void exit() {
  say_Exit();
}
