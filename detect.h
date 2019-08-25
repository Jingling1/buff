#ifndef DETECT_H
#define DETECT_H
#include<iostream>
#include<opencv2/opencv.hpp>

struct point_info
{
	cv::Point position;
	int serial_num;
};

struct armor_info
{
	cv::RotatedRect rect;
	int direction;
};

class BuffDetector
{
public:
	BuffDetector();
	~BuffDetector();

	int assure_img(cv::Mat& ROI, cv::Mat src, cv::Rect rect, armor_info& full_armor, armor_info& half_armor, int sign);

	int get_direction(cv::RotatedRect rect, cv::Mat img);

	int get_rectRatio(cv::Mat gray, cv::RotatedRect test1, cv::RotatedRect test2, float& ratio1, float& ratio2);

	void self_contour_fusion(cv::Mat& ROI);

	void halfblade_proc(armor_info& half_blade, cv::Rect rect, cv::Mat& binary, cv::Mat ROI);

	cv::Point2f get_circle(std::vector<armor_info> armors_info, cv::Mat gray, cv::Mat src);
private:


};

#endif //DETECT_H