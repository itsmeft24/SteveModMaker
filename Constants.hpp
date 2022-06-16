#pragma once

enum class PartSize : int {
	HEAD = 0,
	SIZE_4_12 = 1,
	SIZE_3_12 = 2,
	BODY = 3,
};

enum class BodyType : int {
	STEVE = 4,
	ALEX = 3,
};

const static cv::Size CANVAS_SIZE(968, 1864);

const static cv::Point2f HEAD_SIZE[4] = {
	{0.0, 0.0},
	{800.0, 0.0},
	{800.0, 800.0},
	{0.0, 800.0}
};

const static cv::Point2f SIZE_4_12[4] = {
	{0.0, 0.0},
	{400.0, 0.0},
	{400.0, 1200.0},
	{0.0, 1200.0}
};

const static cv::Point2f SIZE_3_12[4] = {
	{0.0, 0.0},
	{300.0, 0.0},
	{300.0, 1200.0},
	{0.0, 1200.0}
};

const static cv::Point2f BODY_SIZE[4] = {
	{0.0, 0.0},
	{800.0, 0.0},
	{800.0, 1200.0},
	{0.0, 1200.0}
};