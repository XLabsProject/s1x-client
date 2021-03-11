#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include <game/game.hpp>

namespace survival
{
	namespace
	{
		const char* get_commandline_stub()
		{
			static std::string commandline{};
			if (commandline.empty())
			{
				commandline = GetCommandLineA();

				const auto real_mode = game::environment::get_real_mode();
				if (real_mode == launcher::mode::survival)
				{
					commandline += " +survival 01";
				}
				else if (real_mode == launcher::mode::zombies)
				{
					commandline += " +zombiesMode 01";
				}
			}

			return commandline.data();
		}
	}

	class component final : public component_interface
	{
	public:
		void* load_import(const std::string& library, const std::string& function) override
		{
			if (function == "GetCommandLineA")
			{
				return get_commandline_stub;
			}

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(survival::component)
