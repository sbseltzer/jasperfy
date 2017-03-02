#include <orx.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Map */
typedef struct MapData {
  orxLINKLIST_NODE stNode;             /*! Mandatory for orxLINKLIST compatibility */
  const orxSTRING zMapName;            /*! Section name of map */
  const orxSTRING zTileAliasTableName; /*! Section name to generate pstTileAliasTable */
  const orxSTRING zBodyName;           /*! Section name of the map's body definition */
  const orxSTRING zMapLayout;          /*! Map layout string to be parsed */
  orxVECTOR vGridUnitSize;             /*! The size of one grid unit */
  orxVECTOR vGridDimensions;           /*! The total size of the map in grid units */
  orxHASHTABLE *pstTileAliasTable;     /*! tile alias -> tile section name */
  orxHASHTABLE *pstTileIndexTable;     /*! tile alias -> tile index */
  orxHASHTABLE *pstObjectPosTable;     /*! position -> object list */
  orxHASHTABLE *pstTileSetsTable;      /*! */
  orxOBJECT *pstObject;                /*! Object created from zMapName for lifetime management */
  orxBODY *pstBody;                    /*! Body to add tile BodyParts to */
} MapData;

/*! Map layout parsing state */
typedef struct MapLayoutParser {
  const MapData *pstMap;
  const orxU32 u32Length;
  orxU32 u32Index;
  orxVECTOR vCurrentGridPos;
  orxVECTOR vCurrentWorldPos;
  orxSTRING zCurrentTileName;
} MapLayoutParser;

MapLayoutParser map_CreateLayoutParser(const MapData* pstMap);

/*! Return false if reaches end of string - store linebreaks */
static orxBOOL map_ParseLayoutWhiteSpace(MapLayoutParser *pstParser, orxU32 *pu32LineBreaks);

// Assumes skipWhiteSpace has been called before it.
// If the index is on whitespace, it simply won't make any progress.
static orxBOOL map_ParseLayoutTile(MapLayoutParser *pstParser);

orxBOOL map_ParseLayout(MapLayoutParser *pstParser);

void map_AddPhysicsForTile(const orxSTRING _zTileName, orxBODY *_pstWorldBody, orxVECTOR *vGridUnitSize, orxVECTOR *_vTopLeft);

typedef struct MapObjectParser {
  const orxSTRING zMapName;
  orxU32 u32KeyIndex;
} MapObjectParser;

MapObjectParser map_CreateObjectParser(const orxSTRING _zMapName);

orxSTATUS map_NextObjectListPosition(MapObjectParser *_pstParser, const orxSTRING *_zKeyOut, orxVECTOR *vPositionOut);

/*! Load the specified section into a hashtable */
orxHASHTABLE* map_LoadTileTable(const orxSTRING _zName);

MapData *map_CreateMapData(const orxSTRING _zMapName);

void map_DeleteMapData(MapData *_pstMap);

void map_Generate(MapData *pstMapData);

void map_Reload(MapData *_pstMap);

orxSTATUS orxFASTCALL map_ConfigEventHandler(const orxEVENT *_pstEvent);

orxSTATUS map_Init();

orxSTATUS map_Exit();

// Implementation of map generation
void map_LoadMapData(const orxSTRING mapName);


#ifdef __cplusplus
}
#endif /* __cplusplus */
