#pragma once
#include <iostream>

#define ASSERT_HR(hr) \
	if (FAILED(hr))\
	{\
		std::cerr << "assert failed: " << __LINE__ << '@' << __FUNCTION__ << std::endl;\
		exit(hr);\
	}\
\

#define CHECK_HR(block, onFailed) \
{\
	auto __check_hr_hr = block;\
	if (FAILED(__check_hr_hr))\
	{\
		std::cerr << __FILE__ << ": " << __LINE__ << '@' << __FUNCTION__ << ": " << _com_error(__check_hr_hr).ErrorMessage() << std::endl;\
		onFailed;\
	}\
}
