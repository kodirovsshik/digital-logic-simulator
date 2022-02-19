
#include "errors.hpp"
#include "globals.hpp"

#include <iostream>


void wrap1()
{
	__try
	{
		void digilog_main();
		digilog_main();
	}
	__except (1)
	{
		excp_raise("SEH exception handled");
	}
}

void exception_handler(error_t* err, void*)
{
	logger << "Exception handled:\n";
	logger << err->src.file_name() << ':';
	logger << err->src.function_name() << ':';
	logger << err->src.line() << ":\n";
	logger << err->what << '\n';
	logger << err->msg;
}

int main()
{
	logger.add(std::cerr);

	excp_try(wrap1, exception_handler, NULL);

	logger << "Process finished normally\n";
	return 0;
}