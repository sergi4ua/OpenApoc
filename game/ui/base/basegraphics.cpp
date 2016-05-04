#include "game/ui/base/basegraphics.h"
#include "forms/ui.h"
#include "framework/framework.h"
#include "game/state/base/base.h"
#include "game/state/base/facility.h"
#include "game/state/city/building.h"
#include "game/state/city/city.h"
#include "game/state/gamestate.h"
#include <unordered_map>

namespace OpenApoc
{

// key is North South West East (true = occupied, false = vacant)
const std::unordered_map<std::vector<bool>, int> TILE_CORRIDORS = {
    {{true, false, false, false}, 4}, {{false, false, false, true}, 5},
    {{true, false, false, true}, 6},  {{false, true, false, false}, 7},
    {{true, true, false, false}, 8},  {{false, true, false, true}, 9},
    {{true, true, false, true}, 10},  {{false, false, true, false}, 11},
    {{true, false, true, false}, 12}, {{false, false, true, true}, 13},
    {{true, false, true, true}, 14},  {{false, true, true, false}, 15},
    {{true, true, true, false}, 16},  {{false, true, true, true}, 17},
    {{true, true, true, true}, 18}};

int BaseGraphics::getCorridorSprite(sp<Base> base, Vec2<int> pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.x >= Base::SIZE || pos.y >= Base::SIZE ||
	    !base->getCorridors()[pos.x][pos.y])
	{
		return 0;
	}
	bool north = pos.y > 0 && base->getCorridors()[pos.x][pos.y - 1];
	bool south = pos.y < Base::SIZE - 1 && base->getCorridors()[pos.x][pos.y + 1];
	bool west = pos.x > 0 && base->getCorridors()[pos.x - 1][pos.y];
	bool east = pos.x < Base::SIZE - 1 && base->getCorridors()[pos.x + 1][pos.y];
	return TILE_CORRIDORS.at({north, south, west, east});
}

void BaseGraphics::renderBase(Vec2<int> renderPos, sp<Base> base)
{
	// Draw grid
	sp<Image> grid = fw().data->load_image(
	    "PCK:xcom3/UFODATA/BASE.PCK:xcom3/UFODATA/BASE.TAB:0:xcom3/UFODATA/BASE.PCX");
	Vec2<int> i;
	for (i.x = 0; i.x < Base::SIZE; i.x++)
	{
		for (i.y = 0; i.y < Base::SIZE; i.y++)
		{
			Vec2<int> pos = renderPos + i * TILE_SIZE;
			fw().renderer->draw(grid, pos);
		}
	}

	// Draw corridors
	for (i.x = 0; i.x < Base::SIZE; i.x++)
	{
		for (i.y = 0; i.y < Base::SIZE; i.y++)
		{
			int sprite = getCorridorSprite(base, i);
			if (sprite != 0)
			{
				Vec2<int> pos = renderPos + i * TILE_SIZE;
				auto image = UString::format(
				    "PCK:xcom3/UFODATA/BASE.PCK:xcom3/UFODATA/BASE.TAB:%d:xcom3/UFODATA/BASE.PCX",
				    sprite);
				fw().renderer->draw(fw().data->load_image(image), pos);
			}
		}
	}

	// Draw facilities
	sp<Image> circleS = fw().data->load_image(
	    "PCK:xcom3/UFODATA/BASE.PCK:xcom3/UFODATA/BASE.TAB:25:xcom3/UFODATA/BASE.PCX");
	sp<Image> circleL = fw().data->load_image(
	    "PCK:xcom3/UFODATA/BASE.PCK:xcom3/UFODATA/BASE.TAB:26:xcom3/UFODATA/BASE.PCX");
	auto font = ui().GetFont("SMALFONT");
	for (auto &facility : base->getFacilities())
	{
		sp<Image> sprite = facility->type->sprite;
		Vec2<int> pos = renderPos + facility->pos * TILE_SIZE;
		if (facility->buildTime == 0)
		{
			fw().renderer->draw(sprite, pos);
		}
		else
		{
			// Fade out facility
			fw().renderer->drawTinted(sprite, pos, Colour(255, 255, 255, 128));
			// Draw construction overlay
			if (facility->type->size == 1)
			{
				fw().renderer->draw(circleS, pos);
			}
			else
			{
				fw().renderer->draw(circleL, pos);
			}
			// Draw time remaining
			auto textImage = font->getString(Strings::FromInteger(facility->buildTime));
			Vec2<int> textPos = {TILE_SIZE, TILE_SIZE};
			textPos *= facility->type->size;
			textPos -= textImage->size;
			textPos /= 2;
			fw().renderer->draw(textImage, pos + textPos);
		}
	}

	// Draw doors
	sp<Image> doorLeft = fw().data->load_image(
	    "PCK:xcom3/UFODATA/BASE.PCK:xcom3/UFODATA/BASE.TAB:2:xcom3/UFODATA/BASE.PCX");
	sp<Image> doorBottom = fw().data->load_image(
	    "PCK:xcom3/UFODATA/BASE.PCK:xcom3/UFODATA/BASE.TAB:3:xcom3/UFODATA/BASE.PCX");
	for (auto &facility : base->getFacilities())
	{
		for (int y = 0; y < facility->type->size; y++)
		{
			Vec2<int> tile = facility->pos + Vec2<int>{-1, y};
			if (getCorridorSprite(base, tile) != 0)
			{
				Vec2<int> pos = renderPos + tile * TILE_SIZE;
				fw().renderer->draw(doorLeft, pos + Vec2<int>{TILE_SIZE / 2, 0});
			}
		}
		for (int x = 0; x < facility->type->size; x++)
		{
			Vec2<int> tile = facility->pos + Vec2<int>{x, facility->type->size};
			if (getCorridorSprite(base, tile) != 0)
			{
				Vec2<int> pos = renderPos + tile * TILE_SIZE;
				fw().renderer->draw(doorBottom, pos - Vec2<int>{0, TILE_SIZE / 2});
			}
		}
	}
}

sp<RGBImage> BaseGraphics::drawMiniBase(sp<Base> base, FacilityHighlight highlight,
                                        sp<Facility> selected)
{
	auto minibase = mksp<RGBImage>(Vec2<unsigned int>{32, 32});

	// Draw corridors
	Vec2<int> i;
	for (i.x = 0; i.x < Base::SIZE; i.x++)
	{
		for (i.y = 0; i.y < Base::SIZE; i.y++)
		{
			int sprite = getCorridorSprite(base, i);
			if (sprite != 0)
			{
				sprite -= 3;
			}
			Vec2<int> pos = i * MINI_SIZE;
			auto image = UString::format(
			    "RAW:xcom3/UFODATA/MINIBASE.DAT:4:4:%d:xcom3/UFODATA/BASE.PCX", sprite);
			RGBImage::blit(std::dynamic_pointer_cast<RGBImage>(fw().data->load_image(image)),
			               minibase, {0, 0}, pos);
		}
	}

	// Draw facilities
	sp<Image> spriteNormal =
	    fw().data->load_image("RAW:xcom3/UFODATA/MINIBASE.DAT:4:4:16:xcom3/UFODATA/BASE.PCX");
	sp<Image> spriteHighlighted =
	    fw().data->load_image("RAW:xcom3/UFODATA/MINIBASE.DAT:4:4:17:xcom3/UFODATA/BASE.PCX");
	sp<Image> spriteSelected =
	    fw().data->load_image("RAW:xcom3/UFODATA/MINIBASE.DAT:4:4:18:xcom3/UFODATA/BASE.PCX");
	for (auto &facility : base->getFacilities())
	{
		bool highlighted = false;
		switch (highlight)
		{
			case FacilityHighlight::None:
				break;
			case FacilityHighlight::Construction:
				highlighted = (facility->buildTime > 0);
				break;
			case FacilityHighlight::Quarters:
				highlighted = (facility->type->capacityType == FacilityType::Capacity::Quarters);
				break;
			case FacilityHighlight::Stores:
				highlighted = (facility->type->capacityType == FacilityType::Capacity::Stores);
				break;
			case FacilityHighlight::Labs:
				highlighted = (facility->lab != nullptr);
				break;
			case FacilityHighlight::Aliens:
				highlighted = (facility->type->capacityType == FacilityType::Capacity::Aliens);
				break;
		}
		sp<Image> sprite = spriteNormal;
		if (facility == selected)
		{
			sprite = spriteSelected;
		}
		else if (highlighted)
		{
			sprite = spriteHighlighted;
		}
		for (i.x = 0; i.x < facility->type->size; i.x++)
		{
			for (i.y = 0; i.y < facility->type->size; i.y++)
			{
				Vec2<int> pos = (facility->pos + i) * MINI_SIZE;
				RGBImage::blit(std::dynamic_pointer_cast<RGBImage>(sprite), minibase, {0, 0}, pos);
			}
		}
	}

	return minibase;
}

sp<RGBImage> BaseGraphics::drawMinimap(sp<GameState> state, sp<Building> selected)
{
	auto minimap = mksp<RGBImage>(Vec2<unsigned int>{100, 100});
	RGBImageLock l(minimap);

	// Draw the city tiles
	std::map<Vec2<int>, int> minimap_z;
	for (auto &pair : state->cities["CITYMAP_HUMAN"]->initial_tiles)
	{
		auto &pos = pair.first;
		Vec2<int> pos2d = {pos.x, pos.y};
		auto &tile = pair.second;
		auto it = minimap_z.find(pos2d);
		if (it == minimap_z.end() || it->second < pos.z)
		{
			if (tile->minimap_colour.a == 0)
				continue;
			minimap_z[pos2d] = pos.z;
			l.set(pos2d, tile->minimap_colour);
		}
	}

	// Draw all bases as yellow blocks
	for (auto &pair : state->player_bases)
	{
		auto &base = pair.second;
		for (int y = base->building->bounds.p0.y; y < base->building->bounds.p1.y; y++)
		{
			for (int x = base->building->bounds.p0.x; x < base->building->bounds.p1.x; x++)
			{
				l.set({x, y}, {255, 255, 0, 255});
			}
		}
	}

	// Draw the current base as a red block
	for (int y = selected->bounds.p0.y; y < selected->bounds.p1.y; y++)
	{
		for (int x = selected->bounds.p0.x; x < selected->bounds.p1.x; x++)
		{
			l.set({x, y}, {255, 0, 0, 255});
		}
	}

	return minimap;
}

}; // namespace OpenApoc