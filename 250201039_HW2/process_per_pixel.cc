//BURAK TUTUMLU - 250201039 - HW2 - CENG391

#include <cstdlib>
#include <iostream>
#include <string>

#include "image.hpp"

using std::cerr;
using std::clog;
using std::endl;
using std::string;

using ceng391::Image;

void print_usage(const char *prog_name);

int main(int argc, char** argv)
{
        if (argc < 4) {
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }

        string filename = argv[1];
        float multiplier = atof(argv[2]);
        float offset = atof(argv[3]);

        Image *img = Image::from_png(filename, true);
        clog << "Loaded " << img->w() << "x" << img->h()
             << " image from " << filename << endl;
        clog << "Changing brightness by I < " << multiplier
             << " * I + " << offset << endl;

        img->axpc(multiplier, offset);

        string output_filename = "/tmp/output_pp.png";
        img->save_as_png(output_filename);
        clog << "Saved output to " << output_filename << endl;

        delete img;

        return EXIT_SUCCESS;
}

void print_usage(const char *prog_name)
{
        cerr << prog_name << " <input_image> <multipler> <offset>" << endl;
}

/// Local Variables:
/// mode: c++
/// compile-command: "make process-per-pixel"
/// End:
