//
// Created by las on 02.10.2026.
//

#ifndef SIMPLE_LIVE_QTWIDGETS_CPP_HAND_H
#define SIMPLE_LIVE_QTWIDGETS_CPP_HAND_H
#include <vector>
#include <peak/peak.hpp>
#include <peak_ipl/peak_ipl.hpp>
#include <peak/converters/peak_buffer_converter_ipl.hpp>

namespace demoHautRange {
    const int R_MIN = 80, R_MAX = 250;
    const int G_MIN = 50, G_MAX = 200;
    const int B_MIN = 35, B_MAX = 160;
}

struct HAND {
    int center_hand_X; // the x of the hand center
    int center_hand_Y; // the y of the hand center

    int hand_object_width; // the width of the hand object
    int hand_object_height; // the height of the hand object
    int number_hand_object_felds; // the compleat number of hand object felds
    int number_hand_object_felds_valid; // the number of all hand object felds witch have a part of the hand
    int number_hand_object_felds_not_valid; // the number of all hand object felds witch have not a part of the hand
    std::vector<int> hand_object_felds_ids; // The ID from etch feld witch is part of the hand object

    std::vector<int> hand_felds_id; // The ID from etch feld witch is part of the hand
};

struct OBJECT {
    int object_ID = -1;
    int object_x = -1;
    int object_y = -1;
    int object_width = 0;
    int object_height = 0;
    int number_object_felds = 0;
    int number_object_felds_valid = 0;
    int number_object_felds_not_valid = 0;
    std::vector<int> object_felds_ids;
};

struct FELD {
    std::vector<int> pixel_ID;
    std::vector<int> pixel_X;
    std::vector<int> pixel_Y;
    std::vector<std::array<int, 3>> pixel_RGB;
    int feld_ID;
    int feld_x;
    int feld_y;
    int collor; // collor 0, 1, 2
    int number_of_pixels = pixel_ID.size();
    int grub_ID = -1;
};

struct FELD_MAPP {
    std::vector<FELD> felds;
    int number_of_felds = felds.size();
};


class Hand{

    public:
        Hand();

        HAND return_hand();

        bool hand_is_there() {return _hand_is_there;}
        FELD_MAPP serch_hand(const std::shared_ptr<peak::core::Buffer>& buffer);



    private:
        HAND _my_hand;
        bool _hand_is_there = false;

        FELD_MAPP getFelds(const peak::ipl::Image& rgbImage, int width, int height);

        std::array<int, 3> getPixel_RGB(const peak::ipl::Image& rgbImage, int x, int y);

        void feldCollor(FELD* felds);

        bool hasNeighbor(FELD_MAPP *mapp, int id, int number_of_columns, int number_of_rows);

        std::vector<OBJECT> getObjects(FELD_MAPP* mapp, int number_of_columns, int number_of_rows);

        std::vector<FELD_MAPP> getGrubs(FELD_MAPP *mapp, int number_of_columns, int number_of_rows);

};



#endif //SIMPLE_LIVE_QTWIDGETS_CPP_HAND_H