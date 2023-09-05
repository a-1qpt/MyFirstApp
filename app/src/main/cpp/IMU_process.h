#include <iostream>
#include <vector>
#include <cmath>
#include "filtfilt.h"

typedef std::vector<bool> vectorb;
typedef std::vector<std::vector<double>> vvectrod;

struct raw_data
{
	raw_data();
	raw_data(vectord& read_time, vvectrod& read_acc);
	vectord time;
	vvectrod acc;
};

class IMU_cal 
{
public:
	/**
	* Calculate the average number from a N*1 vector
	* data is a N*1 vector
	* \param data
	* \param avg
	* \return
	*/
	bool mean(const vectord& data,double* avg) const;

	/**
	* This functions calls the mean function. The input data is a N*1 vector. Each number is replaced by
	* the average of the consecutive n munbers
	* \param data
	* \param n
	* \param smoothed_data
	* \return
	*/
	bool window_smooth(const vectord& data, int n, vectord* smoothed_data) const;

	/**
	* This function calls the window_smooth data. The input data is a N*3 vector. The input data is
	* smoothed by columns 
	* \param acc
	* \param smoothed_acc
	* \return
	*/
	bool smooth_acc(const vvectrod& acc,int n, vvectrod* smoothed_acc) const;

	/**
	* Calculate the velocity of the ipad without drift using the acceleration data and time data
	* acc is a N*3 vector, time is a N*1 vector, nodrift_vel is a N*3 vector
	* \param time
	* \param acc
	* \param nodrift_vel
	* \return
	*/
	bool get_vel_drift(const vectord& time, const vvectrod& acc,
		vvectrod* nodrift_vel) const;
	
	/**
	* Calculate the stationary point from acceleration data. acc is a N*3 vector, stationary is a N*1 vector.
	* \param acc
	* \param stationary
	* \return
	*/
	bool get_stationary(const vvectrod& acc, vectorb* stationary) const;

	/**
	* Smooth the N*1 vector. Data are fragmented using a slinding window with the length of n each time.
	* Fragmented data are sorted, the orginal data is replaced by the median number.
	* \param data
	* \param n
	* \param md
	* \return
	*/
	bool medfilter(const vectord& data, int n,vectord* md) const;

	/**
	* This is a low pass filter.
	* data is a N*3 vector, lp is a N*1 vector
	* \return
	*/
	bool passfilter(const vectord& data, double pass_freq, vectord* lp) const;

	/**
	* Calculate velocity with drift using acceleration data, time and stationary points
	* acc is a N*3 vector, time is a N*1 vector, stationary is a N*1 vector, drift_vel is a N*3 vector
	* \param acc
	* \param time
	* \param stationary
	* \param drift_velocity
	* \return
	*/
	bool integral_vel(const vvectrod& acc, const vectord& time,
		const vectorb& stationary, vvectrod* drift_vel) const;
	
	/**
	* Calculate the velocity drift to rectify the drifted velocity
	* vel is a N*3 vector, time is a N*1 vector, stationary is a N*1 vector, drift is a N*3 vector
	* \return
	*/
	bool get_drift(const vvectrod& vel, const vectord& time,
		const vectorb& stationary, vvectrod* drift) const;
	
	/**
	* Calculate positions of the ipad using the non-drift velocity data and time data
	* time is a N*1 vctor, vel is a N*3 vector, pos is a N*3 vector
	* \param time
	* \param vel
	* \param pos
	* \return
	*/
	bool integral_pos(const vectord& time, const vvectrod& vel,
		vvectrod* pos) const;
	
	/**
	* Calculate the displacement of the ipad using the starting position and ending position
	* start_pos & end_pos are 1*3 vectors
	* \param start_pos
	* \param end_pos
	* \param dis
	* \return
	*/
	bool cal_dis(const vectord& start_pos, const vectord& end_pos, double* dis) const;

	/**
	* Butterworth low pass filter
	* a,b are 1*2 vectors
	* \param n
	* \param cutoff_freq
	* \param b
	* \param a
	* \return
	*/
	bool lowpass_butteworth(const int n, const double cutoff_freq, vectord* b, vectord* a) const;

	/**
	* Zero phase digital filter
	* a,b are 1*2 vectors, data is N*1 vector
	* \param n
	* \param cutoff_freq
	* \param b
	* \param a
	* \return
	*/
	bool filtfilt(const vectord b, const vectord a, vectord data, vectord* out_data) const;

	/**
	* Calculate the distance
	* \param dis
	* \return
	*/
	bool displacement_inmeter(const vectord& time,const vvectrod& acc,double* dis) const;
};
