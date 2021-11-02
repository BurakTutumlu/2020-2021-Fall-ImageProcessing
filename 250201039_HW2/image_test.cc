//BURAK TUTUMLU - 250201039 - HW2 - CENG391

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

#include "image.hpp"

using std::clog;
using std::cerr;
using std::endl;
using std::ofstream;
using std::vector;

using ceng391::Image;
using ceng391::uchar;

// Ideally these test should return test/fail status. Better yet use a proper
// unit test framework such as gtest.
void test_grayscale_creation_and_pgm_io();
void test_grayscale_png_load_save();
void test_rgba_creation_and_ppm_io();

//exercise -- 1
void filter_2d();       //filterin image with given kernel, which is 2d

//exercise -- 2
void derivative();      //returns the short array, also you can see the png files as an result in /tmp/

//exercise -- 3
void warp_affine();     //interpolation with given values, also there is option for bilinear

int main(int argc, char** argv)
{
        /*
        test_grayscale_creation_and_pgm_io();
        test_rgba_creation_and_ppm_io();
        test_grayscale_png_load_save();
        */

        //for exercise - 1, we are calling filter_2d()
        filter_2d();

        //for exercise - 2, we are calling derivative(), which includes derivative_x and derivative_y
        derivative();

        //for exercise - 3, we are calling warp_affine();
        warp_affine();
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

///////////////////////////////////////////////////////////////////////////////////////////////
//for exercise - 1, we call this method -> filter_2d
void filter_2d()
{
        cerr << "EXERCISE 1 - filter_2d function" << endl;

        Image *img = Image::from_png("../data/small_watch.png", true);  //takes image from data folder

        //1.exercise//////////
        vector<vector<float>> kernel = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};       //one of the edge detection kernel

        //vector<vector<float>> kernel = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}};           //also you can try laplacian edge detection kernel

        //vector<vector<float>> kernel = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};              //also you can try sharpen kernel

        img = img->filter_2d(3, kernel);        //method assigned into the pointer img

        img->save_as_png("/tmp/filter2d.png");  //image saved

        cerr << "OUTPUT IMAGE FILTERED WITH EDGE DETECTION KERNEL" << endl;
        
        delete img;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//for exercise - 2, we call this method -> derivative which includes deriv_x and deriv_y
void derivative()
{
        cerr << "EXERCISE 2 - derivative function" << endl;

        Image *img = Image::from_png("../data/small_city.png", true);   //takes image from data folder

        //2.exercise a -        
        short *deriv_x = img->deriv_x();        //you can see the results of this method as an png in /tmp/
        cerr << "OUTPUT IMAGE DERIVATIVED X " << endl;

        ofstream derivative_x_txt("/tmp/deriv_x.txt");          //opens deriv_x_txt
        for (int i = 0; i < img->h() * img->w(); i++)
        {
                derivative_x_txt << deriv_x[i] << " ";       //If you want see the value of deriv_x uchar, you can delete the comment line of code of this line
        }
        derivative_x_txt.close();

        ////////////////////////////////
        //2.exercise b -
        short *deriv_y = img->deriv_y();        //you can see the results of this method as an png in /tmp/
        cerr << "OUTPUT IMAGE DERIVATIVED Y " << endl;

        ofstream derivative_y_txt("/tmp/deriv_y.txt");           //opens deriv_y_txt
        for (int i = 0; i < img->h() * img->w(); i++)
        {
                derivative_y_txt << deriv_y[i] << " ";       //If you want see the value of deriv_x uchar, you can delete the comment line of code of this line
        }
        derivative_y_txt.close();
        
        delete img;

}
///////////////////////////////////////////////////////////////////////////////////////////////

//for exercise - 3, we call this method -> warp_affine
void warp_affine()
{
        cerr << "EXERCISE 3 - derivative function" << endl;

        Image *img = Image::from_png("../data/small_watch.png", true);
        
        Image *Ir = Image::new_gray(img->w(), img->h());
        
        
        bool affline_center = true;     //if you work for an center, you can assign true
        
        double transformationMatrix[4] = {1.0, 1.0, 0.0, 1.0};  //special matrix
        double translationVector[2] = {10.0, 10.0};     //effects according to x, y
        
        //for bilinear sampling bilinear = true
        bool bilinear = true;           //for bilinearing option

        img -> warp_affine(transformationMatrix, translationVector, Ir, bilinear);
        Ir->save_as_png("/tmp/warp_affine.png"); 
        cerr << "OUTPUT IMAGE AFFINED" << endl;

        delete img;
}
///////////////////////////////////////////////////////////////////////////////////////////////