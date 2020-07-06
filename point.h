#ifndef POINT_H
#define POINT_H

#include <vector>
#include <string>

struct Point {

	int arrival_time = 0;
	int duration = 0;
	std::vector<float> data;
	std::string timestamp;
};

struct BlockNode {
	BlockNode(BlockNode* l, BlockNode* r, int start, int end) {
		left = l;
		right = r;
		ts = start;
		te = end;
	}
	BlockNode* left;
	BlockNode* right;
	int ts;
	int te;
	std::vector<Point> skyline;
};

struct AnsUnit{
	int ans_size;
	int oracle_calls;
	int iterations;
	int lower_bound;
	int upper_bound;
	int candidate_size;
};

struct TimeUnit {
	int ans_size;
	int k;
	int L;
	int tau;
	// sliding window with skipping
	float baseline_time;
	// sliding window with kskyband filter
	float sf_time;
	// sliding window with incremental update
	float sw_time;
	float weighted_time;
	float better_theory_time;
	float theory_time;
};

#endif