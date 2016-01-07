#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>      /* 7/22/2015, added by Leonardo */
#include <utility>
#include <algorithm>
#include <map>

using namespace std;

//typedef pair<int, double>  pairType;  //added by Leonardo

const int USERMAX = 6040;
const int ITEMMAX = 3706;
const int FEATURE = 100;
const int ITERMAX = 60;
const string trainFile = "train.dat";
const string testFile  = "test.dat";

double rating[USERMAX][ITEMMAX];      //用户项目评分矩阵  ,
unsigned long ratingTimes[USERMAX][ITEMMAX];
int I[USERMAX][ITEMMAX];             
double UserF[USERMAX][FEATURE];
double ItemF[ITEMMAX][FEATURE];
double BIASU[USERMAX];
double BIASI[ITEMMAX];
//vector<pairType> vecEntropy;
vector<vector<unsigned long>> vecRatedTimeOfItems;
vector<unsigned long> vecFirstRatedTime;
//vector<int> vecItems;

double lamda = 0.07;
double gamma = 0.021;
double mean;

double predict(int i, int j, unsigned long firstTime, unsigned long currentTime)
{
	double timeliness = exp(-0.004 * (currentTime - firstTime)/86400);     //项目的时效性
	double rate = mean + BIASU[i] + BIASI[j] + timeliness;
	for (int f = 0; f < FEATURE; f++)
		rate += UserF[i][f] * ItemF[j][f];

	if (rate < 1)
		rate = 1;
	else if (rate > 5)
		rate = 5;
	return rate;
}

double calRMSE()
{
	int cnt = 0;
	double total = 0;
	for (int i = 0; i <USERMAX; i++)
	{
		for (int j = 0; j < ITEMMAX; j++)
		{
			if (I[i][j])
			{
				double rate = predict(i,j,vecFirstRatedTime[j], ratingTimes[i][j]);
			    total += (rating[i][j] - rate) * (rating[i][j] - rate);
			    cnt += I[i][j];
			}
			
		}
	}
	double rmse = pow(total / cnt ,0.5);
	return rmse;
}

double calMean()
{
	double total = 0;
	int cnt = 0;
	for (int i = 0; i < USERMAX; i++)
		for (int j = 0; j < ITEMMAX; j++)
		{
			if (I[i][j])
			{
			   total +=  rating[i][j];
			   //cnt += I[i][j];
			   cnt++;
			}
		}
	return total / cnt;
}

/*bool isGreater(const pairType &pairOne, const pairType &pairTwo)
{
	return pairOne.second > pairTwo.second;
}

void calEntropy(const double (*rateMatrix)[ITEMMAX], int userNum, int itemNum)
{
	for (int i = 0; i < userNum; i++) 
	{
		long double entropy = 0; 
		double countRateNum[5];
		double countRateSum = 0.0; 
		memset(countRateNum, 0, sizeof(countRateNum));
		for (int j = 0; j < itemNum; j++)
		{
			if (I[i][j])
			{
				++countRateNum[static_cast<int>(rateMatrix[i][j]) - 1];
				++countRateSum;
			}
		}
		for (int k = 0; k < 5; k++)
		{
			double p = 0;
			if (countRateSum)
				p = countRateNum[k] / countRateSum;
			if(p)
				entropy += p * (-log(p) / log(2));
		}
		vecEntropy.push_back(make_pair(i, entropy));
	}
	cout << "The entropy of all useres have calculated!" << endl;
	sort(vecEntropy.begin(), vecEntropy.end(), isGreater);
	int gtNum = 0;
	for ( vector<pairType>::iterator iter = vecEntropy.begin(); iter != vecEntropy.end(); iter++)
	{
		cout << "User " << (*iter).first + 1 <<"'s " << "entropy is " << (*iter).second << endl;
		if ((*iter).second > 2.1)
			gtNum++;
	//	getchar();
	}
	cout << "The number of users whoes entropy is greater than 2.1: " << gtNum << endl;
} */


void calTimeliness()
{
	unsigned long  halfLife , avrgLife, halfSum = 0, allSum = 0;

	int itemIndex = 0;
	int count = 0;
	for ( vector<vector<unsigned long>>::iterator iter = vecRatedTimeOfItems.begin(); iter != vecRatedTimeOfItems.end(); iter++)
	{
	    sort((*iter).begin(), (*iter).end());
		if ((*iter).size() == 0)
			vecFirstRatedTime.push_back(0);
		else
			vecFirstRatedTime.push_back(*((*iter).begin()));
	    if ((*iter).size() > 20)
		{
		    vector<unsigned long>::iterator midTimeIter, lastTimeIter;
		    midTimeIter = (*iter).begin() + ((*iter).size() / 2);
			lastTimeIter = (*iter).begin() + (*iter).size() - 1;
		    halfSum += (*midTimeIter - *((*iter).begin())) / 86400;
			allSum += (*lastTimeIter - *((*iter).begin())) / 86400;
			count++;
		}
		itemIndex++;
	}
	halfLife = halfSum / count;
	avrgLife = allSum / count;
	cout << "The mean liveTime of items is :" << avrgLife << endl;
	cout << "The mean halfLive of items is :" << halfLife << endl;
	cout << "The counted Item number is " << count << endl;
}


void initBias()
{
	memset(BIASU, 0, sizeof(BIASU));
	memset(BIASI, 0, sizeof(BIASI));
	mean = calMean();
	for (int i = 0; i < USERMAX; i++)
	{
		double total = 0;
		int cnt = 0; 
		for ( int j = 0; j < ITEMMAX; j++)
		{
			if(I[i][j])
			{
				total += rating[i][j] -mean;
				cnt++;
			}
		}
		if (cnt > 0)
			BIASU[i] = total / cnt;
		else
			BIASU[i] = 0;
	}
	for (int j = 0; j < ITEMMAX; j++)
	{
		double total = 0;
		int cnt = 0;
		for (int i = 0; i < USERMAX; i++)
		{
			if (I[i][j])
			{
				total += rating[i][j] - mean;
				cnt++;
			}

		}
		if( cnt > 0)
			BIASI[j] = total / cnt;
		else
			BIASI[j] = 0;
	}
}

void train(const string trainFile)
{
	memset(rating, 0, sizeof(rating));
	memset(ratingTimes, 0, sizeof(ratingTimes));
	memset(I, 0, sizeof(I));
	ifstream in(trainFile);
	if(!in)
	{
		cout<<"File not exit!"<<endl;
		exit(1);
	}
	vecRatedTimeOfItems.resize(ITEMMAX);
	int userId, itemId, rate;
	unsigned long timeStamp;
	while(in>>userId>>itemId>>rate>>timeStamp)
	{
		rating[userId - 1][itemId - 1] = rate;
		ratingTimes[userId - 1][itemId - 1] = timeStamp ;
		I[userId - 1][itemId - 1] = 1;
		vecRatedTimeOfItems[static_cast<vector<vector<unsigned long>>::size_type>(itemId -1)].push_back(timeStamp);
	}   
	initBias();
//	calEntropy(rating, USERMAX, ITEMMAX);
	calTimeliness();
	

	//train matrix decomposation
	for (int i = 0; i < USERMAX; i++)
		for (int f = 0; f < FEATURE; f++)
			UserF[i][f] = (rand() % 10)/10.0;
	for (int j = 0; j < ITEMMAX; j++)
		for (int f = 0; f < FEATURE; f++)
			ItemF[j][f] = (rand() % 10)/10.0;

	int iterCnt = 0;
	while (iterCnt < ITERMAX)
	{
		for (int i = 0; i < USERMAX; i++)
		{
			for ( int j = 0; j < ITEMMAX; j++)
			{
				if (I[i][j])
				{
					double predictRate = predict(i, j, vecFirstRatedTime[j], ratingTimes[i][j]);
					double eui = rating[i][j] - predictRate;
					BIASU[i] += gamma*(eui - lamda*BIASU[i]);
					BIASI[j] += gamma*(eui - lamda*BIASI[j]);
					for (int f = 0; f <FEATURE; f++)
					{
						UserF[i][f] += gamma*(eui*ItemF[j][f] - lamda*UserF[i][f]);
						ItemF[j][f] += gamma*(eui*UserF[i][f] - lamda*ItemF[j][f]);
					}
				}
			}
		}
		
		double rmse = calRMSE();
		
	    cout<<"Loop" << iterCnt<<": rmse is "<< rmse << endl;
	    iterCnt++;
	}
	

}

float test(const string testFile)
{
	ifstream in(testFile);
	if(!in)
	{
		cout << "File not exit!" << endl;
		exit(1);
	}
	int userId, itemId, rate;
	unsigned long timeStamp;
	double total = 0;
	double cnt = 0;
	while(in >> userId >> itemId >> rate >> timeStamp)
	{
		double r = predict(userId - 1, itemId - 1,vecFirstRatedTime[itemId -1], timeStamp);
		total += (r - rate) * (r -rate);
		 cnt += 1;
	}
	float rmse = pow(total /cnt, 0.5);
	cout << "Test rmse is " <<  rmse << endl;
	return rmse;

}
 
int main( int argc , char* argv[])
{
/*	float sumRMSE = 0;
	float meanRMSE;
	
	for (int i = 1; i < 11; i++)
	{
		string trainFile = "train";
	    string testFile = "test";
		ostringstream oss;
		oss.clear();
		oss << i;
		trainFile = trainFile + oss.str() + ".dat";
		testFile = testFile + oss.str() + ".dat";
		cout << trainFile << endl;
		cout << testFile << endl;

		train(trainFile);
		sumRMSE += test(testFile);

	}
	meanRMSE = sumRMSE/ 10;
	cout << "The mean RMSE is: " << meanRMSE << endl;    */
	train(trainFile);
	test(testFile);
	getchar();
	return 0;
}