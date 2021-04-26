#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/dvars.hpp"

#include <utils/hook.hpp>

namespace renderer
{
	namespace
	{
		utils::hook::detour r_init_draw_method_hook;
		utils::hook::detour r_update_front_end_dvar_options_hook;

		int get_fullbright_technique()
		{
			switch (dvars::r_fullbright->current.integer)
			{
			case 3:
				return 13;
			case 2:
				return 25;
			default:
				return game::TECHNIQUE_UNLIT;
			}
		}

		void gfxdrawmethod()
		{
			game::gfxDrawMethod->drawScene = game::GFX_DRAW_SCENE_STANDARD;
			game::gfxDrawMethod->baseTechType = dvars::r_fullbright->current.enabled ? get_fullbright_technique() : game::TECHNIQUE_LIT;
			game::gfxDrawMethod->emissiveTechType = dvars::r_fullbright->current.enabled ? get_fullbright_technique() : game::TECHNIQUE_EMISSIVE;
			game::gfxDrawMethod->forceTechType = dvars::r_fullbright->current.enabled ? get_fullbright_technique() : 182;
		}

		void r_init_draw_method_stub()
		{
			gfxdrawmethod();
		}

		bool r_update_front_end_dvar_options_stub()
		{
			if (dvars::r_fullbright->modified)
			{
				game::Dvar_ClearModified(dvars::r_fullbright);
				game::R_SyncRenderThread();
				
				gfxdrawmethod();
			}

			return r_update_front_end_dvar_options_hook.invoke<bool>();
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_dedi())
			{
				return;
			}

			dvars::r_fullbright = game::Dvar_RegisterInt("r_fullbright", 0, 0, 3, game::DVAR_FLAG_SAVED, "Toggles rendering without lighting");
		
			r_init_draw_method_hook.create(SELECT_VALUE(0x14046C150, 0x140588B00), &r_init_draw_method_stub);
			r_update_front_end_dvar_options_hook.create(SELECT_VALUE(0x1404A5330, 0x1405C3AE0), &r_update_front_end_dvar_options_stub);

			// use "saved" flags for "r_normalMap"
			utils::hook::set<uint8_t>(SELECT_VALUE(0x14047E0B8, 0x14059AD71), game::DVAR_FLAG_SAVED);

			// use "saved" flags for "r_specularMap"
			utils::hook::set<uint8_t>(SELECT_VALUE(0x14047E0DA, 0x14059AD99), game::DVAR_FLAG_SAVED);

			// use "saved" flags for "r_specOccMap"
			utils::hook::set<uint8_t>(SELECT_VALUE(0x14047E0FC, 0x14059ADC1), game::DVAR_FLAG_SAVED);
		}
	};
}

REGISTER_COMPONENT(renderer::component)
