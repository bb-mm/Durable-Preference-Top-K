#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include "point.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>


using namespace std;

#define MIN(A, B) (((A)<(B))?(A):(B))
#define MAX(A, B) (((A)>(B))?(A):(B))

class CSVRow {
    public:
        std::string const& operator[](std::size_t index) const {
            return m_data[index];
        }
        std::size_t size() const {
            return m_data.size();
        }
        void readNextRow(std::istream& str) {
            std::string         line;
            std::getline(str, line);

            std::stringstream   lineStream(line);
            std::string         cell;

            m_data.clear();
            while(std::getline(lineStream, cell, ' '))
            {
                m_data.push_back(cell);
            }
        }
    private:
        std::vector<std::string> m_data;
};

std::istream& operator>>(std::istream& str, CSVRow& data)
{
    data.readNextRow(str);
    return str;
}   

class Loader {
public:
	Loader() {}
	~Loader() {}

	void load1(char* filename); 

	void load2(char* filename);

	void load3(char* filename);

	void load_ndim(char* filename, int ndim);

	vector<Point>& get_db() {
		return db;
	}

	float get_dimension_max(int dim) {
		return dimension_max[dim];
	}

private:
	vector<Point> db;
	vector<float> dimension_max;
};

void Loader::load1(char* filename) {
	ifstream fin(filename);
	if (!fin)
		cout << "FILE ERROR" << endl;
	string line;
	int cnt = 0;
	float a_max = 0;
	
	while(getline(fin, line)) {
		istringstream ss(line);
		string id, t;
		float a;
		if (!(ss >> id >> a >> t))
			break;
		Point p;
		p.arrival_time = cnt;
		p.data.push_back(a);
		p.data.push_back(cnt);
		p.timestamp = t;
		cnt++;
		db.push_back(p);
		a_max = MAX(a,a_max);
	}
	fin.close();
	cout << "total records: " << cnt << endl;
	cout << "dimension_max: " << a_max << endl;
	dimension_max.push_back(a_max);
}

void Loader::load2(char* filename) {
	ifstream fin(filename);
	if (!fin)
		cout << "FILE ERROR" << endl;
	string line;
	int cnt = 0;
	float a_max = 0;
	float b_max = 0;
	while(getline(fin, line)) {
		istringstream ss(line);
		string id, t;
		float a,b;
		if (!(ss >> id >> a >> b >> t))
			break;
		Point p;
		p.arrival_time = cnt;
		p.data.push_back(a);
		p.data.push_back(b);
		p.data.push_back(cnt);
		p.timestamp = t;
		cnt++;
		db.push_back(p);
		a_max = MAX(a,a_max);
		b_max = MAX(b,b_max);
	}
	fin.close();
	cout << "total records: " << cnt << endl;
	cout << "dimension_max: " << a_max << ' ' << b_max << endl;
	dimension_max.push_back(a_max);
	dimension_max.push_back(b_max);
}

void Loader::load3(char* filename) {
	ifstream fin(filename);
	if (!fin)
		cout << "FILE ERROR" << endl;
	string line;
	int cnt = 0;
	float a_max = 0;
	float b_max = 0;
	float c_max = 0;
	while(getline(fin, line)) {
		istringstream ss(line);
		string id, t;
		float a,b,c;
		if (!(ss >> id >> a >> b >> c >> t))
			break;
		Point p;
		p.arrival_time = cnt;
		p.data.push_back(a);
		p.data.push_back(b);
		p.data.push_back(c);
		p.data.push_back(cnt);
		p.timestamp = t;
		cnt++;
		db.push_back(p);
		a_max = MAX(a,a_max);
		b_max = MAX(b,b_max);
		c_max = MAX(c,c_max);
	}
	fin.close();
	cout << "total records: " << cnt << endl;
	cout << "dimension_max: " << a_max << ' ' << b_max << ' ' << c_max << endl;
	dimension_max.push_back(a_max);
	dimension_max.push_back(b_max);
	dimension_max.push_back(c_max);
}

// up to 38 dims
void Loader::load_ndim(char* filename, int ndim) {
	ifstream fin(filename);
	if (!fin)
		cout << "FILE ERROR" << endl;
	int cnt = 0;
	vector<float> dim_max(ndim, 0);
	CSVRow row;
	while(fin >> row) {
		Point p;
		p.arrival_time = cnt;
		//cout << row.size() << endl;
		for (int i=1; i<=ndim; ++i) {
			p.data.push_back(stof(row[i]));
			if (p.data.back() > dim_max[i-1])
				dim_max[i-1] = p.data.back();
		}
		// for (int i=0; i<p.data.size(); ++i)
		// 	cout << i << ':' << p.data[i] << ' ';
		// cout << cnt << endl;
		p.data.push_back(cnt);
		p.timestamp = row[row.size() - 1];
		cnt++;
		db.push_back(p);

		if (cnt % 1000 == 0)
			cout << '\r' << cnt << std::flush;
	}
	fin.close();
	cout << "total records: " << cnt << endl;
	cout << "dimension_max: " << endl;
	for (int i=0; i<dim_max.size(); ++i) {
		cout << i << " : " << dim_max[i] << endl;
		dimension_max.push_back(dim_max[i]);
	}
}

#endif