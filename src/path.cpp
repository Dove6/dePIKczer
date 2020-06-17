#include <array>
#include <windows.h>

#include "exceptions.hpp"
#include "path.hpp"

using namespace std;

array<string, 6> names_to_append = {"RiSP", "RiU", "RiC", "RiWC", "RiKN", "RiKwA"};

array<vector<string>, 6> characteristic_names;

string get_winapi_error_msg(unsigned err_code)
{
	char *buffer = nullptr;
	unsigned buffer_size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0, err_code, 0, LPSTR(&buffer), 0, nullptr);
	string message(buffer, buffer_size);
	LocalFree(buffer);
	return message;
}

void push_back_characteristic_names()
{
	characteristic_names[0].push_back("pirat");
	characteristic_names[0].push_back("skarb");
	characteristic_names[0].push_back("mainmenu");
	characteristic_names[0].push_back("risp");
	characteristic_names[1].push_back("ufo");
	characteristic_names[1].push_back("riu");
	characteristic_names[2].push_back("czaro");
	characteristic_names[2].push_back("ric");
	characteristic_names[3].push_back("wehiku");
	characteristic_names[3].push_back("czasu");
	characteristic_names[3].push_back("riwc");
	characteristic_names[4].push_back("nemo");
	characteristic_names[4].push_back("kapitan");
	characteristic_names[4].push_back("rikn");
	characteristic_names[5].push_back("akcji");
	characteristic_names[5].push_back("rikwa");
}

string compose_out_filename(char **argv, const int starting_argument, const int arg_iter, const cli_options &options)
{
	string result;
	string exe_path;
	if (starting_argument == 1) {
		exe_path = argv[0];
	}
	string element = argv[arg_iter];
	//appending directory
	if (options.custom_dir) {
		if (!CreateDirectoryA(options.dir.c_str(), 0)) {
			unsigned err = GetLastError();
			if (err == ERROR_PATH_NOT_FOUND) {
				throw path_error("Incorrect output directory path!");
			} else if (err != ERROR_ALREADY_EXISTS) {
				throw path_error(get_winapi_error_msg(err));
			}
		}
		result = options.dir;
	} else {
		if (element.find_last_of('\\') != string::npos) {
			result.append(element.substr(0, element.find_last_of('\\')));
		}
	}
	if (result.size() > 0) {
		if (result.back() != '\\') {
			result.append("\\");
		}
	}
	//input filename part
	if (element.find_last_of('\\') != string::npos) {
		result.append(element.substr(element.find_last_of('\\') + 1));
	} else {
		result.append(element);
	}
	if (result[result.size() - 4] == '.') {
		result.erase(result.size() - 4);
	}
	//game name part
	for (unsigned i = 0; i < exe_path.size(); i++) {
		if (exe_path[i] > 64 && exe_path[i] < 91) {
			exe_path[i] += 32;
		}
	}
	for (unsigned i = 0; i < element.size(); i++) {
		if (element[i] > 64 && element[i] < 91) {
			element[i] += 32;
		}
	}
	if (options.add_game_name) {
		for (int i = 0; i < 6; i++) {
			for (unsigned j = 0; j < characteristic_names[i].size(); j++) {
				if (exe_path.find(characteristic_names[i][j]) != string::npos || element.find(characteristic_names[i][j]) != string::npos) {
					result.append("_");
					result.append(names_to_append[i]);
					i = 6;
					break;
				}
			}
		}
	}
	//number part
	string core_filename = result;
	string extension;
	switch (options.format) {
		case BMP: {
			extension = ".bmp";
			break;
		}
		case JPG: {
			extension = ".jpg";
			break;
		}
		case PNG: {
			extension = ".png";
			break;
		}
	}
	result.append(extension);
	ifstream existance_test(result, ios::in);
	unsigned i;
	for (i = 2; existance_test.good() && i <= 1000; i++) {
		existance_test.close();
		result = core_filename + string("_") + to_string((unsigned long long)(i)) + extension;
		existance_test.open(result, ios::in);
	}
	if (i == 1000) {
		throw path_error("Couldn't compose output filename!");
	}
	return result;
}
