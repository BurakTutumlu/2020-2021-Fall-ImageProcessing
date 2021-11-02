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

#include "image.hpp"

using std::clog;
using std::cerr;
using std::endl;

using ceng391::Image;
using ceng391::uchar;

// Ideally these test should return test/fail status. Better yet use a proper
// unit test framework such as gtest.
void test_grayscale_creation_and_pgm_io();
void test_grayscale_png_load_save();
void test_rgba_creation_and_ppm_io();

int main(int argc, char** argv)
{

        test_grayscale_creation_and_pgm_io();
        test_rgba_creation_and_ppm_io();
        test_grayscale_png_load_save();

        return EXIT_SUCCESS;
}

void test_grayscale_creation_and_pgm_io()
{
        clog << "***** Grayscale Creation and PGM I/O *****" << endl;
        Image *img = Image::new_gray(200, 100);

        clog << "Created new image of size " << img->w()
             << "x" << img->h() << " with " << img->n_ch() << endl;

        img->set(10);
        img->set_rect(50, 50, 10, 30, 255);

        img->save_as_pnm("/tmp/test_image");
        clog << "Saved image as /tmp/test_image.pgm" << endl;

        Image *img2 = Image::from_pnm("/tmp/test_image.pgm");
        clog << "Loaded image of size " << img2->w()
             << "x" << img2->h() << " with " << img2->n_ch() << endl;

        if (img->w() != img2->w() || img->h() != img2->h()
            || img->n_ch() != img2->n_ch()) {
                cerr << "LOADED PNM DIMENSIONS ARE DIFFERENT" << endl;
                exit(EXIT_FAILURE);
        }

        for (int y = 0; y < img2->h(); ++y) {
                const uchar *row = img->data(y);
                const uchar *row2 = img2->data(y);
                for (int x = 0; x < img2->w(); ++x) {
                        if (row[x] != row2[x]) {
                                cerr << "LOADED PNM " << x << ", " << y << " VALUES ARE DIFFERENT" << endl;
                                exit(EXIT_FAILURE);
                        }
                }
        }

        delete img;
        delete img2;
}

void test_rgba_creation_and_ppm_io()
{
        clog << "***** PPM Creation and PPM I/O *****" << endl;
        Image *img = Image::new_rgba(200, 100);

        clog << "Created new image of size " << img->w()
             << "x" << img->h() << " with " << img->n_ch() << endl;

        img->set(10);
        img->set_rect(50, 50, 10, 30, 255, 0, 0, 255);

        img->save_as_pnm("/tmp/test_image_rgba");
        clog << "Saved image as /tmp/test_image_rgba.ppm" << endl;

        Image *img2 = Image::from_pnm("/tmp/test_image_rgba.ppm");
        clog << "Loaded image of size " << img2->w()
             << "x" << img2->h() << " with " << img2->n_ch() << endl;

        if (img->w() != img2->w() || img->h() != img2->h()
            || img->n_ch() != img2->n_ch()) {
                cerr << "LOADED PNM DIMENSIONS ARE DIFFERENT" << endl;
                exit(EXIT_FAILURE);
        }

        for (int y = 0; y < img2->h(); ++y) {
                const uchar *row = img->data(y);
                const uchar *row2 = img2->data(y);
                for (int x = 0; x < img2->w(); ++x) {
                        if (row[4*x] != row2[4*x]
                            || row[4*x+1] != row2[4*x+1]
                            || row[4*x+2] != row2[4*x+2]
                            || row2[4*x+3] != 255) {
                                cerr << "LOADED PNM " << x << ", " << y << " VALUES ARE DIFFERENT" << endl;
                                exit(EXIT_FAILURE);
                        }
                }
        }

        delete img;
        delete img2;
}

void test_grayscale_png_load_save()
{
        clog << "***** Grayscale PNG Load/Save *****" << endl;
        // Ideally we must not hardcode paths in strings like this. They should
        // be taken as command line arguments or read from environment
        // variables.
        Image *img = Image::from_png("../data/small_city.png", true);

        clog << "Loaded image of size " << img->w()
             << "x" << img->h() << " with " << img->n_ch() << endl;

        img->save_as_png("/tmp/small_city_gray.png");
        clog << "Saved image as /tmp/small_city_gray.png" << endl;

        delete img;
}
