#include "BMPFormat.hpp"
#include "console.hpp"
#include "exceptions.hpp"
#include "ImageFormat.hpp"
#include "IMGFormat.hpp"
#include "JPGFormat.hpp"
#include "path.hpp"
#include "PNGFormat.hpp"

#define BUFF_SIZE 200

using namespace std;

void write_converted(ofstream &out_file, const vector<unsigned char> &out_data)
{
	out_file.write((const char *)out_data.data(), out_data.size());
}

int main(int argc, char **argv)
{
	push_back_characteristic_names();
	int starting_argument = 1;
	if (argc > 0) {
		string img_test = argv[0];
		img_test.erase(0, img_test.size() - 3);
		for (int i = 0; i < 3; i++) {
			if (img_test[i] > 64 && img_test[i] < 91) {
				img_test[i]++;
			}
		}
		if (argv[0][0] == '-' || argv[0][0] == '/' || img_test == string("img")) {
			starting_argument--;
		}
	}
    if (argc > starting_argument) {
		int arg_iter = starting_argument;
		cli_options options;
		try {
			parse_cli_options(argc, argv, options, arg_iter);
			if (options.verbose) {
				console_writer::get_instance(true);
			}
			for (; arg_iter < argc; arg_iter++) {
                IMGHeader img_header;
                vector<unsigned char> img_data_color, img_data_alpha;
                try {
                    cout << "Processing " << argv[arg_iter] << "...\n";
                    IMGFormat img_image(argv[arg_iter]);
                    img_header = img_image.get_header();
                    img_data_color = img_image.get_original_rgb_pixmap();
                    img_data_alpha = img_image.get_original_alpha_pixmap();
                    cout << img_header << '\n';
                    cout << "Read the image data!\n";
                    cout << "Size (in bytes): " << img_data_color.size() + img_data_alpha.size() << '\n';
                    if (img_data_alpha.size() > 0) {
                        cout << " (including alpha: " << img_data_alpha.size() << ")\n";
                    }

                    if (img_header.compression == 4) { //unspecified/experimental
                        determine_compression_format(img_header, img_data_color);
                        cout << "Unspecified compression format determined: " << uint32_t(img_header.compression) << '\n';
                    }
                    string out_filename;
                    out_filename = compose_out_filename(argv, starting_argument, arg_iter, options);
                    ofstream out_file(out_filename, ios::out | ios::binary);
                    if (out_file.good()) {
                        vector<unsigned char> converted_data;
                        switch (options.format) {
                            case BMP: {
                                converted_data = prepare_bmp_data(img_header, img_data_color, img_data_alpha);
                                cout << "Successfully converted to BMP!\n";
                                cout << "New size (in bytes): " << converted_data.size() << '\n';
                                break;
                            }
                            case JPG: {
                                converted_data = prepare_jpg_data(img_header, img_data_color);
                                cout << "Successfully converted to JPG!\n";
                                cout << "New size (in bytes): " << converted_data.size() << '\n';
                                break;
                            }
                            case PNG: {
                                converted_data = prepare_png_data(img_header, img_data_color, img_data_alpha);
                                cout << "Successfully converted to PNG!\n";
                                cout << "New size (in bytes): " << converted_data.size() << '\n';
                                break;
                            }
                        }
                        switch (options.format) {
                            case BMP: {
                                write_bmp(out_file, img_header, converted_data);
                                break;
                            }
                            default: {
                                write_converted(out_file, converted_data);
                                break;
                            }
                        }
                        cout << "Saved to " << out_filename << "!\n";
                        out_file.close();
                    } else {
                        log_error("Error opening output file!", strerror(errno), argv[arg_iter], out_filename.c_str());
                    }
                } catch (const compression_failure &e) {
                    log_error("Compression failure!", e.what(), argv[arg_iter], img_header);
                } catch (const invalid_size &e) {
                    log_error("Invalid size!", e.what(), argv[arg_iter], img_header);
                } catch (const invalid_structure &e) {
                    log_error("Invalid IMG header structure!", e.what(), argv[arg_iter], img_header);
                } catch (const path_error &e) {
                    log_error("Path-related error!", e.what(), argv[arg_iter], img_header);
                } catch (const exception &e) {
                    log_error("Unexpected exception inside the main loop!", e.what(), argv[arg_iter], img_header);
                }
			}
		} catch (const parsing_error &e) {
			log_error("Console options parsing error!", e.what(), argv[arg_iter]);
		} catch (const help_issued &) {
			print_help();
		} catch (const exception &e) {
			log_error("Unexpected exception during the initialization!", e.what());
		}
    } else {
        console_writer::get_instance(true);
		log_error("Error: No input files specified!\n");
        print_help();
    }
    return 0;
}
