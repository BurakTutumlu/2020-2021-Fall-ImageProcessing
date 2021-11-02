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
#include "image.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cmath>

#include <png.h>

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#ifndef png_infopp_NULL
#  define png_infopp_NULL (png_infopp)NULL
#endif

#ifndef int_p_NULL
# define int_p_NULL (int*)NULL
#endif

using std::cerr;
using std::clog;
using std::cos;
using std::endl;
using std::exit;
using std::fclose;
using std::fopen;
using std::ios;
using std::memcpy;
using std::memset;
using std::ofstream;
using std::round;
using std::sin;
using std::string;

namespace ceng391 {

Image::Image(int width, int height, int n_channels, int step)
        : m_width(width), m_height(height), m_n_channels(n_channels)
{
        if (step < width * n_channels)
                step = width * n_channels;
        m_step = step;
        m_data = new uchar[m_height * m_step];
}

Image::~Image()
{
        // clog << "Deleting image" << endl;
        delete [] m_data;
}

Image *Image::new_gray(int width, int height)
{
        return new Image(width, height, 1);
}

Image *Image::new_rgba(int width, int height)
{
        return new Image(width, height, 4);
}

Image *Image::from_pnm(const std::string &filename)
{
        // Allocate small temporary image
        Image *img = new_gray(1, 1);
        if (!img->load_pnm(filename)) {
                exit(EXIT_FAILURE);
        }
        return img;
}

Image *Image::from_png(const std::string &filename, bool load_as_grayscale)
{
        // Allocate small temporary image
        Image *img = new_gray(1, 1);
        if (!img->load_png(filename, load_as_grayscale)) {
                exit(EXIT_FAILURE);
        }
        return img;
}

Image *Image::from_deriv(int width, int height, const short *deriv)
{
        Image *img = Image::new_gray(width, height);
        for (int y = 0; y < height; ++y) {
                uchar *row = img->data(y);
                const short *row_d = deriv + y * width;
                for (int x = 0; x < width; ++x) {
                        row[x] = (row_d[x] + 4 * 255) / 8;
                }
        }
        return img;
}

uchar Image::sample_bilinear(double x, double y, uchar default_value)
{
        uchar value = default_value;
        // interpolate bilinearly
        int xi = static_cast<int>(x);
        int yi = static_cast<int>(y);
        if (xi >= 0 && yi >= 0
            && xi < m_width-1 && yi < m_height-1) {
                uchar I00 = m_data[yi * m_step + xi];
                uchar I10 = m_data[yi * m_step + (xi+1)];
                uchar I01 = m_data[(yi + 1) * m_step + xi];
                uchar I11 = m_data[(yi + 1) * m_step + (xi+1)];
                double alpha = x - xi;
                double beta = y - yi;
                double interp = (1.0 - alpha) * (1.0 - beta) * I00
                        + (1.0 - alpha) * beta * I01
                        + alpha * (1.0 - beta) * I10
                        + alpha * beta * I11;
                int interpi = static_cast<int>(interp);
                if (interpi < 0)
                        interpi = 0;
                else if (interpi > 255)
                        interpi = 255;
                value = interpi;
        }
        return value;
}

void Image::reallocate(int width, int height, int n_channels)
{
        if (width  != this->m_width ||
            height != this->m_height ||
            n_channels != this->m_n_channels) {
                delete [] m_data;
                int step = width * n_channels;
                m_step = step;
                m_data = new uchar[height * m_step];
                m_width = width;
                m_height = height;
                m_n_channels = n_channels;
        }
}

void Image::to_grayscale()
{
        if (m_n_channels == 1) {
                return;
        }

        uchar *gray_data = new uchar[m_height * m_width];
        for (int y = 0; y < m_height; ++y) {
                const uchar *row_data = this->data(y);
                uchar *row_gray = gray_data + y * m_width;
                for (int x = 0; x < m_width; ++x) {
                        uchar value = rgb_to_gray(row_data[4*x],
                                                  row_data[4*x + 1],
                                                  row_data[4*x + 2]);

                        row_gray[x] = value;
                }
        }

        delete [] m_data;
        m_data = gray_data;
        m_n_channels = 1;
        m_step = m_width;
}

void Image::to_rgba()
{
        if (m_n_channels == 4) {
                return;
        }

        uchar *rgba_data = new uchar[m_height * m_width * 4];
        for (int y = 0; y < m_height; ++y) {
                const uchar *row_data = this->data(y);
                uchar *row_rgba = rgba_data + y * m_width * 4;
                for (int x = 0; x < m_width; ++x) {
                        row_rgba[4*x]     = row_data[x];
                        row_rgba[4*x + 1] = row_data[x];
                        row_rgba[4*x + 2] = row_data[x];
                        row_rgba[4*x + 3] = 255;
                }
        }

        delete [] m_data;
        m_data = rgba_data;
        m_n_channels = 4;
        m_step = m_width * 4;
}

void Image::set_rect(int tlx, int tly, int width, int height, uchar value)
{
        if (tlx < 0) {
                width += tlx;
                tlx = 0;
        }

        if (tly < 0) {
                height += tly;
                tly = 0;
        }

        if (m_n_channels == 1) {
                for (int y = tly; y < tly + height; ++y) {
                        if (y >= m_height)
                                break;
                        uchar *row = m_data + y * m_step;
                        for (int x = tlx; x < tlx + width; ++x) {
                                if (x >= m_width)
                                        break;
                                row[x] = value;
                        }
                }
        } else { // m_n_channels == 4
                for (int y = tly; y < tly + height; ++y) {
                        if (y >= m_height)
                                break;
                        uchar *row = m_data + y * m_step;
                        for (int x = tlx; x < tlx + width; ++x) {
                                if (x >= m_width)
                                        break;
                        row[4*x] = value;
                        row[4*x + 1] = value;
                        row[4*x + 2] = value;
                        row[4*x + 3] = 255;
                        }
                }
        }
}

void Image::set_rect(int tlx, int tly, int width, int height,
                     uchar red, uchar green, uchar blue, uchar alpha)
{
        if (m_n_channels == 1) {
                uchar value = rgb_to_gray(red, green, blue);
                set_rect(tlx, tly, width, height, value);
                return;
        }

        if (tlx < 0) {
                width += tlx;
                tlx = 0;
        }

        if (tly < 0) {
                height += tly;
                tly = 0;
        }

        for (int y = tly; y < tly + height; ++y) {
                if (y >= m_height)
                        break;
                uchar *row = m_data + y * m_step;
                for (int x = tlx; x < tlx + width; ++x) {
                        if (x >= m_width)
                                break;
                        row[4*x] = red;
                        row[4*x + 1] = green;
                        row[4*x + 2] = blue;
                        row[4*x + 3] = alpha;
                }
        }
}

void Image::axpc(float multiplier, float offset)
{
        if (m_n_channels != 1) {
                cerr << "[ERROR][CENG391::Image] Currently offsets can only be added to grayscale images!\n";
                exit(EXIT_FAILURE);
        }

        for (int y = 0; y < this->h(); ++y) {
                uchar *rowp = this->data(y);
                for (int x = 0; x < this->w(); ++x) {
                        int v = rowp[x];
                        v = multiplier * v + offset;
                        if (v < 0) {
                                v = 0;
                        } else if (v > 255) {
                                v = 255;
                        }
                        rowp[x] = v;
                }
        }
}

template <typename T>
static void copy_to_buffer_1d(int n, T* buffer,
                              const uchar *src, int padding)
{
        for (int i = 0; i < padding; ++i)
                buffer[i] = src[0];
        for (int i = 0; i < n; ++i)
                buffer[i + padding] = src[i];
        for (int i = 0; i < padding; ++i)
                buffer[padding + n + i] = src[n - 1];
}

template <typename T>
static void copy_column_to_buffer_1d(int n, T* buffer, const uchar *src,
                                     int step, int padding)
{
        for (int i = 0; i < padding; ++i)
                buffer[i] = src[0];
        for (int i = 0; i < n; ++i)
                buffer[i + padding] = src[i * step];
        for (int i = 0; i < padding; ++i)
                buffer[padding + n + i] = src[n - 1];
}

static void box_filter_buffer_1d(int n, uchar* buffer, int padding)
{
        int filter_size = 2 * padding + 1;
        for (int i = 0; i < n; ++i) {
                int sum = 0;
                for (int j = 0; j < filter_size; ++j)
                        sum += buffer[i + j];
                sum /= filter_size;
                buffer[i] = sum;
        }
}

void Image::box_filter_x(int filter_size)
{
        if (m_n_channels != 1) {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be box filtered!\n";
                exit(EXIT_FAILURE);
        }

        int padding = filter_size / 2;

        uchar *buffer = new uchar[2 * padding + m_width];

        for (int y = 0; y < m_height; ++y) {
                uchar *rowp = this->data(y);
                copy_to_buffer_1d(m_width, buffer, rowp, padding);
                box_filter_buffer_1d(m_width, buffer, padding);
                memcpy(rowp, buffer, m_width * sizeof(uchar));
        }

        delete [] buffer;
}

void Image::box_filter_y(int filter_size)
{
        if (m_n_channels != 1) {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be box filtered!\n";
                exit(EXIT_FAILURE);
        }

        int padding = filter_size / 2;

        uchar *buffer = new uchar[2 * padding + m_height];

        for (int x = 0; x < m_width; ++x) {
                uchar *colp = m_data + x;
                copy_column_to_buffer_1d(m_height, buffer, colp, m_step, padding);
                box_filter_buffer_1d(m_height, buffer, padding);
                for (int y = 0; y < m_height; ++y)
                        colp[y * m_step] = buffer[y];
        }

        delete [] buffer;
}

void Image::box_filter(int filter_size)
{
        box_filter_x(filter_size);
        box_filter_y(filter_size);
}

static void filter_buffer_1d(int n, float* buffer,
                             int padding, const float *kernel)
{
        int filter_size = 2 * padding + 1;
        for (int i = 0; i < n; ++i) {
                float sum = 0.0f;
                for (int j = 0; j < filter_size; ++j)
                        sum += buffer[i + j] * kernel[j];
                buffer[i] = sum;
        }
}


static float *gaussian_kernel(float sigma, int &padding)
{
        const int CUTOFF_FACTOR = 3;
        const float SIGMA_SQR = sigma*sigma;

        padding = CUTOFF_FACTOR * sigma;
        int filter_size = 2 * padding + 1;
        float *kernel = new float[filter_size];
        float kernel_sum = 0.0f;
        for (int i = 0; i < filter_size; ++i) {
                float dx = i - padding;
                kernel[i] = exp(-0.5*dx*dx/SIGMA_SQR);
                kernel_sum += kernel[i];
        }
        for (int i = 0; i < filter_size; ++i)
                kernel[i] /= kernel_sum;
        return kernel;
}

// g(x | mu, sigma) = 1 / (sqrt(2*pi*sigma^2)) * exp(-0.5 * (x - mu)^2 / sigma^2)
void Image::smooth_x(float sigma)
{
        if (m_n_channels != 1) {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be box filtered!\n";
                exit(EXIT_FAILURE);
        }

        int padding = 0;
        float *kernel = gaussian_kernel(sigma, padding);
        float *buffer = new float[2 * padding + m_width];

        for (int y = 0; y < m_height; ++y) {
                uchar *rowp = this->data(y);
                copy_to_buffer_1d(m_width, buffer, rowp, padding);
                filter_buffer_1d(m_width, buffer, padding, kernel);
                for (int x = 0; x < m_width; ++x)
                        rowp[x] = static_cast<uchar>(buffer[x]);
        }

        delete [] buffer;
        delete [] kernel;
}

void Image::smooth_y(float sigma)
{
        if (m_n_channels != 1) {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be box filtered!\n";
                exit(EXIT_FAILURE);
        }

        int padding = 0;
        float *kernel = gaussian_kernel(sigma, padding);
        float *buffer = new float[2 * padding + m_width];

        for (int x = 0; x < m_width; ++x) {
                uchar *colp = m_data + x;
                copy_column_to_buffer_1d(m_height, buffer, colp, m_step, padding);
                filter_buffer_1d(m_height, buffer, padding, kernel);
                for (int y = 0; y < m_height; ++y)
                        colp[y * m_step] = buffer[y];
        }

        delete [] buffer;
        delete [] kernel;
}

void Image::smooth(float sigma)
{
        smooth_x(sigma);
        smooth_y(sigma);
}

// (x, y) is the location of the pixel at the top-left corner of the region to
// be convolved
static int convolve_2d(int x, int y, const uchar *data, int step,
                       int n, const float *K)
{
        float result = 0.0f;

        for (int j = 0; j < n; ++j) {
                for (int i = 0; i < n; ++i) {
                        result += K[i * n + j]
                                * data[(y + j) * step + (x + i)];
                }
        }

        return static_cast<int>(result);
}

Image *Image::filter_2d(int n, const float *K) const
{
        if (m_n_channels != 1) {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be 2d filtered!\n";
                exit(EXIT_FAILURE);
        }

        int w_out = m_width - n + 1;
        int h_out = m_height - n + 1;
        Image *out_img = new_gray(w_out, h_out);

        for (int yo = 0; yo < h_out; ++yo) {
                uchar *row_out = out_img->data(yo);
                for (int xo = 0; xo < w_out; ++xo) {
                        int result = convolve_2d(xo, yo, m_data, m_step, n, K);
                        if (result < 0) {
                                result = 0;
                        } else if (result > 255) {
                                result = 255;
                        }
                        row_out[xo] = result;
                }
        }


        return out_img;
}

short *Image::deriv_x() const
{
        short *dx = new short[m_width * m_height];
        memset(dx, 0, m_width*sizeof(*dx));
        for (int y = 1; y < m_height-1; ++y) {
                const uchar *row = this->data(y);
                const uchar *row_p = row - m_step;
                const uchar *row_n = row + m_step;
                short *row_dx = dx + y * m_width;
                row_dx[0] = 0;
                for (int x = 1; x < m_width-1; ++x) {
                        row_dx[x] = row_p[x+1] - row_p[x-1]
                                + 2 * row[x+1] - 2 * row[x-1]
                                + row_n[x+1] - row_n[x-1];
                }
                row_dx[m_width-1] = 0;
        }
        memset(dx + (m_height-1) * m_width, 0, m_width*sizeof(*dx));

        return dx;
}

short *Image::deriv_y() const
{
        short *dy = new short[m_width * m_height];
        memset(dy, 0, m_width*sizeof(*dy));
        for (int y = 1; y < m_height-1; ++y) {
                const uchar *row_p = this->data(y-1);
                const uchar *row_n = row_p + 2 * m_step;
                short *row_dy = dy + y * m_width;
                row_dy[0] = 0;
                for (int x = 1; x < m_width-1; ++x) {
                        row_dy[x] = row_n[x+1] + 2 * row_n[x] +  row_n[x-1]
                                - (row_p[x+1] + 2 * row_p[x] +  row_p[x-1]);
                }
                row_dy[m_width-1] = 0;
        }
        memset(dy + (m_height-1) * m_width, 0, m_width*sizeof(*dy));
        return dy;
}

void Image::save_as_pnm(const std::string &filename)
{
        if (m_n_channels != 1 && m_n_channels != 4) {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale or RGBA images can be saved as PNM files!\n";
                exit(EXIT_FAILURE);
        }

        ofstream fout;

        if (m_n_channels == 1) {
                const string magic_head = "P5";
                string extended_name = filename + ".pgm";
                fout.open(extended_name.c_str(), ios::out | ios::binary);
                fout << magic_head << "\n";
                fout << m_width << " " << m_height << " 255\n";
                for (int y = 0; y < m_height; ++y) {
                        const uchar *row_data = this->data(y);
                        fout.write(reinterpret_cast<const char*>(row_data), m_width*sizeof(uchar));
                }
        } else { // m_n_channels == 4
                const string magic_head = "P6";
                string extended_name = filename + ".ppm";
                fout.open(extended_name.c_str(), ios::out | ios::binary);
                fout << magic_head << "\n";
                fout << m_width << " " << m_height << " 255\n";
                for (int y = 0; y < m_height; ++y) {
                        const uchar *row_data = this->data(y);
                        for (int x = 0; x < m_width; ++x) {
                                const uchar *pixel_data = row_data + x * 4;
                                fout.write(reinterpret_cast<const char*>(pixel_data),
                                           3*sizeof(uchar));
                        }
                }
        }

        fout.close();
}

bool Image::load_pnm(const std::string &filename)
{
        FILE *fin = fopen(filename.c_str(), "r+b");
        if (!fin) {
                cerr << "[ERROR][CENG391::Image] Failed to open file for reading: " << filename << endl;
                return false;
        }

        char ch1, ch2;
        if (fscanf(fin, "%c%c", &ch1, &ch2) != 2) {
                cerr << "[ERROR][CENG391::Image] Failed to read image header from file: " << filename << endl;
                fclose(fin);
                return false;
        }

        if (ch1 != 'P') {
                cerr << "[ERROR][CENG391::Image] PNM image must start with P, read " << ch1 << endl;
                fclose(fin);
                return false;
        }

        int n_channels = 0;
        if (ch2 == '5') {
                n_channels = 1;
        } else if (ch2 == '6') {
                n_channels = 4;
        } else {
                cerr << "[ERROR][CENG391::Image]  We can only load binary PGM or PPM images read image header P" << ch2 << endl;
                fclose(fin);
        }

        int width = 0;
        int height = 0;
        int range = 0;
        if (fscanf(fin, "%d %d %d", &width, &height, &range) != 3) {
                cerr << "[ERROR][CENG391::Image] Failed to read image dimensions from file: " << filename << endl;
                fclose(fin);
                return false;
        }

        if (range != 255) {
                cerr << "[ERROR][CENG391::Image] Pixel range should be 0-255, read " << range << endl;
                fclose(fin);
                return false;
        }

        do {
                ch1 = fgetc(fin);
        } while (ch1 != '\n' &&  ch1 != EOF);

        reallocate(width, height, n_channels);
        if (n_channels == 1) {
                for (int y = 0; y < m_height; ++y) {
                        uchar *row_data = this->data(y);
                        if (m_width != fread((void *)row_data, sizeof(uchar), m_width, fin)) {
                                cerr << "[ERROR][CENG391::Image] Reading line " << y << " from PGM file " << filename << endl;
                                fclose(fin);
                                return false;
                        }
                }
        } else {
                for (int y = 0; y < m_height; ++y) {
                        uchar *row_data = this->data(y);
                        for (int x = 0; x < m_width; ++x) {
                                uchar *pixel_data = row_data + x * 4;
                                if (3 != fread((void *)pixel_data, sizeof(uchar), 3, fin)) {
                                        cerr << "[ERROR][CENG391::Image] Reading pixel " << x << " on line " << y << " from PGM file " << filename << endl;
                                        fclose(fin);
                                        return false;
                                }
                                pixel_data[3] = 255;
                        }
                }
        }

        fclose(fin);
        return true;
}

bool Image::save_as_png(const std::string &filename)
{
        // We open the output file with C style IO since we are using libpng
        // C-API
        FILE *fout = fopen(filename.c_str(), "w+b");
        if (!fout) {
                cerr << "[ERROR][CENG391::Image] Failed open file for writing: " << filename << endl;
                return false;
        }

        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                      0, 0, 0);
        if (!png_ptr) {
                cerr << "[ERROR][CENG391::Image] Failed to create PNG write structure!" << endl;
                fclose(fout);
                return false;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
                cerr << "[ERROR][CENG391::Image] Failed to create PNG info structure!" << endl;
                png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
                fclose(fout);
                return false;
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
                cerr << "[ERROR][CENG391::Image] Failed to create PNG jump buffer!" << endl;
                png_destroy_write_struct(&png_ptr, &info_ptr);
                fclose(fout);
                return false;
        }

        int color_type;
        switch (this->m_n_channels) {
        case 1: color_type = PNG_COLOR_TYPE_GRAY; break;
        case 4: color_type = PNG_COLOR_TYPE_RGBA; break;
        default:
                cerr << "[ERROR][CENG391::Image] Unsupported image type for saving as PNG!" << endl;
                png_destroy_write_struct(&png_ptr, &info_ptr);
                fclose(fout);
                return false;
        }

        png_init_io(png_ptr, fout);
        png_set_IHDR(png_ptr, info_ptr, this->m_width, this->m_height, 8,
                     color_type, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        png_write_info(png_ptr, info_ptr);

        png_bytepp row_pointers = (png_bytepp)malloc(this->m_height * sizeof(png_bytep));
        if (!row_pointers) {
                cerr << "[ERROR][CENG391::Image]Error creating PNG row pointers" << endl;
                png_destroy_write_struct(&png_ptr, &info_ptr);
                fclose(fout);
                return false;
        }

        for (png_int_32 k = 0; k < this->m_height; k++) {
                row_pointers[k] = (png_bytep)(this->data(k));
        }

        png_write_image(png_ptr, row_pointers);
        png_write_end(png_ptr, info_ptr);

        png_destroy_write_struct(&png_ptr, &info_ptr);
        free(row_pointers);
        fclose(fout);

        return true;
}


bool Image::load_png(const std::string &filename, bool load_as_grayscale)
{
        // We open the output file with C style IO since we are using libpng
        // C-API
        FILE *fin = fopen(filename.c_str(), "r+b");
        if (!fin) {
                cerr << "[ERROR][CENG391::Image] Failed to open file for reading: " << filename << endl;
                return false;
        }

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                     NULL, NULL, NULL);
        if (!png_ptr) {
                cerr << "[ERROR][CENG391::Image] Could not create PNG read structure" << endl;
                fclose(fin);
                return false;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
                cerr << "[ERROR][CENG391::Image] Could not create PNG info pointer" << endl;
                png_destroy_read_struct(&png_ptr, png_infopp_NULL,
                                        png_infopp_NULL);
                fclose(fin);
                return false;
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
                cerr << "[ERROR][CENG391::Image] Could not set jump point for reading PNG file" << endl;
                png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
                fclose(fin);
                return false;
        }

        png_init_io(png_ptr, fin);
        png_read_info(png_ptr, info_ptr);

        png_uint_32 stream_width, stream_height;
        int bit_depth, color_type, interlace_type;
        png_get_IHDR(png_ptr, info_ptr, &stream_width, &stream_height, &bit_depth, &color_type,
                     &interlace_type, int_p_NULL, int_p_NULL);

        png_set_strip_16(png_ptr);
        if (color_type == PNG_COLOR_TYPE_GA) {
                png_set_strip_alpha(png_ptr); /*(not recommended). */
        }

        png_set_packing(png_ptr);
        if (color_type == PNG_COLOR_TYPE_PALETTE) {
                png_set_palette_to_rgb(png_ptr);
        }

        png_set_expand(png_ptr);

        // Depending on the type of image in the file and the load_as_grayscale
        // flag, we determine the desired number of channels of the output
        // image.
        int desired_n_channels = 4;
        if (load_as_grayscale) {
                desired_n_channels = 1;
                png_set_rgb_to_gray_fixed(png_ptr, 1, 30000, 59000);
                png_set_strip_alpha(png_ptr); /*(not recommended). */
        } else {
                if (color_type == PNG_COLOR_TYPE_GRAY ||
                    color_type == PNG_COLOR_TYPE_GA) {
                        desired_n_channels = 1;
                }

                if(color_type == PNG_COLOR_TYPE_RGB ||
                   color_type == PNG_COLOR_TYPE_PALETTE) {
                        png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);
                }
        }

        // If the current image dimensions do not match the image to be loaded,
        // then reallocate with the desired dimensions.
        reallocate(stream_width, stream_height, desired_n_channels);

        png_bytepp row_pointers = (png_bytepp)malloc(this->m_height * sizeof(png_bytep));
        if (!row_pointers) {
                cerr << "[ERROR][CENG391::Image]Error creating PNG row pointers" << endl;
                png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
                fclose(fin);
                return false;
        }
        for (int k = 0; k < this->m_height; k++) {
                row_pointers[k] = (png_bytep)(this->data(k));
        }

        png_read_image(png_ptr, row_pointers);
        png_read_end(png_ptr, info_ptr);

        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

        free(row_pointers);

        fclose(fin);

        return true;
}

//      [ cos(angle) -sin(angle) ]
//  R = [                        ]
//      [ sin(angle)  cos(angle) ]
//                 [ cos(angle)   sin(angle) ]
//  R^{-1} = R^T = [                         ]
//                 [ -sin(angle)  cos(angle) ]
void Image::rotated(Image *Ir, double angle, bool rotate_by_center)
{
        if (m_n_channels != 1) {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be rotated!\n";
                exit(EXIT_FAILURE);
        }

        if (!Ir || Ir->n_ch() != 1) {
                cerr << "[ERROR][CENG391::Image] Rotate requires an allocated grayscale image as its first parameter!\n";
                exit(EXIT_FAILURE);
        }

        double hw  = m_width  / 2.0;
        double hh  = m_height / 2.0;
        double hwr = Ir->m_width  / 2.0;
        double hhr = Ir->m_height / 2.0;

        for (int y = 0; y < Ir->m_height; ++y) {
                uchar *row_r = Ir->data(y);
                for (int x = 0; x < Ir->m_width; ++x) {
                        double xr = x;
                        double yr = y;
                        if (rotate_by_center) {
                                xr -= hwr;
                                yr -= hhr;
                        }
                        double xp =  cos(angle) * xr + sin(angle) * yr;
                        double yp = -sin(angle) * xr + cos(angle) * yr;
                        if (rotate_by_center) {
                                xp += hw;
                                yp += hh;
                        }

                        // do nearest neighbor
                        int xi = round(xp);
                        int yi = round(yp);
                        uchar value = 0; // assuming zero outside image borders
                        if (xi >= 0 && yi >= 0
                            && xi < m_width
                            && yi < m_height) {
                                value = m_data[yi * m_step + xi];
                        }
                        row_r[x] = value;
                }
        }
}

void Image::warp_affine(Image *Ia, const double *A,
                        const double *t, bool interpolate_bilinear)
{
        if (m_n_channels != 1) {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be affine transformed!\n";
                exit(EXIT_FAILURE);
        }

        if (!Ia || Ia->n_ch() != 1) {
                cerr << "[ERROR][CENG391::Image] Warp affine requires an allocated grayscale image as its first parameter!\n";
                exit(EXIT_FAILURE);
        }

        double a00 = A[0];
        double a01 = A[2];
        double a10 = A[1];
        double a11 = A[3];
        double t0 = t[0];
        double t1 = t[1];
        double det_A = a00*a11 - a01*a10;
        if (det_A == 0.0) {
                cerr << "[ERROR][CENG391::Image] Affine matrix is not invertable!\n";
                exit(EXIT_FAILURE);
        }

        double inv_det_A = 1.0 / det_A;

        for (int y = 0; y < Ia->m_height; ++y) {
                uchar *row_r = Ia->data(y);
                for (int x = 0; x < Ia->m_width; ++x) {
                        // Compute xp = A^-1 (x - t)
                        double xa = x - t[0];
                        double ya = y - t[1];
                        double xp = inv_det_A * (a11 * xa - a01 * ya);
                        double yp = inv_det_A * (a00 * ya - a10 * xa);

                        uchar value = 0; // assuming zero outside image borders
                        if (interpolate_bilinear) {
                                value = this->sample_bilinear(xp, yp, value);
                        } else {
                                // do nearest neighbor
                                int xi = round(xp);
                                int yi = round(yp);
                                if (xi >= 0 && yi >= 0
                                    && xi < m_width && yi < m_height) {
                                        value = m_data[yi * m_step + xi];
                                }
                        }
                        row_r[x] = value;
                }
        }
}

}
