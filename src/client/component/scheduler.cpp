#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"
#include "game/game.hpp"
#include <utils/concurrent_list.hpp>
#include <utils/hook.hpp>
#include <utils/thread.hpp>

namespace scheduler
{
	namespace
	{
		struct task
		{
			pipeline type;
			std::function<bool()> handler;
			std::chrono::milliseconds interval{};
			std::chrono::high_resolution_clock::time_point last_call{};
		};

		volatile bool kill = false;
		std::thread thread;
		utils::concurrent_list<task> callbacks;
		utils::hook::detour r_end_frame_hook;

		void execute(const pipeline type)
		{
			for (auto callback : callbacks)
			{
				if (callback->type != type)
				{
					continue;
				}

				const auto now = std::chrono::high_resolution_clock::now();
				const auto diff = now - callback->last_call;

				if (diff < callback->interval) continue;

				callback->last_call = now;

				const auto res = callback->handler();
				if (res == cond_end)
				{
					callbacks.remove(callback);
				}
			}
		}

		void r_end_frame_stub()
		{
			execute(pipeline::renderer);
			r_end_frame_hook.invoke<void>();
		}

		void server_frame_stub()
		{
			game::G_Glass_Update();
			execute(pipeline::server);
		}

		void main_frame_stub()
		{
			execute(pipeline::main);
			game::Com_Frame_Try_Block_Function();
		}
	}

	void schedule(const std::function<bool()>& callback, const pipeline type,
	              const std::chrono::milliseconds delay)
	{
		task task;
		task.type = type;
		task.handler = callback;
		task.interval = delay;
		task.last_call = std::chrono::high_resolution_clock::now();

		callbacks.add(task);
	}

	void loop(const std::function<void()>& callback, const pipeline type,
	          const std::chrono::milliseconds delay)
	{
		schedule([callback]()
		{
			callback();
			return cond_continue;
		}, type, delay);
	}

	void once(const std::function<void()>& callback, const pipeline type,
	          const std::chrono::milliseconds delay)
	{
		schedule([callback]()
		{
			callback();
			return cond_end;
		}, type, delay);
	}

	void on_game_initialized(const std::function<void()>& callback, const pipeline type,
	                         const std::chrono::milliseconds delay)
	{
		schedule([=]()
		{
			const auto dw_init = game::environment::is_sp() ? true : game::Live_SyncOnlineDataFlags(0) == 0;
			if (dw_init && game::Sys_IsDatabaseReady2())
			{
				once(callback, type, delay);
				return cond_end;
			}

			return cond_continue;
		}, pipeline::main);
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			thread = utils::thread::create_named_thread("Async Scheduler", []()
			{
				while (!kill)
				{
					execute(pipeline::async);
					std::this_thread::sleep_for(10ms);
				}
			});
		}

		void post_unpack() override
		{
			r_end_frame_hook.create(SELECT_VALUE(0x1404A3E20, 0x1405C25B0), scheduler::r_end_frame_stub);

			utils::hook::call(SELECT_VALUE(0x1402F7DC2, 0x1403CEEE2), scheduler::main_frame_stub);
			utils::hook::call(SELECT_VALUE(0x140228647, 0x1402F8879), scheduler::server_frame_stub);
		}

		void pre_destroy() override
		{
			kill = true;
			if (thread.joinable())
			{
				thread.join();
			}
		}
	};
}

REGISTER_COMPONENT(scheduler::component)
