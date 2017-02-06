#include <orx.h>

using std::string;
using std::map;

struct MapParser {
  orxU32 index;
  string mapString;
  string currentTile;
  orxVECTOR tileSpace;

  MapParser(orxSTRING layer)
    : index { 0 }, mapString { layer }, currentTile { "" }, tileSpace { 0,0,0 }
  {}

  orxBOOL nextTile() {
    // Update index and tileSpace
    while (mapString[index] == ' ') index++;
    if (index >= orxString_GetLength(mapString)) { return orxFALSE; }
    if ( mapString[index] == '\n' ) {
      index++;
      if ( !nextTile() ) {
        return orxFALSE;
      }
      tileSpace.y++;
      tileSpace.x = 0;
    } else {
      tileSpace.x++;
    }
    // Update current tile ID
    orxU32 length = 0;
    while (mapString[index] != ' ' && mapString[index] != '\n') length++;
    currentTile = mapString.substr(index, length);
    return orxTRUE;
  }
};

void loadTiles(orxSTRING section, map<string, string> &tiles) {}

void processMapString(orxSTRING str) {
  MapParser parser {str};
  while (parser.nexTile()) {
    
  }
}
