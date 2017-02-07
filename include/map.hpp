#include <orx.h>
#include <string>

using std::string;

struct MapParser {
  orxU32 index;
  orxU32 length;
  orxVECTOR gridPosition;
  string mapString;
  string tileID;

  void setMapString(const orxSTRING str) {
    mapString = str;
    index = 0;
    length = orxString_GetLength(mapString.c_str());
    tileID = "";
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
    tileID = mapString.substr(tileStartIndex, index - tileStartIndex);
    gridPosition.fX++;
    orxLOG("tileID is substring from %u to %u: %s", tileStartIndex, index, tileID.c_str());
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

// void loadTilesIDs(const orxSTRING mapName, map<string, string> &tiles) {}

void processMapString(const orxSTRING mapString) {
  MapParser parser;
  // Parse the map.
  if (mapString != orxNULL) {
    parser.setMapString(mapString);
    while (parser.nextTile()) {
      orxLOG("Pos<%f,%f,%f> %s", parser.gridPosition.fX, parser.gridPosition.fY, parser.gridPosition.fZ, parser.tileID.c_str());
    }
  }
}

void loadMapData(const orxSTRING mapName) {
  orxSTRING mapString;
  orxConfig_PushSection(mapName);
  processMapString(orxConfig_GetString("Map"));
  orxConfig_PopSection();
}
