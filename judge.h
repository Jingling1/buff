#ifndef JUDGE_H
#define JUDGE_H

#include<iostream>
#include<opencv2/opencv.hpp>

struct BuffDoc
{
	cv::Point2f center;
	std::vector<cv::RotatedRect> full_blade;
	std::vector<cv::RotatedRect> half_blade;
	cv::RotatedRect armor;
};

struct BladeInfo
{
	int total_frame = 0;
	float angle;
};

struct BreakInfo
{
	int total_frame = 0;
	float angle;
};

class BuffJudge
{
public:
	BuffJudge();
	~BuffJudge();

	void rotate_state_judge();

	void rule_assure();

	cv::Point2f aim_get();

	int frame_start, frame_start_assure = 0, frame_end_assure = 0;
	int rotate_state, state_finish = 0;
	BuffDoc buff_doc_pre, buff_doc_after, buff_doc_now, buff_doc_temp;

	BladeInfo blades[5];
	BreakInfo break_period[4];
	int shine_state;
	std::vector<int> buff_shine_states;

	int predicting = 0;

	int center_get = 1;
	float radius;
private:

};

#endif //JUDGE_H