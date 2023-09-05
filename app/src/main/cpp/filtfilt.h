#pragma once
#include <vector>
#include <exception>
#include <algorithm>
#include "Eigen/Dense"

typedef std::vector<int> vectori;
typedef std::vector<double> vectord;

//using namespace Eigen;

class my_filtfilt
{
public:
	void add_index_range(vectori& indices, int beg, int end, int inc = 1);
	void add_index_const(vectori& indices, int value, size_t numel);
	void append_vector(vectord& vec, const vectord& tail);
	vectord subvector_reverse(const vectord& vec, int idx_end, int idx_start);
	inline int max_val(const vectori& vec);
	void filter(vectord B, vectord A, const vectord& X, vectord& Y, vectord& Zi);
	void filtfilt(vectord B, vectord A, const vectord& X, vectord* Y);

};