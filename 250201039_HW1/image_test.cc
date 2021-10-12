//BURAK TUTUMLU ---- 250201039
// ------------------------------

#include <cstdlib>
#include <iostream>

#include "image.hpp"

using std::clog;
using std::endl;

using std::clog;
using std::endl;

using ceng391::Image;

int main(int argc, char** argv)
{
        Image *img_rgba = Image::new_rgba(90, 90);      //Created rgba image with 90 width and 90 height
        Image *img;
        clog << "Created new image of size " << img_rgba->w()
             << "x" << img_rgba->h() << " with " << img_rgba->n_ch() << endl;

        img_rgba->set_rect(0, 0, 30, 30, 255);          //created rectangle

        img_rgba->save_as_pnm("/tmp/image_rgba");               //saved rgba image
        img = img_rgba->read_pnm("/tmp/image_rgba.ppm");        //read image
        img->save_as_pnm("/tmp/test_image");                    //save read image
        img->to_grayscale();                                    //convert to grayscale
        img->to_rgba();                                         //convert to rgba
        img->save_as_pnm("/tmp/converted");                     //again save the converted image

        delete img;

        return EXIT_SUCCESS;
}
