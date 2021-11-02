//BURAK TUTUMLU - 250201039 - HW2 - CENG391

#include "image.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cmath>

#include <vector>
using std::vector;

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
using std::endl;
using std::exit;
using std::ios;
using std::ofstream;
using std::string;
using std::fopen;
using std::fclose;
using std::memcpy;
using std::cos;
using std::sin;
using std::round;

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

static void copy_to_buffer_1d(int n, uchar* buffer, const uchar *src, int padding)
{
        for (int i = 0; i < padding; ++i)
                buffer[i] = src[0];
        for (int i = 0; i < n; ++i)
                buffer[i + padding] = src[i];
        for (int i = 0; i < padding; ++i)
                buffer[padding + n + i] = src[n - 1];
}

static void copy_column_to_buffer_1d(int n, uchar* buffer, const uchar *src,
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

/* 1- Write a new member function Image::filter 2d that takes an odd
integer n, and an n × n single-precision floating point matrix K. The
function should return a new image that is of size
(w() − n + 1) × (h() − n + 1)
and filtered by the kernel K.
Hint: If n is even you may take n as the largest odd number smaller than
n. Make sure to clamp the filter results to the range [0, 255]. The size of the
output is smaller so that you do not need to worry about the image borders.
*/

Image *Image::filter_2d(int n, vector<vector<float>> kernel)
{
        if (m_n_channels != 1)
        {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be filtered2d!\n";
                exit(EXIT_FAILURE);
        }
        if ((n - 1) % 2 != 0)
        {
                n = n - 1;
        }
        Image *imgNew = new_gray(this->w()-n+1, this->h()-n+1);

        //our starting point changes according to n, we need to specify that as n/2
        for (int i = 0 - (n / 2); i < imgNew->m_width - (n / 2); i++)
        {
                //we need to specify that as n/2
                for (int j = 0 - (n / 2); j < imgNew->m_height - (n / 2); j++)
                { //adding k and l to i and j will make up the difference and allow us to process the whole image
                        float filtertotal = 0;

                        for (int k = 0; k < n; k++)
                        {
                                for (int l = 0; l < n; l++)
                                { 
                                        if (i + k >= 0 && i + k < imgNew->m_width && j + l >= 0 && j + l < imgNew->m_height)
                                        { //don't try to process pixels off the endge of the map
                                                float a = m_data[(j + l) * m_step + (i + k)];
                                                float b = kernel[k][l];
                                                float product = a * b;
                                                filtertotal += product;
                                                
                                        }
                                }
                                
                        }
                        if (filtertotal > 255){         //checks overflow
                                filtertotal = 255;
                        }else if (filtertotal < 0){     //checks underflow
                                filtertotal = 0;
                        }
                        //filter all proccessed for this pixel, write it to dst
                        imgNew->m_data[(j + (n / 2)) * imgNew->m_step + (i + n / 2)] = filtertotal;
                }
        }
        return imgNew;
}

/////////////////////////////////////2. QUESTION////////////////////////////////////
short *Image::border()          //if needed
{
        if (m_n_channels != 1)
        {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be bordered!\n";
                exit(EXIT_FAILURE);
        }

        short *buffer = new short[(m_width + 2) * (m_height + 2)]; // our new x and y must be extended by 2

        memset(buffer, 0, sizeof(buffer) * sizeof(short));

        for (int y = 0; y < m_height; ++y){
                for (int x = 0; x < m_width; ++x){
                        buffer[(y + 1) * m_width + x + 1] = m_data[y * m_width + x];
                }
        }

        return buffer;

}

short *Image::deriv_x()
{
        if (m_n_channels != 1)
        {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be derivatived of x!\n";
                exit(EXIT_FAILURE);
        }

        short x_filter[3][3] = {{-1,0,1}, {-2, 0, 2}, {-1,0,1}};

        short *list = new short[this->w() * this->h()];

        //short *borderr = border();            //I did not choose adding border option
                                                //but this is another to make first colum, last column, first row, last row 0
        for (int y = 0; y < m_height ; y++){
                for (int x = 0; x < m_width; x++){
                        short filtertotal = 0;
                        if (x == 0)                     //making 0 in here
                                filtertotal = 0;
                        else if (x == m_width - 1)      //another control
                                filtertotal = 0;
                        else if (y == 0)                //making 0 in here
                                filtertotal = 0;        
                        else if (y == m_height -1)      //another control
                                filtertotal = 0;
                        else{
                                for (int i = -1; i < 2; i++){
                                        for (int j = -1; j < 2; j++){
                                                filtertotal += m_data[((y + i) * m_width) +(x + j)] * x_filter[i + 1][j + 1];
                                        }
                                }
                        }
                        list[(y * m_width) + x] = filtertotal;
                }
        }    

        Image *img = new_gray(this->m_width, this->m_height);
        for (int x = 0; x < this->m_height * this->m_width; x++){
                img -> m_data[x] = list[x];
        }    

        img -> save_as_png("/tmp/derivative_x.png");
        return list;
}

short *Image::deriv_y()
{
        if (m_n_channels != 1)
        {
                cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be derivatived of y!\n";
                exit(EXIT_FAILURE);
        }

        short x_filter[3][3] = {{-1,-2,-1}, {0, 0, 0}, {1,2,1}};

        short *list = new short[this->w() * this->h()];

        //short *borderr = border();            //I did not choose adding border option
                                                //but this is another to make first colum, last column, first row, last row 0

        for (int y = 0; y < m_height ; y++){
                for (int x = 0; x < m_width; x++){
                        short filtertotal = 0;
                        if (x == 0)
                                filtertotal = 0;
                        else if (x == m_width - 1)
                                filtertotal = 0;
                        else if (y == 0)
                                filtertotal = 0;
                        else if (y == m_height -1)
                                filtertotal = 0;
                        else{
                                for (int i = -1; i < 2; i++){
                                        for (int j = -1; j < 2; j++){
                                                filtertotal += m_data[((y + i) * m_width) +(x + j)] * x_filter[i + 1][j + 1];
                                        }
                                }
                        }
                        list[(y * m_width) + x] = filtertotal;
                }
        }    

        Image *img = new_gray(this->m_width, this->m_height);
        for (int x = 0; x < this->m_height * this->m_width; x++){
                img -> m_data[x] = list[x];
        }    

        img -> save_as_png("/tmp/derivative_y.png");
        return list;
}
/////////////////////////////////////2. QUESTION////////////////////////////////////

/////////////////////////////////////3. QUESTION////////////////////////////////////

void Image::warp_affine(double transformMatrix[], double translationVector[], Image *out, bool bilinear)
{
        if (m_n_channels != 1) {
        cerr << "[ERROR][CENG391::Image] Currently only grayscale images can be transformed by affine!\n";
        exit(EXIT_FAILURE);
    }

    if (!out || out->n_ch() != 1) {
        cerr << "[ERROR][CENG391::Image] Rotate requires an allocated grayscale image as its first parameter!\n";
        exit(EXIT_FAILURE);
    }

    double hw  = m_width  / 2.0;        //make half
    double hh  = m_height / 2.0;        //make half
    double hwr = out->m_width  / 2.0;   //make half
    double hhr = out->m_height / 2.0;   //make half

    bool affine_center = true;          //computations are according to the center

     for (int y = 0; y < out->m_height; ++y) {
            uchar *row_r = out->data(y);
            for (int x = 0; x < out->m_width; ++x) {
                double xr = x;
                double yr = y;
                
                if (affine_center) {    //if true
                        xr -= hwr;
                        yr -= hhr;
                }
                
                double xp =  transformMatrix[0] * xr + transformMatrix[1] * yr + translationVector[0];  //computation on pdf
                double yp =  transformMatrix[2] * xr + transformMatrix[3] * yr + translationVector[1];  //computation on pdf

                if (affine_center) {    //if true
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
                if (bilinear){
                
                	int fx = floor(xp);
                	int fy = floor(yp);
                        double a = xp - fx;
                        double b = yp - fy;
					
                        value = (1 - a) * (1 - b) * m_data[(fy * m_width + fx)] + 
                                a * (1 - b) * m_data[(fy * m_width + fx + 1)] + 
                                (1 - a) * b * m_data[((fy + 1) * m_width + fx)] + 
                                a * b * m_data[((fy + 1) * m_width + fx + 1)];
				}
                row_r[x] = value;
            }
    }

}

}



