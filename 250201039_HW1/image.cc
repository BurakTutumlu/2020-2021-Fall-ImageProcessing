//BURAK TUTUMLU ---- 250201039
// ------------------------------

#include "image.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <string>

using std::cerr;
using std::clog;
using std::ios;
using std::ofstream;
using std::ifstream;
using std::string;
using std::cout;

using namespace std;

namespace ceng391 {

Image::Image(int width, int height, int n_channels, int step)                   //creates an Image with given with, height, channels
        : m_width(width), m_height(height), m_n_channels(n_channels)
{
        if (step < width * n_channels)
                step = width * n_channels;              //step formula is important for understanding the functions
        m_step = step;
        m_data = new uchar[m_height * m_step];
}

Image::~Image()
{
        // clog << "Deleting image" << endl;
        delete [] m_data;
}

Image *Image::new_gray(int width, int height)           //creates an image with channel 1
{                                                       
        return new Image(width, height, 1);             //this is the gray image      
}

Image *Image::new_rgb(int width, int height)            //creates an image with channel 3 
{
        return new Image(width, height, 3);             //this is the RGB image, there is no alpha channel
}

Image *Image::new_rgba(int width, int height)           //creates an image with channel 4
{
        return new Image(width, height, 4);             //this is the RGBA image
}

void Image::set_rect(int tlx, int tly, int width, int height, uchar value)      //creates rectangular with given size, given position and value
{

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
                for (int x = tlx * m_n_channels; x < m_n_channels*(tlx + width); x+=m_n_channels) {
                        if (x >= m_width)
                                break;
                        if (m_n_channels == 3){                 //if channel is 3 we just go over 3 column in each width
                                row[x] = value;
                                row[x+1] = value;
                                row[x+2] = value;
                        }
                        if (m_n_channels == 4){                 //if channel is 4 we just go over 4 column in each width
                                row[x] = value;
                                row[x+1] = value;
                                row[x+2] = value;
                                row[x+3] = value;
                        }
                }
        }
}

void Image::save_as_pnm(const std::string &filename)            
{
        if (m_n_channels == 1) {                        //saves the file as an pgm if it's channel is 1
                //pgm format    
                const string magic_head = "P5";
                ofstream fout;
                string extended_name = filename + ".pgm";
                fout.open(extended_name.c_str(), ios::out | ios::binary);
                fout << magic_head << "\n";
                fout << m_width << " " << m_height << " 255\n";
                for (int y = 0; y < m_height; ++y) {
                        const uchar *row_data = this->data(y);
                        fout.write(reinterpret_cast<const char*>(row_data), m_width*sizeof(uchar));
                delete[] row_data;
                }
        
        fout.close();
        }

        if (m_n_channels == 3 || m_n_channels == 4){            //saves the file as an ppm if it's channel is 3 or 4
                //ppm format
                const string magic_head = "P6";
                ofstream fout; 
                string extended_name = filename + ".ppm";
                fout.open(extended_name.c_str(), ios::out | ios::binary);
                fout << magic_head << "\n";
                fout << m_width << " " << m_height << " 255\n";
                for (int y = 0; y < m_height; ++y){
                        uchar *row_data = this->data(y);
                        uchar *copy_row_data = new uchar[m_width * 3];  //I created new uchar with 3 * width, because we do not need to alpha channel.
                        int copy_index = 0; //index of row_data         //I need to copy the row_data to copy_row_data
                        if (m_n_channels == 4){   // for specify R G B A values
                                for (int i = 0; i < m_step; ++i){  
                                        if ((i + 1) % 4 != 0) { //delete transparency value 
                                                copy_row_data[copy_index] = row_data[i];        //I am deleting the 4 value of RGBA which is alpha 
                                                copy_index++;
                                        }
                                }
                        }
                        else if (m_n_channels == 3){ //RGB values       //there is no alpha channel already
                                copy_row_data = row_data;
                        }
                        fout.write(reinterpret_cast<char*>(copy_row_data), m_width*sizeof(uchar)*3);    //write with 3*
                        
                }
        fout.close();
        }      
}

//reads the contents of the file
Image* Image::read_pnm(const std::string &filename)             //read pnm with file which is given as a parameter
{
        Image *img;

        ifstream ifs;           //library command for reading file, we will go over with this value in al lines
        ofstream fout;

        ifs.open(filename.c_str(), ios::out | ios::binary);

        string format;
        int width, height, alpha;

        ifs >> format;        //what is the format of file
        ifs >> width;           //width
        ifs >> height;          //height
        ifs >> alpha;           //alpha value 
                                //this values are first 4 values of the image file
        cout << format;
        cout << width << height << alpha;
        if(format == "P5")    //pgm image
                img = new_gray(width, height);          //creates gray image 
        else if(format == "P6")       //rgba image
                img = new_rgba(width, height);          //creates RGBA image
        else{
                cerr << "Wrong file format";
                exit(EXIT_FAILURE);
        }

        //uchar *data = new uchar[width * height * img -> n_ch()];
        uchar pixel;
        string file = "read_image";
        string extended_name = file + ".pnm";

        fout.open(extended_name.c_str(), ios::out | ios::binary);
        fout << "P6" << "\n";
        fout << width << " " << height << " 255\n";
        uchar *data = img->data();              //hold the all data

        for (int y = 0; y < width * height * m_n_channels; ++y){  //while go overing all data
                if (img->n_ch() == 4){
                        if ((y + 1) % 4 == 0){      
                                data[y] = 255;  //if there are alpha channels, initialize them to 255
                                continue;  
                                }
                }
                ifs >> pixel;           //reads each pixel
                data[y] = pixel;
        }
            
        for (int y = 0; y < height; ++y){
                uchar *row_data = data + (y * width * 3);
                fout.write(reinterpret_cast<const char*>(row_data), width*sizeof(uchar)*3);
        }
        ifs.close();
        fout.close();
        return img;     
}

void Image::to_grayscale()   //converting from R G B A to grayscale.
{

        if (m_n_channels == 1){
                cerr << "file already in grayscale format\n";
                return;
        }

        if(m_n_channels == 4){
                uchar *new_data = new uchar[m_height * m_width];
                for (int y = 0; y < m_height; y++){
                        uchar *copy_row_data = new_data + y * m_width;    //row data's size is 4 times more than copy row data
                                                                        //new data holds the memory and goes to that row according to the value
                        const uchar *row_data = this->data(y);
                        for (int i = 0; i < m_width; ++i){
                                int value = row_data[4 * i] * 0.3 + row_data[(4 * i) + 1] * 0.59 + row_data[(4 * i) + 2] * 0.11;        //formula in pdf
                                if (value < 0)
                                        value = 0;      
                                if (value > 255)
                                        value = 255;
                                copy_row_data[i] = value;       //initializes to copy_row_data which has 1 channel  
                }       
        }         
                m_data = new_data;      //our image's new data
                m_n_channels = 1;       //our m_n_channels new channels
                m_step = m_width;         //our new step
        }
        
}

void Image::to_rgba()
{
        if (m_n_channels == 4){
                cerr << "file already in RGBA format\n";
                return;
        }

        uchar *new_data = new uchar[m_height * m_width * 4];        //because we will convert 1 channel grayscale to 4 channel RGBA

        for (int y = 0; y < m_height; ++y){
                uchar *copy_row_data = new_data + (y * m_width * 4);
                const uchar *row_data = this->data(y);
                for (int i = 0; i < m_width; ++i){
                        copy_row_data[4 * i] = row_data[i];                     //takes all data one by one and initializes to 4 channel uchar
                        copy_row_data[(4 * i) + 1] = row_data[i];               //takes all data one by one and initializes to 4 channel uchar
                        copy_row_data[(4 * i) + 2] = row_data[i];               //takes all data one by one and initializes to 4 channel uchar
                        copy_row_data[(4 * i) + 3] = row_data[i];               //takes all data one by one and initializes to 4 channel uchar
                }
        }

        m_data = new_data;                 //new channel count
        m_n_channels = 4;                  //new step
        m_step = m_width * 4;              //new data

}
}