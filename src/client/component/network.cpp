#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include "network.hpp"
#include "game_console.hpp"
#include "dvars.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace network
{
	namespace
	{
		std::unordered_map<std::string, callback>& get_callbacks()
		{
			static std::unordered_map<std::string, callback> callbacks{};
			return callbacks;
		}

		bool handle_command(game::netadr_s* address, const char* command, game::msg_t* message)
		{
			const auto cmd_string = utils::string::to_lower(command);
			auto& callbacks = get_callbacks();
			const auto handler = callbacks.find(cmd_string);
			if (handler == callbacks.end())
			{
				return false;
			}

			const auto offset = cmd_string.size() + 5;
			const std::string_view data(message->data + offset, message->cursize - offset);

			handler->second(*address, data);
			return true;
		}

		void handle_command_stub(utils::hook::assembler& a)
		{
			const auto return_unhandled = a.newLabel();

			a.pushad64();

			a.mov(r8, rsi); // message
			a.mov(rdx, rdi); // command
			a.mov(rcx, r14); // netaddr

			a.call_aligned(handle_command);

			a.test(al, al);
			a.jz(return_unhandled);

			// Command handled
			a.popad64();
			a.mov(al, 1);
			a.jmp(0x14020AA10);

			a.bind(return_unhandled);
			a.popad64();
			a.jmp(0x14020A19A);
		}

		int net_compare_base_address(const game::netadr_s* a1, const game::netadr_s* a2)
		{
			if (a1->type == a2->type)
			{
				switch (a1->type)
				{
				case game::netadrtype_t::NA_BOT:
				case game::netadrtype_t::NA_LOOPBACK:
					return a1->port == a2->port;

				case game::netadrtype_t::NA_IP:
					return !memcmp(a1->ip, a2->ip, 4);
				case game::netadrtype_t::NA_BROADCAST:
					return true;
				default:
					break;
				}
			}

			return false;
		}

		int net_compare_address(const game::netadr_s* a1, const game::netadr_s* a2)
		{
			return net_compare_base_address(a1, a2) && a1->port == a2->port;
		}

		void reconnect_migratated_client(void*, game::netadr_s* from, const int, const int, const char*,
		                                 const char*, bool)
		{
			// This happens when a client tries to rejoin after being recently disconnected, OR by a duplicated guid
			// We don't want this to do anything. It decides to crash seemingly randomly
			// Rather than try and let the player in, just tell them they are a duplicate player and reject connection
			game::NET_OutOfBandPrint(game::NS_SERVER, from, "error\nYou are already connected to the server.");
		}
	}

	void on(const std::string& command, const callback& callback)
	{
		get_callbacks()[utils::string::to_lower(command)] = callback;
	}

	void dw_send_to_stub(const unsigned int size, const char* src, game::netadr_s* a3)
	{
		sockaddr s = {};
		game::NetadrToSockadr(a3, &s);
		sendto(*game::query_socket, src, size - 2, 0, &s, 16);
	}

	void send(const game::netadr_s& address, const std::string& command, const std::string& data, const char separator)
	{
		std::string packet = "\xFF\xFF\xFF\xFF";
		packet.append(command);
		packet.push_back(separator);
		packet.append(data);

		send(address, packet);
	}

	void send(const game::netadr_s& address, const std::string& data)
	{
		if (address.type == game::NA_LOOPBACK)
		{
			game::NET_SendLoopPacket(game::NS_CLIENT1, static_cast<int>(data.size()), data.data(), &address);
		}
		else
		{
			game::Sys_SendPacket(static_cast<int>(data.size()), data.data(), &address);
		}
	}

	bool are_addresses_equal(const game::netadr_s& a, const game::netadr_s& b)
	{
		return net_compare_address(&a, &b);
	}

	const char* net_adr_to_string(const game::netadr_s& a)
	{
		if (a.type == game::netadrtype_t::NA_LOOPBACK)
		{
			return "loopback";
		}

		if (a.type == game::netadrtype_t::NA_BOT)
		{
			return "bot";
		}

		if (a.type == game::netadrtype_t::NA_IP || a.type == game::netadrtype_t::NA_BROADCAST)
		{
			if (a.port)
			{
				return utils::string::va("%u.%u.%u.%u:%u", a.ip[0], a.ip[1], a.ip[2], a.ip[3], htons(a.port));
			}

			return utils::string::va("%u.%u.%u.%u", a.ip[0], a.ip[1], a.ip[2], a.ip[3]);
		}

		return "bad";
	}

	game::dvar_t* register_netport_stub(const char* dvarName, int value, int min, int max, unsigned int flags,
	                                    const char* description)
	{
		auto dvar = game::Dvar_RegisterInt("net_port", 27016, 0, 0xFFFFu, game::DVAR_FLAG_LATCHED, "Network port");

		// read net_port from command line
		command::read_startup_variable("net_port");

		return dvar;
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			{
				if (game::environment::is_sp())
				{
					return;
				}

				// redirect dw_sendto to raw socket
				//utils::hook::jump(0x1404D850A, reinterpret_cast<void*>(0x1404D849A));
				utils::hook::call(0x1404D851F, dw_send_to_stub);

				// intercept command handling
				utils::hook::jump(0x14020A175, utils::hook::assemble(handle_command_stub), true);

				// handle xuid without secure connection
				utils::hook::nop(0x14043FFF8, 6);

				utils::hook::jump(0x1403DA700, net_compare_address);
				utils::hook::jump(0x1403DA750, net_compare_base_address);

				// don't establish secure conenction
				utils::hook::set<uint8_t>(0x140232BBD, 0xEB);
				utils::hook::set<uint8_t>(0x140232C9A, 0xEB);
				utils::hook::set<uint8_t>(0x140232F8D, 0xEB);
				utils::hook::set<uint8_t>(0x14020862F, 0xEB);

				// ignore unregistered connection
				utils::hook::jump(0x140439EA9, reinterpret_cast<void*>(0x140439E28));
				utils::hook::set<uint8_t>(0x140439E9E, 0xEB);

				// disable xuid verification
				utils::hook::set<uint8_t>(0x140022319, 0xEB);
				utils::hook::set<uint8_t>(0x140022334, 0xEB);

				// disable xuid verification
				utils::hook::nop(0x14043CC4C, 2);
				utils::hook::set<uint8_t>(0x14043CCA8, 0xEB);

				// ignore configstring mismatch
				utils::hook::set<uint8_t>(0x140211610, 0xEB);

				// ignore dw handle in SV_PacketEvent
				utils::hook::set<uint8_t>(0x140442F6D, 0xEB);
				utils::hook::call(0x140442F61, &net_compare_address);

				// ignore dw handle in SV_FindClientByAddress
				utils::hook::set<uint8_t>(0x14044256D, 0xEB);
				utils::hook::call(0x140442561, &net_compare_address);

				// ignore dw handle in SV_DirectConnect
				utils::hook::set<uint8_t>(0x140439BA8, 0xEB);
				utils::hook::set<uint8_t>(0x140439DA5, 0xEB);
				utils::hook::call(0x140439B9B, &net_compare_address);
				utils::hook::call(0x140439D98, &net_compare_address);

				// increase cl_maxpackets
				dvars::override::Dvar_RegisterInt("cl_maxpackets", 1000, 1, 1000, 0x1);

				// ignore impure client
				utils::hook::jump(0x14043AC0D, reinterpret_cast<void*>(0x14043ACA3));

				// don't send checksum
				utils::hook::set<uint8_t>(0x1404D84C0, 0);

				// don't read checksum
				utils::hook::jump(0x1404D842B, 0x1404D8453);

				// don't try to reconnect client
				utils::hook::call(0x140439D4D, reconnect_migratated_client);
				utils::hook::nop(0x140439D28, 4); // this crashes when reconnecting for some reason

				// allow server owner to modify net_port before the socket bind
				utils::hook::call(0x1404D7A3D, register_netport_stub);
				utils::hook::call(0x1404D7E28, register_netport_stub);

				// ignore built in "print" oob command and add in our own
				utils::hook::set<uint8_t>(0x14020A723, 0xEB);
				on("print", [](const game::netadr_s& addr, const std::string_view& data)
				{
					const std::string message{data};

					if (game::environment::is_dedi())
					{
						printf("%s\n", message.data());
					}
					else
					{
						game_console::print(game_console::con_type_info, "%s\n", message.data());
					}
				});
			}
		}
	};
}

REGISTER_COMPONENT(network::component)
