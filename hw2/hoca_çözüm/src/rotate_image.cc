// ------------------------------
// Written by Mustafa Ozuysal
// Contact <mustafaozuysal@iyte.edu.tr> for comments and bug reports
// ------------------------------
// Copyright (c) 2020, Mustafa Ozuysal
// All rights reserved.
// ------------------------------
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the copyright holders nor the
//       names of his/its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// ------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ------------------------------
#include <cstdlib>
#include <iostream>
#include <cmath>

#include "image.hpp"

using std::cerr;
using std::clog;
using std::endl;
using std::string;
using std::max;
using std::min;
using std::cos;
using std::sin;

using ceng391::Image;

void print_usage(const char *prog_name);

const double PI = 3.14159265359;

static inline double min4(double a, double b, double c, double d)
{
        return min(min(a, b), min(c, d));
}

static inline double max4(double a, double b, double c, double d)
{
        return max(max(a, b), max(c, d));
}

// Computes an affine tranformation that corresponds to a rotation by center
// such that the output image fits in the size given by width x height at the
// output. (The resulting A, t is still an Euclidean transform R, t)
static void affine_from_rotate_by_center(int &width, int &height,
                                         double angle, double *A, double *t)
{
        double w = width;
        double h = height;
        double ca = cos(angle);
        double sa = sin(angle);
        double x_min = min4(0, w*ca, -h*sa, w*ca - h*sa);
        double x_max = max4(0, w*ca, -h*sa, w*ca - h*sa);
        double y_min = min4(0, w*sa, h*ca, h*ca + w*sa);
        double y_max = max4(0, w*sa, h*ca, h*ca + w*sa);
        double wp = x_max - x_min;
        double hp = y_max - y_min;

        double hw = w / 2.0;
        double hh = h / 2.0;
        double hwp = wp / 2.0;
        double hhp = hp / 2.0;
        A[0] = ca;
        A[1] = sa;
        A[2] = -sa;
        A[3] = ca;
        t[0] = hwp - ca*hw + sa*hh;
        t[1] = hhp - sa*hw - ca*hh;
        width = wp+0.5;
        height = hp+0.5;
}

int main(int argc, char** argv)
{
        if (argc < 3) {
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

        int width = img->w();
        int height = img->h();
        double A[4];
        double t[2];
        affine_from_rotate_by_center(width, height, angle * PI / 180.0,
                                     &A[0], &t[0]);
        Image *Ia = Image::new_gray(width, height);
        img->warp_affine(Ia, &A[0], &t[0], true);
        output_filename = "/tmp/output_rotated_by_affine.png";
        Ia->save_as_png(output_filename);
        clog << "Saved affine rotated image to " << output_filename << endl;

        delete img;
        delete Ir;
        delete Ia;

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
