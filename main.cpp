#include<iostream>
#include<opencv2/opencv.hpp>

#include "detect.h"
#include "judge.h"
#include "tools.h"

//参数为所给视频适用参数

int main()
{
	double time = cv::getTickCount();
	cv::VideoCapture cap("1.mov");
	BuffDetector detector;
	BuffJudge judge;
	int frame_count = 0;
	cv::Point2f aim;
	while (cap.isOpened())
	{
		frame_count++;
		std::cout << frame_count << std::endl;
		cv::Mat src, gray, binary;
		cap >> src;
		if (src.empty())
		{
			std::cout << "播放完成" << std::endl;
			break;
		}
		cv::resize(src, src, src.size() / 2);

		cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
		gray.convertTo(binary, CV_64F, 1.0 / 255, 0);
		double gamma = 2.8;
		cv::pow(binary, gamma, binary);
		binary.convertTo(binary, CV_8U, 255, 0);
		cv::threshold(binary, binary, 80, 255, CV_THRESH_BINARY);

		//在该帧图像上画上上一步的预测点
		if (judge.predicting)
		{
			cv::circle(src, aim, 0, cv::Scalar(0, 255, 0), 5);
			std::cout << "aim position: " << aim << std::endl;
		}

		std::vector<std::vector<cv::Point>> contours;
		findContours(binary, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		std::vector<armor_info> full_blade;
		std::vector<armor_info> half_blade;
		for (int i = 0; i < contours.size(); i++)
		{
			cv::Rect rect = cv::boundingRect(contours[i]);
			if (rect.area() < 400)
			{
				continue;
			}

			cv::Mat ROI = binary(rect);
			armor_info temp_full_blade, temp_half_blade;

			int sign_origin = detector.assure_img(ROI, src, rect, temp_full_blade, temp_half_blade, 1);
			if (sign_origin)
			{
				temp_full_blade.rect.center.x += rect.x;
				temp_full_blade.rect.center.y += rect.y;
				full_blade.push_back(temp_full_blade);
				continue;
			}

			//调节gamma值之后发现没起到作用
			/*detector.self_contour_fusion(ROI);
			int sign_self = detector.assure_img(ROI, src, rect, temp_full_blade, temp_half_blade, 1);
			if (sign_self)
			{
				temp_full_blade.rect.center.x += rect.x;
				temp_full_blade.rect.center.y += rect.y;
				full_blade.push_back(temp_full_blade);
				continue;
			}*/

			//对半亮扇叶区域ROI进行处理
			if (temp_full_blade.rect.center == cv::Point2f(0, 0) && temp_half_blade.rect.center != cv::Point2f(0, 0))
			{
				detector.halfblade_proc(temp_half_blade, rect, binary, ROI);
				half_blade.push_back(temp_half_blade);
			}
		}

		cv::Point2f center;
		if (full_blade.size() > 0 && judge.center_get == 1)
		{
			center = detector.get_circle(full_blade, binary, src);
			cv::circle(src, center, 0, cv::Scalar(0, 0, 255), 3);
		}
		else if (half_blade.size() > 0 && judge.center_get == 1)
		{
			center = detector.get_circle(half_blade, binary, src);
			cv::circle(src, center, 0, cv::Scalar(0, 0, 255), 3);
		}

		//击打完成后结束
		if (full_blade.size() == 5)
		{
			std::cout << "识别完成" << std::endl;
			time = 1000 * ((double)cv::getTickCount() - time) / cv::getTickFrequency();
			std::cout << std::fixed << time << "ms" << std::endl;
			cv::imshow("src", src);
			cv::waitKey(0);
			return 1;
		}

		//记录下来前后状态
		//在该份程序中不起作用
		if (full_blade.size() + half_blade.size() > 0 && judge.state_finish)
		{
			judge.buff_doc_pre = judge.buff_doc_now;

			judge.buff_doc_temp.full_blade.clear();
			for (int m = 0; m < full_blade.size(); m++)
			{
				cv::RotatedRect sign_rect = full_blade[m].rect;
				judge.buff_doc_temp.full_blade.push_back(sign_rect);
			}
			judge.buff_doc_temp.half_blade.clear();
			for (int m = 0; m < half_blade.size(); m++)
			{
				cv::RotatedRect sign_rect = half_blade[m].rect;
				judge.buff_doc_temp.half_blade.push_back(sign_rect);
			}
			judge.buff_doc_temp.center = center;
			if (half_blade.size() > 0)
			{
				judge.buff_doc_temp.armor = judge.buff_doc_temp.half_blade[0];//错误处理
				std::cout << "actual position: " << judge.buff_doc_temp.armor.center << std::endl;
			}
			judge.buff_doc_now = judge.buff_doc_temp;

			//为记录已打扇叶相对位置而设，但未完成扇叶相对位置
			//记录了扇叶shine_state，当num2（half_blade数量）为1时，开启predicting
			judge.rule_assure();

			if (judge.predicting)
			{
				aim = judge.aim_get();
			}
		}
		
		//获取判断旋转方式第一帧
		if (half_blade.size() > 0 && judge.frame_start_assure == 0)
		{
			judge.frame_start = frame_count;
			std::cout << "frame_start: " << frame_count << std::endl;
			judge.frame_start_assure = 1;
			judge.buff_doc_pre.full_blade.clear();
			for (int m = 0; m < full_blade.size(); m++)
			{
				cv::RotatedRect sign_rect = full_blade[m].rect;
				judge.buff_doc_pre.full_blade.push_back(sign_rect);
			}
			judge.buff_doc_pre.half_blade.clear();
			for (int m = 0; m < half_blade.size(); m++)
			{
				cv::RotatedRect sign_rect = half_blade[m].rect;
				judge.buff_doc_pre.half_blade.push_back(sign_rect);
			}
			judge.buff_doc_pre.center = center;
			judge.buff_doc_pre.armor = judge.buff_doc_pre.half_blade[0];
		}

		//获取判断旋转方式第二帧
		if (half_blade.size() > 0 && (frame_count - judge.frame_start) >= 3 && judge.frame_start_assure == 1 
			&& (frame_count - judge.frame_start) <= 10 && judge.state_finish == 0)//防止frame_start后第三帧未检测到击打装甲出错
		{
			for (int m = 0; m < full_blade.size(); m++)
			{
				cv::RotatedRect sign_rect = full_blade[m].rect;
				judge.buff_doc_after.full_blade.push_back(sign_rect);
			}
			for (int m = 0; m < half_blade.size(); m++)
			{
				cv::RotatedRect sign_rect = half_blade[m].rect;
				judge.buff_doc_after.half_blade.push_back(sign_rect);
			}
			judge.buff_doc_after.center = center;
			judge.buff_doc_after.armor = judge.buff_doc_after.half_blade[0];
			judge.frame_end_assure = 1;

			judge.rotate_state_judge();
			//std::cout << "rotate_state: " <<  judge.rotate_state << std::endl;
			judge.buff_doc_now = judge.buff_doc_after;
		}

		//cv::imshow("binary", binary);
		cv::imshow("src", src);

		cv::waitKey(1);
	}

	time = 1000 * ((double)cv::getTickCount() - time) / cv::getTickFrequency();
	std::cout << std::fixed << time << "ms" << std::endl;

	cv::waitKey(0);
	return 0;
}