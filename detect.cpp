#include "detect.h"
#include "tools.h"

BuffDetector::BuffDetector()
{

}

BuffDetector::~BuffDetector()
{

}

int BuffDetector::assure_img(cv::Mat& ROI, cv::Mat src, cv::Rect rect, armor_info& full_armor, armor_info& half_armor, int sign)
{
	int sign1 = 0, sign2 = 0;
	cv::Mat inv;
	cv::Mat flood = cv::Mat::ones(cv::Size(ROI.cols + 2, ROI.rows + 2), ROI.type());
	cv::threshold(flood, flood, 0, 255, CV_THRESH_BINARY);
	cv::bitwise_not(ROI, inv);
	inv.copyTo(flood(cv::Rect(1, 1, ROI.cols, ROI.rows)));
	cv::floodFill(flood, cv::Point(0, 0), cv::Scalar(0));

	cv::Mat flood_proced(ROI.size(), ROI.type());
	flood(cv::Rect(1, 1, ROI.cols, ROI.rows)).copyTo(flood_proced);
	std::vector<std::vector<cv::Point>> contours;
	std::vector<std::vector<cv::Point>> armors;
	std::vector<std::vector<cv::Point>> bars;
	cv::findContours(flood, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++)
	{
		cv::RotatedRect rect = cv::minAreaRect(contours[i]);
		if (rect.size.height > rect.size.width)
		{
			std::swap(rect.size.width, rect.size.height);
			rect.angle += 90;
		}
		if (rect.size.width / rect.size.height > 1.5 && rect.size.width / rect.size.height < 2.5 && rect.size.area() > 400)//能量机关大小不同需要调参  另一种情况 > 50 && rect.size.area() < 300
		{
			armors.push_back(contours[i]);
		}
	}
	if (armors.size() == 1)
	{
		sign1 = 1;
	}

	for (int i = 0; i < contours.size(); i++)
	{
		cv::RotatedRect rect = cv::minAreaRect(contours[i]);
		if (rect.size.height > rect.size.width)
		{
			std::swap(rect.size.width, rect.size.height);
			rect.angle += 90;
		}
		if (rect.size.width / rect.size.height > 3 && rect.size.width / rect.size.height < 8 && rect.size.area() > 50)
		{
			bars.push_back(contours[i]);
		}
	}
	if (bars.size() > 0)
	{
		sign2 = 1;
	}
	if (sign1 == 1 && sign2 == 1)
	{
		full_armor.rect = minAreaRect(armors[0]);
		if (full_armor.rect.size.height > full_armor.rect.size.width)
		{
			std::swap(full_armor.rect.size.width, full_armor.rect.size.height);
			full_armor.rect.angle += 90;
		}
		full_armor.direction = get_direction(full_armor.rect, flood);
		return 1;
	}
	if (sign1 == 1 && sign2 == 0 && sign == 1)
	{
		half_armor.rect = minAreaRect(armors[0]);
		if (half_armor.rect.size.height > half_armor.rect.size.width)
		{
			std::swap(half_armor.rect.size.width, half_armor.rect.size.height);
			half_armor.rect.angle += 90;
		}
		if (half_armor.rect.size.area() / (ROI.rows * ROI.cols) < 0.15)
		{
			flood_proced.copyTo(ROI);
		}
	}
	return 0;
}


int BuffDetector::get_direction(cv::RotatedRect rect, cv::Mat img)
{
	cv::RotatedRect test1, test2;
	get_parallelRect(rect, 2, test1, test2);
	float ratio1, ratio2;
	int flag = get_rectRatio(img, test1, test2, ratio1, ratio2);
	if (flag == 0)
	{
		if (ratio1 < ratio2)
		{
			return 2;
		}
		else
		{
			return 1;
		}
	}
	else if (flag == 1)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}


int BuffDetector::get_rectRatio(cv::Mat gray, cv::RotatedRect test1, cv::RotatedRect test2, float& ratio1, float& ratio2)
{
	cv::Point2f vertices_temp[4];
	cv::Mat rot_mat, rotate, ROI;
	cv::Size dst_sz(gray.size());
	int nonzero_num;
	test1.points(vertices_temp);
	cv::RotatedRect circle;
	for (int i = 0; i < 4; i++)
	{
		if (vertices_temp[i].x > gray.cols || vertices_temp[i].x < 0 || vertices_temp[i].y < 0 || vertices_temp[i].y > gray.rows)
		{
			ratio1 = 0;
			return 2;
		}
	}
	test2.points(vertices_temp);
	for (int i = 0; i < 4; i++)
	{
		if (vertices_temp[i].x > gray.cols || vertices_temp[i].x < 0 || vertices_temp[i].y < 0 || vertices_temp[i].y > gray.rows)
		{
			ratio2 = 0;
			return 1;
		}
	}
	
	rot_mat = cv::getRotationMatrix2D(test1.center, test1.angle, 1);
	warpAffine(gray, rotate, rot_mat, dst_sz);
	ROI = rotate(cv::Rect(test1.center.x - (test1.size.width / 2), test1.center.y - (test1.size.height / 2), test1.size.width, test1.size.height));
	nonzero_num = countNonZero(ROI);
	ratio1 = nonzero_num / (float)(ROI.cols * ROI.rows);
	rot_mat = cv::getRotationMatrix2D(test2.center, test2.angle, 1);
	warpAffine(gray, rotate, rot_mat, dst_sz);
	ROI = rotate(cv::Rect(test2.center.x - (test2.size.width / 2), test2.center.y - (test2.size.height / 2), test2.size.width, test2.size.height));
	nonzero_num = countNonZero(ROI);
	ratio2 = nonzero_num / (float)(ROI.cols * ROI.rows);
	return 0;
}


//原本想用来填补能量机关扇叶图形
void BuffDetector::self_contour_fusion(cv::Mat& ROI)
{
	std::vector<std::vector<cv::Point>> contours;
	std::vector<std::vector<point_info>> points_set;
	cv::findContours(ROI, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	int contour_index;
	int max_num = 0;
	if (contours.size() == 0)
		return;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > max_num)
		{
			max_num = contours[i].size();
			contour_index = i;
		}
		std::vector<point_info> points;
		if (contours[i].size() < 4)
		{
			for (int j = 0; j < contours[i].size(); j++)
			{
				point_info sign;
				sign.position = contours[i][j];
				sign.serial_num = j;
				points.push_back(sign);
			}
		}
		if (contours[i].size() > 4)
		{
			point_info sign;
			sign.position = contours[i][0];
			sign.serial_num = 0;
			points.push_back(sign);
		}
		int size_ = contours[i].size();
		for (int j = 0; j < size_ - 3; j++)
		{
			cv::Point point1, point2, point3, point4;
			point1 = contours[i][j];
			point2 = contours[i][j + 1];
			point3 = contours[i][j + 2];
			point4 = contours[i][j + 3];

			cv::Point vector1 = point2 - point1;
			cv::Point vector2 = point4 - point3;
			float cos_theta = (vector1.x * vector2.x + vector1.y * vector2.y) / (2 * norm(vector1) * norm(vector2));

			if (cos_theta == 0 || cos_theta < 0)
			{
				point_info sign;
				sign.position = point3;
				sign.serial_num = j + 2;
				points.push_back(sign);
			}
		}
		if (contours[i].size() > 4)
		{
			point_info sign;
			sign.position = contours[i][contours[i].size() - 1];
			sign.serial_num = contours[i].size() - 1;
			points.push_back(sign);
		}
		points_set.push_back(points);
	}

	if (points_set[contour_index].size() < 30)//调参
	{
		return;
	}
	std::vector<cv::Point> contour_before = contours[contour_index];
	std::vector<cv::Point> contour_later;
	contour_later.assign(contour_before.begin(), contour_before.end());
	double max_area = 0;
	cv::Point point[2];
	int size_p = points_set[contour_index].size();
	for (int a = 0; a < size_p - 1; a++)
	{
		cv::Point se_point = points_set[contour_index][a].position;
		int se_num = points_set[contour_index][a].serial_num;
		for (int b = a + 1; b < points_set[contour_index].size(); b++)
		{
			int seek_num = points_set[contour_index][b].serial_num;
			if (seek_num == contours[contour_index].size() - 1)
				continue;
			contour_later.erase(contour_later.begin() + se_num + 1, contour_later.begin() + seek_num);//应注意序号
			double area = contourArea(contour_later);
			if (area > max_area)
			{
				point[0] = points_set[contour_index][a].position;
				point[1] = points_set[contour_index][b].position;
				max_area = area;
			}
			contour_later.clear();
			contour_later.assign(contour_before.begin(), contour_before.end());
		}
	}
	if (point[0].x == point[1].x)
	{
		if (point[0].y > point[1].y)
		{
			std::swap(point[0], point[1]);
		}
		for (int m = point[0].y; m <= point[1].y; m++)
		{
			ROI.at<uchar>(m, point[0].x) = 255;
		}
	}
	else
	{
		if (point[0].x > point[1].x)
		{
			std::swap(point[0], point[1]);
		}
		for (int m = point[0].x; m <= point[1].x; m++)
		{
			float n = 1.0 * (point[0].y - point[1].y) / (point[0].x - point[1].x) * (m - point[0].x) + point[0].y;//����1.0
			if (int(n) < ROI.rows && int(n) > 0)
			{
				ROI.at<uchar>(int(n), m) = 255;
			}
			if ((int(n) + 1) < ROI.rows && (int(n) + 1) > 0)
			{
				ROI.at<uchar>(int(n) + 1, m) = 255;
			}
			if ((int(n) - 1) < ROI.rows && (int(n) - 1) > 0)
			{
				ROI.at<uchar>(int(n) - 1, m) = 255;
			}
		}
	}
}


//处理半亮扇叶
void BuffDetector::halfblade_proc(armor_info& temp_half_blade, cv::Rect rect, cv::Mat& binary, cv::Mat ROI)
{
	temp_half_blade.rect.center.x += rect.x;
	temp_half_blade.rect.center.y += rect.y;
	cv::RotatedRect ex_rect, ex_rect1, ex_rect2;
	test1_process(ex_rect1, temp_half_blade.rect, 5, 1);
	test2_process(ex_rect2, temp_half_blade.rect, 5, 1);
	float ratio1 = 0, ratio2 = 0;
	int flag = get_rectRatio(binary, ex_rect1, ex_rect2, ratio1, ratio2);
	if (flag == 1)
	{
		temp_half_blade.direction = 1;
		ex_rect = ex_rect1;
	}
	else if (flag == 2)
	{
		temp_half_blade.direction = 2;
		ex_rect = ex_rect2;
	}
	else
	{
		if (ratio1 > ratio2)
		{
			temp_half_blade.direction = 1;
			ex_rect = ex_rect1;
		}
		else
		{
			temp_half_blade.direction = 2;
			ex_rect = ex_rect2;
		}
	}
	/*if (ex_rect.size.height > ex_rect.size.width)
	{
		std::swap(ex_rect.size.height, ex_rect.size.width);
		ex_rect.angle += 90;
	}*/
	if (temp_half_blade.rect.size.area() / (rect.height * rect.width) < 0.15)
	{
		ROI.copyTo(binary(rect));
	}
	else
	{
		cv::Mat rot_mat = cv::getRotationMatrix2D(ex_rect.center, ex_rect.angle, 1);
		cv::Mat rotate, ROI1;
		cv::warpAffine(binary, rotate, rot_mat, binary.size());
		ROI1 = cv::Mat::zeros(cv::Size(ex_rect.size.width, ex_rect.size.height), binary.type());
		ROI1.copyTo(rotate(cv::Rect(ex_rect.center.x - (ex_rect.size.width / 2), ex_rect.center.y - (ex_rect.size.height / 2), ex_rect.size.width, ex_rect.size.height)));
		rot_mat = cv::getRotationMatrix2D(ex_rect.center, -ex_rect.angle, 1);
		cv::warpAffine(rotate, binary, rot_mat, binary.size());
		cv::threshold(binary, binary, 1, 255, CV_THRESH_BINARY);
	}
}


//得到圆心
cv::Point2f BuffDetector::get_circle(std::vector<armor_info> armors_info, cv::Mat gray, cv::Mat src)
{
	cv::Point2f circle;
	cv::RotatedRect sign_rect;
	cv::Point2f sign_point;
	cv::Point2f vertices[4];
	if (armors_info.size() < 1)
	{
		return circle;
	}
	else
	{
		armors_info[0].rect.points(vertices);
		for (int i = 0; i < 4; i++)
		{
			line(src, vertices[i], vertices[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
		}
		if (armors_info[0].direction == 1)
		{
			test1_process(sign_rect, armors_info[0].rect, 7, 0);
		}
		else
		{
			test2_process(sign_rect, armors_info[0].rect, 7, 0);
		}
	}
	sign_point = sign_rect.center;
	sign_rect.size.width *= 2;
	sign_rect.size.height *= 2;

	sign_rect.points(vertices);
	/*for (int i = 0; i < 4; i++)
	{
		line(src, vertices[i], vertices[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
	}*/
	std::vector<cv::Point2f> points;
	for (int i = 0; i < 4; i++)
	{
		points.push_back(vertices[i]);
	}
	cv::Rect rect = boundingRect(points);
	line(src, cv::Point(rect.x, rect.y), cv::Point(rect.x, rect.y + rect.height), cv::Scalar(0, 255, 0), 2);
	line(src, cv::Point(rect.x, rect.y + rect.height), cv::Point(rect.x + rect.width, rect.y + rect.height), cv::Scalar(0, 255, 0), 2);
	line(src, cv::Point(rect.x + rect.width, rect.y + rect.height), cv::Point(rect.x + rect.width, rect.y), cv::Scalar(0, 255, 0), 2);
	line(src, cv::Point(rect.x + rect.width, rect.y), cv::Point(rect.x, rect.y), cv::Scalar(0, 255, 0), 2);
	cv::Mat ROI_img = gray(rect);
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(ROI_img, contours, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
	std::vector<cv::RotatedRect> circle_rects;
	for (int i = 0; i < contours.size(); i++)
	{
		cv::RotatedRect circle_rect = cv::minAreaRect(contours[i]);
		if (circle_rect.size.width < circle_rect.size.height)
		{
			std::swap(circle_rect.size.width, circle_rect.size.height);
			circle_rect.angle += 90;
		}
		if (circle_rect.size.width / circle_rect.size.height <= 2 && circle_rect.size.area() > armors_info[0].rect.size.area() / 11 &&
			circle_rect.size.area() < armors_info[0].rect.size.area() / 3)//1.5
		{
			circle_rects.push_back(circle_rect);
		}
	}
	/*circle_rects[0].points(vertices);
	for (int i = 0; i < 4; i++)
	{
		line(src, vertices[i], vertices[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
	}
	/*cv::drawContours(src, contours, 2, cv::Scalar(255, 0, 0), 1);
	imshow("src", src);*/
	if (circle_rects.size() > 1)
	{
		float max_area = 0;
		int max_index = circle_rects.size();
		for (int i = 0; i < circle_rects.size(); i++)
		{
			if (circle_rects[i].size.area() > max_area)
			{
				max_area = circle_rects[i].size.area();
				max_index = i;
			}
		}
		circle = circle_rects[max_index].center;
		circle.x += rect.x;
		circle.y += rect.y;
		return circle;
	}
	if (circle_rects.size() == 1)
	{
		circle = circle_rects[0].center;
		circle.x += rect.x;
		circle.y += rect.y;
		return circle;
	}
	return circle;
}