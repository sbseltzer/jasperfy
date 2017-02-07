#include <orx.h>
#include <string>

using std::string;

struct MapParser {
  orxU32 index;
  orxU32 length;
  orxVECTOR tileSpace;
  string mapString;
  string currentTile;

  void setMapString(const orxSTRING str) {
    mapString = str;
    index = 0;
    length = orxString_GetLength(mapString.c_str());
    currentTile = "";
    orxVector_Set(&tileSpace, 0.0, 0.0, 0.0);
    orxLOG("mapString: %u\n\"%s\"", length, mapString.c_str());
  }

  orxBOOL skipSpaces() {
    if (index >= length) { return orxFALSE; }
    orxCHAR ch = mapString[index];
    while ( ch == ' ' && index < length ) {
      orxLOG("Skipping spaces: mapString[%u] = '%c'", index, ch);
      ch = mapString[++index];
    }
    return ( index < length );
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
  orxBOOL updateCurrentTile() {
    if (index >= length) { return orxFALSE; }
    orxCHAR ch = mapString[index];
    orxU32 tileStartIndex = index;
    while (ch != ' ' && ch != '\r' && ch != '\n' && index < length) {
      orxLOG("Reading currentTile: mapString[%u] = '%c'", index, ch);
      ch = mapString[++index];
    }
    currentTile = mapString.substr(tileStartIndex, index - tileStartIndex);
    tileSpace.fX++;
    orxLOG("currentTile is substring from %u to %u: %s", tileStartIndex, index, currentTile.c_str());
    return ( index < length );
  }

  orxBOOL nextTile() {
    // Skip spaces, returning false if it detects end of string.
    // if (!skipSpaces()) { return orxFALSE; }

    orxLOG("Parsing next tile");
    // Skip whitespace, keeping a count of linebreaks along the way.
    orxU32 lineBreaks = 0;
    orxBOOL continueParsing = skipWhiteSpace(lineBreaks);
    orxLOG("lineBreaks: %u", lineBreaks);
    tileSpace.fY += lineBreaks;
    // If a linebreak was hit, reset tilespace column to 0.
    if (lineBreaks > 0) { tileSpace.fX = 0; }
    // If we hit the end of the string, return as such.
    if (!continueParsing) { return orxFALSE; }

    // Update current tile ID and return false if it detects end of string.
    return updateCurrentTile();
  }
};

// void loadTilesIDs(const orxSTRING mapName, map<string, string> &tiles) {}

void processMapString(const orxSTRING mapString) {
  MapParser parser;
  // Parse the map.
  if (mapString != orxNULL) {
    parser.setMapString(mapString);
    while (parser.nextTile()) {
      orxLOG("Pos<%f,%f,%f> %s", parser.tileSpace.fX, parser.tileSpace.fY, parser.tileSpace.fZ, parser.currentTile.c_str());
    }
  }
}

void loadMapData(const orxSTRING mapName) {
  orxSTRING mapString;
  orxConfig_PushSection(mapName);
  processMapString(orxConfig_GetString("Map"));
  orxConfig_PopSection();
}
