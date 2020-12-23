﻿#include "pch.h"

#ifndef GLUT_DISABLE_ATEXIT_HACK
#define GLUT_DISABLE_ATEXIT_HACK
#endif
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //for matrices
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <algorithm>
#include <set>
#include<time.h>
#include <vector>
#include <cstring>
#include <iostream>
#include<map>
using namespace std;


class Vector3D
{
public:
	float x;									// the x value of this Vector3D
	float y;									// the y value of this Vector3D
	float z;									// the z value of this Vector3D

	float eps = 1e-8;

	Vector3D()									// Constructor to set x = y = z = 0
	{
		x = 0;
		y = 0;
		z = 0;
	}

	Vector3D(float x, float y, float z)			// Constructor that initializes this Vector3D to the intended values of x, y and z
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3D& operator= (Vector3D v)			// operator= sets values of v to this Vector3D. example: v1 = v2 means that values of v2 are set onto v1
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	Vector3D operator+ (Vector3D v)				// operator+ is used to add two Vector3D's. operator+ returns a new Vector3D
	{
		return Vector3D(x + v.x, y + v.y, z + v.z);
	}

	Vector3D operator- (Vector3D v)				// operator- is used to take difference of two Vector3D's. operator- returns a new Vector3D
	{
		return Vector3D(x - v.x, y - v.y, z - v.z);
	}

	Vector3D operator* (float value)			// operator* is used to scale a Vector3D by a value. This value multiplies the Vector3D's x, y and z.
	{
		return Vector3D(x * value, y * value, z * value);
	}

	Vector3D operator/ (float value)			// operator/ is used to scale a Vector3D by a value. This value divides the Vector3D's x, y and z.
	{
		return Vector3D(x / value, y / value, z / value);
	}

	Vector3D& operator+= (Vector3D v)			// operator+= is used to add another Vector3D to this Vector3D.
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3D& operator-= (Vector3D v)			// operator-= is used to subtract another Vector3D from this Vector3D.
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3D& operator*= (float value)			// operator*= is used to scale this Vector3D by a value.
	{
		x *= value;
		y *= value;
		z *= value;
		return *this;
	}

	Vector3D& operator/= (float value)			// operator/= is used to scale this Vector3D by a value.
	{
		x /= value;
		y /= value;
		z /= value;
		return *this;
	}

	bool operator == (Vector3D value) {
		if (fabs(x-value.x)<=eps && fabs(y - value.y)<=eps && fabs(z - value.z)<=eps)return true;
		return false;
	}


	Vector3D operator- ()						// operator- is used to set this Vector3D's x, y, and z to the negative of them.
	{
return Vector3D(-x, -y, -z);
	}

	float Length()								// length() returns the length of this Vector3D
	{
		return sqrtf(x * x + y * y + z * z);
	};

	void Unitize()								// unitize() normalizes this Vector3D that its direction remains the same but its length is 1.
	{
		float length = this->Length();
		if (length == 0)
			return;
		x /= length;
		y /= length;
		z /= length;
	}

	Vector3D Unit()								// unit() returns a new Vector3D. The returned value is a unitized version of this Vector3D.
	{
		float length = this->Length();
		if (length == 0)
			return *this;
		return Vector3D(x / length, y / length, z / length);
	}

};

float GetRandomFloat(float low, float up) {
	if (low > up) {
		swap(low, up);
	}
	float fResult = low + ((up - low) * rand()) / (RAND_MAX + 1);
	return fResult;
}

//视角参数
const float PI = acos(-1);
float R = 10, hu = 0.0, du = 90, OriX = -1, OriY = -1;   //du是视点和x轴的夹角,hu是视点和水平面的夹角,r是视点绕y轴的半径，
float C = PI / 180.0;    //弧度和角度转换参数
float ViewH = 0;//视点中心高度
int GRID_SIZE = 5;  //地板砖边长的一半

int PathDisplayMode = 1;
const int CityNum = 10;
const int KCityNum = 10;
const int K = 3;
Vector3D City[CityNum];
Vector3D KCity[KCityNum];
vector<int>PreKc[K];
vector<int>NowKc[K];
float OriginLen, PermuLen, AnnealLen, AntcolonyLen, DPLen;
float DP[1 << CityNum][CityNum], PreDP[CityNum][CityNum];
Vector3D Km[K];
int OriginOrder[CityNum];
int DPOrder[CityNum];
int PermuOrder[CityNum];
int AnnealOrder[CityNum];
int AntcolonyOrder[CityNum];

const int KmeansCenterNum = 9;
const int MaxCityNum = 1000;
const int DPCityNum = 9;

float CalPathDistance(int(&CityOrder)[CityNum]) {
	float tempLen = 0.0f;
	for (int i = 0; i < CityNum; i++) {
		tempLen += (City[CityOrder[i]] - City[CityOrder[(i + 1) % CityNum]]).Length();
	}
	return tempLen;
}

void OriginShortest() {
	for (int i = 0; i < CityNum; i++) {
		OriginOrder[i] = i;
	}
	OriginLen = CalPathDistance(OriginOrder);
	printf("OriginShortest:%f ", OriginLen);
	for (int i = 0; i < CityNum; i++) {
		printf("%d ", OriginOrder[i]);
	}
	printf("\n");
}

void PermutationShortest() {
	do {
		float tempLen = CalPathDistance(OriginOrder);
		if (tempLen < PermuLen) {
			PermuLen = tempLen;
			memcpy(PermuOrder, OriginOrder, sizeof(PermuOrder));
		}
	} while (next_permutation(OriginOrder, OriginOrder + CityNum));
	printf("PermutationShortest:%f ", PermuLen);
	for (int i = 0; i < CityNum; i++) {
		printf("%d ", PermuOrder[i]);
	}
	printf("\n");
}

class TSP {
public :
	vector<int> VisitOrder;
	vector<int> ReginCitySet[KmeansCenterNum];
	vector<Vector3D> City;
	vector<Vector3D> KmeansCenterPoint;
	vector<int>Path;
	float DP[1 << DPCityNum][DPCityNum], PreDP[DPCityNum][DPCityNum];
	int to, in;

	void TSPInit() {
		srand(time(0));
		for (int i = 0; i < MaxCityNum; i++) {
			ReginCitySet[0].push_back(i);
			City.push_back(Vector3D(GetRandomFloat(-5.0f, 5.0f), GetRandomFloat(0.0f, 5.0f), GetRandomFloat(-5.0f, 5.0f)));
			printf("City%d %f %f %f\n", i, City[i].x, City[i].y, City[i].z);
		}
	}

	void Kmeans() {
		for (int i = 0; i < KmeansCenterNum; i++) {
			ReginCitySet[i].clear();
		}
		vector<int>tmp;
		for (int i = 0; i < KmeansCenterNum; ++i)
			tmp.push_back(i);
		random_shuffle(tmp.begin(), tmp.end());
		for (int i = 0; i < KmeansCenterNum; ++i) {
			KmeansCenterPoint.push_back(City[tmp[i]]);
		}
		while (true) {
			vector<int>tmpReginCitySet[KmeansCenterNum];
			for (int i = 0; i < City.size(); ++i) {
				int center = 0;
				float minDistance = 1e20;
				for (int j = 0; j < KmeansCenterNum; ++j) {
					float distance = (City[i] - KmeansCenterPoint[j]).Length();
					if (distance < minDistance) {
						center = j;
						minDistance = distance;
					}
				}
				tmpReginCitySet[center].push_back(i);
			}
			for (int i = 0; i < KmeansCenterNum; ++i) {
				Vector3D tmpTot;
				for (int j = 0; j < tmpReginCitySet[i].size(); ++j) {
					tmpTot += City[tmpReginCitySet[i][j]];
				}
				KmeansCenterPoint[i] = tmpTot / tmpReginCitySet[i].size();
			}
			int flag = 0;
			for (int i = 0; i < KmeansCenterNum; ++i) {
				if (tmpReginCitySet[i] != ReginCitySet[i]) {
					flag = 1;
					break;
				}
			}
			if (!flag)
				break;
			for (int i = 0; i < KmeansCenterNum; ++i)
				swap(tmpReginCitySet[i], ReginCitySet[i]);
		}
		for (int i = 0; i < KmeansCenterNum; ++i) {
			printf("Region%d:", i);
			for (int j = 0; j < ReginCitySet[i].size(); ++j) {
				printf("%d ", ReginCitySet[i][j]);
			}
			printf("\n");
		}
	}

	float DPShortest() {
		for (int i = 0; i < KmeansCenterPoint.size(); i++) {
			VisitOrder.push_back(i);
		}
		float DPLen = 1e20;
		for (int i = 0; i < KmeansCenterPoint.size(); ++i) {
			for (int j = 0; j < KmeansCenterPoint.size(); ++j) {
				PreDP[i][j] = (KmeansCenterPoint[i] - KmeansCenterPoint[j]).Length();
			}
		}
		for (int i = 0; i < (1 << KmeansCenterPoint.size()); ++i) {
			for (int j = 0; j < KmeansCenterPoint.size(); ++j) {
				DP[i][j] = 1e10;
			}
		}
		DP[1][0] = 0;
		for (int i = 0; i < (1 << KmeansCenterPoint.size()); ++i) {
			for (int j = 0; j < KmeansCenterPoint.size(); ++j) {
				if (i >> j & 1) {
					for (int k = 0; k < KmeansCenterPoint.size(); ++k) {
						if ((i - (1 << j)) >> k & 1) {
							DP[i][j] = min(DP[i][j], DP[i - (1 << j)][k] + PreDP[k][j]);
						}
					}
				}
			}
		}
		for (int i = 0; i < KmeansCenterPoint.size(); ++i) {
			if (DPLen > DP[(1 << KmeansCenterPoint.size()) - 1][i] + PreDP[i][0]) {
				DPLen = DP[(1 << KmeansCenterPoint.size()) - 1][i] + PreDP[i][0];
				VisitOrder[KmeansCenterPoint.size() - 1] = i;
			}
		}
		int BackPath = (1 << KmeansCenterPoint.size()) - 1 - (1 << VisitOrder[KmeansCenterPoint.size() - 1]);
		for (int i = KmeansCenterPoint.size() - 2; i > 0; --i) {
			int len = 1e10;
			for (int j = KmeansCenterPoint.size(); j > 0; --j) {
				if ((BackPath >> j) & 1 && len > DP[BackPath][j] + PreDP[j][VisitOrder[i + 1]]) {
					VisitOrder[i] = j;
					len = DP[BackPath][j] + PreDP[j][VisitOrder[i + 1]];
				}
			}
			BackPath = BackPath - (1 << VisitOrder[i]);
		}
		printf("DPShortest:%f ", DPLen);
		for (int i = 0; i < KmeansCenterPoint.size(); i++) {
			printf("%d ", VisitOrder[i]);
		}
		printf("\n");
		return DPLen;
	}

	float DPShortestNoCircleMid(int start,int end) {
		for (int i = 0; i < KmeansCenterPoint.size(); i++) {
			VisitOrder.push_back(i);
		}
		float DPLen = 1e20;
		for (int i = 0; i < KmeansCenterPoint.size(); ++i) {
			for (int j = 0; j < KmeansCenterPoint.size(); ++j) {
				PreDP[i][j] = (KmeansCenterPoint[i] - KmeansCenterPoint[j]).Length();
			}
		}
		for (int i = 0; i < (1 << KmeansCenterPoint.size()); ++i) {
			for (int j = 0; j < KmeansCenterPoint.size(); ++j) {
				DP[i][j] = 1e10;
			}
		}
		DP[1<<start][start] = 0;
		for (int i = 0; i < (1 << KmeansCenterPoint.size()); ++i) {
			for (int j = 0; j < KmeansCenterPoint.size(); ++j) {
				if (j == start || j == end)continue;
				if (i >> j & 1) {
					for (int k = 0; k < KmeansCenterPoint.size(); ++k) {
						if ((i - (1 << j)) >> k & 1) {
							DP[i][j] = min(DP[i][j], DP[i - (1 << j)][k] + PreDP[k][j]);
						}
					}
				}
			}
		}
		for (int i = 0; i < KmeansCenterPoint.size(); ++i) {
			if (i == start || i == end)continue;
			if (DPLen > DP[(1 << KmeansCenterPoint.size()) - 1-(1<<end)][i] + PreDP[i][end]) {
				DPLen = DP[(1 << KmeansCenterPoint.size()) - 1-(1<<end)][i] + PreDP[i][end];
				VisitOrder[KmeansCenterPoint.size() - 2] = i;
			}
		}
		VisitOrder[KmeansCenterPoint.size() - 1] = end;
		int BackPath = (1 << KmeansCenterPoint.size()) - 1 - (1 << VisitOrder[KmeansCenterPoint.size() - 1]) - (1 << VisitOrder[KmeansCenterPoint.size() - 2]);
		for (int i = KmeansCenterPoint.size() - 3; i > 0; --i) {
			int len = 1e10;
			for (int j = KmeansCenterPoint.size(); j >= 0; --j) {
				if (j == start || j == end)continue;
				if ((BackPath >> j) & 1 && len > DP[BackPath][j] + PreDP[j][VisitOrder[i + 1]]) {
					VisitOrder[i] = j;
					len = DP[BackPath][j] + PreDP[j][VisitOrder[i + 1]];
				}
			}
			BackPath = BackPath - (1 << VisitOrder[i]);
		}
		VisitOrder[0] = start;
		printf("DPShortestNoCircleMid:%f ", DPLen);
		for (int i = 0; i < KmeansCenterPoint.size(); i++) {
			printf("%d ", VisitOrder[i]);
		}
		printf("\n");
		return DPLen;
	}

	float DPShortestNoCircle(int start,int end) {
		for (int i = 0; i < City.size(); i++) {
			VisitOrder.push_back(i);
		}
		if (City.size() < 2) {
			ReginCitySet[0].push_back(0);
			return 0.0;
		}
		if (City.size() == 2) {
			VisitOrder[0] = start;
			VisitOrder[1] = end;
			ReginCitySet[0].push_back(start);
			ReginCitySet[1].push_back(end);
			return (City[start] - City[end]).Length();
		}
		float DPLen = 1e20;
		for (int i = 0; i < City.size(); ++i) {
			for (int j = 0; j < City.size(); ++j) {
				PreDP[i][j] = (City[i] - City[j]).Length();
			}
		}
		for (int i = 0; i < (1 << City.size()); ++i) {
			for (int j = 0; j < City.size(); ++j) {
				DP[i][j] = 1e10;
			}
		}
		DP[1<<start][start] = 0;
		for (int i = 0; i < (1 << City.size()); ++i) {
			for (int j = 0; j < City.size(); ++j) {
				if (i >> j & 1) {
					for (int k = 0; k < City.size(); ++k) {
						if ((i - (1 << j)) >> k & 1) {
							DP[i][j] = min(DP[i][j], DP[i - (1 << j)][k] + PreDP[k][j]);
						}
					}
				}
			}
		}
		for (int i = 0; i < City.size(); ++i) {
			if (DPLen > DP[(1 << City.size()) - 1 - (1 << end)][i] + PreDP[i][end]) {
				DPLen = DP[(1 << City.size()) - 1 - (1 << end)][i] + PreDP[i][end];
				VisitOrder[City.size() - 2] = i;
			}
		}
		VisitOrder[City.size() - 1] = end;
		int BackPath = (1 << City.size()) - 1 - (1 << VisitOrder[City.size() - 1]) - (1 << VisitOrder[City.size() - 2]);
		for (int i = City.size() - 3; i > 0; --i) {
			int len = 1e10;
			for (int j = City.size(); j >= 0; --j) {
				if (j == start || j == end)continue;
				if ((BackPath >> j) & 1 && len > DP[BackPath][j] + PreDP[j][VisitOrder[i + 1]]) {
					VisitOrder[i] = j;
					len = DP[BackPath][j] + PreDP[j][VisitOrder[i + 1]];
				}
			}
			BackPath = BackPath - (1 << VisitOrder[i]);
		}
		VisitOrder[0] = start;
		printf("DPShortestNoCircle:%f ", DPLen);
		for (int i = 0; i < City.size(); i++) {
			printf("%d ", VisitOrder[i]);
			ReginCitySet[i].push_back(VisitOrder[i]);
		}
		printf("\n");
		return DPLen;
	}
};

void DFS(int deep, TSP& tsp, int start, int end) {
	if (tsp.City.size() < DPCityNum) {
		tsp.DPShortestNoCircle(start,end);
		for (int i = 0; i < tsp.City.size(); ++i)
			tsp.Path.push_back(tsp.VisitOrder[i]);
		return;
	}
	else {
		tsp.Kmeans();
		if (!deep) {
			tsp.DPShortest();
			pair<int, int>last;
			pair<int, int> to;
			pair<int, int> in;
			for (int i = 0; i < KmeansCenterNum; ++i) {
				double toDistance = 1e10;
				double inDistance = 1e10;
				if (!i) {
					for (int j = 0; j < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++j) {
						for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i + 1]].size(); ++k)
							if ((tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length() < toDistance) {
								to.first = j;
								to.second = k;
								toDistance = (tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length();
							}
					}
					for (int j = 0; j < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++j) {
						for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[KmeansCenterNum - 1]].size(); ++k) {
							if ((tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[KmeansCenterNum - 1]][k]]).Length() < inDistance) {
								if (j == to.first)continue;
								in.first = k;
								in.second = j;
								last.first = k;
								last.second = j;
								inDistance = (tsp, City[tsp.ReginCitySet[i][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[KmeansCenterNum - 1]][k]]).Length();
							}
						}
					}
					TSP Ntsp;
					for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++k) {
						Ntsp.City.push_back(tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][k]]);
					}
					DFS(deep + 1, Ntsp, in.second, to.first);
					for (int j = 0; j < Ntsp.City.size(); ++j)
						tsp.Path.push_back(tsp.ReginCitySet[tsp.VisitOrder[i]][Ntsp.Path[j]]);
				}
				else if (i == KmeansCenterNum - 2) {
					for (int j = 0; j < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++j)
						for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i + 1]].size(); ++k)
							if ((tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length() < toDistance) {
								if (j == in.second || k == last.first)continue;
								to.first = j;
								to.second = k;
								toDistance = (tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length();
							}
					TSP Ntsp;
					for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++k) {
						Ntsp.City.push_back(tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][k]]);
					}
					DFS(deep + 1, Ntsp, in.second, to.first);
					for (int j = 0; j < Ntsp.City.size(); ++j)
						tsp.Path.push_back(tsp.ReginCitySet[tsp.VisitOrder[i]][Ntsp.Path[j]]);
				}
				else if (i == KmeansCenterNum - 1) {
					TSP Ntsp;
					for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++k) {
						Ntsp.City.push_back(tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][k]]);
					}
					DFS(deep + 1, Ntsp, in.second, last.first);
					for (int j = 0; j < Ntsp.City.size(); ++j)
						tsp.Path.push_back(tsp.ReginCitySet[tsp.VisitOrder[i]][Ntsp.Path[j]]);
				}
				else {
					for (int j = 0; j < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++j)
						for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i + 1]].size(); ++k)
							if ((tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length() < toDistance) {
								if (j == in.second)continue;
								to.first = j;
								to.second = k;
								toDistance = (tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length();
							}
					TSP Ntsp;
					for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++k) {
						Ntsp.City.push_back(tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][k]]);
					}
					DFS(deep + 1, Ntsp, in.second, to.first);
					for (int j = 0; j < Ntsp.City.size(); ++j)
						tsp.Path.push_back(tsp.ReginCitySet[tsp.VisitOrder[i]][Ntsp.Path[j]]);
				}
				swap(in, to);
			}
		}
		else {
			int nowStart = 0;
			int nowEnd = 0;
			for (int i = 0; i < KmeansCenterNum; ++i)
				for (int j = 0; j < tsp.ReginCitySet[i].size(); ++j) {
					if (tsp.ReginCitySet[i][j] == start)
						nowStart = i;
					if (tsp.ReginCitySet[i][j] == end)
						nowEnd = i;
				}
			if(nowStart==nowEnd)
				for(int i=0;i<KmeansCenterNum;++i)
					if (i != nowStart) {
						nowEnd = i;
						break;
					}
			tsp.DPShortestNoCircleMid(nowStart, nowEnd);
			pair<int, int> to;
			pair<int, int> in;
			for (int i = 0; i < KmeansCenterNum; ++i) {
				double toDistance = 1e10;
				double inDistance = 1e10;
				if (!i) {
					int s = 0;
					for (int j = 0; j < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++j)
						for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i + 1]].size(); ++k)
							if ((tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length() < toDistance) {
								if (tsp.ReginCitySet[tsp.VisitOrder[i]][j] == start)continue;
								to.first = j;
								to.second = k;
								toDistance = (tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length();
							}
					TSP Ntsp;
					for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++k) {
						if (start == tsp.ReginCitySet[tsp.VisitOrder[i]][k])
							s = k;
						Ntsp.City.push_back(tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][k]]);
					}
					DFS(deep + 1, Ntsp, s, to.first);
					for (int j = 0; j < Ntsp.City.size(); j++)
							tsp.Path.push_back(tsp.ReginCitySet[tsp.VisitOrder[i]][Ntsp.Path[j]]);
				}
				else if (i == KmeansCenterNum - 2) {
					int e = 0;
					to = make_pair(0, 0);
					for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i + 1]].size(); ++k) {
						if (end == tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k])
							e = k;
					}
					for (int j = 0; j < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++j)
						for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i + 1]].size(); ++k)
							if ((tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length() < toDistance) {
								if (k == e || j == in.second)continue;
								to.first = j;
								to.second = k;
								toDistance = (tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length();
							}
					TSP Ntsp;
					for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++k) {
						Ntsp.City.push_back(tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][k]]);
					}
					DFS(deep + 1, Ntsp, in.second, to.first);
					for (int j = 0; j < Ntsp.City.size(); ++j)
						tsp.Path.push_back(tsp.ReginCitySet[tsp.VisitOrder[i]][Ntsp.Path[j]]);
				}
				else if (i == KmeansCenterNum - 1) {
					TSP Ntsp;
					int e = 0;
					for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++k) {
						if (end == tsp.ReginCitySet[tsp.VisitOrder[i]][k])
							e = k;
						Ntsp.City.push_back(tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][k]]);
					}
					DFS(deep + 1, Ntsp, in.second, e);
					for (int j = 0; j < Ntsp.City.size(); ++j)
						tsp.Path.push_back(tsp.ReginCitySet[tsp.VisitOrder[i]][Ntsp.Path[j]]);
				}
				else {
					to = make_pair(0, 0);
					for (int j = 0; j < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++j)
						for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i + 1]].size(); ++k)
							if ((tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length() < toDistance) {
								if (j == in.second)continue;
								to.first = j;
								to.second = k;
								toDistance = (tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][j]] - tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i + 1]][k]]).Length();
							}
					TSP Ntsp;
					int s = 0, e = 0;
					for (int k = 0; k < tsp.ReginCitySet[tsp.VisitOrder[i]].size(); ++k)
						Ntsp.City.push_back(tsp.City[tsp.ReginCitySet[tsp.VisitOrder[i]][k]]);
					DFS(deep + 1, Ntsp, in.second, to.first);
					for (int j = 0; j < Ntsp.City.size(); ++j)
						tsp.Path.push_back(tsp.ReginCitySet[tsp.VisitOrder[i]][Ntsp.Path[j]]);
				}
				swap(in, to);
			}
		}
	}
}

void AnnealShortest() {
	float TperaNow = 1, TperaEnd = 10e-20, CoolRate = 0.99, MaxIter = CityNum*100;
	for (int i = 0; i < MaxIter; i++) {
		int ExcPosHe = (int)GetRandomFloat(1, CityNum);
		int ExcPosTa = (int)GetRandomFloat(1, CityNum);
		if (ExcPosHe > ExcPosTa) {
			swap(ExcPosHe , ExcPosTa);
		}

		float DeltaLen= (City[AnnealOrder[ExcPosHe-1]] - City[AnnealOrder[ExcPosTa]]).Length();
		DeltaLen += (City[AnnealOrder[ExcPosHe]] - City[AnnealOrder[(ExcPosTa+1)%CityNum]]).Length();
		DeltaLen -= (City[AnnealOrder[ExcPosHe - 1]] - City[AnnealOrder[ExcPosHe]]).Length();
		DeltaLen -= (City[AnnealOrder[ExcPosTa]] - City[AnnealOrder[(ExcPosTa+1) % CityNum]]).Length();
		if ((DeltaLen < 0)|| (exp(-1 * DeltaLen / TperaNow)>GetRandomFloat(0, 1))) {
			for (int i = ExcPosHe, j = ExcPosTa; i < j; i++, j--) {
				swap(AnnealOrder[i], AnnealOrder[j]);
			}
			AnnealLen += DeltaLen;
		}
		TperaEnd *= CoolRate;
		if (TperaNow < TperaEnd) {
			break;
		}
	}
	printf("AnnealShortest:%f ", AnnealLen);
	for (int i = 0; i < CityNum; i++) {
		printf("%d ", AnnealOrder[i]);
	}
	printf("\n");
}

void AntcolonyShortest() {
	const int AntNum = CityNum*10,MaxIter= CityNum*10;
	float InformFact = 1.0f, RandFact = 1.0f, RandVola = 0.2f, perAntInformC = 10;
	int AntPath[AntNum][CityNum] = { 0 }, InformResi[CityNum][CityNum];
	float AntPathLen[AntNum], AntPathLenMin = RAND_MAX * 1.0f;
	for (int i = 0; i < CityNum; i++) {
		for (int j = 0; j < CityNum; j++) {
			InformResi[i][j] = 1.5f;
		}
	}

	for (int i = 0; i < MaxIter; i++) {
		for (int j = 0; j < AntNum; j++) {
			vector<int> CityResi;
			for (int k = 0; k < CityNum; k++) {
				CityResi.push_back(k);
			}
			int CityNowIdx = (int)GetRandomFloat(0, CityNum);
			AntPath[j][0] = CityResi[CityNowIdx];
			CityResi.erase(CityResi.begin() + CityNowIdx);
			for (int k = 0; k < CityNum-1; k++) {
				vector<float> CityResiPosib;
				float PosibSum = 0.0f;
				for (int m = 0; m < CityResi.size(); m++) {
					float tmpPosib = pow(InformResi[k][m], InformFact)*pow(1.0f / (City[AntPath[j][k]] - City[CityResi[m]]).Length(), RandFact);
					PosibSum += tmpPosib;
					CityResiPosib.push_back(tmpPosib);
				}
				float PosibRandGet = GetRandomFloat(0, PosibSum);
				PosibSum = 0.0f;
				for (int m = 0; m <CityResi.size(); m++) {
					PosibSum += CityResiPosib[m];
					//printf("%f...%f\n", PosibSum, PosibRandGet);
					if ((PosibSum > PosibRandGet)||(m== CityResi.size()-1)) {
						AntPath[j][k + 1] = CityResi[m];
						CityResi.erase(CityResi.begin() + m);
						break;
					}
				}
			}
		}

		
		for (int j = 0; j < AntNum; j++) {
			AntPathLen[j] = CalPathDistance(AntPath[j]);
			if (AntPathLen[j] < AntcolonyLen) {
				AntcolonyLen = AntPathLen[j];
				memcpy(AntcolonyOrder, AntPath[j],sizeof(AntcolonyOrder));
			}
		}

		for (int j = 0; j < CityNum; j++) {
			for (int k = 0; k < CityNum; k++) {
				InformResi[j][k] *= (1 - RandVola);
			}
		}
		for (int j = 0; j < AntNum; j++) {
			for (int k = 0; k < CityNum; k++) {
				InformResi[AntPath[j][k]][AntPath[j][(k + 1)%CityNum]] +=
					(perAntInformC* (City[AntPath[j][k]] - City[AntPath[j][(k + 1) % CityNum]]).Length() / AntPathLen[j]);
			}
		}
	}
	printf("AntcolonyShortest:%f ", AntcolonyLen);
	for (int i = 0; i < CityNum; i++) {
		printf("%d ", AntcolonyOrder[i]);
	}
	printf("\n");
}

void DrawGrid()  //画地板
{
	glBegin(GL_LINES);
	glColor3f(0.5f, 0.5f, 0.5f);
	for (int i = -GRID_SIZE; i <= GRID_SIZE; i++)
	{
		glVertex3f((float)i, 0.0f, (float)-GRID_SIZE);
		glVertex3f((float)i, 0.0f, (float)GRID_SIZE);

		glVertex3f((float)-GRID_SIZE, 0.0f, (float)i);
		glVertex3f((float)GRID_SIZE, 0.0f, (float)i);

		//glVertex3f((float)GRID_SIZE, 0.0f, (float)i);
		//glVertex3f((float)GRID_SIZE, GRID_SIZE, (float)i);

		//glVertex3f((float)-GRID_SIZE, 0.0f, (float)i);
		//glVertex3f((float)-GRID_SIZE, GRID_SIZE, (float)i);

		//glVertex3f((float)i, 0.0f, (float)GRID_SIZE);
		//glVertex3f((float)i, GRID_SIZE, (float)GRID_SIZE);

		//glVertex3f((float)i, 0.0f, (float)-GRID_SIZE);
		//glVertex3f((float)i, GRID_SIZE, (float)-GRID_SIZE);
	}
	glEnd();
}

void DrawCity() {
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	/*for (int i = 0; i < CityNum; i++) {
		glColor3f(1.0f, 0.0f, 0.0f);
		if (i == 0 || i == CityNum-1) {
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(City[i].x, City[i].y, City[i].z);
			continue;
		}
		glVertex3f(City[i].x, City[i].y, City[i].z);
	}*/
	glEnd();
}

void DrawPath(int Mode) {
	glBegin(GL_LINE_STRIP);
	switch (Mode) {
	case 1://Origin
		glColor3f(0.0f, 1.0f, 0.0f);
		for (int i = 0; i < CityNum; i++) {
			glVertex3f(City[OriginOrder[i]].x, City[OriginOrder[i]].y, City[OriginOrder[i]].z);
		}
		glVertex3f(City[OriginOrder[0]].x, City[OriginOrder[0]].y, City[OriginOrder[0]].z);
		break;
	case 2://Permutation
		glColor3f(0.0f, 1.0f, 0.5f);
		for (int i = 0; i < CityNum; i++) {
			glVertex3f(City[PermuOrder[i]].x, City[PermuOrder[i]].y, City[PermuOrder[i]].z);
		}
		glVertex3f(City[PermuOrder[0]].x, City[PermuOrder[0]].y, City[PermuOrder[0]].z);
		break;
	case 3://Anneal
		glColor3f(0.0f, 1.0f, 1.0f);
		for (int i = 0; i < CityNum; i++) {
			glVertex3f(City[AnnealOrder[i]].x, City[AnnealOrder[i]].y, City[AnnealOrder[i]].z);
		}
		glVertex3f(City[AnnealOrder[0]].x, City[AnnealOrder[0]].y, City[AnnealOrder[0]].z);
		break;
	case 4://Antcolony
		glColor3f(0.5f, 1.0f, 0.0f);
		for (int i = 0; i < CityNum; i++) {
			glVertex3f(City[AntcolonyOrder[i]].x, City[AntcolonyOrder[i]].y, City[AntcolonyOrder[i]].z);
		}
		glVertex3f(City[AntcolonyOrder[0]].x, City[AntcolonyOrder[0]].y, City[AntcolonyOrder[0]].z);
		break;
	case 5://DP
		glColor3f(1.0f, 1.0f, 0.0f);
		for (int i = 0; i < CityNum; ++i) {
			glVertex3f(City[DPOrder[i]].x, City[DPOrder[i]].y, City[DPOrder[i]].z);
		}
		glVertex3f(City[DPOrder[0]].x, City[DPOrder[0]].y, City[DPOrder[0]].z);
		break;
	default:
		printf("No %d Mode\n", Mode);
		break;
	}
	glEnd();
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除缓冲，GL_COLOR_BUFFER_BIT ：颜色缓冲标志位
	glLoadIdentity();                                       //重置当前矩阵为4*4的单位矩阵
	gluLookAt(R * cos(C * du), R * cos(C * hu), R * sin(C * du), 0.0f, ViewH, 0.0, 0.0, 1.0, 0.0);   //从视点看远点

	DrawGrid();
	DrawCity();
	//DrawPath(PathDisplayMode);

	glutSwapBuffers();                                      //交换两个缓冲区指针
}

void onMouseDown(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)  //记录鼠标按下位置
		OriX = x, OriY = y;
}
void onMouseMove(int x, int y)   //处理鼠标拖动
{
	du += (x - OriX); //鼠标在窗口x轴方向上的增量加到视点与x轴的夹角上，就可以左右转
	hu += (y - OriY);  //鼠标在窗口y轴方向上的改变加到视点y的坐标上，就可以上下转
	if (hu > 180)
		hu = 180;
	if (hu < -180)
		hu = -180;
	OriX = x, OriY = y;  //将此时的坐标作为旧值，为下一次计算增量做准备
}
void onMouseWheel(int button, int dir, int x, int y)//处理滚轮事件
{
	dir > 0 ? R++ : R--;
	R == 0 ? R++ : R = R;
}
void onSpecialKeys(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_UP:
		ViewH++; break;
	case GLUT_KEY_DOWN:
		ViewH--; break;
	}
}

void onKeyBoards(unsigned char key, int x, int y) {
	switch (key) {
	case '0':
		GetRandomFloat(-5, 5); break;
	case '1':
		OriginShortest();
		PathDisplayMode = 1; break;
	case '2':
		PermutationShortest();
		PathDisplayMode = 2; break;
	case '3':
		AnnealShortest();
		PathDisplayMode = 3; break;
	case '4':
		AntcolonyShortest();
		PathDisplayMode = 4; break;
	}
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);    //截图;1、2为视口的左下角;3、4为视口的宽度和高度
	glMatrixMode(GL_PROJECTION);    //将当前矩阵指定为投影矩阵
	glLoadIdentity();
	gluPerspective(75.0, (float)w / h, 1.0, 1000.0); //1、视野在Y-Z平面的角度[0,180];2、投影平面宽度与高度的比率;3、近截剪面到视点的距离;4、远截剪面到视点的距离
	glMatrixMode(GL_MODELVIEW);     //对模型视景矩阵堆栈应用随后的矩阵操作.
}
TSP tsp;
int main(int argc, char* argv[])
{
	//glutInit(&argc, argv);                                          //初始化glut库
	//glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);      //设置初始显示模式
	//glEnable(GL_DEPTH_TEST);
	//glutInitWindowSize(1024, 1024);
	//glutCreateWindow("Mass-Spring Model");

	//
	//glutReshapeFunc(reshape);
	//glutDisplayFunc(renderScene);
	//glutIdleFunc(renderScene);          //设置不断调用显示函数
	//glutMouseFunc(onMouseDown);
	//glutMotionFunc(onMouseMove);
	//glutMouseWheelFunc(onMouseWheel);
	//glutSpecialFunc(onSpecialKeys);
	//glutKeyboardFunc(onKeyBoards);
	//glutMainLoop();//enters the GLUT event processing loop.
	tsp.TSPInit();
	DFS(0, tsp, 0, KmeansCenterNum - 1);
	for (int i = 0; i < tsp.City.size(); ++i)
		printf("%d ", tsp.Path[i]);
	return 0;
}



