#pragma once

#include "game/game.hpp"

namespace fastfiles
{
	const char* get_current_fastfile();
	void reallocate_asset_pool(const game::XAssetType type, const unsigned int new_size);
}
