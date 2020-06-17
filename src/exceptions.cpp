#include "exceptions.hpp"

using namespace std;

compression_failure::compression_failure(const string &what_arg)
: runtime_error(what_arg)
{}
compression_failure::compression_failure(const char *what_arg)
: runtime_error(what_arg)
{}
compression_failure::compression_failure()
: runtime_error("Unknown compression error!\n")
{}
compression_failure::compression_failure(const compression_failure &e)
: runtime_error(e.what())
{}

invalid_size::invalid_size(const string &what_arg)
: logic_error(what_arg)
{}
invalid_size::invalid_size(const char *what_arg)
: logic_error(what_arg)
{}
invalid_size::invalid_size()
: logic_error("Unknown data size error!\n")
{}
invalid_size::invalid_size(const invalid_size &e)
: logic_error(e.what())
{}

invalid_structure::invalid_structure(const string &what_arg)
: runtime_error(what_arg)
{}
invalid_structure::invalid_structure(const char *what_arg)
: runtime_error(what_arg)
{}
invalid_structure::invalid_structure()
: runtime_error("Unknown file structure error!\n")
{}
invalid_structure::invalid_structure(const invalid_structure &e)
: runtime_error(e.what())
{}

io_failure::io_failure(const string &what_arg)
: runtime_error(what_arg)
{}
io_failure::io_failure(const char *what_arg)
: runtime_error(what_arg)
{}
io_failure::io_failure()
: runtime_error("Unknown I/O error!\n")
{}
io_failure::io_failure(const io_failure &e)
: runtime_error(e.what())
{}

parsing_error::parsing_error(const string &what_arg)
: runtime_error(what_arg)
{}
parsing_error::parsing_error(const char *what_arg)
: runtime_error(what_arg)
{}
parsing_error::parsing_error()
: runtime_error("Unknown console options parsing error!\n")
{}
parsing_error::parsing_error(const parsing_error &e)
: runtime_error(e.what())
{}

path_error::path_error(const string &what_arg)
: runtime_error(what_arg)
{}
path_error::path_error(const char *what_arg)
: runtime_error(what_arg)
{}
path_error::path_error()
: runtime_error("Unknown path-related error!\n")
{}
path_error::path_error(const parsing_error &e)
: runtime_error(e.what())
{}
