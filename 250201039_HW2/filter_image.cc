//BURAK TUTUMLU - 250201039 - HW2 - CENG391

#include <cstdlib>
#include <iostream>

#include "image.hpp"

using std::cerr;
using std::clog;
using std::endl;
using std::string;

using ceng391::Image;

void print_usage(const char *prog_name);

int main(int argc, char** argv)
{
        if (argc < 3) {
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }

        string filename = argv[1];
        int filter_size = atoi(argv[2]);

        Image *img = Image::from_png(filename, true);
        clog << "Loaded " << img->w() << "x" << img->h()
             << " image from " << filename << endl;
        clog << "Filtering image with a box filter of size "
             << filter_size << endl;
        img->box_filter_x(filter_size);

        string output_filename = "/tmp/output_box_x.png";
        img->save_as_png(output_filename);
        clog << "Saved box filtered in x to output to " << output_filename << endl;

        img->load_png(filename, true);
        img->box_filter_y(filter_size);
        output_filename = "/tmp/output_box_y.png";
        img->save_as_png(output_filename);
        clog << "Saved box filtered in y to output to " << output_filename << endl;

        img->load_png(filename, true);
        img->box_filter(filter_size);
        output_filename = "/tmp/output_box.png";
        img->save_as_png(output_filename);
        clog << "Saved box filtered in x&y to output to " << output_filename << endl;

        delete img;

        return EXIT_SUCCESS;
}

void print_usage(const char *prog_name)
{
        cerr << prog_name << " <input_image> <filter_length>" << endl;
}

/// Local Variables:
/// mode: c++
/// compile-command: "make filter-image"
/// End:
