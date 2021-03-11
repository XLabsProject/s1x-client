#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game_console.hpp"

#include <utils/hook.hpp>

namespace logger
{
	namespace
	{
		void print_error(const char* msg)
		{
			game_console::print(game_console::con_type_error, msg);
		}

		void print_warning(const char* msg)
		{
			game_console::print(game_console::con_type_warning, msg);
		}

		void print(const char* msg)
		{
			game_console::print(game_console::con_type_info, msg);
		}

		// nullsub_56
		void nullsub_56()
		{
			utils::hook::call(0x1400D2572, print_warning);
			utils::hook::call(0x1400D257E, print_warning);
			utils::hook::call(0x1400D2586, print_warning);
			utils::hook::call(0x1400D2592, print_warning);

			utils::hook::call(0x1400D2B7D, print_warning);
			utils::hook::call(0x1400D2B89, print_warning);
			utils::hook::call(0x1400D2B91, print_warning);
			utils::hook::call(0x1400D2B9D, print_warning);

			utils::hook::call(0x1400D78ED, print_warning);
			utils::hook::call(0x1400D78F9, print_warning);
			utils::hook::call(0x1400D7901, print_warning);
			utils::hook::call(0x1400D790D, print_warning);

			utils::hook::call(0x1400D84F8, print_warning);
			utils::hook::call(0x1400D850C, print_warning);

			utils::hook::call(0x1400D8C08, print_warning);
			utils::hook::call(0x1400D8C28, print_warning);
			utils::hook::call(0x1400D8C34, print_warning);

			utils::hook::call(0x1400D8CD8, print_warning);
			utils::hook::call(0x1400D8CF8, print_warning);
			utils::hook::call(0x1400D8D04, print_warning);
			utils::hook::call(0x1400D8D1D, print_warning);

			utils::hook::call(0x1400DAE67, print_warning);
			utils::hook::call(0x1400DB019, print_warning);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (!game::environment::is_mp()) return;

			nullsub_56();
		}
	};
}

REGISTER_COMPONENT(logger::component)
