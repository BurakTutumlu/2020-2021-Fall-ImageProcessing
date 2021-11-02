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

const double PI = 3.14159265359;

int main(int argc, char **argv)
{
        if (argc < 3)
        {
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }

        string filename = argv[1];
        double angle = atof(argv[2]);
        bool rotate_by_center = true;

        Image *img = Image::from_png(filename, true);
        clog << "Loaded " << img->w() << "x" << img->h()
             << " image from " << filename << endl;
        clog << "Rotating image by " << angle << " degrees " << endl;

        Image *Ir = Image::new_gray(2*img->w(), 2*img->h());
        img->rotated(Ir, angle * PI / 180.0, rotate_by_center);
        string output_filename = "/tmp/output_rotated.png";
        Ir->save_as_png(output_filename);
        clog << "Saved rotated image to " << output_filename << endl;
        

        delete img;
        return EXIT_SUCCESS;
}

void print_usage(const char *prog_name)
{
        cerr << prog_name << " <input_image> <angle_in_degrees>" << endl;
}

/// Local Variables:
/// mode: c++
/// compile-command: "make rotate-image"
/// End:
