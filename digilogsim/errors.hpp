
#ifndef _DIGILOG_ERRORS_HPP_
#define _DIGILOG_ERRORS_HPP_


#include <source_location>
#include <ostream>
#include <string>



struct error_t
{
	std::source_location src;
	const char* what = nullptr;
	std::string msg;
};

using excp_callback_t = void(*)(error_t*, void* user_param);



void _dls_raise(error_t);



#define excp_raise(what, msg) _dls_raise(error_t{std::source_location::current(), what, msg})

void excp_try(void (*try_block)(), excp_callback_t catch_block, void* user_param);


#endif //!_DIGILOG_ERRORS_HPP_
