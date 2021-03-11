#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/dvars.hpp"

#include <utils/hook.hpp>

namespace gameplay
{
	namespace
	{
		void stuck_in_client_stub(void* entity)
		{
			if (dvars::g_playerEjection->current.enabled)
			{
				reinterpret_cast<void(*)(void*)>(0x1402DA310)(entity); // StuckInClient
			}
		}

		void cm_transformed_capsule_trace_stub(struct trace_t* results, const float* start, const float* end,
		                                       struct Bounds* bounds, struct Bounds* capsule, int contents,
		                                       const float* origin, const float* angles)
		{
			if (dvars::g_playerCollision->current.enabled)
			{
				reinterpret_cast<void(*)
					(struct trace_t*, const float*, const float*, struct Bounds*, struct Bounds*, unsigned int,
					 const float*, const float*)>
					(0x1403AB1C0)
					(results, start, end, bounds, capsule, contents, origin, angles); // CM_TransformedCapsuleTrace
			}
		}

		const auto pm_bouncing_stub_mp = utils::hook::assemble([](utils::hook::assembler& a)
		{
			const auto no_bounce = a.newLabel();
			const auto loc_14014DF48 = a.newLabel();

			a.push(rax);

			a.mov(rax, qword_ptr(reinterpret_cast<int64_t>(&dvars::pm_bouncing)));
			a.mov(al, byte_ptr(rax, 0x10));
			a.cmp(byte_ptr(rbp, -0x2D), al);

			a.pop(rax);
			a.jz(no_bounce);
			a.jmp(0x14014DFB0);

			a.bind(no_bounce);
			a.cmp(dword_ptr(rsp, 0x70), 0);
			a.jnz(loc_14014DF48);
			a.jmp(0x14014DFA2);

			a.bind(loc_14014DF48);
			a.jmp(0x14014DF48);
		});
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_sp()) return;

			// Implement player ejection dvar
			dvars::g_playerEjection = game::Dvar_RegisterBool("g_playerEjection", true, game::DVAR_FLAG_REPLICATED,
			                                                  "Flag whether player ejection is on or off");
			utils::hook::call(0x1402D5E4A, stuck_in_client_stub);

			// Implement player collision dvar
			dvars::g_playerCollision = game::Dvar_RegisterBool("g_playerCollision", true, game::DVAR_FLAG_REPLICATED,
			                                                   "Flag whether player collision is on or off");
			utils::hook::call(0x1404563DA, cm_transformed_capsule_trace_stub); // SV_ClipMoveToEntity
			utils::hook::call(0x1401F7F8F, cm_transformed_capsule_trace_stub); // CG_ClipMoveToEntity

			// Implement bouncing dvar
			utils::hook::jump(0x14014DF91, pm_bouncing_stub_mp, true);
			dvars::pm_bouncing = game::Dvar_RegisterBool("pm_bouncing", false,
			                                             game::DVAR_FLAG_REPLICATED, "Enable bouncing");

			// Change jump_slowdownEnable dvar flags to just "replicated"
			utils::hook::set<uint8_t>(0x140135992, game::DVAR_FLAG_REPLICATED);
		}
	};
}

REGISTER_COMPONENT(gameplay::component)
