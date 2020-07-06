#include "data_loader.h"
#include "solution.h"
#include <string>
#include <iostream>
#include <fstream>
#include <numeric>
#include <random>

using namespace std;

#define QUERY_SIZE 100
#define BLOCK_SIZE 128
#define NODE_SIZE_THRESHOLD 64
#define D 2

int min(int a, int b) {
	return a<b ? a:b;
}

int max(int a, int b) {
	return a>b ? a:b;
}

// compute the score of a point when given a perference vector
float score(Point const& p, std::vector<float>& f) {
	//assert(p.data.size()-1 == f.size());
	float sum = 0;
	for (int i=0;i<f.size(); ++i)
		sum += p.data[i] * f[i];
	return sum;
}


// void topk_test(Solution solver, vector<Point>& db, int ts, int te, vector<float>& f, int k) {
// 	vector<tuple_t> res = solver.incremental_topk(db, ts, te, f, k);
// 	vector<tuple_t> truth = solver.baseline_topk(db, ts, te, f, k);

// 	for (auto& item : res) 
// 		cout << get<0>(item) << ' ' << get<1>(item) << endl;
// 	cout << "===" << endl;
// 	for (int i=0;i<k;++i) 
// 		cout << get<0>(truth[i]) << ' ' << get<1>(truth[i]) << endl;
// }

// void sequential_test(Solution solver, vector<Point>& db, int ts, int te, vector<float>& f, int k, int tau) {
// 	cout << "Sequential Baseline..." << endl;
// 	vector<AnsUnit> unit;
// 	vector<tuple_t> ans = solver.sequential(db, ts,te,tau,f,k, unit);
// 	cout << "Answer size: " << ans.size() << endl;
// 	for (auto& item : ans) 
// 		cout << get<0>(item) << ' ' << get<1>(item) << endl;
// }

// void sequential_plus_test(Solution& solver, vector<Point>& db, int block_size, int ts, int te, vector<float>& f, int k, int tau) {
// 	//solver.grouping(db, block_size);
// 	cout << "Sequential Plus..." << endl; 
// 	vector<AnsUnit> unit;
// 	vector<tuple_t> ans = solver.sequential_plus(db, block_size, ts, te, tau, f, k, unit);
// 	cout << "Answer size: " << ans.size() << endl;
// 	for (auto& item : ans) 
// 		cout << get<0>(item) << ' ' << get<1>(item) << endl;
// }

// void sequential_correctness_check(Solution& solver, vector<Point>& db, int block_size, int ts, int te, vector<float>& f, int k, int tau) {
// 	//solver.grouping(db, block_size);
// 	//solver.blocktree_index(db);
// 	vector<AnsUnit> unit;
// 	clock_t t_start,t_end;
// 	t_start = clock();
// 	vector<tuple_t> v1 = solver.sequential(db, ts,te,tau,f,k, unit);
// 	t_end = clock();
// 	cout << "sequential time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

// 	t_start = clock();
// 	vector<tuple_t> v2 = solver.sequential_plus(db, block_size, ts, te, tau, f, k, unit);
// 	t_end = clock();
// 	cout << "sequential plus time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

// 	t_start = clock();
// 	vector<tuple_t> v3 = solver.sequential_tree_plus(db, ts, te, tau, f, k, unit);
// 	t_end = clock();
// 	cout << "adaptive sequential time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

// 	cout << "sequential plus: " << solver.answer_check(v1,v2) << endl;
// 	cout << "adaptive sequential: " << solver.answer_check(v1,v3) << endl;
// }

void epsilon_sequence(int max, float epsilon, vector<int>& v) {
	int start = 0;
	while (start <= max) {
		v.push_back(start);
		start = int(start * (1+epsilon)) + 1;
	}
	if (v.back() < max)
		v.push_back(max);
}

int find_upper_bound(vector<int>& v, int k) {
	for (int i=0; i<v.size(); ++i) {
		if (v[i] >= k-1)
			return i;
	}
}

template<size_t N>
void overall_correctness_check(Solution<N>& solver, vector<Point>& db, int block_size, vector<int>& klist, 
								int ts, int te, vector<float>& f, int k, int tau, 
								vector<AnsUnit>& unit1, vector<AnsUnit>& unit2, vector<TimeUnit>& tu,
								vector<float>& f1, int flag = 1) {
	
	//int idx_k = upper_bound(klist.begin(), klist.end(), k) - klist.begin();
	//int idx_k = find_upper_bound(klist, k);
	//int idx_k = k-1;
	//cout << k << ' ' << klist[idx_k] << endl;

	TimeUnit tunit;

	clock_t t_start,t_end;
	t_start = clock();
	vector<tuple_t> v0 = solver.sequential_basic(db, ts,te,tau,f,k);
	t_end = clock();
	tunit.sw_time = (double) (t_end - t_start) / CLOCKS_PER_SEC;

	t_start = clock();
	vector<tuple_t> v1 = solver.sequential(db, ts,te,tau,f,k, unit1);
	t_end = clock();
	tunit.baseline_time = (double) (t_end - t_start) / CLOCKS_PER_SEC;
	//cout << "sequential time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

	if (solver.answer_check(v0,v1) == false) {
		cout << "sequential error" << endl;
		cout << ts << ' ' << te << ' ' << k << ' ' << tau << endl;
	 	cout << '(' << f[0] << ',' << f[1] << ')' << endl;
	 	cout << v0.size() << '/' << v1.size() << endl;
	 	// for (int i=0;i<v1.size();++i) {
	 	// 	if (i >= v0.size())
	 	// 		cout << "\t" << '(' << get<0>(v1[i]) << ',' << get<1>(v1[i]) << ')' << endl;
	 	// 	else
	 	// 		cout << '(' << get<0>(v0[i]) << ',' << get<1>(v0[i]) << ')' << ' ' << '(' << get<0>(v1[i]) << ',' << get<1>(v1[i]) << ')' << endl;
	 	// }
	}

	double total_extra_time = 0;	
	t_start = clock();
	vector<tuple_t> v2 = solver.weighted_prunning_v2(db, ts, te, tau, f, k, k-1, unit1, tunit, total_extra_time);
	t_end = clock();
	tunit.weighted_time = (double) (t_end - t_start) / CLOCKS_PER_SEC;
	//cout << "weighted prunning time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;
	//cout << v2.size() << endl;
	sort(v2.begin(), v2.end(), TimeCompare());
	if (solver.answer_check(v1,v2) == false) {
		cout << "weighted pruning error" << endl;
	 	cout << v1.size() << '/' << v2.size() << ' '<< solver.f1_score(v1,v2) << endl;
	 	f1[f1.size()-1]=solver.f1_score(v1,v2);
	}

	if (flag) {
		// pure sorting and blocking mechanism
		t_start = clock();
		vector<tuple_t> v4 = solver.weighted_prunning(db, ts, te, tau, f, k, k-1, unit1, tunit, total_extra_time);
		t_end = clock();
		tunit.sf_time = (double) (t_end - t_start) / CLOCKS_PER_SEC;
	}

	total_extra_time = 0;
	t_start = clock();
	vector<tuple_t> v5 = solver.efficient_theoretical_weighted_prunning(db, ts, te, tau, f, k, unit1, total_extra_time);
	t_end = clock();
	tunit.theory_time = (double) (t_end - t_start) / CLOCKS_PER_SEC;
	sort(v5.begin(), v5.end(), TimeCompare());
	if (solver.answer_check(v1,v5) == false) {
		cout << "theoretical solution error" << endl;
		cout << ts << ' ' << te << ' ' << k << ' ' << tau << endl;
	 	cout << '(' << f[0] << ',' << f[1] << ')' << endl;
	 	cout << v1.size() << '/' << v5.size() << endl;
	 	// for (int i=0;i<v5.size();++i) {
	 	// 	if (i >= v1.size())
	 	// 		cout << "    " << '(' << get<0>(v5[i]) << ',' << get<1>(v5[i]) << ')' << endl;
	 	// 	else
	 	// 		cout << '(' << get<0>(v1[i]) << ',' << get<1>(v1[i]) << ')' << ' ' << '(' << get<0>(v5[i]) << ',' << get<1>(v5[i]) << ')' << endl;
	 	// }
	}
	total_extra_time = 0;
	t_start = clock();
	vector<tuple_t> v3 = solver.efficient_theoretical_weighted_prunning_v2(db, ts, te, tau, f, k, unit1, total_extra_time);
	t_end = clock();
	tunit.better_theory_time = (double) (t_end - t_start) / CLOCKS_PER_SEC;
	sort(v3.begin(), v3.end(), TimeCompare());
	if (solver.answer_check(v0,v3) == false) {
		cout << "theoretical solution error #2" << endl;
		cout << ts << ' ' << te << ' ' << k << ' ' << tau << endl;
	 	cout << v0.size() << '/' << v3.size() << endl;
	}
	/*
	t_start = clock();
	vector<tuple_t> v3 = solver.sequential_plus(db, block_size, ts, te, tau, f, k, unit1);
	t_end = clock();
	tunit.sequential_block_time = (double) (t_end - t_start) / CLOCKS_PER_SEC;
	//cout << "sequential block time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

	t_start = clock();
	vector<tuple_t> v4 = solver.sequential_tree_plus(db, ts, te, tau, f, k, unit1);
	t_end = clock();
	tunit.sequential_tree_time = (double) (t_end - t_start) / CLOCKS_PER_SEC;
	//cout << "sequential tree time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;
	*/
	tunit.ans_size = v1.size();
	tunit.k = k;
	tunit.L = te - ts + 1;
	tunit.tau = tau;
	

	tu.emplace_back(tunit);
	f1.push_back(1.0);
}

// void sequential_tree_plus_test(Solution& solver, vector<Point>& db, int ts, int te, vector<float>& f, int k, int tau) {
	
// 	vector<tuple_t> ans;
// 	vector<AnsUnit> unit;
// 	ans = solver.sequential_tree_plus(db, ts, te, tau, f, k, unit);
// 	cout << "adaptive sequential..." << endl;
// 	cout << "Answer size: " << ans.size() << endl;
// 	for (auto& item : ans) 
// 		cout << get<0>(item) << ' ' << get<1>(item) << endl;
// }

// void weighted_prunning_test(Solution& solver, vector<Point>& db, int ts, int te, vector<float>& f, int k, int tau) {
// 	vector<int> klist{1,2,3,4,5};
// 	solver.kskyband_duration(db, klist);
// 	// int idx_k = upper_bound(klist.begin(), klist.end(), k) - klist.begin() - 1;
// 	int idx_k = find_upper_bound(klist, k);
// 	cout << k << ' ' << idx_k << endl;
// 	vector<AnsUnit> unit;
// 	TimeUnit tu;
// 	double total_extra_time = 0;

// 	vector<tuple_t> ans;
// 	ans = solver.weighted_prunning(db, ts, te, tau, f, k, idx_k, unit, tu, total_extra_time);
// 	cout << "weighted prunning..." << endl;
// 	cout << "Answer size: " << ans.size() << endl;
// 	sort(ans.begin(), ans.end(), TimeCompare());
// 	for (auto& item : ans) 
// 		cout << get<0>(item) << ' ' << get<1>(item) << endl;
// }

// void theoretical_solution_test(Solution& solver, vector<Point>& db, int ts, int te, vector<float>& f, int k, int tau) {
// 	vector<AnsUnit> unit;

// 	vector<tuple_t> v1 = solver.sequential(db, ts,te,tau,f,k, unit);
	
// 	// cout << "Answer size: " << v1.size() << endl;
// 	// for (auto& item : v1) 
// 	// 	cout << get<0>(item) << ' ' << get<1>(item) << endl;
	

// 	//cout << "============" << endl;

// 	// vector<tuple_t> v2 = solver.theoretical_weighted_pruning(db, ts, te, tau, f, k, unit);
// 	// sort(v2.begin(), v2.end(), TimeCompare());
// 	double total_extra_time = 0;
// 	vector<tuple_t> v2 = solver.efficient_theoretical_weighted_prunning(db, ts, te, tau, f, k, unit, total_extra_time);
// 	sort(v2.begin(), v2.end(), TimeCompare());
	
// 	// cout << "Answer size: " << v2.size() << endl;
// 	// for (auto& item : v2) 
// 	// 	cout << get<0>(item) << ' ' << get<1>(item) << endl;
	

// 	cout << "equal? " << solver.answer_check(v1,v2) << endl;
// }

// void treap_test() {
// 	treap test;
// 	vector<int> v{1,0,2,4,4,4,3,3,3,7,6,4,1,6,6,3,12,7,1,9,5,2,2,11,3,4,1,11,2,3,2,8,7,5,2,6,0,3,9,4,3,3,4,10,10,13,3,4,5,2,1,1,1,3,9,8,5,8,0,7,2,2,5,8,4,5,4,0,6,8,0,2,2,3,0,5,1,5,2,12,1,4,2,6,9,2,6,7,6,4,2,3,6,5,1,3,5,1,10,3,1,7,4,6,7,5,3,6,7,2,2,3,5};
// 	for(int& i : v)
// 		test.insert(i);
// 	cout << test.get_min() << endl;
// }

void bound_analysis(vector<AnsUnit>& v, string& file, string& suffix, string type) {
	string filename = file + "_" + suffix + "_" + type + ".csv";

	ofstream fout(filename);
	if (!fout) {
		cout << "FILE_ERROR" << endl;
		return;
	}
	cout << "write out..." << endl;
	//sort(v.begin(),v.end(), UnitCompare());
	for (int i=0; i<v.size(); ++i) {
		fout << v[i].ans_size << " " << v[i].oracle_calls << " " << v[i].lower_bound << " " 
			<< v[i].upper_bound << " " << v[i].iterations << " " << v[i].candidate_size << endl;
	}
	fout.close();
}

void runtime_comparison(vector<TimeUnit>& v, string& file, string type) {
	string filename = file + "_runtime_" + type + ".csv";

	ofstream fout(filename);
	if (!fout) {
		cout << "FILE_ERROR" << endl;
		return;
	}
	cout << "write out..." << endl;
	//sort(v.begin(),v.end(), UnitCompare());

	for (int i=0; i<v.size(); ++i) {
		fout << v[i].ans_size << " " << v[i].k << " " << v[i].L << " " 
			<< v[i].tau << " " << v[i].baseline_time << ' ' << v[i].sf_time << ' '
			<< v[i].sw_time << ' ' << v[i].weighted_time << ' ' << v[i].better_theory_time << ' '
			<< v[i].theory_time << endl;
	}
	fout.close();
}

void query_generator_by_tau(int dim, int db_size, int query_size, vector<int>& ts, vector<int>& te, vector<vector<float> >& fs, vector<int>& ks, vector<int>& durability) {
	// Fix length and vary tau
	srand(time(0));
	default_random_engine generator;
	int t1, t2, k, L;
	L = db_size / 2, t1 = db_size - 1 - L, t2 = db_size - 10;
  	uniform_real_distribution<float> uniform_f(0.0,1.0);

  	ofstream fout;
  	fout.open("data/query_generator_by_tau.dat");
  	if (!fout) {
  		cout << "ERROR" << endl;
  		return;
  	}

  	//for (int tau=100000; tau<=4500000; tau+=500000) { // for 10M
  	//for (int tau=10000; tau<=460000; tau+=50000) { // for 1M
  	for (float percentage : vector<float>{0.01, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.46}) {
  		int tau = int(percentage * db_size);
	  	for (int i=0; i<query_size; ++i) {
	  		vector<float> weight;
	  		weight.push_back(0);
	  		weight.push_back(1);
	  		for (int i=0; i<dim-1; ++i) weight.push_back(uniform_f(generator));
	  		std::sort(weight.begin(), weight.end());
	  		k = 10;
	  		ts.push_back(t1);
	  		te.push_back(t2);
	  		ks.push_back(k);
	  		durability.push_back(tau);
	  		fout << k << ' ' << t1 << ' ' << t2 << ' ' << tau << ' ';
 	  		vector<float> random_weights;
	  		for (int i=1;i<weight.size(); ++i) {
	  			random_weights.push_back(weight[i] - weight[i-1]);
	  			fout << random_weights.back() << ' ';
	  			//cout << std::accumulate(random_weights.begin(), random_weights.end(), 0.0) << endl;
	  		}
	  		float sum = 0;
	  		std::accumulate(random_weights.begin(), random_weights.end(), sum);
	  		//cout << sum << endl;
	  		assert(abs(sum) - 1 < 0.0001);
	  		assert(random_weights.size() == dim);
	  		fs.push_back(random_weights);
	  		fout << endl;
	  	}	
  	}
  	fout.close();
}

void query_generator_by_k(int dim, int db_size, int query_size, vector<int>& ts, vector<int>& te, vector<vector<float> >& fs, vector<int>& ks, vector<int>& durability) {
	// Fix length, tau and vary k
	srand(time(0));
	default_random_engine generator;
	int t1, t2, k, L;
	L = db_size / 2, t1 = db_size - 1 - L, t2 = db_size - 10;
  	uniform_real_distribution<float> uniform_f(0.0,1.0);

  	ofstream fout;
  	fout.open("data/query_generator_by_k.dat");
  	if (!fout) {
  		cout << "ERROR" << endl;
  		return;
  	}

  	// int tau = 410000; // for 1M
  	// float tau = 4500000; // for 10M;
  	int tau = int(db_size * 0.2);
  	for (int k=5; k<=50; k+=5) {
	  	for (int i=0; i<query_size; ++i) {
	  		vector<float> weight;
	  		weight.push_back(0);
	  		weight.push_back(1);
	  		for (int i=0; i<dim-1; ++i) weight.push_back(uniform_f(generator));
	  		std::sort(weight.begin(), weight.end());
	  		ts.push_back(t1);
	  		te.push_back(t2);
	  		ks.push_back(k);
	  		durability.push_back(tau);
	  		fout << k << ' ' << t1 << ' ' << t2 << ' ' << tau << ' ';
	  		vector<float> random_weights;
	  		for (int i=1;i<weight.size(); ++i) {
	  			random_weights.push_back(weight[i] - weight[i-1]);
	  			fout << random_weights.back() << ' ';
	  		}
	  		float sum = 0;
	  		std::accumulate(random_weights.begin(), random_weights.end(), sum);
	  		assert(abs(sum) - 1 < 0.0001);
	  		assert(random_weights.size() == dim);
	  		fs.push_back(random_weights);
	  		fout << endl;
	  	}	
  	}
  	fout.close();
}

void query_generator_by_L(int dim, int db_size, int query_size, vector<int>& ts, vector<int>& te, vector<vector<float> >& fs, vector<int>& ks, vector<int>& durability) {
	// Fix length, tau and vary k
	srand(time(0));
	default_random_engine generator;
	int t1, t2, k;
	t2 = db_size - 10;
  	uniform_real_distribution<float> uniform_f(0.0,1.0);

  	ofstream fout;
  	fout.open("data/query_generator_by_L.dat");
  	if (!fout) {
  		cout << "ERROR" << endl;
  		return;
  	}

  	// int tau = 410000; // for 1M
  	// float tau = 4500000; // for 10M;
  	int tau = int(db_size * 0.2);
  	//for (int L=500000; tau<=5000000; tau+=500000) { // for 10M
  	//for (int L=50000; L<=500000; L+=50000) { // for 1M
  	for (float percentage : vector<float>{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.76}) {
  		int L = int(db_size * percentage);
	  	for (int i=0; i<query_size; ++i) {
	  		vector<float> weight;
	  		weight.push_back(0);
	  		weight.push_back(1);
	  		for (int i=0; i<dim-1; ++i) {
	  			weight.push_back(uniform_f(generator));
	  		}
	  		std::sort(weight.begin(), weight.end());
	  		t1 = db_size - 1 - L;
	  		k = 10;
	  		ts.push_back(t1);
	  		te.push_back(t2);
	  		ks.push_back(k);
	  		durability.push_back(tau);
	  		fout << k << ' ' << t1 << ' ' << t2 << ' ' << tau << ' ';
	  		vector<float> random_weights;
	  		for (int i=1;i<weight.size(); ++i) {
	  			random_weights.push_back(weight[i] - weight[i-1]);
	  			fout << random_weights.back() << ' ';
	  		}
	  		float sum = 0;
	  		std::accumulate(random_weights.begin(), random_weights.end(), sum);
	  		assert(abs(sum) - 1 < 0.0001);
	  		assert(random_weights.size() == dim);
	  		fs.push_back(random_weights);
	  		fout << endl;
	  	}	
  	}
  	fout.close();
}

void mean_and_std(vector<float>& v, float& avg, float& std) {
	avg = 0, std = 0;
	for(int i=0;i<v.size();++i)
		avg += v[i];
	avg = avg / v.size();

	for (int i=0;i<v.size();++i)
		std += (v[i] - avg) * (v[i] - avg);
	std = sqrt(std / v.size());

}

void exp_by_default(vector<Point>& db, int dim, string token, string index_file) {
	Solution<D> solver(NODE_SIZE_THRESHOLD);
	srand(time(0));
	default_random_engine generator;
  	uniform_real_distribution<float> uniform_f(0.0,1.0);
  	vector<vector<float>> fs;
  	for (int i=0; i<QUERY_SIZE; ++i) {
  		vector<float> weight;
  		weight.push_back(0);
  		weight.push_back(1);
  		for (int i=0; i<dim-1; ++i) {
  			weight.push_back(uniform_f(generator));
  		}
  		std::sort(weight.begin(), weight.end());
  		vector<float> random_weights;
  		for (int i=1;i<weight.size(); ++i) {
  			random_weights.push_back(weight[i] - weight[i-1]);
  		}
  		fs.push_back(random_weights);
  	}

	solver.grouping(db, BLOCK_SIZE);
	solver.blocktree_index(db);
	vector<int> klist(1, 9);
	for (int kv : klist)
		cout << kv << ' ';
	cout << endl;
	solver.kskyband_duration_rtree(db, klist);
	int ts = db.size()/2, te = db.size()-10, ks = 10, durability = 0.2 * db.size();
	vector<float> f1_score;

	string suffix = "number_of_topk_queries";
	string type = "by_default_"+ std::to_string(dim);
	vector<AnsUnit> unit1;
	vector<AnsUnit> unit2;
	vector<TimeUnit> TU;

	for(int i=0;i<QUERY_SIZE;++i) {
		cout << '\r' << i << flush;
		overall_correctness_check(solver, db, BLOCK_SIZE, klist, ts, te, fs[i], ks, durability, 
								unit1, unit2, TU, f1_score, 0);
	}
	cout << endl;
	bound_analysis(unit1, token, suffix, type);
	runtime_comparison(TU, token, type);
}

void exp_by_tau(vector<Point>&db, int dim, string token, string index_file) {

	Solution<D> solver(NODE_SIZE_THRESHOLD);
	
	solver.grouping(db, BLOCK_SIZE);
	solver.blocktree_index(db);
	//solver.rtree_index(db);
	//vector<Point> coreset_db = solver.all_range_coreset(db, k_max);
	//solver.rtree_index(coreset_db);
	vector<int> klist;
	for (int i=5; i<=50; i+=5) klist.push_back(i-1);
	//epsilon_sequence(k_max, epsilon, klist);
	for (int kv : klist)
		cout << kv << ' ';
	cout << endl;
	if (D > 5) {
		// solver.rtree_index(db);
		// solver.kskyband_duration_rtree_with_range_search(db, klist);
		solver.load_duration_rtree_index(index_file);
	}
	else
		solver.kskyband_duration_rtree(db, klist);
	
	vector<int> ts, te, ks, durability;
	vector<vector<float> > fs;
	vector<float> f1_score;
	query_generator_by_tau(dim, db.size(), QUERY_SIZE, ts, te, fs, ks, durability);

	// string suffix1 = "practical", suffix2 = "new_theory";
	string suffix = "number_of_topk_queries";
	string type = "by_tau_" + std::to_string(dim) + "_v2";
	vector<AnsUnit> unit1;
	vector<AnsUnit> unit2;
	vector<TimeUnit> TU;

	for(int i=0;i<ts.size();++i) {
		cout << '\r' << i << flush;
		overall_correctness_check(solver, db, BLOCK_SIZE, klist, ts[i], te[i], fs[i], ks[i], durability[i], 
								unit1, unit2, TU, f1_score);
	}
	cout << endl;
	bound_analysis(unit1, token, suffix, type);
	runtime_comparison(TU, token, type);
	float avg_f1, std_f1;
	mean_and_std(f1_score, avg_f1, std_f1);
	cout << "avg f1: " << avg_f1 << endl;
	cout << "std f1: " << std_f1 << endl;
}

void exp_by_k(vector<Point>&db, int dim, string token, string index_file) {

	Solution<D> solver(NODE_SIZE_THRESHOLD);
	
	solver.grouping(db, BLOCK_SIZE);
	solver.blocktree_index(db);

	vector<int> klist;
	for (int i=5; i<=50; i+=5) klist.push_back(i-1);
	//epsilon_sequence(k_max, epsilon, klist);
	for (int kv : klist)
		cout << kv << ' ';
	cout << endl;
	if (D > 5) {
		// solver.rtree_index(db);
		// solver.kskyband_duration_rtree_with_range_search(db, klist);
		solver.load_duration_rtree_index(index_file);
	}
	else
		solver.kskyband_duration_rtree(db, klist);
	
	vector<int> ts, te, ks, durability;
	vector<vector<float> > fs;
	vector<float> f1_score;
	query_generator_by_k(dim, db.size(), QUERY_SIZE, ts, te, fs, ks, durability);

	// string suffix1 = "practical", suffix2 = "new_theory";
	string suffix = "number_of_topk_queries";
	string type = "by_k_"+ std::to_string(dim) + "_v2";
	vector<AnsUnit> unit1;
	vector<AnsUnit> unit2;
	vector<TimeUnit> TU;

	for(int i=0;i<ts.size();++i) {
		cout << '\r' << i << flush;
		overall_correctness_check(solver, db, BLOCK_SIZE, klist, ts[i], te[i], fs[i], ks[i], durability[i], 
								unit1, unit2, TU, f1_score);
	}
	cout << endl;
	bound_analysis(unit1, token, suffix, type);
	runtime_comparison(TU, token, type);
	float avg_f1, std_f1;
	mean_and_std(f1_score, avg_f1, std_f1);
	cout << "avg f1: " << avg_f1 << endl;
	cout << "std f1: " << std_f1 << endl;
}

void exp_by_L(vector<Point>&db, int dim, string token, string index_file) {

	Solution<D> solver(NODE_SIZE_THRESHOLD);
	
	solver.grouping(db, BLOCK_SIZE);
	solver.blocktree_index(db);

	vector<int> klist;
	for (int i=5; i<=50; i+=5) klist.push_back(i-1);
	//epsilon_sequence(k_max, epsilon, klist);
	for (int kv : klist)
		cout << kv << ' ';
	cout << endl;
	if (D > 5) {
		// solver.rtree_index(db);
		// solver.kskyband_duration_rtree_with_range_search(db, klist);
		solver.load_duration_rtree_index(index_file);
	}
	else
		solver.kskyband_duration_rtree(db, klist);
	
	vector<int> ts, te, ks, durability;
	vector<vector<float> > fs;
	vector<float> f1_score;
	query_generator_by_L(dim, db.size(), QUERY_SIZE, ts, te, fs, ks, durability);

	// string suffix1 = "practical", suffix2 = "new_theory";
	string suffix = "number_of_topk_queries";
	string type = "by_L_" + std::to_string(dim) + "_v2";
	vector<AnsUnit> unit1;
	vector<AnsUnit> unit2;
	vector<TimeUnit> TU;

	for(int i=0;i<ts.size();++i) {
		cout << '\r' << i << flush;
		overall_correctness_check(solver, db, BLOCK_SIZE, klist, ts[i], te[i], fs[i], ks[i], durability[i], 
								unit1, unit2, TU, f1_score);
	}
	cout << endl;
	bound_analysis(unit1, token, suffix, type);
	runtime_comparison(TU, token, type);
	float avg_f1, std_f1;
	mean_and_std(f1_score, avg_f1, std_f1);
	cout << "avg f1: " << avg_f1 << endl;
	cout << "std f1: " << std_f1 << endl;
}

void print_vector(vector<float>& v) {
	for (auto i : v)
		cout << i << ',';
	cout << endl;
}

void sorting_cost(vector<Point>&db, int dim, string index_file) {
	Solution<D> solver(NODE_SIZE_THRESHOLD);
	solver.grouping(db, BLOCK_SIZE);
	solver.blocktree_index(db);

	vector<int> klist;
	for (int i=5; i<=50; i+=5) klist.push_back(i-1);
	for (int kv : klist)
		cout << kv << ' ';
	cout << endl;
	if (D > 5) {
		solver.load_duration_rtree_index(index_file);
	}
	else
		solver.kskyband_duration_rtree(db, klist);
	
	clock_t t_start, t_end;
	vector<int> ts, te, ks, durability;
	vector<vector<float> > fs;
	double total_extra_time = 0;
	vector<AnsUnit> unit1;
	TimeUnit tunit;
	float avg,deviation;

	// By tau
	query_generator_by_tau(dim, db.size(), QUERY_SIZE, ts, te, fs, ks, durability);
	cout << "===Vary Tau===" << endl;
	vector<float> tau_avg, tau_std, temp;

	for(int i=0;i<ts.size();++i) {
		cout << '\r' << i << flush;
		t_start = clock();
		vector<tuple_t> v = solver.weighted_prunning(db, ts[i], te[i], durability[i], fs[i], ks[i], 0, unit1, tunit, total_extra_time);
		t_end = clock();
		temp.push_back((double) (t_end - t_start) / CLOCKS_PER_SEC);
		if (temp.size() == QUERY_SIZE) {
			mean_and_std(temp, avg, deviation);
			tau_avg.push_back(avg);
			tau_std.push_back(deviation);
			temp.clear();
		}
	}
	cout << endl;
	print_vector(tau_avg);
	print_vector(tau_std);
	// By k
	ts.clear(), te.clear(), ks.clear(), durability.clear(), fs.clear(), temp.clear();
	query_generator_by_k(dim, db.size(), QUERY_SIZE, ts, te, fs, ks, durability);
	cout << "===Vary K===" << endl;
	vector<float> k_avg, k_std;
	for(int i=0;i<ts.size();++i) {
		cout << '\r' << i << flush;
		t_start = clock();
		vector<tuple_t> v = solver.weighted_prunning(db, ts[i], te[i], durability[i], fs[i], ks[i], 0, unit1, tunit, total_extra_time);
		t_end = clock();
		temp.push_back((double) (t_end - t_start) / CLOCKS_PER_SEC);
		if (temp.size() == QUERY_SIZE) {
			mean_and_std(temp, avg, deviation);
			k_avg.push_back(avg);
			k_std.push_back(deviation);
			temp.clear();
		}
	}
	cout << endl;
	print_vector(k_avg);
	print_vector(k_std);
	// By L
	ts.clear(), te.clear(), ks.clear(), durability.clear(), fs.clear(), temp.clear();
	query_generator_by_L(dim, db.size(), QUERY_SIZE, ts, te, fs, ks, durability);
	cout << "===Vary L===" << endl;
	vector<float> l_avg, l_std;
	for(int i=0;i<ts.size();++i) {
		cout << '\r' << i << flush;
		t_start = clock();
		vector<tuple_t> v = solver.weighted_prunning(db, ts[i], te[i], durability[i], fs[i], ks[i], 0, unit1, tunit, total_extra_time);
		t_end = clock();
		temp.push_back((double) (t_end - t_start) / CLOCKS_PER_SEC);
		if (temp.size() == QUERY_SIZE) {
			mean_and_std(temp, avg, deviation);
			l_avg.push_back(avg);
			l_std.push_back(deviation);
			temp.clear();
		}
	}
	cout << endl;
	print_vector(l_avg);
	print_vector(l_std);

}

vector<tuple_t> fixed_window_durable_preference_topk(vector<Point>& db, int ts,int te, int tau, vector<float>& f, int k) {
	vector<tuple_t> ans;
	for (int t = te; t >= ts; t -= tau) {
		vector<tuple_t> window;
		cout << t << '-' << max(t - tau, ts) << endl;
		for (int i = t; i > max(t - tau, ts); --i) {
			window.emplace_back(make_pair(i, score(db[i], f)));
		}
		sort(window.begin(), window.end(), ScoreCompare());
		for (int i=0; i<k; ++i)
			ans.emplace_back(window[i]);
	}
	return ans;
}

vector<tuple_t> just_topk(vector<Point>& db, int ts,int te, vector<float>& f, int k) {
	vector<tuple_t> ans;
	vector<tuple_t> window;
	for (int t = te; t >= ts; --t) {
		window.emplace_back(make_pair(t, score(db[t], f)));
	}
	sort(window.begin(), window.end(), ScoreCompare());
	for (int i=0; i<k; ++i)
		ans.emplace_back(window[i]);
	return ans;
}

template<size_t N>
void query_benchmark(Solution<N>& solver, vector<Point>& db, int ts, int te, int tau, vector<float>& f, int k) {
	cout << "*************" << endl;
	clock_t t_start,t_end;
	vector<AnsUnit> unit1;
	t_start = clock();
	vector<tuple_t> v1 = solver.sequential(db, ts,te,tau,f,k, unit1);
	t_end = clock();
	cout << "asw time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

	double total_extra_time = 0;

	t_start = clock();
	vector<tuple_t> v5 = solver.efficient_theoretical_weighted_prunning(db, ts, te, tau, f, k, unit1, total_extra_time);
	t_end = clock();
	cout << "ps time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

	t_start = clock();
	vector<tuple_t> v2 = solver.efficient_theoretical_weighted_prunning_v2(db, ts, te, tau, f, k, unit1, total_extra_time);
	t_end = clock();
	cout << "ps-o time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

	t_start = clock();
	vector<tuple_t> v0 = solver.sequential_basic(db, ts,te,tau,f,k);
	t_end = clock();
	cout << "sw time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

	t_start = clock();
	TimeUnit tunit;
	vector<tuple_t> v3 = solver.weighted_prunning_v2(db, ts, te, tau, f, k, k-1, unit1, tunit, total_extra_time);
	t_end = clock();
	cout << "skyband time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

	t_start = clock();
	vector<tuple_t> v6 = solver.weighted_prunning(db, ts, te, tau, f, k, k-1, unit1, tunit, total_extra_time);
	t_end = clock();
	cout << "sorting time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << endl;

	sort(v2.begin(), v2.end(), TimeCompare());
	sort(v5.begin(), v5.end(), TimeCompare());
	sort(v3.begin(), v3.end(), TimeCompare());
	sort(v6.begin(), v6.end(), TimeCompare());

	if (!solver.answer_check(v0, v1))
		cout << "sequential error :" << v0.size() << '/' << v1.size() << endl;
	if (!solver.answer_check(v1, v5))
		cout << "ps error : " << v1.size() << '/' << v5.size() << endl;
	if (!solver.answer_check(v0, v2))
		cout << "ps#2 error : " << v0.size() << '/' << v2.size() << endl;
	if (!solver.answer_check(v1, v3))
		cout << "skyband error : " << v1.size() << '/' << v3.size() << endl;
	if (!solver.answer_check(v0, v6))
		cout << "sorting error : " << v0.size() << '/' << v6.size() << endl;
}

int main(int argc, char* argv[]) {
	int dimension = atoi(argv[1]);
	assert(D == dimension);
	Loader worker;
	string filename(argv[2]);
	string token = filename.substr(0, filename.find('.'));
	if (dimension == 1)
		worker.load1(argv[2]);
	else if (dimension == 2)
		worker.load2(argv[2]);
	else if (dimension == 3)
		worker.load3(argv[2]);
	else
		worker.load_ndim(argv[2], dimension);

	int exp_type = atoi(argv[3]);
	string index_file(argv[4]);
	vector<Point> db = worker.get_db();
	cout << db.size() << endl;

	if (exp_type == 0) {
		cout << "=== by default ===" << endl;
		exp_by_default(db, dimension, token, index_file);
	}
	if (exp_type == 1) {
		cout << "=== vary tau ===" << endl;
		exp_by_tau(db, dimension, token, index_file);
	}
	if (exp_type == 2) {
		cout << "=== vary k ===" << endl;
		exp_by_k(db, dimension, token, index_file);
	}
	if (exp_type == 3) {
		cout << "=== vary L ===" << endl;
		exp_by_L(db, dimension, token, index_file);
	}
	if (exp_type == 7) {
		cout << "=== pure sorting baseline ===" << endl;
		sorting_cost(db, dimension, index_file);
	}
	if (exp_type == 4) { // for proof-of-concept experiments
		cout << "=== proof-of-concept ===" << endl;
		Solution<D> solver(NODE_SIZE_THRESHOLD);
		solver.grouping(db, BLOCK_SIZE);
		solver.blocktree_index(db);
		int ts = 205000, te = 905702; // an approximate 20 year window
		int tau = 150000; // an approximate 5 year duration
		vector<float> f{1};
		int k = 1;
		vector<AnsUnit> dummy;
		cout << ts << ' ' << te << ' ' << tau << endl;
		vector<tuple_t> ans = solver.sequential(db, ts, te, tau, f, k, dummy);
		cout << "sliding window answer size: " << ans.size() << endl;
		sort(ans.begin(), ans.end(), ScoreCompare());
		for (auto game : ans) {
			for (auto v : db[get<0>(game)].data) 
				cout << v << " - ";
			cout << db[get<0>(game)].timestamp << endl;
		}
		cout << "=======================" << endl;
		vector<tuple_t> baseline = fixed_window_durable_preference_topk(db, ts, te, tau, f, k);
		cout << "fixed window answer size: " << baseline.size() << endl;
		for (auto game : baseline) {
			for (auto v : db[get<0>(game)].data)
				cout << v << " - ";
			cout << db[get<0>(game)].timestamp << endl;
		}
		cout << "=======================" << endl;
		vector<tuple_t> topk = just_topk(db, ts, te, f, ans.size());
		cout << "just topk answer size: " << topk.size() << endl;
		for (auto game : topk) {
			for (auto v : db[get<0>(game)].data)
				cout << v << " - ";
			cout << db[get<0>(game)].timestamp << endl;
		}
	}
	if (exp_type == 5) { // for test and debug
		cout << "=== Debug and test ===" << endl;
		Solution<D> solver(NODE_SIZE_THRESHOLD);
	
		//solver.grouping(db, BLOCK_SIZE);
		//solver.blocktree_index(db);
		solver.rtree_index(db);

		vector<int> klist;
		for (int i=5; i<=50; i+=5) klist.push_back(i-1);
		//epsilon_sequence(k_max, epsilon, klist);
		for (int kv : klist)
			cout << kv << ' ';
		cout << endl;
		//solver.kskyband_duration_rtree(db, klist);
		solver.kskyband_duration_rtree_with_range_search(db, klist);
	}
	if (exp_type == 6) { // for query time benchmarking
		cout << "=== Query benchmarking ===" << endl;
		Solution<D> solver(NODE_SIZE_THRESHOLD);

		vector<int> klist;
		for (int i=5; i<=50; i+=5) klist.push_back(i-1);
	
		solver.grouping(db, BLOCK_SIZE);
		solver.blocktree_index(db);
		if (D > 5)
			solver.load_duration_rtree_index(index_file);
		else
			solver.kskyband_duration_rtree(db, klist);

		cout << "=====by tau=====" << endl;
		srand(time(0));
		default_random_engine generator;
		int t1, t2, k, L, tau;
		L = db.size() / 2, t1 = db.size() - 1 - L, t2 = db.size() - 10;
	  	uniform_real_distribution<float> uniform_f(0.0,1.0);

	  	for (float percentage : vector<float>{0.01, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.46}) {
	  		tau = int(percentage * db.size());
		  	cout << percentage * 100 << '%' << endl;
	  		vector<float> weight;
	  		weight.push_back(0);
	  		weight.push_back(1);
	  		for (int i=0; i<dimension-1; ++i) weight.push_back(uniform_f(generator));
	  		std::sort(weight.begin(), weight.end());
	  		k = 10;
 	  		vector<float> random_weights;
	  		for (int i=1;i<weight.size(); ++i) {
	  			random_weights.push_back(weight[i] - weight[i-1]);
	  		}
	  		query_benchmark(solver, db, t1, t2, tau, random_weights, k);
  		}

  		cout << "=====by k=====" << endl;
  		tau = int(db.size() * 0.2);
	  	for (int i=5; i<=50; i+=5) {
	  		k = i;
	  		cout << k << endl;
	  		vector<float> weight;
	  		weight.push_back(0);
	  		weight.push_back(1);
	  		for (int i=0; i<dimension-1; ++i) weight.push_back(uniform_f(generator));
	  		std::sort(weight.begin(), weight.end());
	  		vector<float> random_weights;
	  		for (int i=1;i<weight.size(); ++i) {
	  			random_weights.push_back(weight[i] - weight[i-1]);
	  		}
	  		query_benchmark(solver, db, t1, t2, tau, random_weights, k);
	  	}

	  	cout << "=====by L=====" << endl;
	  	tau = int(db.size() * 0.2);
	  	for (float percentage : vector<float>{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.76}) {
	  		L = int(db.size() * percentage);
	  		cout << percentage * 100 << '%' << endl;
	  		vector<float> weight;
	  		weight.push_back(0);
	  		weight.push_back(1);
	  		for (int i=0; i<dimension-1; ++i) {
	  			weight.push_back(uniform_f(generator));
	  		}
	  		std::sort(weight.begin(), weight.end());
	  		t1 = db.size() - 1 - L;
	  		k = 10;
	  		vector<float> random_weights;
	  		for (int i=1;i<weight.size(); ++i) {
	  			random_weights.push_back(weight[i] - weight[i-1]);
	  		}
	  		query_benchmark(solver, db, t1, t2, tau, random_weights, k);
	  	}
	}
	cout << "Done!" << endl;
	return 0;
}
