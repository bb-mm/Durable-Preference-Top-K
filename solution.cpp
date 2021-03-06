#include "solution.h"
#include <time.h>
#include <assert.h>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <set>
#include <queue>
#include <random>
#include <math.h>

#define INF 100000000
#define PI 3.1415926535

#define MIN(A, B) (((A)<(B))?(A):(B))
#define MAX(A, B) (((A)>(B))?(A):(B))

using namespace std;
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

template <size_t N>
float Solution<N>::f1_score(vector<tuple_t>& v1, vector<tuple_t>& v2) {
	float precision= (float) v1.size() / v2.size(), recall=1;
	return 2 * (precision * recall) / (precision + recall);
}

template <size_t N>
void Solution<N>::coreset(vector<Point>& db, vector<Point>& coreset_db, vector<Point>& temp, int k) {
	default_random_engine generator;
	uniform_real_distribution<float> uniform_f(0.0,1.0);
	set<int> unique_point;
	float w1,w2;
	vector<tuple_t> sorted_w;
	vector<float> f(2,0);
	int t;
	//cout << temp.size() << endl;
	// 100 random preference vectors
	for (int i=0;i<100;++i) {
		w1 = uniform_f(generator);
		w2 = 1-w1;
		f[0] = w1;
		f[1] = w2;
		sorted_w.clear();
		for (auto& p : temp) {
			sorted_w.emplace_back(make_pair(p.arrival_time, score(p, f)));
		}
		sort(sorted_w.begin(), sorted_w.end(), Compare());
		
		for (int j=0;j<MIN(k, sorted_w.size());++j) {
			t = get<0>(sorted_w[j]);
			if (unique_point.find(t) == unique_point.end()) {
				unique_point.insert(t);
				coreset_db.emplace_back(db[t]);
			}
		}
	}
}

template <size_t N>
vector<Point> Solution<N>::all_range_coreset(vector<Point>& db, int k) {
	vector<Point> coreset_db;
	int min_range = db.size() * 0.01;
	vector<Point> temp;
	for (int i=0; i<db.size(); i+= min_range) {
		temp.clear();
		cout << '\r' << i << flush;
		for (int j=i; j<MIN(db.size(), i+min_range); ++j) {
			temp.emplace_back(db[j]);
		}
		//compute coreset
		coreset(db, coreset_db, temp, k);
	}
	cout << "original data size: " << db.size() << " coreset data size: " << coreset_db.size() << endl;
	return coreset_db;
}

// group consecutive data points into clusters
// and compute skyline point for each group
template <size_t N>
void Solution<N>::grouping(vector<Point>& db, int block_size) {
	int blocks = db.size() / block_size;
	cout << "data grouping..." << endl;
	cout << "block size: " << block_size << endl;
	cout << "total blocks: " << blocks << endl;

	for(int i=0; i<blocks; ++i) {
		db_blocks.emplace_back(
			vector<Point>(db.begin()+i*block_size, db.begin()+(i+1)*block_size)
			);
		db_block_skyline.emplace_back(get_skyline(db_blocks.back()));
	}
	// if there is a last block,
	// process the last block
	int left = db.size() % block_size;
	if (left > 0) {
		db_blocks.emplace_back(vector<Point>(db.end()-left, db.end()));
		db_block_skyline.emplace_back(get_skyline(db_blocks.back()));
	}

	// size check
	int cnt = 0, skyline_cnt = 0;
	for (int i=0; i<db_blocks.size(); ++i) {
		for (int j=0; j<db_blocks[i].size(); ++j)
			cnt += 1;
		for (int j=0; j<db_block_skyline[i].size(); ++j)
			skyline_cnt += 1;
	}
	
	cout << "total size: " << cnt  << endl;
	cout << "skyline size: " << skyline_cnt << endl;
	return;
}

// compute the score of a point when given a perference vector
template <size_t N>
float Solution<N>::score(Point const& p, std::vector<float>& f) {
	//assert(p.data.size()-1 == f.size());
	float sum = 0;
	for (int i=0;i<f.size(); ++i)
		sum += p.data[i] * f[i];
	return sum;
}

// check whether p dominates q
template <size_t N>
bool Solution<N>::dominate(Point& p, Point& q, int ndim) {
	int no_worse_cnt = 0;
	int better_cnt = 0;

	for (int i=0; i<ndim; ++i) {
		if (p.data[i] >= q.data[i]) {
			no_worse_cnt++;
			if (p.data[i] > q.data[i])
				better_cnt++;
		}
		else
			return false;
	}

	if (better_cnt > 0)
		return true;
	else if (better_cnt == 0)
		return p.data.back() > q.data.back();
	else
		return false;

}

template <size_t N>
int Solution<N>::get_max_timestamp(vector<tuple_t>& blocks, int curr_id) {
	int max_block_id = 0;
	for (auto& item : blocks) {
		if (get<0>(item) <= curr_id && get<0>(item) > max_block_id)
			max_block_id = get<0>(item);
	}
	//printf("max block id: %d\n", max_block_id);
	return db_blocks[max_block_id].back().arrival_time;
}

template <size_t N>
int Solution<N>::get_max_timestamp(vector<tuple_t>& v) {
	int max_t = 0;
	for (auto& item: v) {
		if (get<0>(item) > max_t)
			max_t = get<0>(item);
	}
	return max_t;
}

template <size_t N>
bool Solution<N>::exists(int i, vector<tuple_t>& candidate_block) {
	for (auto& item : candidate_block) {
		if (i == get<0>(item))
			return true;
	}
	return false;
}

template <size_t N>
vector<Point> Solution<N>::get_skyline(vector<Point>& v) {

	vector<Point> skyline;
	for (int i=0; i<v.size(); ++i) {
		bool dominated = false;
		for (int j=0; j<v.size(); ++j) {
			if (dominate(v[j], v[i], v[i].data.size() - 1)) {
				dominated = true;
				break;
			}
		}
		
		if (!dominated) {
			skyline.emplace_back(v[i]);
		}
	}
	return skyline;
}

// verify whether the current point is a durable p-topk
// if yes, expire this point in the topk result
template <size_t N>
bool Solution<N>::topk_verification(vector<tuple_t>& topk, vector<tuple_t>& ans, int t) {
	for (auto v = topk.begin(); v != topk.end(); ++v) {
		if (get<0>(*v) == t) {
			ans.emplace_back(*v);
			topk.erase(v);
			return true;
		}
	}
	return false;
}

template <size_t N>
bool Solution<N>::incremental_topk_verification(vector<Point>& db, node_t& n, vector<tuple_t>& topk, vector<float>& f, int k, int t) {
	int ts = get<1>(n)->ts, te = get<1>(n)->te;
	for(int i=ts; i<=te; ++i) {
		if (topk.size() < k) {
			topk.emplace_back(make_pair(i, score(db[i], f)));
			make_heap(topk.begin(), topk.end(), Compare());
		}
		else {
			float v = score(db[i], f);
			if (v > get<1>(topk[0])) {
				pop_heap(topk.begin(), topk.end(), Compare());
				topk.pop_back();
				topk.emplace_back(make_pair(i,v));
				make_heap(topk.begin(), topk.end(), Compare());
			}
		}
	}
	for (auto& item : topk) {
		if (get<0>(item) == t){
			return true;
		}
	}
	return false;
}

template <size_t N>
vector<tuple_t> Solution<N>::sequential(vector<Point>& db, int ts,int te, int tau, vector<float>& f, int k, vector<AnsUnit>& unit) {
	vector<tuple_t> ans;
	// maintain topk results for sliding windows
	vector<tuple_t> topk;
	bool found = true;
	int t_curr = te;
	int topk_queries_cnt = 0;

	assert(db[0].data.size()-1 == f.size());

	// for block skyline tree
	unordered_map<int, bool> reported;
	unordered_map<BlockNode*, tuple_t> max_cache;

	while (t_curr >= ts) {
		topk_queries_cnt++;
		topk = block_tree_topk_with_condition(db, reported, max_cache, t_curr - tau, t_curr, f, k);
		found = topk_verification(topk, ans, t_curr);
		if (found)
			t_curr--;
		else {
			t_curr = get_max_timestamp(topk);
		}
	}

	// while (t_curr >= ts) {
	// 	// need to recompute topk
	// 	// case1: initial window
	// 	// case2: find a qualified point
	// 	if (restart) {
	// 		topk_queries_cnt += 1;
	// 		//topk = incremental_topk(db, t_curr-tau, t_curr, f, k);
	// 		topk = block_tree_topk_with_condition(db, reported, max_cache, t_curr - tau, t_curr, f, k);
	// 		// move to the next consecutive window to verify
	// 	}
	// 	// incremental update the topk by sequentially walk on each timestamp
	// 	else {
	// 		int new_t = t_curr-tau;
	// 		if (topk.size() < k) {
	// 			topk.emplace_back(make_pair(new_t,score(db[new_t], f)));
	// 			if (ans.size() == k)
	// 				make_heap(topk.begin(), topk.end(), Compare());
	// 		}
	// 		else {
	// 			if (score(db[new_t], f) > get<1>(topk[0])) {
	// 				pop_heap(topk.begin(), topk.end(), Compare());
	// 				topk.pop_back();
	// 				topk.emplace_back(make_pair(new_t, score(db[new_t],f)));
	// 				push_heap(topk.begin(), topk.end(), Compare());
	// 			}
	// 		}
	// 	}
	// 	restart = topk_verification(topk, ans, t_curr);
	// 	t_curr--;
	// }
	AnsUnit info;
	info.ans_size = ans.size();
	info.oracle_calls = topk_queries_cnt;
	unit.emplace_back(info);
	return ans;
}

template <size_t N>
vector<tuple_t> Solution<N>::sequential_plus(vector<Point>& db, int block_size, int ts, int te, int tau, vector<float>& f, int k, vector<AnsUnit>& unit) {
	int bid_start = ts / block_size;
	int bid_end = te / block_size;
	int window_block = tau / block_size;
	int skip_length = 0;
	int topk_queries_cnt = 0;
	vector<tuple_t> candidate_block;
	vector<tuple_t> topk;
	vector<tuple_t> ans;

	if (bid_end - bid_start + 1 < k || window_block < k)
		return sequential(db, ts,te, tau, f,k, unit);
	else {
		int t_curr = te;
		int id1, id2;
		bool restart = true;
		// block-based processing
		while (t_curr >= ts) {
			id1 = t_curr / block_size;
			id2 = (t_curr - tau) / block_size;
			//printf("restart: %d", restart);
			if (restart) {
				candidate_block = incremental_topk_block(id2, id1, f, k+2);
			}
			else {
				//int new_ts = t_curr - tau;
				int bid_s = (t_curr - tau) / block_size; 
				//int new_te = new_ts + skip_length;
				int bid_e = (t_curr - tau + skip_length) / block_size; 
				for (int i=bid_e;i>=bid_s;--i) {
					if (exists(i, candidate_block))
						continue;
					float v_max = get_block_max(i, f);
					if (v_max > get<1>(candidate_block[0])) {
						pop_heap(candidate_block.begin(), candidate_block.end(), Compare());
						candidate_block.pop_back();
						candidate_block.emplace_back(make_pair(i, v_max));
						push_heap(candidate_block.begin(), candidate_block.end(), Compare());
					}
				}
			}
			// std::cout << id1 << " [";
			// for (auto& item : candidate_block)
			// 	std::cout << get<0>(item) << ' ';
			// std::cout << ']' << std::endl;
			// if (!exists(id1, candidate_block)) {
			// 	int next = get_max_timestamp(candidate_block, id1);
			// 	//printf("t_curr:%d (%d), next:%d (%d)\n", t_curr, t_curr/block_size, next, next/block_size);
			// 	assert(next <= t_curr);
			// 	skip_length = t_curr - next;
			// 	t_curr = next;
			// 	restart = false;
			// }
			// else {
			topk_queries_cnt++;
			topk = efficient_topk(candidate_block, t_curr - tau, t_curr, f, k);
			restart = topk_verification(topk, ans, t_curr);
			if (!restart) {
				// skip to the next candidate point
				int next = get_max_timestamp(topk);
				skip_length = t_curr - next;
				t_curr = next;
			}
			else {
				t_curr--;
			}
			//}
		}
	}
	AnsUnit info;
	info.ans_size = ans.size();
	info.oracle_calls = topk_queries_cnt;
	unit.emplace_back(info);
	return ans;
}

// use candidate blocks (at most k) to compute topk
template <size_t N>
vector<tuple_t> Solution<N>::efficient_topk(vector<tuple_t> candidate_block, int ts, int te, vector<float>& f, int k) {
	vector<tuple_t> ans;
	vector<int> timestamp;

	// break tie using timestamp
	for (auto& v : candidate_block)
		timestamp.push_back(get<0>(v));
	sort(timestamp.begin(), timestamp.end(), greater<int>());

	for (int i=0; i<timestamp.size(); ++i) {
		for (auto& p: db_blocks[timestamp[i]]) {
			
			if (p.arrival_time < ts || p.arrival_time > te)
				continue;
			if (ans.size() < k) {
				ans.emplace_back(make_pair(p.arrival_time, score(p, f)));
				if (ans.size() == k) 
					make_heap(ans.begin(), ans.end(), Compare());
			}
			else {
				float v = score(p, f);
				if (v > get<1>(ans[0]) || (v == get<1>(ans[0]) && p.arrival_time > get<0>(ans[0]))) {
					pop_heap(ans.begin(), ans.end(), Compare());
					ans.pop_back();
					ans.emplace_back(make_pair(p.arrival_time, v));
					push_heap(ans.begin(), ans.end(), Compare());
				}
			}
		}
	}
	return ans;
}

template <size_t N>
float Solution<N>::get_block_max(int id, vector<float>& f) {
	float max = 0;
	for (auto& point : db_block_skyline[id]) {
		if (score(point, f) > max) {
			max = score(point, f);
		}
	}
	return max;
}

template <size_t N>
vector<tuple_t> Solution<N>::incremental_topk_block(int start, int end, vector<float>& f, int k) {
	// pair(block_id, block_max)
	vector<tuple_t> ans;
	for (int i=end; i>=start; --i) {
		if (ans.size() < k) {
			float v_max =  get_block_max(i, f);
			ans.emplace_back(make_pair(i, v_max));
			if (ans.size() == k) 
				make_heap(ans.begin(), ans.end(), Compare());
		}
		else {
			float v_max = get_block_max(i, f);
			if (v_max > get<1>(ans[0])) {
				pop_heap(ans.begin(), ans.end(), Compare());
				ans.pop_back();
				ans.emplace_back(make_pair(i, v_max));
				push_heap(ans.begin(), ans.end(), Compare());
			}
		}
	}
	return ans;
}

template <size_t N>
vector<tuple_t> Solution<N>::incremental_topk(vector<Point>& db, int ts,int te, vector<float>& f, int k) {
	// clock_t t_start,t_end;
	// t_start = clock();
	vector<tuple_t> ans;
	for (int i=te; i>=ts; --i) {
		if (ans.size() < k) {
			ans.emplace_back(make_pair(i, score(db[i], f)));
			if (ans.size() == k)
				make_heap(ans.begin(), ans.end(), Compare());
		}
		else {
			if (score(db[i], f) > get<1>(ans[0])) {
				pop_heap(ans.begin(), ans.end(), Compare());
				ans.pop_back();
				ans.emplace_back(make_pair(i, score(db[i], f)));
				push_heap(ans.begin(), ans.end(), Compare());
			}
		}
	}
	// t_end = clock();
	// cout << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;
	return ans;
}

template <size_t N>
vector<tuple_t> Solution<N>::incremental_topk_with_condition(vector<Point>& db, unordered_map<int, bool>& reported,
														  int ts,int te, vector<float>& f, int k) {
	// clock_t t_start,t_end;
	// t_start = clock();
	vector<tuple_t> ans;
	for (int i=te; i>=ts; --i) {
		if (reported[i])
			continue;
		if (ans.size() < k) {
			ans.emplace_back(make_pair(i, score(db[i], f)));
			if (ans.size() == k)
				make_heap(ans.begin(), ans.end(), Compare());
		}
		else {
			if (score(db[i], f) > get<1>(ans[0])) {
				pop_heap(ans.begin(), ans.end(), Compare());
				ans.pop_back();
				ans.emplace_back(make_pair(i, score(db[i], f)));
				push_heap(ans.begin(), ans.end(), Compare());
			}
		}
	}
	// t_end = clock();
	// cout << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;
	return ans;
}

template <size_t N>
vector<tuple_t> Solution<N>::baseline_topk(vector<Point>& db, int ts,int te, vector<float>& f, int k) {
	// clock_t t_start,t_end;
	// t_start = clock();
	vector<tuple_t> ans;
	for (int i=te; i>=ts; --i)
		ans.emplace_back(make_pair(i, score(db[i], f)));

	sort(ans.begin(), ans.end(), Compare());
	// t_end = clock();
	// cout << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;
	return ans;
}

template <size_t N>
void Solution<N>::rtree_index(vector<Point>& db) {
	// cout << "building rtree..." << endl;
	// clock_t t_start,t_end;
	// t_start = clock();
	// int cnt = 0;
 //    for (auto& item : db) {
 //    	if (cnt % 1000 == 0)
 //    		cout << '\r' << cnt << flush;
 //    	// the last dimension is arriving time
 //    	d_point p(item.data[0], item.data[1], item.data[2]);
 //    	rtree.insert(p);
 //    	cnt++;
 //    }
 //    t_end = clock();
 //    cout << endl;
	// cout << "r tree construction done in " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;
}

template <size_t N>
vector<int> Solution<N>::baseline_halfspace_report(vector<Point>& db, int ts,int te,vector<float>& f, float threshold) {
	vector<int> ans;
	for (int i=te;i>=ts;--i) {
		if (score(db[i], f) > threshold)
			ans.push_back(db[i].arrival_time);
	}
	return ans;
}

// void Solution<N>::result_postprocess(set<int>& ans, vector<d_point>& result, vector<float>& f, float v_threshold, int dim) {
// 	float v;
// 	for (auto& item : result) {
// 		v = 0;
// 		if (dim == 3) {
// 			v += bg::get<0>(item) * f[0];
// 			v += bg::get<1>(item) * f[1];
// 			if (v > v_threshold)
// 				ans.insert(bg::get<2>(item));
// 		} 
// 		else if (dim == 4) {
// 			// for later use
// 			assert (dim <= 3);
// 		}
// 		else {
// 			// for later use
// 			assert (dim <= 4);
// 		}
// 	}
// }

template <size_t N>
vector<tuple_t> Solution<N>::block_tree_topk_with_condition(vector<Point>& db, unordered_map<int, bool>& reported, unordered_map<BlockNode*, tuple_t>& max_cache, int ts,int te, vector<float>& f, int k) {
	
	multiset<node_t, NodeCompare> candidates;
	vector<node_t> topk_nodes;
	vector<tuple_t> topk_v;
	adaptive_blocking(root, max_cache, ts, te, candidates, f);
	//std::cout <<"candidate size: " << candidates.size() << std::endl;

	while(topk_nodes.size() < k && !candidates.empty()) {
		BlockNode* top = get<1>(*candidates.begin());
		//std::cout << top->te - top->ts << std::endl;
		if (top->te - top->ts > node_size_threshold) {
			// pop up the top element, and replace it with its children.
			candidates.erase(candidates.begin());
			BlockNode* lchild = top->left;
			BlockNode* rchild = top->right;
			if (lchild != nullptr) {
				if (max_cache.find(lchild) != max_cache.end()) {
					candidates.insert(make_pair(max_cache[lchild], lchild));
				}
				else {
					tuple_t max_item = get_node_max(lchild, f);
					candidates.insert(make_pair(max_item, lchild));
					max_cache[lchild] = max_item;
				}
			}
			if (rchild != nullptr) {
				if (max_cache.find(rchild) != max_cache.end()) {
					candidates.insert(make_pair(max_cache[rchild], rchild));
				}
				else {
					tuple_t max_item = get_node_max(rchild, f);
					candidates.insert(make_pair(max_item, rchild));
					max_cache[rchild] = max_item;
				}
			}
		}
		else {
			topk_nodes.push_back(*candidates.begin());
			// pop up the top element
			candidates.erase(candidates.begin());
		}
	}

	// topk computation using topk_nodes
	// assert(topk_nodes.size() == k);
	//std::cout << topk_nodes.size() << std::endl;
	for (auto& node : topk_nodes) {
		for (int t=node.second->te; t>= node.second->ts; --t) {
			if (reported[t]) continue;
			if (topk_v.size() < k) {
				topk_v.emplace_back(make_pair(t, score(db[t], f)));
				if (topk_v.size() == k) make_heap(topk_v.begin(), topk_v.end(), Compare());
			}
			else {
				float v = score(db[t], f);
				if (v > get<1>(topk_v[0]) || (v == get<1>(topk_v[0]) && t > get<0>(topk_v[0]))) {
					pop_heap(topk_v.begin(), topk_v.end(), Compare());
					topk_v.pop_back();
					topk_v.emplace_back(make_pair(t, v));
					make_heap(topk_v.begin(), topk_v.end(), Compare());
				}
			}
		}
	}
	return topk_v;
}

// first recursive down
// if there are very few points, then do the loose boxes
// set<int> Solution::rtree_halfspace_report_inside(Point& p, int ts, int te, vector<float>& f, int k, double& extra_time) {
// 	set<int> ans;
	
// 	float v_threshold = score(p, f);

	
// 	box good_box1(d_point(v_threshold / f[0], 0, ts), d_point(INF, v_threshold / f[1], te));
// 	box good_box2(d_point(0, v_threshold / f[1], ts), d_point(INF, INF, te));
	
// 	vector<d_point> result;

// 	clock_t t1, t2;
	
	
// 	result.clear();
// 	rtree.query(bgi::intersects(good_box1), std::back_inserter(result));
// 	result_postprocess(ans, result,f,v_threshold, p.data.size());
// 	// if (ans.size() > k) 
// 	// 	return ans;

// 	result.clear();
// 	rtree.query(bgi::intersects(good_box2), std::back_inserter(result));
// 	result_postprocess(ans, result,f,v_threshold, p.data.size());
// 	// if (ans.size() > k)	
// 	// 	return ans;

// 	//recursively forming small query box inside the search polygon
	
// 	float x1=0, y1=v_threshold / f[1], x2=v_threshold / f[0], y2=0;
// 	float x,y;
// 	int iterations = 0;
// 	// FIFO, doing BFS on sub range boxes.
// 	queue<float> query_points;
// 	query_points.push(x1);
// 	query_points.push(y1);
// 	query_points.push(x2);
// 	query_points.push(y2);
// 	int ans_delta = 0;
// 	// while (iterations < 1024 && result.size() < 3*k) {
// 	while (iterations < 256) {
// 		x1 = query_points.front(); query_points.pop();
// 		y1 = query_points.front(); query_points.pop();
// 		x2 = query_points.front(); query_points.pop();
// 		y2 = query_points.front(); query_points.pop();
// 		x = (x1 + x2) / 2;
// 		y = (y1 + y2) / 2;
// 		box b(d_point(x,y,ts), d_point(x2,y1,te));
// 		result.clear();
// 		rtree.query(bgi::intersects(b), std::back_inserter(result));
// 		ans_delta = ans.size();
// 		result_postprocess(ans, result,f,v_threshold, p.data.size());
// 		ans_delta = ans.size() - ans_delta;
// 		// if (ans.size() > k)	
// 		// 	return ans;

// 		// first push search region 1
// 		query_points.push(x1); query_points.push(y1);
// 		query_points.push(x); query_points.push(y);
// 		// then push search region 2
// 		query_points.push(x); query_points.push(y);
// 		query_points.push(x2); query_points.push(y2);

// 		iterations++;

// 		// if (iterations > 64 && ans_delta < 0.2 * k)
// 		// 	break;
// 	}
// 	// if (ans.size() > k)	
// 	// 	return ans;
// 	// continue to do loose box range search on the remaining area
// 	// need further prune out unqualified regions
// 	clock_t t_start, t_end;
// 	t_start = clock();
// 	while (!query_points.empty()) {
// 		x1 = query_points.front(); query_points.pop();
// 		y1 = query_points.front(); query_points.pop();
// 		x2 = query_points.front(); query_points.pop();
// 		y2 = query_points.front(); query_points.pop();
// 		box b(d_point(x1, y2, ts), d_point(x2, y1, te));
// 		// construct several unqualified regions
// 		// box non_b(d_point(x1, y2, ts), d_point((x1+x2)/2, (y1+y2)/2, te));
// 		result.clear();
// 		rtree.query(bgi::intersects(b), std::back_inserter(result));
// 		result_postprocess(ans, result, f, v_threshold, p.data.size());
// 		// if (ans.size() > k)	{
// 		// 	t_end = clock();
// 		// 	extra_time = (double) (t_end - t_start) / CLOCKS_PER_SEC;
// 		// 	return ans;
// 		// }
// 	}
// 	t_end = clock();
// 	extra_time = (double) (t_end - t_start) / CLOCKS_PER_SEC;
// 	return ans;
// }

// set<int> Solution::rtree_halfspace_report_outside(Point& p, int ts, int te, vector<float>& f, int k) {
// 	set<int> ans;
	
// 	float v_threshold = score(p, f);

// 	vector<float> xs, ys;
// 	int x_pos, y_pos;
// 	float interval = (float) v_threshold / f[0] / integration_pieces;
// 	for (int i=0; i<integration_pieces; ++i) {
// 		x_pos = i * interval;
// 		xs.push_back(x_pos);
// 		y_pos = (v_threshold - x_pos * f[0]) / f[1];
// 		ys.push_back(y_pos);
// 	}
// 	xs.push_back(v_threshold / f[0]);
// 	ys.push_back(0);

	
// 	box good_box1(d_point(v_threshold / f[0], 0, ts), d_point(INF, v_threshold / f[1], te));
// 	box good_box2(d_point(0, v_threshold / f[1], ts), d_point(INF, INF, te));
	
// 	vector<d_point> result;

// 	clock_t t1, t2;
	
	
// 	result.clear();
// 	rtree.query(bgi::intersects(good_box1), std::back_inserter(result));
// 	result_postprocess(ans, result,f,v_threshold, p.data.size());
// 	if (ans.size() > k) 
// 		return ans;

// 	result.clear();
// 	rtree.query(bgi::intersects(good_box2), std::back_inserter(result));
// 	result_postprocess(ans, result,f,v_threshold, p.data.size());
// 	if (ans.size() > k)
// 		return ans;

// 	// integrations with loose bounding box
// 	for (int i=0; i<integration_pieces; ++i) {
// 		box b(d_point(xs[i], ys[i+1], ts), d_point(xs[i+1], v_threshold/f[1], te));
// 		result.clear();
// 		rtree.query(bgi::intersects(b), std::back_inserter(result));
// 		result_postprocess(ans, result,f,v_threshold, p.data.size());
// 		if (ans.size() > k)
// 			return ans;
// 	}
	
// 	return ans;
// }

template <size_t N>
bool Solution<N>::answer_check(vector<tuple_t>& v1, vector<tuple_t>& v2) {
	if (v1.size() != v2.size()) {
		return false;
	}

	for (int i=0; i<v1.size(); ++i) {
		if (get<0>(v1[i]) != get<0>(v2[i])) {
			return false;
		}
	}
	return true;
}

template <size_t N>
vector<Point> Solution<N>::merge_skyline(BlockNode* l,BlockNode* r) {

	if (l == nullptr)
		return r->skyline;
	if (r == nullptr)
		return l->skyline;

	vector<Point> ans;
	for(auto& p : l->skyline) {
		bool dominated = false;
		for (auto& q : r->skyline) {
			if (dominate(q, p, p.data.size()-1)) {
				dominated = true;
				break;
			}
		}
		if (!dominated)
			ans.emplace_back(p);
	}
	for(auto& p : r->skyline) {
		bool dominated = false;
		for (auto& q : l->skyline) {
			if (dominate(q, p, p.data.size()-1)) {
				dominated = true;
				break;
			}
		}
		if (!dominated)
			ans.emplace_back(p);
	}
	return ans;
}

template <size_t N>
BlockNode* Solution<N>::toBLKTree(vector<Point>& db, int start, int end) {
	if (start > end)
		return nullptr;

	if (start == end) {
		BlockNode* node = new BlockNode(nullptr, nullptr, start, end);
		node->skyline.emplace_back(db[start]);
		return node;
	}

	// avoid overflow
	int mid = start + (end - start) / 2;
	BlockNode* lchild = toBLKTree(db, start, mid);
	BlockNode* rchild = toBLKTree(db, mid+1, end);

	BlockNode* parent = new BlockNode(lchild, rchild, start, end);
	// parent->left = lchild;
	// parent->right = rchild;
	// get skyline from its childs
	parent->skyline = merge_skyline(lchild, rchild);
	
	return parent;

}

template <size_t N>
void Solution<N>::blocktree_index(vector<Point>& db) {
	cout << "Block Tree construction..." << endl;
	clock_t t_start = clock();
	root = toBLKTree(db, 0, db.size()-1);
	clock_t t_end = clock();
	cout << root->ts << ' ' << root->te << ' ' << root->skyline.size() << endl;
	cout << "construction done " << ' ' << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;
	return;
}

template <size_t N>
bool Solution<N>::isLeaf(BlockNode* node) {
	return node->left == nullptr && node->right == nullptr;
}

template <size_t N>
void Solution<N>::deleteTree(BlockNode* node) {
	if (node == nullptr)
		return;

	deleteTree(node->left);
	deleteTree(node->right);

	delete node;
	return;
}

template <size_t N>
tuple_t Solution<N>::get_node_max(BlockNode* node, vector<float>& f) {
	float max = -1;
	int max_t = -1;
	
	for (auto& p : node->skyline) {
		float v = score(p, f);
		if (v > max) {
			max = v;
			max_t = p.arrival_time;
		}
	}
	return make_pair(max_t, max);
}

template <size_t N>
int Solution<N>::get_max_timestamp(multiset<node_t, NodeCompare>& v, int t) {
	int max = 0;
	for (auto& item : v) {
		if (get<1>(item)->te < t && get<1>(item)->te > max)
			max = get<1>(item)->te;
	}
	return max;
}

// check the relationship between two intervals n(n1, n2) and m(m1, m2)
// return code: 
// 0 - n doesn't overlap with m
// 1 - n intersects m
// 2 - n contains m
template <size_t N>
int Solution<N>::relationship(int n1, int n2, int m1, int m2) {
	if ((m1 > n2) || (n1 > m2))
		return 0;
	else if ((n1 <= m1) && (n2 >= m2))
		return 2;
	else
		return 1;
}


// locate canonical nodes in tree for a query window (ts, te)
template <size_t N>
void Solution<N>::adaptive_blocking(BlockNode* node, unordered_map<BlockNode*, tuple_t>& cache,
								int ts, int te, multiset<node_t, NodeCompare>& v, vector<float>& f) {
	if (node == nullptr)
		return;

	int n_ts = node->ts, n_te = node->te;
	int relation = relationship(ts, te, n_ts, n_te);
	// cout << n_ts << ' ' << n_te << ' ' << relation << endl;
	if (relation == 0)
		return;
	else if (relation == 1) {
		adaptive_blocking(node->left, cache, ts, te, v, f);
		adaptive_blocking(node->right, cache, ts, te, v, f);
		return;
	}
	else {
		// assert (relation == 2);
		if (cache.find(node) != cache.end()) {
			//cout << "cache hit" << endl;
			v.insert(make_pair(cache[node], node));
			//cout << v.size() << endl;
		}
		else {
			tuple_t max_item = get_node_max(node, f);
			v.insert(make_pair(max_item, node));
			cache[node] = max_item;
		}
		//v.emplace_back(make_pair(get_node_max(node, f), node));
		return;
	}
}

template <size_t N>
bool Solution<N>::node_prunning(multiset<node_t, NodeCompare>& candidates, int t, int k) {
	//assert(candidates.size() > k);
	bool found = false;
	int cnt = 0;
	auto it = candidates.begin();
	while (cnt < k && it != candidates.end()) {
		if (get<1>(*it)->ts <= t && t <= get<1>(*it)->te) {
				found = true;
				break;
		}
		++it;
		cnt += 1;
	}
	return found;
}

template <size_t N>
bool Solution<N>::topk_verification(vector<Point>& db, unordered_map<BlockNode*, vector<tuple_t> >& cache,
			vector<node_t>& topk_nodes, vector<tuple_t>& ans, 
			vector<tuple_t>& topk, vector<float>& f, int k, int right, int left) {
	
	int ts,te;
	vector<tuple_t> temp;
	for (auto& n : topk_nodes) {
		BlockNode* node = get<1>(n);
		if (cache.find(node) != cache.end()) {
			//cout << "cache hit" << endl;
			temp.insert(temp.end(), cache[node].begin(), cache[node].end());
		}
		else {
			vector<tuple_t> temp_topk;
			ts = node->ts;
			te = node->te;
			for(int i=te; i>=ts; --i) {
				if (temp_topk.size() < k) {
					temp_topk.emplace_back(make_pair(i, score(db[i], f)));
					if (temp_topk.size() == k)
						make_heap(temp_topk.begin(), temp_topk.end(), Compare());
				}
				else {
					float v = score(db[i], f);
					if (v > get<1>(temp_topk[0])) {
						pop_heap(temp_topk.begin(), temp_topk.end(), Compare());
						temp_topk.pop_back();
						temp_topk.emplace_back(make_pair(i,v));
						make_heap(temp_topk.begin(), temp_topk.end(), Compare());
					}
				}
			}
			temp.insert(temp.end(), temp_topk.begin(), temp_topk.end());
			cache[node] = temp_topk;
		}
	}
	sort(temp.begin(), temp.end(), Compare());
	for (int i=0; i<k; ++i) {
		topk.emplace_back(temp[i]);
		if (get<0>(temp[i]) == right){
			ans.emplace_back(temp[i]);
			return true;
		}
	}
	return false;
}

template <size_t N>
vector<tuple_t> Solution<N>::sequential_tree_plus(vector<Point>& db, int ts, int te, int tau, vector<float>& f, int k, vector<AnsUnit>& unit) {
	vector<tuple_t> ans;
	int topk_queries_cnt = 0;
	// cache computed nodes with its max value
	unordered_map<BlockNode*, tuple_t> max_cache;
	unordered_map<BlockNode*, vector<tuple_t> > topk_cache;
	int t_curr = te;
	// step 1: locate candidate nodes in tree
	//vector<node_t> candidates;
	multiset<node_t, NodeCompare> candidates;
	vector<node_t> topk_nodes;
	vector<tuple_t> topk_v;
	adaptive_blocking(root, max_cache, t_curr - tau, t_curr, candidates, f);
	// cout << candidates.size() << endl;
	// for (auto& item :  candidates) 
	// 	cout << get<0>(get<0>(item)) << ' ' << get<1>(get<0>(item)) << ' ' << get<1>(item)->ts << ' ' << get<1>(item)->te << endl;
	//make_heap(candidates.begin(), candidates.end(), NodeCompare());
	
	bool flag = false;

	while (t_curr >= ts) {
		
		// can be further optimized
		bool found = flag || node_prunning(candidates, t_curr, k - topk_nodes.size());
		
		if (!found) {

 			// move to the next, insert new adaptive data blocks into candidate set
 			for (auto& item : topk_nodes)
 				candidates.insert(item);
			int next = get_max_timestamp(candidates, t_curr);
			//int skip_length = t_curr - next;
			t_curr = next;
			candidates.clear();
			topk_nodes.clear();
			topk_v.clear();
			
			adaptive_blocking(root, max_cache, t_curr - tau, t_curr, candidates, f);
			//make_heap(candidates.begin(), candidates.end(), NodeCompare());
		}
		else {
			
			BlockNode* top = get<1>(*candidates.begin());
			//pop_heap(candidates.begin(), candidates.end(), NodeCompare());

			// remove expired nodes
			if (t_curr < top->ts) {
				//candidates.pop_back();
				candidates.erase(candidates.begin());
			}
			else {
				// if the node is too large, split into half
				if (top->te - top->ts > node_size_threshold) {
					//candidates.pop_back();
					candidates.erase(candidates.begin());
					BlockNode* lchild = top->left;
					BlockNode* rchild = top->right;
					if (lchild != nullptr) {
						if (max_cache.find(lchild) != max_cache.end()) {
							//cout << "cache hit" << endl;
							//candidates.push_back(make_pair(max_cache[lchild], lchild));
							candidates.insert(make_pair(max_cache[lchild], lchild));
						}
						else {
							tuple_t max_item = get_node_max(lchild, f);
							//candidates.push_back(make_pair(max_item, lchild));
							candidates.insert(make_pair(max_item, lchild));
							max_cache[lchild] = max_item;
						}
						//candidates.push_back(make_pair(get_node_max(lchild, f), lchild));
						//push_heap(candidates.begin(), candidates.end(), NodeCompare());
					}
					if (rchild != nullptr) {
						if (max_cache.find(rchild) != max_cache.end()) {
							//cout << "cache hit" << endl;
							//candidates.push_back(make_pair(max_cache[rchild], rchild));
							candidates.insert(make_pair(max_cache[rchild], rchild));
						}
						else {
							tuple_t max_item = get_node_max(rchild, f);
							//candidates.push_back(make_pair(max_item, rchild));
							candidates.insert(make_pair(max_item, rchild));
							max_cache[rchild] = max_item;
						}
						//candidates.push_back(make_pair(get_node_max(rchild, f), rchild));
						//push_heap(candidates.begin(), candidates.end(), NodeCompare());
					}
				}
				else { // push into candidate blocks for computation
					if (top->ts <= t_curr && t_curr <= top->te)
						flag = true;
					topk_nodes.push_back(*candidates.begin());
					//candidates.pop_back();
					candidates.erase(candidates.begin());
					
					if (topk_nodes.size() >= k || candidates.size() == 0) {
						//cout << candidates.size() << endl;
						topk_queries_cnt += 1;
						if (topk_verification(db, topk_cache, topk_nodes, ans, topk_v, f, k, t_curr, t_curr-tau)) {
							t_curr -= 1;
						}
						else {
							// move to next candidate
							int next = get_max_timestamp(topk_v);
							//cout << "next: " << next << endl;
							int skip_length = t_curr - next;
							t_curr = next;
						}
						
						flag = false;
						candidates.clear();
						adaptive_blocking(root, max_cache, t_curr - tau, t_curr, candidates, f);
						//make_heap(candidates.begin(), candidates.end(), NodeCompare());
						topk_nodes.clear();
						topk_v.clear();
					}
				}
			}
		}
	}
	AnsUnit info;
	info.ans_size = ans.size();
	info.oracle_calls = topk_queries_cnt;
	unit.emplace_back(info);
	return ans;
}

template <size_t N>
void Solution<N>::kskyband_duration(vector<Point>& db, vector<int>& ks) {
	cout << "compute kskyband duration..." << endl;
	clock_t t_start, t_end;
	//intialization
	for (int i=0; i<db.size(); ++i)
		duration.emplace_back(vector<int>(ks.size(), 0));

	// computation
	t_start = clock();
	// for each value of k, compute kskyband duration for each point
	for (int e=db.size()-1; e>=0; --e) {
		int dominant_cnt = 0;
		int idx = 0;
		for (int s=e; s>=0; --s) {
			if (dominate(db[s], db[e], db[e].data.size() - 1)) {
				
				if (idx >= ks.size())
					break;
				// for every possible value of k, record the time duration (e - s)
				if (dominant_cnt == ks[idx]) {
					duration[e][idx] = e - s + 1;
					idx++;
				}
				dominant_cnt++;
			}
		}
		for(int i=idx;i<ks.size();++i)
			duration[e][i] = e + 1;
	}
	t_end = clock();
	cout << "in " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;
}

template <size_t N>
void Solution<N>::kskyband_duration_rtree(vector<Point>& db, vector<int>& ks) {
	cout << "compute kskyband duration indexed with RTree..." << endl;
	clock_t t_start, t_end;

	// computation
	t_start = clock();
	// for each value of k, compute kskyband duration for each point
	for (int e=db.size()-1; e>=0; --e) {
		if (e % 1000 == 0)
    		cout << '\r' << e << flush;
		int dominant_cnt = 0;
		int idx = 0;
		for (int s=e; s>=0; --s) {
			if (dominate(db[s], db[e], db[e].data.size() - 1)) {
				if (idx >= ks.size())
					break;
				// for every possible value of k, record the time duration (e - s)
				if (dominant_cnt == ks[idx]) {
					if (e-s+1 >= db.size() * 0.01) {
					//if (e-s+1 >= 20) { // for debug
						dur_point p(e, dominant_cnt, e-s+1);
						dur_rtree.insert(p);
					}
					idx++;
				}
				dominant_cnt++;
			}
		}
		for(int i=idx;i<ks.size();++i) {
			//duration[e][i] = e + 1;
			if (e+1 >= db.size() * 0.01) {
			//if (e+1 >= 20) { //for debug
				dur_point p(e, ks[i], e+1);
				dur_rtree.insert(p);
			}
		}
	}
	t_end = clock();
	cout << endl;
	cout << "in " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;
}

template <size_t N>
vector<int> Solution<N>::get_durable_candidates(int ts, int te, int idx_k, int tau) {
	vector<int> ans;
	for(int i=ts; i<=te; ++i) {
		if (duration[i][idx_k] >= tau)
			ans.push_back(i);
		//cout << i << endl;
	}
	return ans;
}

template <size_t N>
vector<int> Solution<N>::rtree_get_durable_candidates(int ts, int te, int k, int tau) {
	vector<dur_point> result;

	dur_box b(dur_point(ts, k, tau), dur_point(te, k, INF));
	
	dur_rtree.query(bgi::intersects(b), std::back_inserter(result));

	vector<int> ans;
	for (auto& item : result) {
		ans.push_back(bg::get<0>(item));
	}
	
	return ans;
}

template <size_t N>
void Solution<N>::insort(vector<int>& v, int x) {
	//cout << "insert: " << x << endl;
	/*
	if (v.empty())
		v.push_back(x);
	auto it = lower_bound(v.begin(), v.end(), x);
	v.insert(it, x);
	*/
	v.push_back(x);
	sort(v.begin(), v.end());
}

template <size_t N>
int Solution<N>::find_le(vector<int>& v, int x) {
	auto it = upper_bound(v.begin(), v.end(), x);
	int i = it - v.begin();
	if (i)
		return i-1;
	else
		return 0;
}

template <size_t N>
int Solution<N>::find_ge(vector<int>& v, int x) {
	auto it = lower_bound(v.begin(), v.end(), x);
	int i = it - v.begin();
	if (i != v.size()) 
		return i;
	else
		return i-1;
}

template <size_t N>
int Solution<N>::find_le(set<int>& s, int x) {
	//auto it = upper_bound(v.begin(), v.end(), x);
	auto it = s.upper_bound(x);
	//int i = it - v.begin();
	int i = distance(s.begin(), it);
	if (i)
		return i-1;
	else
		return 0;
}

template <size_t N>
int Solution<N>::find_ge(set<int>& s, int x) {
	//auto it = lower_bound(v.begin(), v.end(), x);
	auto it = s.lower_bound(x);
	//int i = it - v.begin();
	int i = distance(s.begin(), it);
	if (i != s.size()) 
		return i;
	else
		return i-1;
}

template <size_t N>
int Solution<N>::count_intersections(vector<int>& l, vector<int>& r, int x) {
	if (l.empty())
		return 0;

	int left_idx = find_ge(r, x);
	int right_idx = find_le(l, x);

	if (x < l[left_idx])
		return 0;
	if (x > r[right_idx])
		return 0;
	if (x > r[left_idx] && x < l[right_idx])
		return 0;

	return right_idx - left_idx + 1;
}

template <size_t N>
int Solution<N>::count_intersections(set<int>& s, unordered_map<int,int>& m, int x) {
	if (s.empty())
		return 0;
	auto it = s.lower_bound(x);
	if (*it == x)
		return m[*it];
	else {
		it--;
		return m[*it];
	}
}

template <size_t N>
int Solution<N>::set_iterator(set<int>& s, int offset) {
	auto it = s.begin();
	int cnt = 0;
	while (cnt < offset) {
		cnt++;
		it++;
	}
	return *it;
}

template <size_t N>
int Solution<N>::count_intersections(set<int>& l, set<int>& r, int x) {
	if (l.empty())
		return 0;

	int left_idx = find_ge(r, x);
	int right_idx = find_le(l, x);

	int ll,rr,rl,lr;
	ll = set_iterator(l,left_idx);
	lr = set_iterator(l, right_idx);
	rr = set_iterator(r, right_idx);
	rl = set_iterator(r, left_idx);

	if (x < ll)
		return 0;
	if (x > rr)
		return 0;
	if (x > rl && x < lr)
		return 0;

	return right_idx - left_idx + 1;
}

template <size_t N>
void Solution<N>::erase_from_multiset(multiset<int>& s, int v) {
	auto it = s.find(v);
	if (it != s.end())
		s.erase(it);
	else {
		//cout << "SET DELETION ERROR!" << endl;
	}
}

template <size_t N>
void Solution<N>::interval_update(set<int>& v, multiset<int>& interval_cnt, unordered_map<int,int>& m, int ts, int te, int k, int t1, int t2) {

	//guaranteed: ts <= t1 <= t2 <= te 
	if (t1 < ts)
		t1 = ts;
	if (t2 > te)
		t2 = te + 1;

	v.insert(t1);
	v.insert(t2);
	vector<set<int>::iterator> temp;	

	int t;

	for(auto i = v.find(t1); i != v.end(); ++i) {
		
		t = *i;
		// right endpoint of an interval
		if (t > te)
			break;
		if (t == t2) {
			if (m.find(t) == m.end()) {
				m[t] = m[*(prev(i))] - 1;
				if (m[t] > k)
					temp.push_back(i);
					//v.erase(i);
				interval_cnt.insert(m[t]);
			}
			break;
		}
		auto it = m.find(t);
		// old entry
		if (it != m.end()) {
			erase_from_multiset(interval_cnt, m[t]);
			m[t] += 1;
			interval_cnt.insert(m[t]);
			if (m[t] > k)
				temp.push_back(i);
				//v.erase(i);
		}
		else { // new entry
			m[t] = m[*(prev(i))] + 1;
			interval_cnt.insert(m[t]);
			if (m[t] > k)
				temp.push_back(i);
				//v.erase(i);
		}
	}

	for (auto& item : temp)
		v.erase(item);
	
	return;
}

template <size_t N>
void Solution<N>::interval_update(set<int>& v, map<int,int>& m, int ts, int te, int k, int t1, int t2) {

	//guaranteed: ts <= t1 <= t2 <= te 
	if (t1 < ts)
		t1 = ts;
	if (t2 > te)
		t2 = te + 1;

	v.insert(t1);
	v.insert(t2);
	//vector<set<int>::iterator> temp;	

	int t;

	for(auto i = v.find(t1); i != v.end(); ++i) {
		
		t = *i;
		// right endpoint of an interval
		if (t > te)
			break;
		if (t == t2) {
			if (m.find(t) == m.end()) {
				m[t] = m[*(prev(i))] - 1;
			}
			break;
		}
		auto it = m.find(t);
		// old entry
		if (it != m.end()) {
			m[t] += 1;
		}
		else { // new entry
			m[t] = m[*(prev(i))] + 1;
		}
	}
	return;
}

template <size_t N>
vector<tuple_t> Solution<N>::weighted_prunning(vector<Point>& db, int ts,int te, int tau, vector<float>& f, int k, int idx_k, vector<AnsUnit>& unit, TimeUnit& tu, double& total_extra_time) {
	
	// further optimization : use coreset to support approximate halfspace reporting
	int iterations = 0;
	int oracle_calls = 0;
	int true_negative_cnt = 0;
	double extra_time = 0;
	// for intersection counting
	set<int> left, right;

	assert(db[0].data.size()-1 == f.size());

	// for block skyline tree
	unordered_map<int, bool> reported;
	unordered_map<BlockNode*, tuple_t> max_cache;
	vector<tuple_t> topk_list;

	// remember overlapping cnt for each start endpoints
	unordered_map<int,int> cnt_map{{ts, 0}};

	// multiset to maintain the minimum intersection count for query window
	multiset<int> region;

	// initialize
	region.insert(0);

	//clock_t t_start, t_end;
	vector<tuple_t> ans;
	//t_start = clock();

	vector<int> candidates = rtree_get_durable_candidates(ts - tau, te, k-1, tau);
	unordered_map<int,bool> visited;
	
	vector<tuple_t> sorted_candidates;
	for (int t : candidates) {
		visited[t] = true;
		sorted_candidates.emplace_back(make_pair(t, score(db[t], f)));
	}
	sort(sorted_candidates.begin(), sorted_candidates.end(), Compare());
	//t_end = clock();

	
	// maintain all disjoint intervals as we insert new intervals
	set<int> endpoints{ts};

	int t_curr, intersections;
	float v_threshold;

	double hsr_time = 0;
	//double update_time = 0;
	//double insort_time = 0;
	clock_t t1,t2;
	//t_start = clock();
	for (int i=0; i<sorted_candidates.size(); ++i) {
	
		// if (i >= k * (te - ts + 1) / tau) {
		// 	// run a k overlapping check
		// 	// if yes, break;
		// 	// if not, continue;
		// 	int min_count = *(region.begin());
		// 	//cout << "min count: " << min_count << endl;
		// 	if (min_count > k) {
		// 		break;
		// 	}
		// }

		iterations++;
		t_curr = get<0>(sorted_candidates[i]);
		v_threshold = get<1>(sorted_candidates[i]);
		//cout << t_curr << endl;
		
		if (t_curr >= ts) {
			
			// check whether the point is already covered by k intervals
			//if (count_intersections(endpoints, cnt_map, t_curr) < k) {
			if (count_intersections(left, right, t_curr) < k) {
				// if no, run a halfspace reporting query on interval (interval_start - tau, interval_start)
				set<int> true_topk;
				oracle_calls++;
				topk_list = block_tree_topk_with_condition(db, reported, max_cache, t_curr - tau, t_curr, f, k);
				for (auto& item : topk_list) true_topk.insert(item.first);
				if (true_topk.find(t_curr) != true_topk.end())
					ans.emplace_back(sorted_candidates[i]);
				else {
					for (auto& t : true_topk) {
						if (score(db[t], f) < v_threshold || t == t_curr) continue;
						//if (find(candidates.begin(), candidates.end(), t) == candidates.end()) {
						if (visited.find(t) == visited.end()) {
							visited[t] = true;
							interval_update(endpoints, region, cnt_map, ts, te, k, t, t+tau+1);
							left.insert(t);
							right.insert(t+tau);
							if (*(region.begin()) >= k)
								break;
						}
					}
				}	
			}
			// if yes, continue
		}
		interval_update(endpoints, region, cnt_map, ts, te, k, t_curr, t_curr+tau+1);
		left.insert(t_curr);
		right.insert(t_curr+tau);
		visited[t_curr] = true;
		if (*(region.begin()) >= k)
			break;
	}
	
	AnsUnit au;
	au.ans_size = ans.size();
	au.lower_bound = k * (int((te - ts + 1) / tau) + 1);
	au.upper_bound = ans.size() + k * (int((te - ts + 1) / tau) + 1);
	au.oracle_calls = oracle_calls;
	au.iterations = iterations;
	au.candidate_size = sorted_candidates.size();
	unit.push_back(au);

	//tu.hsr_time = hsr_time;

	//cout << count_intersections(endpoint_start, endpoint_end, 80026) << endl;

	return ans;
}

template <size_t N>
tuple_t Solution<N>::find_next_max(vector<Point>& db, vector<float>& f, int ts, int te, map<int,bool>& visited/*set<int>& max_points, vector<pair<int, int> >& removed_area*/) {
	float max_v=-1;
	int max_t=-1;
	bool flag = true;
	int t_curr = te;

	while (t_curr >= ts) {
		if (!visited[t_curr]) {
			float v = score(db[t_curr], f);
			if (v > max_v) {
				max_v = v;
				max_t = t_curr;
			}
		}
		t_curr--;
	}
	return make_pair(max_t, max_v);
}

template <size_t N>
vector<tuple_t> Solution<N>::theoretical_weighted_pruning(vector<Point>& db, int ts,int te, int tau, vector<float>& f, int k, vector<AnsUnit>& unit) {
	//cout << "simple version of theorecital solution" << endl;
	int iterations = 0;
	int oracle_calls = 0;
	unordered_map<int, bool> reported;
	vector<tuple_t> ans;

	vector<int> left, right;
	vector<tuple_t> sorted_candidates;
	for (int t=ts-tau;t<=te;++t) {
		sorted_candidates.emplace_back(make_pair(t, score(db[t], f)));
	}
	sort(sorted_candidates.begin(), sorted_candidates.end(), Compare());

	int count = 0;

	for (auto& item : sorted_candidates) {
		int t = get<0>(item);
		float v = get<1>(item);

		if (t < ts)
			continue;

		count = 0;
		for (int i=0; i<left.size(); ++i) {
			if (left[i] <= t && t <= right[i])
				count++;
		}

		if (count >= k)
			continue;
		else {
			vector<int> topk = baseline_halfspace_report(db, t - tau, t - 1, f, v);

			if (topk.size() < k) {
				ans.emplace_back(item);
			}
			else {
				oracle_calls++;
				vector<tuple_t> topk_v;
				for (int& topk_t : topk)
					topk_v.emplace_back(make_pair(topk_t, score(db[topk_t], f)));
				sort(topk_v.begin(), topk_v.end(), Compare());

				for (int i=0; i<k; ++i) {
					int tv = get<0>(topk_v[i]);
					if (reported[tv] == false) {
						reported[tv] = true;
						left.push_back(tv);
						right.push_back(tv+tau);
					}
				}
			}
			reported[t] = true;
			left.push_back(t);
			right.push_back(t+tau);
		}
	}

	AnsUnit au;
	au.ans_size = ans.size();
	au.lower_bound = k * (int((te - ts + 1) / tau) + 1);
	au.upper_bound = ans.size() + k * (int((te - ts + 1) / tau) + 1);
	au.oracle_calls = oracle_calls;
	au.iterations = 0;
	// used as an upper bound for # of HSR
	au.candidate_size = 0;
	unit.push_back(au);


	return ans;

}

template <size_t N>
vector<tuple_t> Solution<N>::efficient_theoretical_weighted_prunning(vector<Point>& db, int ts, int te, int tau, vector<float>& f, int k, vector<AnsUnit>& unit, double& total_extra_time) {
	int iterations = 0;
	int oracle_calls = 0;
	int extra_topk_count = 0;
	int auxiliary_count = 0;
	int sub_interval_count = 0;

	double extra_time = 0;
	assert(db[0].data.size()-1 == f.size());

	vector<tuple_t> ans;
	// for intersection counting
	set<int> left, right;
	unordered_map<int, bool> reported;
	unordered_map<int, bool> empty_reported;
	unordered_map<BlockNode*, tuple_t> max_cache;
	vector<tuple_t> topk_list;
	// remember each sub-interval's topk : key - sub_interval_count, value - priority_queue
	unordered_map<int, priority_queue<tuple_t, vector<tuple_t>, CompareReverse> > local_topk;
	// key - sub_interval_count, value - left/right endpoints of the sub_interval
	unordered_map<int, pair<int,int> > sub_interval_endpoints;
	// for each timestamp, remeber which sub-interval it comes from
	// key - time, value - sub_interval_count
	unordered_map<int, int> sub_interval;

	priority_queue<tuple_t, vector<tuple_t>, CompareReverse> max_heap;

	// prepare initial sub-interval local topks
	vector<tuple_t> local_points;
	//vector<tuple_t> ground_truth;
	for (int i=ts; i<=te; i+=tau) {
		//local_points.clear();
		//ground_truth.clear();
		// topk from a length-tau window
		extra_topk_count++;
		local_points = block_tree_topk_with_condition(db, reported, max_cache, i, MIN(i+tau-1, te), f, k);
		// for (int j=i; j<i+tau && j<=te ; ++j) {
		// 	ground_truth.emplace_back(make_pair(j, score(db[j], f)));
		// }
		// // // get local topk
		// sort(ground_truth.begin(), ground_truth.end(), Compare());
		// std::cout << "===" << std::endl;
		// for (int i=0; i<k && i<ground_truth.size();++i)
		// 	printf("(%d, %f), (%d, %f)\n", ground_truth[i].first, ground_truth[i].second, local_points[i].first, local_points[i].second);
		priority_queue<tuple_t, vector<tuple_t>, CompareReverse> local_max_heap;
		for (int i=0; i<k && i<local_points.size(); ++i) 
			local_max_heap.push(local_points[i]);
		local_topk[sub_interval_count] = local_max_heap;
		sub_interval_endpoints[sub_interval_count] = make_pair(i, MIN(i+tau, te));
		sub_interval_count++;
	}

	// put initial seed into max_heap
	// printf("initial size: %ld\n", local_topk.size());
	for (auto it = local_topk.begin(); it != local_topk.end(); ++it) {
		tuple_t record = (it->second).top();
		sub_interval[record.first] = it->first;
		max_heap.push(record);
		(it->second).pop();
	}

	// main procedure starts
	while (!max_heap.empty()) {
		// get the top record
		tuple_t curr_top = max_heap.top();
		int curr_time = curr_top.first;
		float curr_score = curr_top.second;
		int curr_index = sub_interval[curr_time];
		max_heap.pop();

		if (reported[curr_time])
			continue;

		iterations++;

		// check intersection count
		// if already greater than k, update intervals anyway
		if (count_intersections(left, right, curr_time) >= k) {
			auxiliary_count++;
			// do a pruning check, if the right endpoint of the local interval 
			// is also has intersection count more than k, then no need to 
			// put any other points into the heap.
			if (count_intersections(left, right, sub_interval_endpoints[curr_index].second) >= k)
				continue;
			// add next top record into max_heap from origin local top list
			if (!local_topk[curr_index].empty()) {
				tuple_t record = local_topk[curr_index].top();
				max_heap.push(record);
				local_topk[curr_index].pop();
				sub_interval[record.first] = curr_index;
			}
		}
		// else, it's a candidate
		else {
			// set<int> true_topk;
			// true_topk = rtree_halfspace_report_outside(db[curr_time], curr_time - tau, curr_time - 1, f, k);
			// vector<int> true_topk = baseline_halfspace_report(db, curr_time - tau, curr_time, f, curr_score);
			// increment the number of top-k queries
			oracle_calls++;
			set<int> true_topk;
			// = rtree_halfspace_report_inside(db[curr_time], curr_time - tau, curr_time - 1, f, k, extra_time);
			topk_list = block_tree_topk_with_condition(db, empty_reported, max_cache, curr_time - tau, curr_time, f, k);
			for (auto& item : topk_list)
				true_topk.insert(item.first);
			//total_extra_time += extra_time;
			//if (true_topk.size() < k)
			if (true_topk.find(curr_time) != true_topk.end())
				ans.push_back(curr_top);
			else {
				for (auto& t : true_topk) {
					if (score(db[t], f) < curr_score || t == curr_time) continue;
					if (reported[t]) continue;
					reported[t] = true;
					left.insert(t);
					right.insert(t+tau);
				}
			}
			// split the current interval, and add new candidate local topks into max_heap
			int interval_index = sub_interval[curr_time];
			int sub_ts = sub_interval_endpoints[interval_index].first;
			int sub_te = sub_interval_endpoints[interval_index].second;
			
			// left new interval
			vector<tuple_t> local_topk_left;
			if (curr_time - sub_ts <= k * node_size_threshold)
				local_topk_left = incremental_topk_with_condition(db, reported, sub_ts, curr_time-1, f, k);
			else
				local_topk_left = block_tree_topk_with_condition(db, reported, max_cache, sub_ts, curr_time-1, f, k);
			priority_queue<tuple_t, vector<tuple_t>, CompareReverse> local_max_heap_1;
			for (auto& item : local_topk_left)
				local_max_heap_1.push(item);
			if (!local_max_heap_1.empty()) {
				extra_topk_count++;
				local_topk[sub_interval_count] = local_max_heap_1;
				sub_interval_endpoints[sub_interval_count] = make_pair(sub_ts, curr_time-1);
				// add into new top record into max_heap
				tuple_t new_top_record = local_topk[sub_interval_count].top();
				max_heap.push(new_top_record);
				local_topk[sub_interval_count].pop();
				sub_interval[new_top_record.first] = sub_interval_count;
				sub_interval_count++;
			}
			
			// right new interval
			vector<tuple_t> local_topk_right;
			if (sub_te - curr_time <= k * node_size_threshold)
				local_topk_right = incremental_topk_with_condition(db, reported, curr_time+1, sub_te, f, k);
			else
				local_topk_right = block_tree_topk_with_condition(db, reported, max_cache, curr_time+1, sub_te, f, k);
			priority_queue<tuple_t, vector<tuple_t>, CompareReverse> local_max_heap_2;
			for (auto& item : local_topk_right)
				local_max_heap_2.push(item);
			if (!local_max_heap_2.empty()) {
				extra_topk_count++;
				local_topk[sub_interval_count] = local_max_heap_2;
				sub_interval_endpoints[sub_interval_count] = make_pair(curr_time+1, sub_te);
				// add into new top record into max_heap
				tuple_t new_top_record = local_topk[sub_interval_count].top();
				max_heap.push(new_top_record);
				local_topk[sub_interval_count].pop();
				sub_interval[new_top_record.first] = sub_interval_count;
				sub_interval_count++;
			}
		}
		reported[curr_time] = true;
		left.insert(curr_time);
		right.insert(curr_time+tau);
	}

	AnsUnit au;
	au.ans_size = ans.size();
	// used as number of topk queries for finding the next heavy point.
	au.lower_bound = extra_topk_count;
	au.upper_bound = ans.size() + k * (int((te - ts + 1) / tau) + 1);
	au.oracle_calls = oracle_calls;
	// used as auxiliary points
	au.iterations = auxiliary_count;
	// used as an upper bound for # of auxiliary points
	au.candidate_size = k * au.upper_bound;
	unit.push_back(au);

	return ans;
}
