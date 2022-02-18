
#include "errors.hpp"

#include <utility>
#include <vector>


static excp_callback_t current_callback;
static void* current_user_param;

void _dls_raise(error_t err)
{
	current_callback(&err, current_user_param);
	exit(1);
}

void excp_try(void(*try_block)(), excp_callback_t catch_block, void* user_param)
{
	current_callback = catch_block;
	current_user_param = user_param;
	try_block();
}
