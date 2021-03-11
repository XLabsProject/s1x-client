#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"
#include "server_list.hpp"
#include "network.hpp"
#include "command.hpp"
#include "game/game.hpp"
#include "dvars.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace dedicated
{
	namespace
	{
		utils::hook::detour gscr_set_dynamic_dvar_hook;
		utils::hook::detour com_quit_f_hook;

		void init_dedicated_server()
		{
			static bool initialized = false;
			if (initialized) return;
			initialized = true;

			// R_LoadGraphicsAssets
			reinterpret_cast<void(*)()>(0x1405A54F0)();
		}

		void send_heartbeat()
		{
			game::netadr_s target{};
			if (server_list::get_master_server(target))
			{
				network::send(target, "heartbeat", "S1");
			}
		}

		std::vector<std::string>& get_startup_command_queue()
		{
			static std::vector<std::string> startup_command_queue;
			return startup_command_queue;
		}

		void execute_startup_command(int client, int /*controllerIndex*/, const char* command)
		{
			if (game::Live_SyncOnlineDataFlags(0) == 0)
			{
				game::Cbuf_ExecuteBufferInternal(0, 0, command, game::Cmd_ExecuteSingleCommand);
			}
			else
			{
				get_startup_command_queue().emplace_back(command);
			}
		}

		void execute_startup_command_queue()
		{
			const auto queue = get_startup_command_queue();
			get_startup_command_queue().clear();

			for (const auto& command : queue)
			{
				game::Cbuf_ExecuteBufferInternal(0, 0, command.data(), game::Cmd_ExecuteSingleCommand);
			}
		}

		std::vector<std::string>& get_console_command_queue()
		{
			static std::vector<std::string> console_command_queue;
			return console_command_queue;
		}

		void execute_console_command(const int client, const char* command)
		{
			if (game::Live_SyncOnlineDataFlags(0) == 0)
			{
				game::Cbuf_AddText(client, command);
				game::Cbuf_AddText(client, "\n");
			}
			else
			{
				get_console_command_queue().emplace_back(command);
			}
		}

		void execute_console_command_queue()
		{
			const auto queue = get_console_command_queue();
			get_console_command_queue().clear();

			for (const auto& command : queue)
			{
				game::Cbuf_AddText(0, command.data());
				game::Cbuf_AddText(0, "\n");
			}
		}

		void sync_gpu_stub()
		{
			std::this_thread::sleep_for(1ms);
		}

		game::dvar_t* gscr_set_dynamic_dvar()
		{
			auto s = game::Scr_GetString(0);
			auto* dvar = game::Dvar_FindVar(s);
			if (dvar && !strncmp("scr_", dvar->name, 4))
			{
				return dvar;
			}

			return gscr_set_dynamic_dvar_hook.invoke<game::dvar_t*>();
		}

		void kill_server()
		{
			for (auto i = 0; i < *game::mp::svs_numclients; ++i)
			{
				if (game::mp::svs_clients[i].header.state >= 3)
				{
					game::SV_GameSendServerCommand(i, game::SV_CMD_CAN_IGNORE,
					                               utils::string::va("r \"%s\"", "EXE_ENDOFGAME"));
				}
			}

			com_quit_f_hook.invoke<void>();
		}
	}

	void initialize()
	{
		command::execute("exec default_xboxlive.cfg", true);
		command::execute("onlinegame 1", true);
		command::execute("xblive_privatematch 1", true);
	}

	class component final : public component_interface
	{
	public:
		void* load_import(const std::string& library, const std::string& function) override
		{
			return nullptr;
		}

		void post_unpack() override
		{
			if (!game::environment::is_dedi())
			{
				return;
			}

			// Disable VirtualLobby
			dvars::override::Dvar_RegisterBool("virtualLobbyEnabled", false, game::DVAR_FLAG_NONE | game::DVAR_FLAG_READ);

			// Disable r_preloadShaders
			dvars::override::Dvar_RegisterBool("r_preloadShaders", false, game::DVAR_FLAG_NONE | game::DVAR_FLAG_READ);

			// Don't allow sv_hostname to be changed by the game
			dvars::disable::Dvar_SetString("sv_hostname");

			// Hook R_SyncGpu
			utils::hook::jump(0x1405A7630, sync_gpu_stub);

			utils::hook::jump(0x14020C6B0, init_dedicated_server);

			// delay startup commands until the initialization is done
			utils::hook::call(0x1403CDF63, execute_startup_command);

			// delay console commands until the initialization is done
			utils::hook::call(0x1403CEC35, execute_console_command);
			utils::hook::nop(0x1403CEC4B, 5);

			// patch GScr_SetDynamicDvar to behave better
			gscr_set_dynamic_dvar_hook.create(0x140312D00, &gscr_set_dynamic_dvar);

			utils::hook::nop(0x1404AE6AE, 5); // don't load config file
			utils::hook::nop(0x1403AF719, 5); // ^
			utils::hook::set<uint8_t>(0x1403D2490, 0xC3); // don't save config file
			utils::hook::set<uint8_t>(0x14022AFC0, 0xC3); // disable self-registration
			utils::hook::set<uint8_t>(0x1404DA780, 0xC3); // init sound system (1)
			utils::hook::set<uint8_t>(0x14062BC10, 0xC3); // init sound system (2)
			utils::hook::set<uint8_t>(0x1405F31A0, 0xC3); // render thread
			utils::hook::set<uint8_t>(0x140213C20, 0xC3); // called from Com_Frame, seems to do renderer stuff
			utils::hook::set<uint8_t>(0x1402085C0, 0xC3);
			// CL_CheckForResend, which tries to connect to the local server constantly
			utils::hook::set<uint8_t>(0x14059B854, 0); // r_loadForRenderer default to 0
			utils::hook::set<uint8_t>(0x1404D6952, 0xC3); // recommended settings check - TODO: Check hook
			utils::hook::set<uint8_t>(0x1404D9BA0, 0xC3); // some mixer-related function called on shutdown
			utils::hook::set<uint8_t>(0x1403B2860, 0xC3); // dont load ui gametype stuff
			utils::hook::nop(0x14043ABB8, 6); // unknown check in SV_ExecuteClientMessage
			utils::hook::nop(0x140439F15, 4); // allow first slot to be occupied
			utils::hook::nop(0x14020E01C, 2); // properly shut down dedicated servers
			utils::hook::nop(0x14020DFE9, 2); // ^
			utils::hook::nop(0x14020E047, 5); // don't shutdown renderer

			utils::hook::set<uint8_t>(0x140057D40, 0xC3); // something to do with blendShapeVertsView
			utils::hook::nop(0x14062EA17, 8); // sound thing

			utils::hook::set<uint8_t>(0x1404D6960, 0xC3); // cpu detection stuff?
			utils::hook::set<uint8_t>(0x1405AEC00, 0xC3); // gfx stuff during fastfile loading
			utils::hook::set<uint8_t>(0x1405AEB10, 0xC3); // ^
			utils::hook::set<uint8_t>(0x1405AEBA0, 0xC3); // ^
			utils::hook::set<uint8_t>(0x140275640, 0xC3); // ^
			utils::hook::set<uint8_t>(0x1405AEB60, 0xC3); // ^
			utils::hook::set<uint8_t>(0x140572640, 0xC3); // directx stuff
			utils::hook::set<uint8_t>(0x1405A1340, 0xC3); // ^
			utils::hook::set<uint8_t>(0x140021D60, 0xC3); // ^ - mutex
			utils::hook::set<uint8_t>(0x1405A17E0, 0xC3); // ^

			utils::hook::set<uint8_t>(0x1400534F0, 0xC3); // rendering stuff
			utils::hook::set<uint8_t>(0x1405A1AB0, 0xC3); // ^
			utils::hook::set<uint8_t>(0x1405A1BB0, 0xC3); // ^
			utils::hook::set<uint8_t>(0x1405A21F0, 0xC3); // ^
			utils::hook::set<uint8_t>(0x1405A2D60, 0xC3); // ^
			utils::hook::set<uint8_t>(0x1405A3400, 0xC3); // ^

			// shaders
			utils::hook::set<uint8_t>(0x140057BC0, 0xC3); // ^
			utils::hook::set<uint8_t>(0x140057B40, 0xC3); // ^

			utils::hook::set<uint8_t>(0x1405EE040, 0xC3); // ^ - mutex

			utils::hook::set<uint8_t>(0x1404DAF30, 0xC3); // idk
			utils::hook::set<uint8_t>(0x1405736B0, 0xC3); // ^

			utils::hook::set<uint8_t>(0x1405A6E70, 0xC3); // R_Shutdown
			utils::hook::set<uint8_t>(0x1405732D0, 0xC3); // shutdown stuff
			utils::hook::set<uint8_t>(0x1405A6F40, 0xC3); // ^
			utils::hook::set<uint8_t>(0x1405A61A0, 0xC3); // ^

			utils::hook::set<uint8_t>(0x14062C550, 0xC3); // sound crashes

			utils::hook::set<uint8_t>(0x140445070, 0xC3); // disable host migration

			utils::hook::set<uint8_t>(0x1403E1A50, 0xC3); // render synchronization lock
			utils::hook::set<uint8_t>(0x1403E1990, 0xC3); // render synchronization unlock

			utils::hook::set<uint8_t>(0x1400E517B, 0xEB);
			// LUI: Unable to start the LUI system due to errors in main.lua

			utils::hook::nop(0x1404CC482, 5); // Disable sound pak file loading
			utils::hook::nop(0x1404CC471, 2); // ^
			utils::hook::set<uint8_t>(0x140279B80, 0xC3); // Disable image pak file loading

			// initialize the game after onlinedataflags is 32 (workaround)
			scheduler::schedule([=]()
			{
				if (game::Live_SyncOnlineDataFlags(0) == 32 && game::Sys_IsDatabaseReady2())
				{
					scheduler::once([]()
					{
						command::execute("xstartprivateparty", true);
						command::execute("disconnect", true); // 32 -> 0
					}, scheduler::pipeline::main, 1s);
					return scheduler::cond_end;
				}

				return scheduler::cond_continue;
			}, scheduler::pipeline::main, 1s);

			scheduler::on_game_initialized([]()
			{
				initialize();

				printf("==================================\n");
				printf("Server started!\n");
				printf("==================================\n");

				// remove disconnect command
				game::Cmd_RemoveCommand(reinterpret_cast<const char*>(751));

				execute_startup_command_queue();
				execute_console_command_queue();

				// Send heartbeat to dpmaster
				scheduler::once(send_heartbeat, scheduler::pipeline::server);
				scheduler::loop(send_heartbeat, scheduler::pipeline::server, 2min);
				command::add("heartbeat", send_heartbeat);
			}, scheduler::pipeline::main, 1s);

			command::add("killserver", kill_server);
			com_quit_f_hook.create(0x1403D08C0, &kill_server);
		}
	};
}

REGISTER_COMPONENT(dedicated::component)
