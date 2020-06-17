#ifndef DEPIKCZER_CONSOLE_HPP
#define DEPIKCZER_CONSOLE_HPP

#include <iostream>
#include <string>

#include "IMGFormat.hpp"

enum output_format {
	BMP,
	JPG,
	PNG
};

struct cli_options {
	enum output_format format;
	bool decompress;
	bool custom_dir;
    bool add_game_name;
    bool verbose;
	std::string dir;
};

class console_writer {
public:
	static console_writer &get_instance(bool cout_enabled = false);
private:
	bool attached;
    bool allocated;
	console_writer(bool cout_enabled);
	~console_writer();
};

class tee_streambuf : public std::streambuf {
	std::streambuf *s1;
	std::streambuf *s2;
	virtual int overflow(int c);
	virtual int sync();
public:
	tee_streambuf(std::streambuf *s1, std::streambuf *s2);
};

class tee_ostream : public std::ostream {
	tee_streambuf s;
public:
	tee_ostream(std::ostream &o1, std::ostream &o2);
};

enum output_format parse_output_format(std::string input);

void parse_cli_options(const int argc, char **argv, cli_options &options, int &arg_iter);

void print_help();

void log_error(const char *description);

void log_error(const char *description, const char *error_string);

void log_error(const char *description, const char *error_string, const char *subject);

void log_error(const char *description, const char *error_string, const char *subject, const IMGHeader &img_header);

void log_error(const char *description, const char *error_string, const char *subject, const char *additional_info);

#endif // DEPIKCZER_CONSOLE_HPP
