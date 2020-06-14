// piklib8_shim.cpp : Definiuje eksportowane funkcje dla aplikacji DLL.
//

#include "stdafx.h"
#include "piklib8_shim.h"
#include "piklib8_CLZW.h"
#include <cstring>

PIKLIB8SHIM_API char *piklib_CLZWCompression2_compress(char *input_string, int input_size, int *output_size)
{
	CLZWCompression2 clzw(input_string, input_size);
	char *ret = clzw.compress(*output_size);
	return ret;
}

PIKLIB8SHIM_API char *piklib_CLZWCompression2_decompress(char *input_string, int input_size)
{
	CLZWCompression2 clzw(input_string, input_size);
	char *ret = clzw.decompress();
	return ret;
}

PIKLIB8SHIM_API void piklib_CLZWCompression2_deallocate(char *output_string)
{
	delete[] output_string;
}
