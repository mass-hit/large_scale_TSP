#include "pch.h"

#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS

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
using namespace std;


//视角参数
const float PI = acos(-1);
float viewR = 10, viewPitch = 0.0, viewYaw = 90;   //du是视点和x轴的夹角,hu是视点和水平面的夹角,r是视点绕y轴的半径，
float viewC = PI / 180.0;    //弧度和角度转换参数
float ViewOriH = 0, viewOriX = -1, viewOriY = -1;//视点中心高度
float viewWindowW=1024, viewWindowH=1024;
int GRID_SIZE = 5;  //地板砖边长的一半

//FPS
HWND g_hWnd;
char strFrameRate[15] = "TSP Display";
float framesPerSecond = 0.0f;// This will store our fps
float lastTime = 0.0f;// This will hold the time from the last frame


const int KmeansCenterNum = 3;
const int MaxCityNum = 14;
int InitCityNum = 1;



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
		if (fabs(x-value.x)<=eps && fabs(y - value.y)<=eps && fabs(z - value.z)<=eps)
			return true;
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

class TSP {
public:

	vector<int> VisitOrder;
	vector<int>ReginCitySet[KmeansCenterNum];
	vector<Vector3D> City;
	vector<Vector3D> KmeansCenterPoint;
	float DP[1<<MaxCityNum][MaxCityNum], PreDP[MaxCityNum][MaxCityNum];

	TSP() {
		srand(time(0));
		for (int i = 0; i < InitCityNum; i++) {
			ReginCitySet[0].push_back(i);
			VisitOrder.push_back(i);
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

	float CalPathDistance() {
		float tempLen = 0.0f;
		for (int i = 0; i < City.size(); i++) {
			tempLen += (City[VisitOrder[i]] - City[VisitOrder[(i + 1) % VisitOrder.size()]]).Length();
		}
		return tempLen;
	}

	float DPShortest() {
		for (int i = 0; i < City.size(); i++){
			VisitOrder[i]=i;
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
		DP[1][0] = 0;
		for (int i = 0; i < (1 << City.size()); ++i){
			for (int j = 0; j < City.size(); ++j){
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
			if (DPLen > DP[(1 << City.size()) - 1][i] + PreDP[i][0]) {
				DPLen = DP[(1 << City.size()) - 1][i] + PreDP[i][0];
				VisitOrder[City.size()-1] = i;
			}
		}
		int BackPath = (1 << City.size()) - 1 - (1 << VisitOrder[City.size() - 1]);
		for (int i = City.size() - 2; i >0; --i) {
			int len = 1e10;
			for (int j = City.size(); j > 0; --j) {
				if ((BackPath >> j) & 1 && len > DP[BackPath][j] + PreDP[j][VisitOrder[i+1]]) {
					VisitOrder[i] = j;
					len = DP[BackPath][j] + PreDP[j][VisitOrder[i+1]];
				}
			}
			BackPath = BackPath - (1 << VisitOrder[i]);
		}
		printf("DPShortest:%f ", DPLen);
		for (int i = 0; i < City.size(); i++) {
			printf("%d ", VisitOrder[i]);
		}
		printf("\n");
		return DPLen;
	}

	float PermutationShortest() {
		float PermuLen=1e20;
		vector<int> tmpVisitOrder;
		tmpVisitOrder.assign(VisitOrder.begin(), VisitOrder.end());
		do {
			float tempLen = CalPathDistance();
			if (tempLen < PermuLen) {
				PermuLen = tempLen;
				copy(tmpVisitOrder.begin(), tmpVisitOrder.end(), VisitOrder.begin());
			}
		} while (next_permutation(tmpVisitOrder.begin(), tmpVisitOrder.end()));
		
		printf("PermutationShortest:%f ", PermuLen);
		for (int i = 0; i < City.size(); i++) {
			printf("%d ", VisitOrder[i]);
		}
		printf("\n");
		return PermuLen;
	}

};

TSP TSP1;


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
	}
	glEnd();
}

void DrawCity() {
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	for (int i = 0; i < KmeansCenterNum; i++) {
		for (int j = 0; j < TSP1.ReginCitySet[i].size(); j++) {
			float tmpColor = i*1.0 / KmeansCenterNum;
			glColor3f(0.0f, tmpColor, 1 - tmpColor);
			glVertex3f(TSP1.City[TSP1.ReginCitySet[i][j]].x, TSP1.City[TSP1.ReginCitySet[i][j]].y, TSP1.City[TSP1.ReginCitySet[i][j]].z);
		}
	}
	glEnd();
}

void DrawPath() {
	glBegin(GL_LINE_STRIP);
		glColor3f(1.0f, 1.0f, 0.0f);
		for (int i = 0; i < TSP1.VisitOrder.size(); ++i) {
			glVertex3f(TSP1.City[TSP1.VisitOrder[i]].x, TSP1.City[TSP1.VisitOrder[i]].y, TSP1.City[TSP1.VisitOrder[i]].z);
		}
		glVertex3f(TSP1.City[TSP1.VisitOrder[0]].x, TSP1.City[TSP1.VisitOrder[0]].y, TSP1.City[TSP1.VisitOrder[0]].z);
	glEnd();
}

void CalculateFrameRate() {
	float currentTime = GetTickCount() * 0.001f;
	++framesPerSecond;
	if (currentTime - lastTime > 1.0f) {
		lastTime = currentTime;
		sprintf(strFrameRate, "FPS_%d", int(framesPerSecond));
		SetWindowText(g_hWnd, strFrameRate);
		framesPerSecond = 0;
	}
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除缓冲，GL_COLOR_BUFFER_BIT ：颜色缓冲标志位
	glLoadIdentity();                                       //重置当前矩阵为4*4的单位矩阵
	gluLookAt(viewR * cos(viewC * viewYaw), viewR * cos(viewC * viewPitch), viewR * sin(viewC * viewYaw), 0.0f, ViewOriH, 0.0, 0.0, 1.0, 0.0);   //从视点看远点

	DrawGrid();
	DrawCity();
	DrawPath();
	glutSwapBuffers();       //交换两个缓冲区指针
	CalculateFrameRate();
	if (framesPerSecond > 60.0f) {
		Sleep(50);
	}
}

void onMouseDown(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {  //记录鼠标按下位置
		viewOriX = x, viewOriY = y;
	}

	if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {  //记录鼠标按下位置
		if (TSP1.City.size() >= MaxCityNum) {
			printf("Beyond the calculation scale of DP\n");
		}
		else {
			int tmpCityEndNum = TSP1.City.size();
			TSP1.City.push_back(Vector3D((x - viewWindowW / 2)*(viewR / viewWindowW), ViewOriH, (y - viewWindowH / 2)*(viewR / viewWindowH)));
			TSP1.ReginCitySet[0].push_back(tmpCityEndNum);
			TSP1.VisitOrder.push_back(tmpCityEndNum);
			printf("City%d %f %f %f\n", tmpCityEndNum, TSP1.City[tmpCityEndNum].x, TSP1.City[tmpCityEndNum].y, TSP1.City[tmpCityEndNum].z);
		}
	}
}
void onMouseMove(int x, int y)   //处理鼠标拖动
{
	viewYaw += (x - viewOriX); //鼠标在窗口x轴方向上的增量加到视点与x轴的夹角上，就可以左右转
	viewPitch += (y - viewOriY);  //鼠标在窗口y轴方向上的改变加到视点y的坐标上，就可以上下转
	if (viewPitch > 180)
		viewPitch = 180;
	if (viewPitch < -180)
		viewPitch = -180;
	viewOriX = x, viewOriY = y;  //将此时的坐标作为旧值，为下一次计算增量做准备
}
void onMouseWheel(int button, int dir, int x, int y)//处理滚轮事件
{
	dir > 0 ? viewR++ : viewR--;
	viewR == 0 ? viewR++ : viewR = viewR;
}
void onSpecialKeys(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_UP:
		ViewOriH++; break;
	case GLUT_KEY_DOWN:
		ViewOriH--; break;
	}
}

void onKeyBoards(unsigned char key, int x, int y) {
	switch (key) {
	case '0':
		TSP1.Kmeans();break;
	case '1':
		TSP1.DPShortest();break;
	case '2':
		TSP1.PermutationShortest(); break;
	}
}

void reshape(int w, int h)
{
	viewWindowW = w, viewWindowH = h;
	glViewport(0, 0, w, h);    //截图;1、2为视口的左下角;3、4为视口的宽度和高度
	glMatrixMode(GL_PROJECTION);    //将当前矩阵指定为投影矩阵
	glLoadIdentity();
	gluPerspective(75.0, (float)w / h, 1.0, 1000.0); //1、视野在Y-Z平面的角度[0,180];2、投影平面宽度与高度的比率;3、近截剪面到视点的距离;4、远截剪面到视点的距离
	glMatrixMode(GL_MODELVIEW);     //对模型视景矩阵堆栈应用随后的矩阵操作.
}


int main(int argc, char* argv[])
{
	
	glutInit(&argc, argv);                                          //初始化glut库
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);      //设置初始显示模式
	glEnable(GL_DEPTH_TEST);
	glutInitWindowSize(viewWindowW, viewWindowH);
	glutCreateWindow(strFrameRate);
	g_hWnd = FindWindow(NULL, strFrameRate);

	glutReshapeFunc(reshape);
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);          //设置不断调用显示函数
	glutMouseFunc(onMouseDown);
	glutMotionFunc(onMouseMove);
	glutMouseWheelFunc(onMouseWheel);
	glutSpecialFunc(onSpecialKeys);
	glutKeyboardFunc(onKeyBoards);
	glutMainLoop();//enters the GLUT event processing loop.
	return 0;
}



