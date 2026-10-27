//
// Created by las on 02.10.2026.
//

#include "hand.h"
#include <peak/peak.hpp>
#include <peak_ipl/peak_ipl.hpp>
#include <peak/converters/peak_buffer_converter_ipl.hpp>

#include <iostream>
#include <ostream>

#include <chrono>

Hand::Hand() {

}

FELD_MAPP Hand::serch_hand(const std::shared_ptr<peak::core::Buffer>& buffer) {

        auto image = peak::BufferTo<peak::ipl::Image>(buffer);
        peak::ipl::ImageConverter converter;
        auto rgbImage = converter.Convert(image, peak::ipl::PixelFormatName::RGB8);

        const int width  = static_cast<int>(buffer->Width());
        const int height = static_cast<int>(buffer->Height());

        FELD_MAPP mapp = getFelds(rgbImage, width, height);

        getObjects(&mapp);

        //std::cout << mapp.felds[0].pixel_RGB[0][0] << "\n";
        //std::cout << mapp.felds[660].collor;

        return mapp;
}

HAND Hand::return_hand() {
        return _my_hand;
}


FELD_MAPP Hand::getFelds(const peak::ipl::Image& rgbImage, int width, int height) {
        FELD_MAPP mapp;



        const int nuber_of_felds = (width / 10) * (height / 10);



        for (int feeld = 0; feeld < nuber_of_felds; ++feeld) {
                //std::cout << "feld " << feeld << " from " << nuber_of_felds << "\n";
                FELD curentFeld;


                for (int y = 0; y < 10; ++y) {
                        for (int i = 0; i < 10; ++i) {
                                curentFeld.pixel_X.push_back(i);
                        }
                }


                for (int y = 0; y < 10; ++y) {
                        for (int x = 0; x < 10; ++x) {
                                curentFeld.pixel_Y.push_back(y);
                        }
                }


                for (int j = 0; j < 100; ++j) {
                        curentFeld.pixel_ID.push_back(j);
                }


                for (int x = 0; x < 10; ++x) {
                        for (int y = 0; y < 10; ++y) {

                        }
                }



                //const auto ende2 = std::chrono::steady_clock::now();
                //const std::chrono::duration<double, std::milli> dauer2 = ende2 - start2;
                //std::cout << "Alle Pixel cords + ID hat " << dauer2.count() << " ms gebraucht\n";

                //const auto start1 = std::chrono::steady_clock::now();

                for (int i = 0; i < curentFeld.pixel_ID.size(); ++i) {
                        int x;
                        int y;

                        x = curentFeld.pixel_X[i] + 10 * (feeld % (width / 10));
                        y = curentFeld.pixel_Y[i] + 10 * (feeld / (width / 10));

                        //const auto start = std::chrono::steady_clock::now();
                        curentFeld.pixel_RGB.push_back(getPixel_RGB(rgbImage, x, y));
                        //const auto ende = std::chrono::steady_clock::now();
                        //const std::chrono::duration<double, std::milli> dauer = ende - start;

                        //std::cout << "getPixel_RGB aufruf + speichern hat " << dauer.count() << " ms gebraucht\n";
                        //std::cout << "Pixel: " << i << " abgeschlossen " << std::endl;
                }

                feldCollor(&curentFeld);

                curentFeld.feld_x = feeld % (width / 10);
                curentFeld.feld_y = feeld / (width / 10);
                curentFeld.feld_ID = feeld;

                mapp.felds.push_back(curentFeld);

        }


        return mapp;
}



std::array<int, 3> Hand::getPixel_RGB(const peak::ipl::Image& rgbImage, int x, int y) {

        //const auto start = std::chrono::steady_clock::now();

        const uint32_t width  = static_cast<uint32_t>(rgbImage.Width());
        const uint32_t height = static_cast<uint32_t>(rgbImage.Height());

        if (x >= width || y >= height)
                throw std::out_of_range("Pixel liegt außerhalb des Bildes");

        const uint8_t* data = rgbImage.Data();
        const size_t offset = (static_cast<size_t>(y) * width + x) * 3;

        //const auto ende = std::chrono::steady_clock::now();
        //const std::chrono::duration<double, std::milli> dauer = ende - start;

        //std::cout << "getPixel_RGB hat " << dauer.count() << " ms gebraucht\n";

        return { data[offset], data[offset + 1], data[offset + 2] };
}

void Hand::feldCollor(FELD* felds) {

        int in_range = 0;

        for (int i=0; i < 100; ++i) {

                float pixel_R = felds->pixel_RGB[i][0];
                float pixel_G = felds->pixel_RGB[i][1];
                float pixel_B = felds->pixel_RGB[i][2];



                // if ((pixel_R >= demoHautRange::R_MIN && pixel_R < demoHautRange::R_MAX) &&
                //         (pixel_G >= demoHautRange::G_MIN && pixel_G < demoHautRange::G_MAX) &&
                //         (pixel_B >= demoHautRange::B_MIN && pixel_B < demoHautRange::B_MAX)) {
                //
                //         ++in_range;
                // }

                if (pixel_R >= 80 &&
                        (pixel_R > pixel_G && pixel_G >= pixel_B) &&
                        pixel_R - pixel_G >= 15 &&
                        pixel_G - pixel_B >= 8 &&
                        (pixel_R / pixel_G < 1.56 && pixel_R / pixel_G > 1.07) &&
                        pixel_G / pixel_B < 1.48 &&
                        ((pixel_R - pixel_B) / pixel_R < 0.53 && (pixel_R - pixel_B) / pixel_R > 0.29))
                {
                        ++in_range;
                }
        }

        if (in_range < 20) {
                felds->collor = 2;
        }
        else if (in_range >= 20 && in_range < 55) {
                felds->collor = 1;
        }
        else if (in_range >= 55 && in_range < 101) {
                felds->collor = 0;
        }        else {
                throw std::out_of_range("Unvalid! 'in_range' is a number witch shudent exist");
        }

}

void Hand::getObjects(FELD_MAPP *mapp) {

        std::cout << '\n';
        std::cout << '\n';
        std::cout << '\n';
        int count = 1;
        static int frameCount = -1;

        frameCount++;

        std::cout << "\nFrame: " << frameCount << '\n';

        for (int i=0; i < mapp->felds.size(); ++i) {


                if (mapp->felds[i].collor != 2) {
                        std::cout << count << ": " << mapp->felds[i].feld_ID << "->" << mapp->felds[i].collor << "\n";
                        ++count;
                }
        }
}
