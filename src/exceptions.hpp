#ifndef DEPIKCZER_EXCEPTIONS_HPP
#define DEPIKCZER_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

class compression_failure : public std::runtime_error {
public:
	compression_failure(const std::string &what_arg);
	compression_failure(const char *what_arg);
	compression_failure();
	compression_failure(const compression_failure &e);
};

class help_issued : public std::exception {
};

class invalid_size : public std::logic_error {
public:
	invalid_size(const std::string &what_arg);
	invalid_size(const char *what_arg);
	invalid_size();
	invalid_size(const invalid_size &e);
};

class invalid_structure : public std::runtime_error {
public:
	invalid_structure(const std::string &what_arg);
	invalid_structure(const char *what_arg);
	invalid_structure();
	invalid_structure(const invalid_structure &e);
};

class io_failure : public std::runtime_error {
public:
	io_failure(const std::string &what_arg);
	io_failure(const char *what_arg);
	io_failure();
	io_failure(const io_failure &e);
};

class parsing_error : public std::runtime_error {
public:
	parsing_error(const std::string &what_arg);
	parsing_error(const char *what_arg);
	parsing_error();
	parsing_error(const parsing_error &e);
};

class path_error : public std::runtime_error {
public:
	path_error(const std::string &what_arg);
	path_error(const char *what_arg);
	path_error();
	path_error(const parsing_error &e);
};

#endif // DEPIKCZER_EXCEPTIONS_HPP
