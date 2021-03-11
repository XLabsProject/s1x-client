#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "command.hpp"
#include "game_console.hpp"

namespace stats
{
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (!game::environment::is_mp())
			{
				return;
			}

			command::add("setPlayerDataInt", [](const command::params& params)
			{
				if (params.size() < 2)
				{
					printf("usage: setPlayerDataInt <data>, <value>");
					return;
				}

				// SL_FindString
				const auto lookup_string = game::SL_FindString(params.get(1));
				const auto value = atoi(params.get(2));

				// SetPlayerDataInt
				reinterpret_cast<void(*)(signed int, unsigned int, unsigned int, unsigned int)>(0x1403BF550)(
					0, lookup_string, value, 0);
			});

			command::add("getPlayerDataInt", [](const command::params& params)
			{
				if (params.size() < 2)
				{
					printf("usage: getPlayerDataInt <data>\n");
					return;
				}

				// SL_FindString
				const auto lookup_string = game::SL_FindString(params.get(1));

				// GetPlayerDataInt
				const auto result = reinterpret_cast<int(*)(signed int, unsigned int, unsigned int)>(0x1403BE860)(
					0, lookup_string, 0);
				game_console::print(game_console::con_type_info, "%d\n", result);
			});

			command::add("unlockstats", []()
			{
				command::execute("setPlayerDataInt prestige 30");
				command::execute("setPlayerDataInt experience 1002100");
			});
		}
	};
}

REGISTER_COMPONENT(stats::component)
