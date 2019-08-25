#include "judge.h"

BuffJudge::BuffJudge()
{

}

BuffJudge::~BuffJudge()
{

}

void BuffJudge::rotate_state_judge()
{
	cv::Point2f vector1 = buff_doc_pre.armor.center - buff_doc_pre.center;
	cv::Point2f vector2 = buff_doc_after.armor.center - buff_doc_after.center;
	double angle1 = atan2(vector1.y, vector1.x) / CV_PI * 180 + 180;
	double angle2 = atan2(vector2.y, vector2.x) / CV_PI * 180 + 180;
	radius = (sqrtf(pow(vector1.x, 2) + pow(vector1.y, 2)) + sqrtf(pow(vector2.x, 2) + pow(vector2.y, 2))) / 2;
	if (fabs(angle1 - angle2) < 3)
	{
		rotate_state = 0;
		center_get = 0;
	}
	else if (angle1 > angle2)
	{
		rotate_state = 1;//逆时针
	}
	else
	{
		rotate_state = 2;//顺时针
	}
	state_finish = 1;
}


void BuffJudge::rule_assure()
{
	int num1 = buff_doc_now.full_blade.size();
	int num2 = buff_doc_now.half_blade.size() > 0 ? 1 : 0;
	int shine_state = 2 * num1 + num2;
	if (buff_shine_states.size() < 5)
	{
		buff_shine_states.push_back(shine_state);
	}
	else if(buff_shine_states.size() == 5)
	{
		buff_shine_states.erase(buff_shine_states.begin(), buff_shine_states.begin() + 1);
		buff_shine_states.push_back(shine_state);
	}

	if (num1 > 0 && num2 == 0)
	{
		break_period[num1 - 1].total_frame++;
	}

	if (num2 == 1)
	{
		blades[num1].total_frame++;
		if (center_get)
		{
			cv::Point2f vec = buff_doc_now.armor.center - buff_doc_now.center;
			blades[num1].angle = atan2(vec.y, vec.x) / CV_PI * 180 + 180;
		}

		if (num1 > 0)
		{
			break_period[num1 - 1].angle = break_period[num1 - 1].total_frame * 1.6364;//²ÎÊýŽýµ÷Õû
		}
	}

	if (num2 > 0)//buff_shine_states[buff_shine_states.size() - 1] == buff_shine_states[buff_shine_states.size() - 2]
	{
		predicting = 1;
	}
	else
	{
		predicting = 0;
	}
}


cv::Point2f BuffJudge::aim_get()
{
	cv::Point2f aim;
	cv::Point2f vec;
	float delta_angle = 1.6364;
	float angle;
	switch (rotate_state)
	{
	case 0:
		aim = buff_doc_now.armor.center;
		return aim;

	case 1:
		vec = buff_doc_now.armor.center - buff_doc_now.center;
		radius = (norm(vec) + radius) / 2;//获得更准的radius
		angle = atan2(vec.y, vec.x) / CV_PI * 180 - delta_angle;//+ 180
		if (angle < 0)
		{
			angle += 360;
		}
		aim.x = buff_doc_now.center.x + radius * cos(angle / 180 * CV_PI);
		aim.y = buff_doc_now.center.y + radius * sin(angle / 180 * CV_PI);
		return aim;

	case 2:
		vec = buff_doc_now.armor.center - buff_doc_now.center;
		radius = (norm(vec) + radius) / 2;//获得更准的radius
		angle = atan2(vec.y, vec.x) / CV_PI * 180 + delta_angle;// + 180
		if (angle > 360)
		{
			angle -= 360;
		}
		aim.x = buff_doc_now.center.x + radius * cos(angle / 180 * CV_PI);
		aim.y = buff_doc_now.center.y + radius * sin(angle / 180 * CV_PI);
		return aim;
	}
}