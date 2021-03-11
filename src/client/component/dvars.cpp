#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "dvars.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace dvars
{
	class dvar_bool
	{
	public:
		std::string name;
		bool value;
		unsigned int flags;
		std::string description;
	};

	class dvar_float
	{
	public:
		std::string name;
		float value;
		float min;
		float max;
		unsigned int flags;
		std::string description;
	};

	class dvar_int
	{
	public:
		std::string name;
		int value;
		int min;
		int max;
		unsigned int flags;
		std::string description;
	};

	class dvar_string
	{
	public:
		std::string name;
		std::string value;
		unsigned int flags;
		std::string description;
	};

	class dvar_setbool
	{
	public:
		std::string name;
		bool boolean;
	};

	class dvar_setfloat
	{
	public:
		std::string name;
		float fl;
	};

	class dvar_setint
	{
	public:
		std::string name;
		int integer;
	};

	class dvar_setstring
	{
	public:
		std::string name;
		std::string string;
	};

	namespace
	{
		template <typename T>
		T* find_dvar(std::vector<T>* vec, const std::string& name)
		{
			for (auto i = 0ull; i < vec->size(); i++)
			{
				if (name == vec->at(i).name)
				{
					return &vec->at(i);
				}
			}
			return nullptr;
		}
	}

	namespace disable
	{
		static std::vector<dvar_setbool> set_bool_disables;
		static std::vector<dvar_setfloat> set_float_disables;
		static std::vector<dvar_setint> set_int_disables;
		static std::vector<dvar_setstring> set_string_disables;

		void Dvar_SetBool(const std::string& name)
		{
			dvar_setbool values;
			values.name = name;
			set_bool_disables.push_back(std::move(values));
		}

		void Dvar_SetFloat(const std::string& name)
		{
			dvar_setfloat values;
			values.name = name;
			set_float_disables.push_back(std::move(values));
		}

		void Dvar_SetInt(const std::string& name)
		{
			dvar_setint values;
			values.name = name;
			set_int_disables.push_back(std::move(values));
		}

		void Dvar_SetString(const std::string& name)
		{
			dvar_setstring values;
			values.name = name;
			set_string_disables.push_back(std::move(values));
		}
	}

	namespace override
	{
		static std::vector<dvar_bool> register_bool_overrides;
		static std::vector<dvar_float> register_float_overrides;
		static std::vector<dvar_int> register_int_overrides;
		static std::vector<dvar_string> register_string_overrides;

		static std::vector<dvar_setbool> set_bool_overrides;
		static std::vector<dvar_setfloat> set_float_overrides;
		static std::vector<dvar_setint> set_int_overrides;
		static std::vector<dvar_setstring> set_string_overrides;

		void Dvar_RegisterBool(const std::string& name, bool value, unsigned int flags, const std::string& description)
		{
			dvar_bool values;
			values.name = name;
			values.value = value;
			values.flags = flags;
			values.description = description;
			register_bool_overrides.push_back(std::move(values));
		}

		void Dvar_RegisterFloat(const std::string& name, float value, float min, float max, unsigned int flags,
		                        const std::string& description)
		{
			dvar_float values;
			values.name = name;
			values.value = value;
			values.min = min;
			values.max = max;
			values.flags = flags;
			values.description = description;
			register_float_overrides.push_back(std::move(values));
		}

		void Dvar_RegisterInt(const std::string& name, int value, int min, int max, unsigned int flags,
		                      const std::string& description)
		{
			dvar_int values;
			values.name = name;
			values.value = value;
			values.min = min;
			values.max = max;
			values.flags = flags;
			values.description = description;
			register_int_overrides.push_back(std::move(values));
		}

		void Dvar_RegisterString(const std::string& name, const std::string& value, unsigned int flags,
		                         const std::string& description)
		{
			dvar_string values;
			values.name = name;
			values.value = value;
			values.flags = flags;
			values.description = description;
			register_string_overrides.push_back(std::move(values));
		}

		void Dvar_SetBool(const std::string& name, bool boolean)
		{
			dvar_setbool values;
			values.name = name;
			values.boolean = boolean;
			set_bool_overrides.push_back(std::move(values));
		}

		void Dvar_SetFloat(const std::string& name, float fl)
		{
			dvar_setfloat values;
			values.name = name;
			values.fl = fl;
			set_float_overrides.push_back(std::move(values));
		}

		void Dvar_SetInt(const std::string& name, int integer)
		{
			dvar_setint values;
			values.name = name;
			values.integer = integer;
			set_int_overrides.push_back(std::move(values));
		}

		void Dvar_SetString(const std::string& name, const std::string& string)
		{
			dvar_setstring values;
			values.name = name;
			values.string = string;
			set_string_overrides.push_back(std::move(values));
		}
	}

	utils::hook::detour dvar_register_bool_hook;
	utils::hook::detour dvar_register_float_hook;
	utils::hook::detour dvar_register_int_hook;
	utils::hook::detour dvar_register_string_hook;

	utils::hook::detour dvar_set_bool_hook;
	utils::hook::detour dvar_set_float_hook;
	utils::hook::detour dvar_set_int_hook;
	utils::hook::detour dvar_set_string_hook;

	game::dvar_t* dvar_register_bool(const char* name, bool value, unsigned int flags, const char* description)
	{
		auto* var = find_dvar(&override::register_bool_overrides, name);
		if (var)
		{
			value = var->value;
			flags = var->flags;
			description = var->description.data();
		}

		return dvar_register_bool_hook.invoke<game::dvar_t*>(name, value, flags, description);
	}

	game::dvar_t* dvar_register_float(const char* name, float value, float min, float max, unsigned int flags,
	                                  const char* description)
	{
		auto* var = find_dvar(&override::register_float_overrides, name);
		if (var)
		{
			value = var->value;
			min = var->min;
			max = var->max;
			flags = var->flags;
			description = var->description.data();
		}

		return dvar_register_float_hook.invoke<game::dvar_t*>(name, value, min, max, flags, description);
	}

	game::dvar_t* dvar_register_int(const char* name, int value, int min, int max, unsigned int flags,
	                                const char* description)
	{
		auto* var = find_dvar(&override::register_int_overrides, name);
		if (var)
		{
			value = var->value;
			min = var->min;
			max = var->max;
			flags = var->flags;
			description = var->description.data();
		}

		return dvar_register_int_hook.invoke<game::dvar_t*>(name, value, min, max, flags, description);
	}

	game::dvar_t* dvar_register_string(const char* name, const char* value, unsigned int flags, const char* description)
	{
		auto* var = find_dvar(&override::register_string_overrides, name);
		if (var)
		{
			value = var->value.data();
			flags = var->flags;
			description = var->description.data();
		}

		return dvar_register_string_hook.invoke<game::dvar_t*>(name, value, flags, description);
	}

	void dvar_set_bool(game::dvar_t* dvar, bool boolean)
	{
		auto* var = find_dvar(&disable::set_bool_disables, dvar->name);
		if (var)
		{
			return;
		}

		var = find_dvar(&override::set_bool_overrides, dvar->name);
		if (var)
		{
			boolean = var->boolean;
		}

		return dvar_set_bool_hook.invoke<void>(dvar, boolean);
	}

	void dvar_set_float(game::dvar_t* dvar, float fl)
	{
		auto* var = find_dvar(&disable::set_float_disables, dvar->name);
		if (var)
		{
			return;
		}

		var = find_dvar(&override::set_float_overrides, dvar->name);
		if (var)
		{
			fl = var->fl;
		}

		return dvar_set_float_hook.invoke<void>(dvar, fl);
	}

	void dvar_set_int(game::dvar_t* dvar, int integer)
	{
		auto* var = find_dvar(&disable::set_int_disables, dvar->name);
		if (var)
		{
			return;
		}

		var = find_dvar(&override::set_int_overrides, dvar->name);
		if (var)
		{
			integer = var->integer;
		}

		return dvar_set_int_hook.invoke<void>(dvar, integer);
	}

	void dvar_set_string(game::dvar_t* dvar, const char* string)
	{
		auto* var = find_dvar(&disable::set_string_disables, dvar->name);
		if (var)
		{
			return;
		}

		var = find_dvar(&override::set_string_overrides, dvar->name);
		if (var)
		{
			string = var->string.data();
		}

		return dvar_set_string_hook.invoke<void>(dvar, string);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			dvar_register_bool_hook.create(SELECT_VALUE(0x140371850, 0x1404C0BE0), &dvar_register_bool);
			dvar_register_float_hook.create(SELECT_VALUE(0x140371C20, 0x1404C0FB0), &dvar_register_float);
			dvar_register_int_hook.create(SELECT_VALUE(0x140371CF0, 0x1404C1080), &dvar_register_int);
			dvar_register_string_hook.create(SELECT_VALUE(0x140372050, 0x1404C1450), &dvar_register_string);

			dvar_set_int_hook.create(SELECT_VALUE(0x140372B70, 0x1404C1F30), &dvar_set_bool);
			dvar_set_int_hook.create(SELECT_VALUE(0x140373420, 0x1404C2A10), &dvar_set_float);
			dvar_set_int_hook.create(SELECT_VALUE(0x1403738D0, 0x1404C2F40), &dvar_set_int);
			dvar_set_string_hook.create(SELECT_VALUE(0x140373DE0, 0x1404C3610), &dvar_set_string);
		}
	};
}

REGISTER_COMPONENT(dvars::component)
