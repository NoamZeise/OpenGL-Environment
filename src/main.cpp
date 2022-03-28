#include <stdexcept>
#include <iostream>

#include "app.h"


int main()
{
	try
	{
		#ifndef NDEBUG
		std::cout << "In debug mode" << std::endl;
		#endif
		App app;
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
