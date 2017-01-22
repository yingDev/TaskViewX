#pragma once
#include <iostream>


inline HRESULT& LAST_HRULST()
{
	static thread_local HRESULT VALUE = S_OK;
	
	return VALUE;
}

#define ASSERT_HR(hr) \
	if (FAILED(hr))\
	{\
		std::cerr << "assert failed: " << __LINE__ << '@' << __FUNCTION__ << std::endl;\
		exit(hr);\
	}\
\

#define CHECK_HR(block, onFailed) \
{\
	LAST_HRULST() = block;\
	if (FAILED(LAST_HRULST()))\
	{\
		std::cerr << __FILE__ << ": " << __LINE__ << '@' << __FUNCTION__ << ": " << _com_error(LAST_HRULST()).ErrorMessage() << std::endl;\
		onFailed;\
	}\
}
