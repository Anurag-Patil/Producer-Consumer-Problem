#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <thread>
#include <unistd.h>
#include <pthread.h>
#include <random>
#include <string>

using namespace std;

int capacity, np, nc, cntp, cntc;
double mup, muc;
default_random_engine pro_generator;
default_random_engine con_generator;
vector<int> buffer;
vector<thread> producer_thread;
vector<thread> consumer_thread;
int p_index = 0;
int o_index = 0;
ofstream fout,f_out;
sem_t full,empty,mutex;
int p_sum = 0,c_sum = 0;

int produce_item();
int get_time(string a);
string getSysTime();
void producer();
void consumer();

int main()
{
	srand(time(0));

	ifstream fin;
	fin.open("inp-params.txt");
	fin>>capacity>>np>>nc>>cntp>>cntc>>mup>>muc;
	fin.close();

	buffer.resize(capacity);

	sem_init(&empty,0,capacity);
	sem_init(&full,0,0);
	sem_init(&mutex,0,1);

	fout.open("semaphore_output.txt");
	f_out.open("semaphore_stats.txt");

	for(int i = 0; ((i < np)||(i < nc)); i++)
	{
		if(i < np)
			producer_thread.push_back(thread(producer));
		if(i < nc)
			consumer_thread.push_back(thread(consumer));
	}

	for(thread &t : producer_thread)
	{
		t.join();
	}

	for(thread &t : consumer_thread)
	{
		t.join();
	}

	f_out<<"Producer consumer problem implementation semaphores:\n";
	f_out<<"mup = "<<mup<<"; muc = "<<muc<<"; mup/muc = "<<(double(mup)) / muc<<";\n";
	f_out<<"Buffer size :"<<capacity<<";\n";
	f_out<<"No. of producer thread :"<<np<<"; No. of item each producer thread produce :"<<cntp<<";\n";
	f_out<<"No. of consumer thread :"<<nc<<"; No. of item each consumer thread consume :"<<cntc<<";\n";
	f_out<<"Total waiting time involved in producing "<<(np * cntp) <<" items :"<<p_sum<<" seconds.\n";
	f_out<<"Total waiting time involved in consuming "<<(nc * cntc) <<" items :"<<c_sum<<" seconds.\n";
	f_out<<"Average waiting time of producing 1 item :"<<(double(p_sum)) / (np*cntp)<<";\n";
	f_out<<"Average waiting time of consuming 1 item :"<<(double(c_sum)) / (nc*cntc)<<";\n";

	fout.close();
	f_out.close();

	return 0;
}

int produce_item()
{
	int item =  (rand() % 1000);
	return item;
}

void producer()
{
	exponential_distribution<double> pro_distribution(1 / mup);
	thread::id id = this_thread::get_id();
	for(int i = 0; i < cntp; i++)
	{
		int item = produce_item();
		string tempi_time = getSysTime();
		int in = get_time(tempi_time);

		sem_wait(&empty);
		sem_wait(&mutex);

		buffer[p_index] = item;
		string temp_time = getSysTime();
		int f = get_time(temp_time);
		fout<<(i + 1)<<"th item: "<<item <<" produced by thread "<<id<<" at "<<temp_time<<" into buffer location "<<p_index + 1<<".\n";
		p_index = (p_index + 1) % capacity;

		sem_post(&mutex);
		sem_post(&full);

		p_sum = p_sum + (f - in);
		double temp = pro_distribution(pro_generator);
		useconds_t t = temp * 1000000;
		usleep(t);
	}
}

void consumer()
{
	exponential_distribution<double> con_distribution(1 / muc);
	thread::id id = this_thread::get_id();
	for(int i = 0; i < cntc; i++)
	{
		string tempi_time = getSysTime();
		int in = get_time(tempi_time);

		sem_wait(&full);
		sem_wait(&mutex);

		int item = buffer[o_index];
		string temp_time = getSysTime();
		int f = get_time(temp_time);
		fout<<(i + 1)<<"th item: "<<item <<" consumed by thread "<<id<<" at "<<temp_time<<" from buffer location "<<o_index + 1<<".\n";
		o_index = (o_index + 1) % capacity;

		sem_post(&mutex);
		sem_post(&empty);

		c_sum = c_sum + (f - in);
		double temp = con_distribution(con_generator);
		useconds_t t = temp * 1000000;
		usleep(t);
	}
}

string getSysTime()
{
	time_t current_time = time(NULL);
	string temp = ctime(&current_time);
	temp.pop_back();
	return temp;
}

int get_time(string s)
{
	int time_day = ((s[8] - 48) * 10) + (s[9] - 48);
	int time_hr = ((s[11] - 48) * 10) + (s[12] - 48);
	int time_min = ((s[14] - 48) * 10) + (s[15] - 48);
	int time_sec = ((s[17] - 48) * 10) + (s[18] - 48);
	int time_total = (time_day * 3600 * 24) + (time_hr * 3600) + (time_min * 60) + time_sec;
	return time_total;
}