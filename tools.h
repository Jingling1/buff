#ifndef TOOLS_H
#define TOOLS_H

#include<iostream>
#include<opencv2/opencv.hpp>

void drawRect(cv::Rect rect, cv::Mat src);

void get_parallelRect(cv::RotatedRect rect, float length, cv::RotatedRect& test1, cv::RotatedRect& test2);

void test1_process(cv::RotatedRect& test1, cv::RotatedRect armor_rect, float length, int sign);

void test2_process(cv::RotatedRect& test2, cv::RotatedRect armor_rect, float length, int sign);

#endif //TOOLS_H