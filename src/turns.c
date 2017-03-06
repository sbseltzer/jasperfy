// Book-keeping
static orxBANK *spstTurnTakerBank;
static orxBANK *spstTurnsBank;

// Structs
typedef struct TurnTaker {
  orxLINKLIST_NODE stNode;
  orxU32 u32Priority;
} TurnTaker;

typedef struct Turns {
  TurnTaker *pstCurrent;
  orxLINKLIST stTurnTakers;
} Turns;

// List of functions to implement
turns_Init();
turns_Exit();
turns_TurnTakerCount(turns);
turns_AddTurnTaker(turns, taker);
turns_RemoveTurnTaker(turns, taker);
turns_Iterate(turns, &taker);
turns_CreateTurns();
turns_CreateTurnTaker();
turns_CreateTurnTaker(priority);

// Usage code
void init() {
  turns_Init();
  pstTurns = turns_CreateTurns();
  pstPlayerTurnTaker = turns_CreateTurnTaker();
  turns_AddTurnTaker(turns, pstPlayerTurnTaker);
}
void exit() {
  turns_Exit();
}
void update() {
  static TurnTaker *taker = orxNULL;
  if (!bIsMultiFrameTurn) {
    turns_Iterate(turns, &taker);
  }
  if (taker == pstPlayerTurnTaker) {
    bIsMultiFrameTurn = orxTRUE;
    // prompt player for taking turn - then set bIsMultiFrameTurn to orxFALSE
  } else {
    // tell NPC to take turn
  }
}
