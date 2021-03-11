#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "system_check.hpp"

#include "game/game.hpp"

#include <utils/io.hpp>
#include <utils/cryptography.hpp>

namespace system_check
{
	namespace
	{
		std::string read_zone(const std::string& name)
		{
			std::string data{};
			if (utils::io::read_file(name, &data))
			{
				return data;
			}

			if (utils::io::read_file("zone/" + name, &data))
			{
				return data;
			}

			return {};
		}

		std::string hash_zone(const std::string& name)
		{
			const auto data = read_zone(name);
			return utils::cryptography::sha256::compute(data, true);
		}

		bool verify_hashes(const std::unordered_map<std::string, std::string>& zone_hashes)
		{
			for (const auto& zone_hash : zone_hashes)
			{
				const auto hash = hash_zone(zone_hash.first);
				if (hash != zone_hash.second)
				{
					return false;
				}
			}

			return true;
		}

		bool is_system_valid()
		{
			static std::unordered_map<std::string, std::string> mp_zone_hashes =
			{
				{"patch_common_mp.ff", "23B15B4EF0AC9B52B3C6F9F681290B25B6B24B49F17238076A3D7F3CCEF9A0E1"},
			};

			static std::unordered_map<std::string, std::string> sp_zone_hashes =
			{
				{"patch_common.ff", "4624A974C6C7F8BECD9C343E7951722D8378889AC08ED4F2B22459B171EC553C"},
				{"patch_common_zm_mp.ff", "DA16B546B7233BBC4F48E1E9084B49218CB9271904EA7120A0EB4CB8723C19CF"},
			};

			return verify_hashes(mp_zone_hashes) && (game::environment::is_dedi() || verify_hashes(sp_zone_hashes));
		}
	}

	bool is_valid()
	{
		static auto valid = is_system_valid();
		return valid;
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			if (!is_valid())
			{
				MessageBoxA(nullptr, "Your game files are outdated or unsupported.\n"
				            "Please get the latest officially supported Call of Duty: Advanced Warfare files, or you will get random crashes and issues.",
				            "Invalid game files!", MB_ICONINFORMATION);
			}
		}
	};
}

REGISTER_COMPONENT(system_check::component)
