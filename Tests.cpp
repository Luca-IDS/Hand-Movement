//
// Created by las on 30.10.2026.
//


#include "hand.h"

TEST Hand::test_size(std::vector<OBJECT> *objects, int i) {

    int widthe = (*objects)[i].object_width;
    int height = (*objects)[i].object_height;
    int number_of_felds = (*objects)[i].number_object_felds;

    TEST test1;
    TEST test2;
    TEST test3;
    TEST test4;

    if (widthe <= 4 || height <= 4) {
        test1.probability = 3;
        test1.priority =  3;
    }

    if (number_of_felds <= 30) {
        test2.probability = 7;
        test2.priority = 2;
    }

    if (widthe >= 27) {
        test3.probability = 7;
        test3.priority = 2;

    }

    if (number_of_felds >= 300) {
        test4.probability = 10;
        test4.priority = 2;
    }
    

    double value =
    (test1.probability * test1.priority +
     test2.probability * test2.priority +
     test3.probability * test3.priority +
     test4.probability * test4.priority) /
    (test1.priority + test2.priority +
     test3.priority + test4.priority);


    TEST return_value;

    return_value.probability = value;
    return_value.priority = 4;
    return_value.value = value * 4;

    return return_value;

}
