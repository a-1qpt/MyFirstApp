#include "IMU_process.h"
using namespace std;

raw_data::raw_data() 
{
	time = vectord();
	acc = vvectrod();
}

raw_data::raw_data(vectord& read_time, vvectrod& read_acc)
{
	time = read_time;
	acc = read_acc;
}

bool IMU_cal::mean(const vectord& data, double* avg) const
{
	if (data.size() == 0)
	{
		return false;
	}
	double sum = 0;
	for (int i = 0; i < data.size(); i++)
	{
		sum += data[i];
	}
	*avg = sum / data.size();
	return true;
}

bool IMU_cal::window_smooth(const vectord& data, int n,vectord* smoothed_data) const
{
	if (data.size() == 0 || n == 0)
	{
		return false;
	}
	if (n == 1)
	{
		*smoothed_data = data;
		return true;
	}
	int L = data.size();
	for (int i = 0; i < L; i++)
	{
		if (i < n-1)
		{
			(*smoothed_data)[i] = data[i];
		}
		else
		{
			double sum = 0;
			for (int j = i - n+1; j < i+1; j++)
			{
				sum += data[j];
			}
			(*smoothed_data)[i] = sum / n;
		}
	}
	return true;
}

bool IMU_cal::smooth_acc(const vvectrod& acc, int n, vvectrod* smoothed_acc) const
{
	if (acc.size() == 0 || acc[0].size() != 3)
	{
		return false;
	}
	vvectrod sm_acc(acc.size(), vectord(3, 0.0));
	for (int i = 0; i < 3; i++)
	{
		vectord acc_proc(acc.size(), 0.0);
		for (int k = 0; k < acc.size(); k++)
		{
			acc_proc[k] = acc[k][i];
		}
		vectord acc_line(acc.size(), 0.0);
		bool result = window_smooth(acc_proc, n, &acc_line);
		for (int j = 0; j < acc.size(); j++)
		{
			(*smoothed_acc)[j][i] = acc_line[j];
		}
	}
	return true;
}

bool IMU_cal::get_vel_drift(const vectord& time, const vvectrod& acc,
	vvectrod* nodrift_vel) const
{
	if (acc.size() != time.size() || time.size() == 0)
	{
		return false;
	}
	vectorb stationary(acc.size(), 0);
	get_stationary(acc, &stationary);
	for (int i=0;i<50;i++)
	{
		stationary[i] = 1;
	}
	//save_mydatavb(stationary);

	vvectrod drift_vel(acc.size(), vectord(3, 0.0));
	integral_vel(acc, time, stationary, &drift_vel);
	//save_mydatavvd(drift_vel);
	vvectrod drift(acc.size(), vectord(3,0.0));
	get_drift(drift_vel, time, stationary, &drift);
	//save_mydatavvd(drift);
	for (int j=0;j<acc.size();j++)
	{
		for (int k = 0; k < 3; k++)
		{
			(*nodrift_vel)[j][k] = drift_vel[j][k] - drift[j][k];
		}
	}
	return true;
}

bool IMU_cal::get_stationary(const vvectrod& acc, vectorb* stationary) const
{
	if (acc.size() == 0 || acc[0].size() != 3)
	{
		return false;
	}
	vectord acc_norm(acc.size(),0.0);
	for (int i = 0; i < acc.size(); i++)
	{
		double a = 0;
		for (int j = 0; j < 3; j++)
		{
			a += pow(acc[i][j], 2);
		}
		acc_norm[i] = sqrt(a);
	}
		vectord md(acc_norm.size(),0.0);
		medfilter(acc_norm, 755, &md);
		for (int k = 0; k < acc_norm.size(); k++)
		{
			acc_norm[k] -= md[k];
		}
		vectord lp(acc_norm.size(), 0.0);
		passfilter(acc_norm, 0.2, &lp);
		for (int h = 0; h < acc_norm.size(); h++)
		{
			(*stationary)[h] = lp[h] < 0.05;
		}

	return true;
}

bool IMU_cal::medfilter(const vectord& data, int n, vectord* md) const
{
	if (data.size() == 0 || n < 1)
	{
		return false;
	}
	int L = data.size();
	for (int i = 0; i < L; i++)
	{
		int start = max(0, i - n / 2);
		int end = min(L - 1, i + n / 2);
		vectord window(data.begin() + start, data.begin() + end + 1);
		sort(window.begin(), window.end());
		(*md)[i] = window[window.size()/2];
	}
	return true;
}

bool IMU_cal::passfilter(const vectord& data, double pass_freq, vectord* lp) const
{
	vectord a;
	vectord b;
	a.push_back(1);
	a.push_back(-0.8816);
	b.push_back(0.0592);
	b.push_back(0.0592);
	lowpass_butteworth(1, pass_freq, &b, &a);
	filtfilt(b, a, data, lp);
	return true;
}

bool IMU_cal::integral_vel(const vvectrod& acc, const vectord& time,
	const vectorb& stationary, vvectrod* drift_vel) const
{
	if (time.size() != acc.size() || time.size() == 0)
	{
		return false;
	}
	int L = time.size();
	/*for (int i = 1; i < L ; i++)
	{
		if (!stationary[i])
		{
			for (int k = 0; k < 3; k++)
			{
				(*drift_vel)[i][k] = (*drift_vel)[i - 1][k] + (acc[i][k]+ acc[i-1][k]) * (time[i]-time[i-1]) / 2;
			}
		}
	}*/
	vectord sub_data(3, 0.0);
	for (int i = 0; i < L-1; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			sub_data[j] += (acc[i][j] + acc[i + 1][j]) * (time[i + 1] - time[i]) / 2;
			if (stationary[i] != 0)
			{
				sub_data[j] = 0.0;
			}
			(*drift_vel)[i + 1][j] = sub_data[j];
		}
	}
	return true;
}

bool IMU_cal::get_drift(const vvectrod& vel, const vectord& time,
	const vectorb& stationary, vvectrod* drift) const
{
	if (time.size() != vel.size() || time.size() == 0)
	{
		return false;
	}
	vectori drift_start;
	vectori drift_end;
	for (int i = 0; i < stationary.size()-1; i++)
	{
		if ((stationary[i + 1] == 0) && (stationary[i] == 1))
		{
			drift_start.push_back(i);
		}
		if ((stationary[i + 1] == 1) && (stationary[i] == 0))
		{
			drift_end.push_back(i);
		}
	}
	if (drift_end.size())
	{
		if (drift_start[0] > drift_end[0])
		{
			drift_start.insert(drift_start.begin(), 0);
		}
	}

	for (int i = 0; i < drift_start.size(); i++)
	{
		cout << drift_start[i] << "  ";
	}
	cout << "drift_start" << endl;
	for (int i = 0; i < drift_end.size(); i++)
	{
		cout << drift_end[i] << "  ";
	}
	cout << "drift_end"<<endl;

	for (int i = 0; i < drift_end.size(); i++)
	{
		int ts = drift_start[i];
		int te = drift_end[i];
		double move_duration = time[te] - time[ts];
		for (int j = 0; j < 3; j++)
		{
			double vel_end = vel[te + 1][j];
			double vel_drift_t = vel_end / move_duration;
			for (int k = ts; k < te + 1; k++)
			{
				(*drift)[k][j] = vel_drift_t * (time[k]- time[ts]);
			}
		}
	
	}
	return true;
}

bool IMU_cal::integral_pos(const vectord& time, const vvectrod& vel,
	vvectrod* pos) const
{
	if (time.size() != vel.size() || time.size() == 0)
	{
		return false;
	}
	int L = time.size();
	for (int i=1;i<L;i++)
	{
		for (int j = 0; j < (vel[0]).size(); j++)
		{
			(*pos)[i][j] = (*pos)[i - 1][j] + (vel[i - 1][j] + vel[i][j]) * (time[i] - time[i - 1]) / 2;
		}
	}
	return true;
}

bool IMU_cal::cal_dis(const vectord& start_pos, const vectord& end_pos, double* dis) const
{
	if (start_pos.size() != end_pos.size())
	{
		return false;
	}
	for (int i = 0; i < 3; ++i) {
		cout << start_pos[i] << " ";
	}
	cout << endl;
	for (int i = 0; i < 3; ++i) {
		cout << end_pos[i] << " ";
	}
	cout << endl;
	int L = start_pos.size();
	double sum = 0;
	for (int i = 0; i < L; i++)
	{
		sum += pow((end_pos[i] - start_pos[i]),2);
	}
	*dis = sqrt(sum);
	return true;
}

bool IMU_cal::lowpass_butteworth(const int n, const double cutoff_freq, vectord* b, vectord* a) const
{
	return true;
}

bool IMU_cal::filtfilt(const vectord b, const vectord a, vectord data, vectord* out_data) const
{
	if (data.size() == 0)
	{
		return false;
	}
	my_filtfilt aa;
	aa.filtfilt(b,a,data,out_data);
	return true;
}

bool IMU_cal::displacement_inmeter(const vectord& time,const vvectrod& acc,double* dis) const
{
	vvectrod smoothed_acc(time.size(), vectord(3, 0.0));
	vvectrod nodrift_vel(time.size(), vectord(3, 0.0));
	vvectrod pos(time.size(), vectord(3, 0.0));
	smooth_acc(acc, 3, &smoothed_acc);
	get_vel_drift(time, smoothed_acc, &nodrift_vel);
	integral_pos(time, nodrift_vel, &pos);
	vectord start_pos = pos[1];
	vectord end_pos = pos[pos.size() - 1];
	cal_dis(start_pos, end_pos, dis);
	cout << (*dis) * 100 << "cm" << endl;
	return true;
}