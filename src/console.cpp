#include <io.h>
#include <windows.h>

#include "console.hpp"
#include "exceptions.hpp"
#include "IMGFormat.hpp"

using namespace std;

console_writer &console_writer::get_instance(bool cout_enabled)
{
    static console_writer instance(cout_enabled);
    return instance;
}

console_writer::console_writer(bool cout_enabled)
: attached(false), allocated(false)
{
    attached = (AttachConsole(ATTACH_PARENT_PROCESS) != 0);
    if (!attached) {
        attached = (AllocConsole() != 0);
        allocated = attached;
    }
    if (attached) {
        if (cout_enabled) {
            HANDLE console_output = GetStdHandle(STD_OUTPUT_HANDLE);
            int system_output = _open_osfhandle(intptr_t(console_output), 0x4000);
            FILE *c_output_handle = _fdopen(system_output, "w");
            freopen_s(&c_output_handle, "CONOUT$", "w", stdout);
        }
        HANDLE console_error = GetStdHandle(STD_ERROR_HANDLE);
        int system_error = _open_osfhandle(intptr_t(console_error), 0x4000);
        FILE *c_error_handle = _fdopen(system_error, "w");
        freopen_s(&c_error_handle, "CONOUT$", "w", stderr);

        HANDLE console_input = GetStdHandle(STD_INPUT_HANDLE);
        int system_input = _open_osfhandle(intptr_t(console_input), 0x4000);
        FILE *c_input_handle = _fdopen(system_input, "r");
        freopen_s(&c_input_handle, "CONIN$", "r", stdin);

        cout << '\n';
    }
}

console_writer::~console_writer()
{
    if (attached) {
        for (int i = 0; i < 80 * 25; i++) {
            SendMessage(GetConsoleWindow(), WM_CHAR, '\b', 0);
        }
        if (allocated) {
            cin.get();
        }
        SendMessage(GetConsoleWindow(), WM_CHAR, '\r', 0);
        FreeConsole();
    }
}

int tee_streambuf::overflow(int c)
{
    if (c == EOF) {
        return !EOF;
    } else {
        const int ret1 = s1->sputc(c),
                  ret2 = s2->sputc(c);
        return (ret1 == EOF || ret2 == EOF ? EOF : c);
    }
}

int tee_streambuf::sync()
{
    const int ret1 = s1->pubsync(),
              ret2 = s2->pubsync();
    return (ret1 == 0 && ret2 == 0 ? 0 : -1);
}

tee_streambuf::tee_streambuf(streambuf *s1, streambuf *s2)
: s1(s1), s2(s2)
{}

tee_ostream::tee_ostream(ostream &o1, ostream &o2)
: ostream(&s), s(o1.rdbuf(), o2.rdbuf())
{}

enum output_format parse_output_format(string input)
{
	for (unsigned i = 0; i < input.size(); i++) {
		if (input[i] < 91) {
			input[i] += 32;
		}
	}
	if (input == string("bmp")) {
		return BMP;
	} else if (input == string("jpg") || input == string("jpeg")) {
		return JPG;
	} else if (input == string("png")) {
		return PNG;
	} else {
		throw parsing_error("Unknown output format!\n");
	}
}

void parse_cli_options(const int argc, char **argv, cli_options &options, int &arg_iter)
{
	//defaults
	options.decompress = true;
	options.format = PNG;
	options.custom_dir = false;
	options.add_game_name = false;
	options.verbose = false;
	//parsing
	string arg;
	for (; arg_iter < argc; arg_iter++) {
		if (argv[arg_iter][0] == '/' || argv[arg_iter][0] == '-') {
			arg = argv[arg_iter];
			if ((arg.size() == 2 && arg[1] == 'd') ||
				(arg.find("decompress") != string::npos &&
				(arg.size() == 11 || (arg.size() == 12 && arg[1] == '-')))) {
				//-d, /d, -decompress, /decompress, --decompress
				options.decompress = true;
			} else if ((arg.size() == 2 && arg[1] == 'c') ||
						(arg.find("compress") != string::npos &&
						(arg.size() == 9 || (arg.size() == 10 && arg[1] == '-')))) {
				//-c, /c, -compress, /compress, --compress
				options.decompress = false;
				throw parsing_error("Not implemented!\n");
			} else if ((arg.size() == 2 && arg[1] == 'f') ||
						arg.substr(1, 10) == string("out-format") ||
						(arg.substr(2, 10) == string("out-format") && arg[1] == '-')) {
				//-f, /f, -out-format, /out-format, --out-format
				if (arg.size() == 2) {
					if (arg_iter + 1 < argc) {
						arg_iter++;
						options.format = parse_output_format(string(argv[arg_iter]));
					} else {
						throw parsing_error("Option argument missing!\n");
					}
				} else {
					if (arg.size() > 10U + (arg[1] == '-' ? 2 : 1) + 1) {
						options.format = parse_output_format(arg.substr(10 + (arg[1] == '-' ? 2 : 1) + 1)); //length + prefix ("--") + suffix ('=')
					} else {
						throw parsing_error("Option argument missing!\n");
					}
				}
			} else if ((arg.size() == 2 && arg[1] == 'g') ||
				(arg.find("add-game-name") != string::npos &&
				(arg.size() == 14 || (arg.size() == 15 && arg[1] == '-')))) {
				//-g, /g, -add-game-name, /add-game-name, --add-game-name
				options.add_game_name = true;
			} else if ((arg.size() == 2 && arg[1] == 'h') ||
						(arg.find("help") != string::npos &&
						(arg.size() == 5 || (arg.size() == 6 && arg[1] == '-')))) {
				//-h, /h, -help, /help, --help
				throw help_issued();
			} else if ((arg.size() == 2 && arg[1] == 'o') ||
						arg.substr(1, 7) == string("out-dir") ||
						(arg.substr(2, 7) == string("out-dir") && arg[1] == '-')) {
				//-o, /o, -out-dir, /out-dir, --out-dir
				options.custom_dir = true;
				if (arg.size() == 2) {
					if (arg_iter + 1 < argc) {
						arg_iter++;
						options.dir = string(argv[arg_iter]);
					} else {
						throw parsing_error("Option argument missing!\n");
					}
				} else {
					if (arg.size() > 7U + (arg[1] == '-' ? 2 : 1) + 1) {
						options.dir = arg.substr(7 + (arg[1] == '-' ? 2 : 1) + 1); //length + prefix ("--") + suffix ('=')
					} else {
						throw parsing_error("Option argument missing!\n");
					}
				}
			} else if ((arg.size() == 2 && arg[1] == 'v') ||
				(arg.find("verbose") != string::npos &&
				(arg.size() == 8 || (arg.size() == 9 && arg[1] == '-')))) {
				//-v, /v, -verbose, /verbose, --verbose
				options.verbose = true;
			} else {
				throw parsing_error("Unknown option!\n");
			}
		} else {
			return;
		}
	}
}

void print_help()
{
	console_writer::get_instance(true);
	cout << "Usage:\n"
			"  dePIKczer [options] filename1[, filename2, ...]\n"
			"Available options (have to be prefixed with / or -):\n"
			"  d,                  decompress IMG file (default)\n"
			"  decompress\n"
			"  c,                  convert BMP, JPG or PNG file to IMG\n"
			"  compress              (not implemented yet)\n"
			"  f [format],         specify output file format\n"
			"  out-format=[format]   (for decompression only)\n"
			"                        available formats:\n"
			"                          bmp,\n"
			"                          jpg, jpeg,\n"
			"                          png (default)\n"
			"  g                   enable adding game suffix to output filenames\n"
			"  add-game-name         (experimental)\n"
			"  h,                  show this help message\n"
			"  help\n"
			"  o [sciezka],        use given directory for storing output files\n"
			"  out-dir=[sciezka]     (input files directory is used by default)\n"
			"  v,                  print additional debug data (forces console window\n"
			"  verbose               to be shown)\n";
}

void log_error(const char *description)
{
	console_writer::get_instance();
	string log_path = getenv("appdata");
	log_path.append("\\dePIKczer");
	CreateDirectoryA(log_path.c_str(), 0);
	ofstream log_file(log_path + string("\\error.log"), ios::out | ios::app);
	if (log_file.good()) {
		tee_ostream errout(cerr, log_file);
		errout << description << "\n\n";
		log_file.close();
	} else {
		cerr << description << '\n';
		cerr << "[Couldn't write to the log file]\n\n";
	}
}

void log_error(const char *description, const char *error_string)
{
	console_writer::get_instance();
	string log_path = getenv("appdata");
	log_path.append("\\dePIKczer");
	CreateDirectoryA(log_path.c_str(), 0);
	ofstream log_file(log_path + string("\\error.log"), ios::out | ios::app);
	if (log_file.good()) {
		tee_ostream errout(cerr, log_file);
		errout << description << "\n  " << error_string << "\n\n";
		log_file.close();
	} else {
		cerr << description << "\n  " << error_string << '\n';
		cerr << "[Couldn't write to the log file]\n\n";
	}
}

void log_error(const char *description, const char *error_string, const char *subject)
{
	console_writer::get_instance();
	string log_path = getenv("appdata");
	log_path.append("\\dePIKczer");
	CreateDirectoryA(log_path.c_str(), 0);
	ofstream log_file(log_path + string("\\error.log"), ios::out | ios::app);
	if (log_file.good()) {
		tee_ostream errout(cerr, log_file);
		errout << description << "\n  " << "for " << subject << "  \n" << error_string << "\n\n";
		log_file.close();
	} else {
		cerr << description << "\n  " << "for " << subject << "  \n" << error_string << '\n';
		cerr << "[Couldn't write to the log file]\n\n";
	}
}

void log_error(const char *description, const char *error_string, const char *subject, const IMGHeader &img_header)
{
	console_writer::get_instance();
	string log_path = getenv("appdata");
	log_path.append("\\dePIKczer");
	CreateDirectoryA(log_path.c_str(), 0);
	ofstream log_file(log_path + string("\\error.log"), ios::out | ios::app);
	if (log_file.good()) {
		tee_ostream errout(cerr, log_file);
		errout << description << "\n  for " << subject << "\n  " << error_string << '\n' << img_header << '\n';
		log_file.close();
	} else {
		cerr << description << "\n  for " << subject << "\n  " << error_string << '\n' << img_header;
		cerr << "[Couldn't write to the log file]\n\n";
	}
}

void log_error(const char *description, const char *error_string, const char *subject, const char *additional_info)
{
	console_writer::get_instance();
	string log_path = getenv("appdata");
	log_path.append("\\dePIKczer");
	CreateDirectoryA(log_path.c_str(), 0);
	ofstream log_file(log_path + string("\\error.log"), ios::out | ios::app);
	if (log_file.good()) {
		tee_ostream errout(cerr, log_file);
		errout << description << "\n  " << "for " << subject << "\n  " << error_string << "\n  (" << additional_info << ")\n\n";
		log_file.close();
	} else {
		cerr << description << "\n  " << "for " << subject << "\n  " << error_string << "\n  (" << additional_info << ")\n";
		cerr << "[Couldn't write to the log file]\n\n";
	}
}
