#include <orx.h>
#include <string>

using std::string;

struct MapParser {
  orxU32 index;
  orxU32 length;
  const string mapString;
  const orxHASHTABLE *tileIDs;
  orxVECTOR gridPosition;
  orxSTRING tileSection;

  MapParser(const orxSTRING str, const orxHASHTABLE *tileTable)
    : mapString (str), tileIDs (tileTable), index (0), tileSection ((orxSTRING)orxNULL) 
  {
      length = orxString_GetLength(mapString.c_str());
      orxVector_Set(&gridPosition, 0.0, 0.0, 0.0);
      orxLOG("mapString: %u\n\"%s\"", length, mapString.c_str());
  }

  orxBOOL skipWhiteSpace(orxU32 &lineBreaks) {
    if (index >= length) { return orxFALSE; }
    orxCHAR ch = mapString[index];
    while ( ( ch == ' ' || ch == '\r' || ch == '\n' ) && index < length ) {
      orxLOG("Skipping whitespace: mapString[%u] = '%c'", index, ch);
      if (ch == '\n') { lineBreaks++; }
      ch = mapString[++index];
    }
    return ( index < length );
  }

  // Assumes skipWhiteSpace has been called before it.
  // If the index is on whitespace, it simply won't make any progress.
  orxBOOL updateTileID() {
    if (index >= length) { return orxFALSE; }
    orxCHAR ch = mapString[index];
    orxU32 tileStartIndex = index;
    while (ch != ' ' && ch != '\r' && ch != '\n' && index < length) {
      orxLOG("Reading tileID: mapString[%u] = '%c'", index, ch);
      ch = mapString[++index];
    }
    string tileID = mapString.substr(tileStartIndex, index - tileStartIndex);
    tileSection = (orxSTRING)orxHashTable_Get(tileIDs, (orxU64)orxString_ToCRC(tileID.c_str()));
    gridPosition.fX++;
    orxLOG("tileID is substring from %u to %u: %s %s", tileStartIndex, index, tileID.c_str(), tileSection);
    return ( index < length );
  }

  orxBOOL nextTile() {
    orxLOG("Parsing next tile");
    // Skip whitespace, keeping a count of linebreaks along the way.
    orxU32 lineBreaks = 0;
    orxBOOL continueParsing = skipWhiteSpace(lineBreaks);
    orxLOG("lineBreaks: %u", lineBreaks);
    gridPosition.fY += lineBreaks;
    // If a linebreak was hit, reset tilespace column to 0.
    if (lineBreaks > 0) { gridPosition.fX = 0; }
    // If we hit the end of the string, return as such.
    if (!continueParsing) { return orxFALSE; }

    // Update current tile ID and return false if it detects end of string.
    return updateTileID();
  }
};

void loadTilesIDs(const orxSTRING sectionName, orxHASHTABLE** tileTable) {
  orxConfig_PushSection(sectionName);
  const orxU32 numKeys = orxConfig_GetKeyCounter();
  if (*tileTable == orxNULL) {
    *tileTable = orxHashTable_Create(numKeys, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
  }
  for (orxU32 keyIndex = 0; keyIndex < numKeys; keyIndex++) {
    const orxSTRING key = orxConfig_GetKey(keyIndex);
    orxLOG("attempting to load tileID %s", key);
    // Make sure the key value is a valid section
    const orxSTRING value = orxConfig_GetString(key);
    if (orxConfig_HasSection(value)) {
      orxHashTable_Add(*tileTable, (orxU64) orxString_ToCRC(key), (void *) (orxU64) value);
      orxLOG("%s -> %s", key, value);
    }
  }
  orxConfig_PopSection();
}

void processMapString(const orxSTRING mapString, const orxHASHTABLE *tileTable) {
  // Parse the map.
  if (mapString != orxNULL) {
    MapParser parser ( mapString, tileTable );
    while (parser.nextTile()) {
      orxLOG("Pos<%f,%f,%f> %s", parser.gridPosition.fX, parser.gridPosition.fY, parser.gridPosition.fZ, parser.tileSection);
    }
  }
}

void loadMapData(const orxSTRING mapName, const orxSTRING tileTableName) {
  orxHASHTABLE *tileTable = orxNULL;
  loadTilesIDs(tileTableName, &tileTable);
  orxConfig_PushSection(mapName);
  processMapString(orxConfig_GetString("Map"), tileTable);
  orxConfig_PopSection();
}
