//
// Created by las on 30.10.2026.
//


#include "hand.h"

TEST Hand::test_size(std::vector<OBJECT> *objects, int i) {

    TEST return_value;
    int test_priority = 2;

    int width = (*objects)[i].object_width;
    int height = (*objects)[i].object_height;
    int felds = (*objects)[i].number_object_felds;
    int valid_felds = (*objects)[i].number_object_felds_valid;
    int not_valid_felds = (*objects)[i].number_object_felds_not_valid;

    // Min
    if (width >= 7 && width <= 27 &&
        height >= 10 && height <= 25) {
        if (not_valid_felds >= felds * 25 / 100 && not_valid_felds <= felds * 70 / 100) {
            // Medium
            if (width >= 9 && width <= 25 &&
                height >= 13 && height <= 24) {
                if (not_valid_felds >= felds * 33 / 100 && not_valid_felds <= felds * 65 / 100) {
                    // Optimal
                    if (width >= 11 && width <= 23 &&
                        height >= 15 && height <= 22) {
                        if (not_valid_felds >= felds * 40 / 100 && not_valid_felds <= felds * 60 / 100) {
                            //100
                        }
                        //80
                    }
                    //60
                }
                //40
            }
            //20
        }
        //10
    }
    else {
        return_value.value = 0;
        return_value.priority = test_priority;

        return return_value;
    }

    return return_value;
}


TEST Hand::test_mooving(std::vector<OBJECT> *objects, int i) {

    TEST return_value;

    return return_value;
}

TEST Hand::test_object_appear(std::vector<OBJECT> *objects, int i) {

    TEST return_value;

    return return_value;
}



