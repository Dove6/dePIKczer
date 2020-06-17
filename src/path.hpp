#ifndef DEPIKCZER_PATH_HPP
#define DEPIKCZER_PATH_HPP

#include <string>

#include "console.hpp"

std::string get_winapi_error_msg(unsigned err_code);

void push_back_characteristic_names();

std::string compose_out_filename(char **argv, const int starting_argument, const int arg_iter, const cli_options &options);

#endif // DEPIKCZER_PATH_HPP
