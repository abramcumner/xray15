////////////////////////////////////////////////////////////////////////////
//	Module 		: backend.h
//	Created 	: 14.04.2008
//  Modified 	: 14.04.2008
//	Author		: Dmitriy Iassenev
//	Description : script debugger backend class
////////////////////////////////////////////////////////////////////////////

#ifndef CS_LUA_DEBUGGER_BACKEND_H_INCLUDED
#define CS_LUA_DEBUGGER_BACKEND_H_INCLUDED

struct lua_State;
struct lua_Debug;

namespace cs {
namespace lua_debugger {

enum icon_type {
	icon_type_nil				= 0u,
	icon_type_boolean			= 0u,
	icon_type_number			= 0u,
	icon_type_string			= 0u,
	icon_type_table				= 1u,
	icon_type_function			= 3u,
	icon_type_thread			= 0u,
	icon_type_class				= 2u,
	icon_type_class_base		= 5u,
	icon_type_class_instance	= 4u,
	icon_type_unknown			= 0u,
}; // enum icon_type

struct DECLSPEC_NOVTABLE backend {
public:
	virtual	void	CS_LUA_DEBUGGER_CALL	type_to_string	(char* buffer, unsigned int size, lua_State* state, int index, bool& use_in_description) = 0;
	virtual	void	CS_LUA_DEBUGGER_CALL	value_to_string	(char* buffer, unsigned int size, lua_State* state, int index, icon_type& icon_type, bool full_description) = 0;
}; // struct DECLSPEC_NOVTABLE backend

struct DECLSPEC_NOVTABLE value_to_expand {
	virtual	void	CS_LUA_DEBUGGER_CALL	add_value		(char const* id, char const* type, char const* value, icon_type icon_type) = 0;
}; // struct DECLSPEC_NOVTABLE value_to_expand

} // namespace lua_debugger
} // namespace cs

#endif // #ifndef CS_LUA_DEBUGGER_BACKEND_H_INCLUDED