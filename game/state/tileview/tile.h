#pragma once
#include "framework/includes.h"
#include "game/state/tileview/tileobject.h"
#include "library/sp.h"
#include <functional>
#include <set>
#include <vector>

// DANGER WILL ROBINSON - MADE UP VALUES AHEAD
// I suspect quantities of distance/velocity are stored in units of {32,32,16} (same as the voxel
// size)?
// And progressing them by 1/15th of that every tick looks about right?
#define TICK_SCALE (15)
#define VELOCITY_SCALE (Vec3<float>{32, 32, 16})

namespace OpenApoc
{

class Image;
class TileMap;
class Tile;
class Collision;
class VoxelMap;
class Renderer;
class TileView;
class Projectile;
class TileObjectProjectile;
class Vehicle;
class TileObjectVehicle;
class Scenery;
class TileObjectScenery;
class Doodad;
class TileObjectDoodad;

class TileTransform
{
  public:
	virtual Vec2<float> tileToScreenCoords(Vec3<float> coords) const = 0;
	virtual Vec3<float> screenToTileCoords(Vec2<float> screenPos, float z) const = 0;
};

enum class TileViewMode
{
	Isometric,
	Strategy,
};

class Tile
{
  public:
	TileMap &map;
	Vec3<int> position;

	std::set<sp<TileObject>> ownedObjects;
	std::set<sp<TileObject>> intersectingObjects;

	// FIXME: This is effectively a z-sorted list of ownedObjects - can this be merged somehow?
	std::vector<std::vector<sp<TileObject>>> drawnObjects;

	Tile(TileMap &map, Vec3<int> position, int layerCount);

	Collision findCollision(Vec3<float> lineSegmentStart, Vec3<float> lineSegmentEnd);
};

class CanEnterTileHelper
{
  public:
	// Returns true if this object can move from 'from' to 'to'. The two tiles must be adjacent!
	virtual bool canEnterTile(Tile *from, Tile *to) const = 0;
	virtual ~CanEnterTileHelper() = default;
};

class TileMap
{
  private:
	std::vector<Tile> tiles;
	std::vector<std::set<TileObject::Type>> layerMap;

  public:
	Tile *getTile(int x, int y, int z)
	{
		assert(x >= 0);
		assert(x < size.x);
		assert(y >= 0);
		assert(y < size.x);
		assert(z >= 0);
		assert(z < size.x);
		return &this->tiles[z * size.x * size.y + y * size.x + x];
	}
	Tile *getTile(Vec3<int> pos) { return this->getTile(pos.x, pos.y, pos.z); }
	// Returns the tile this point is 'within'
	Tile *getTile(Vec3<float> pos)
	{
		return this->getTile(static_cast<int>(pos.x), static_cast<int>(pos.y),
		                     static_cast<int>(pos.z));
	}
	Vec3<int> size;

	TileMap(Vec3<int> size, std::vector<std::set<TileObject::Type>> layerMap);
	~TileMap();

	std::list<Tile *> findShortestPath(Vec3<int> origin, Vec3<int> destination,
	                                   unsigned int iterationLimit,
	                                   const CanEnterTileHelper &canEnterTile);

	Collision findCollision(Vec3<float> lineSegmentStart, Vec3<float> lineSegmentEnd);

	void addObjectToMap(sp<Projectile>);
	void addObjectToMap(sp<Vehicle>);
	void addObjectToMap(sp<Scenery>);
	void addObjectToMap(sp<Doodad>);

	int getLayer(TileObject::Type type) const;
	int getLayerCount() const;
};
}; // namespace OpenApoc