#include "tools.h"

//画出boundingRect
void drawRect(cv::Rect rect, cv::Mat src)
{
	line(src, cv::Point(rect.x, rect.y), cv::Point(rect.x + rect.width, rect.y), cv::Scalar(255, 0, 0));
	line(src, cv::Point(rect.x + rect.width, rect.y), cv::Point(rect.x + rect.width, rect.y + rect.height), cv::Scalar(255, 0, 0));
	line(src, cv::Point(rect.x + rect.width, rect.y + rect.height), cv::Point(rect.x, rect.y + rect.height), cv::Scalar(255, 0, 0));
	line(src, cv::Point(rect.x, rect.y + rect.height), cv::Point(rect.x, rect.y), cv::Scalar(255, 0, 0));
}


//由一个RotatedRect出发，得到与之平行的两个方向、不同比例的RotatedRect
void get_parallelRect(cv::RotatedRect rect, float length, cv::RotatedRect& test1, cv::RotatedRect& test2)
{
	float spec = (length - 1) / 2;
	test1.angle = rect.angle;
	test2.angle = rect.angle;
	float height = rect.size.height;
	test1.size.height = length * height;
	test2.size.height = length * height;
	test1.size.width = rect.size.width;
	test2.size.width = rect.size.width;
	double abs_angle = fabs(rect.angle) / 180 * CV_PI;//注意C++中cos等等使用弧度
	
	if (rect.angle == 90)
	{
		test1.center = cv::Point2f(rect.center.x - spec * height, rect.center.y);
		test2.center = cv::Point2f(rect.center.x + spec * height, rect.center.y);
	}
	else if (rect.angle < 0)
	{
		test1.center = cv::Point2f(rect.center.x - spec * height * sin(abs_angle), rect.center.y - spec * height * cos(abs_angle));
		test2.center = cv::Point2f(rect.center.x + spec * height * sin(abs_angle), rect.center.y + spec * height * cos(abs_angle));
	}
	else if (rect.angle > 0)
	{
		test1.center = cv::Point2f(rect.center.x - spec * height * sin(abs_angle), rect.center.y + spec * height * cos(abs_angle));
		test2.center = cv::Point2f(rect.center.x + spec * height * sin(abs_angle), rect.center.y - spec * height * cos(abs_angle));
	}
	else if (rect.angle == 0)
	{
		test1.center = cv::Point2f(rect.center.x, rect.center.y + spec * height);
		test2.center = cv::Point2f(rect.center.x, rect.center.y - spec * height);
	}
	else
	{
		std::cout << "problem in getting test rects" << std::endl;
	}
}


//sign = 0时，不指定angle,用于得到沿1方向框出圆心的boundingRect
//sign = 1时，指定angle，得到平行的1方向RotatedRect
void test1_process(cv::RotatedRect& test1, cv::RotatedRect armor_rect, float length, int sign)
{
	float spec;
	if (sign == 0)
	{
		spec = length - 1.5;
	}
	else
	{
		spec = (length - 1) / 2;
	}
	float height = armor_rect.size.height;
	double abs_angle = fabs(armor_rect.angle) / 180 * CV_PI;
	if (sign == 0)
	{
		test1.size.height = 2 * height;
	}
	else
	{
		test1.size.height = length * height;
		test1.angle = armor_rect.angle;
	}

	test1.size.width = armor_rect.size.width;
	if (armor_rect.angle == 90)
	{
		test1.center = cv::Point2f(armor_rect.center.x - spec * height, armor_rect.center.y);
	}
	else if (armor_rect.angle < 0)
	{
		test1.center = cv::Point2f(armor_rect.center.x - spec * height * sin(abs_angle), armor_rect.center.y - spec * height * cos(abs_angle));
	}
	else if (armor_rect.angle > 0)
	{
		test1.center = cv::Point2f(armor_rect.center.x - spec * height * sin(abs_angle), armor_rect.center.y + spec * height * cos(abs_angle));
	}
	else if (armor_rect.angle == 0)//ÀÎŒÇ==
	{
		test1.center = cv::Point2f(armor_rect.center.x, armor_rect.center.y + spec * height);
	}
}


//sign = 0时，不指定angle,用于得到沿2方向框出圆心的boundingRect
//sign = 1时，指定angle，得到平行的2方向RotatedRect
void test2_process(cv::RotatedRect& test2, cv::RotatedRect armor_rect, float length, int sign)
{
	float spec;
	if (sign == 0)
	{
		spec = length - 1.5;
	}
	else
	{
		spec = (length - 1) / 2;
	}
	float height = armor_rect.size.height;
	double abs_angle = fabs(armor_rect.angle) / 180 * CV_PI;
	if (sign == 0)
	{
		test2.size.height = 2 * height;
	}
	else
	{
		test2.size.height = length * height;
		test2.angle = armor_rect.angle;
	}
	test2.size.width = armor_rect.size.width;

	if (armor_rect.angle == 90)
	{
		test2.center = cv::Point2f(armor_rect.center.x + spec * height, armor_rect.center.y);
	}
	else if (armor_rect.angle < 0)
	{
		test2.center = cv::Point2f(armor_rect.center.x + spec * height * sin(abs_angle), armor_rect.center.y + spec * height * cos(abs_angle));
	}
	else if (armor_rect.angle > 0)
	{
		test2.center = cv::Point2f(armor_rect.center.x + spec * height * sin(abs_angle), armor_rect.center.y - spec * height * cos(abs_angle));
	}
	else if (armor_rect.angle == 0)
	{
		test2.center = cv::Point2f(armor_rect.center.x, armor_rect.center.y - spec * height);
	}
}