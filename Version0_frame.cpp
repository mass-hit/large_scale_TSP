#include "pch.h"

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
#include <vector>
#include <cstring>
#include <iostream>
using namespace std;


class Vector3D
{
public:
	float x;									// the x value of this Vector3D
	float y;									// the y value of this Vector3D
	float z;									// the z value of this Vector3D

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


	Vector3D operator- ()						// operator- is used to set this Vector3D's x, y, and z to the negative of them.
	{
		return Vector3D(-x, -y, -z);
	}

	float Length()								// length() returns the length of this Vector3D
	{
		return sqrtf(x*x + y * y + z * z);
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

class MassPoint {
public:
	float m;									// The mass value
	Vector3D pos;								// Position in space
	Vector3D vel;								// Velocity
	Vector3D force;								// Force applied on this mass at an instance


	void Initial(float m, Vector3D pos, Vector3D vel)								// Constructor
	{
		this->m = m;
		this->pos = pos;
		this->vel = vel;
		force.x = 0;
		force.y = 0;
		force.z = 0;
	}
};


class Stem {
public:
	//茎秆参数
	float StemLen ;
	int FloorNum ;
	float SideLen ;
	float FloorHeight ;
	float StemWeight ;
	float MassWeight ;

	MassPoint mass[20 + 1][3];
	int FPS = 9000;//帧率
	float FrameTime = 1.0f / FPS;//时间片长度
	float VelDamp = 0.98f;//速度衰减参数
	int FlashTime = 0;//速度衰减计数
	float TremorHold = 50.0f;//震颤阈值
	float OvershootHold = 5e+20f;//超弹阈值

	//顶端作用力
	Vector3D deviaforce = Vector3D(0.0f, 0.0f, 0.0f);

	//弹性系数
	const float K1 = 1000;//结构弹簧
	const float K2 = 10000;//拉伸弹簧
	const float K3 = 100;//扭转弹簧
	const float K4 = 10000;//弯曲弹簧

	Stem(float StemLen,int FloorNum,float SideLen,float StemWeight){
		this->StemLen = StemLen;
		this->FloorNum = FloorNum;
		this->SideLen = SideLen;
		this->StemWeight = StemWeight;
		this->MassWeight = StemWeight / (3 * (FloorNum + 1));
		this->FloorHeight = StemLen / FloorNum;
	}

	void DrawStem()  //画茎秆
	{
		//glTranslatef(BuildPos.x, BuildPos.y, BuildPos.z);
		glLineWidth(1.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		for (int i = 0; i < FloorNum + 1; i++)
		{
			glColor3f(0.0f, 1.0f, 0.0f);
			//x轴直角边
			glVertex3f(mass[i][0].pos.x, mass[i][0].pos.y, mass[i][0].pos.z);
			glVertex3f(mass[i][1].pos.x, mass[i][1].pos.y, mass[i][1].pos.z);
			//y轴直角边
			glVertex3f(mass[i][0].pos.x, mass[i][0].pos.y, mass[i][0].pos.z);
			glVertex3f(mass[i][2].pos.x, mass[i][2].pos.y, mass[i][2].pos.z);
			//斜边
			glVertex3f(mass[i][2].pos.x, mass[i][2].pos.y, mass[i][2].pos.z);
			glVertex3f(mass[i][1].pos.x, mass[i][1].pos.y, mass[i][1].pos.z);
			glColor3f(0.0f, 1.0f, 1.0f);
			//拉伸弹簧
			for (int j = 0; j < 3 && i != FloorNum; j++) {
				glVertex3f(mass[i][j].pos.x, mass[i][j].pos.y, mass[i][j].pos.z);
				glVertex3f(mass[i + 1][j].pos.x, mass[i + 1][j].pos.y, mass[i + 1][j].pos.z);
			}
			//扭转弹簧
			for (int j = 0; j < 3 && i != FloorNum; j++) {
				glVertex3f(mass[i][j].pos.x, mass[i][j].pos.y, mass[i][j].pos.z);
				glVertex3f(mass[i + 1][(j + 1) % 3].pos.x, mass[i + 1][(j + 1) % 3].pos.y, mass[i + 1][(j + 1) % 3].pos.z);
				glVertex3f(mass[i][j].pos.x, mass[i][j].pos.y, mass[i][j].pos.z);
				glVertex3f(mass[i + 1][(j + 2) % 3].pos.x, mass[i + 1][(j + 2) % 3].pos.y, mass[i + 1][(j + 2) % 3].pos.z);
			}
		}
		glEnd();

		//弯曲弹簧
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		for (int i = 0; i < FloorNum - 1; i++) {
			for (int j = 0; j < 3; j++) {
				glVertex3f(mass[i][j].pos.x, mass[i][j].pos.y, mass[i][j].pos.z);
				glVertex3f(mass[i + 2][j].pos.x, mass[i + 2][j].pos.y, mass[i + 2][j].pos.z);
			}
		}
		glEnd();
		//glTranslatef(-BuildPos.x, -BuildPos.y, -BuildPos.z);
	}
	void ForceCal(int a, int b) {
		//gravity
		mass[a][b].force += Vector3D(0.0f, (mass[a][b].pos.y > 0 ? 1 : -1)*-9.8f*MassWeight,  0.0f);
		//结构弹簧
		mass[a][b].force += (mass[a][(b + 1) % 3].pos - mass[a][b].pos).Unit()*(abs((mass[a][b].pos - mass[a][(b + 1) % 3].pos).Length()) - SideLen)*K1;
		mass[a][b].force += (mass[a][(b + 2) % 3].pos - mass[a][b].pos).Unit()*(abs((mass[a][b].pos - mass[a][(b + 2) % 3].pos).Length()) - SideLen)*K1;
		//printf("%f,%f,%f\n", mass[a][b].force.x, mass[a][b].force.y, mass[a][b].force.z);
		//printf("%f,%f,%f\n", mass[a][b].vel.x, mass[a][b].vel.y, mass[a][b].vel.z);

		//拉伸弹簧
		if (a != FloorNum)
			mass[a][b].force += (mass[a + 1][b].pos - mass[a][b].pos).Unit()*(abs((mass[a + 1][b].pos - mass[a][b].pos).Length()) - FloorHeight)*K2;
		if (a != 0)
			mass[a][b].force += (mass[a - 1][b].pos - mass[a][b].pos).Unit()*(abs((mass[a - 1][b].pos - mass[a][b].pos).Length()) - FloorHeight)*K2;

		//扭曲弹簧
		float DiagLen = sqrt(FloorHeight*FloorHeight + SideLen * SideLen);
		if (a != FloorNum)//左边的
			mass[a][b].force += (mass[a + 1][(b + 1) % 3].pos - mass[a][b].pos).Unit()*(abs((mass[a + 1][(b + 1) % 3].pos - mass[a][b].pos).Length()) - DiagLen)*K2;
		if (a != 0)
			mass[a][b].force += (mass[a - 1][(b + 1) % 3].pos - mass[a][b].pos).Unit()*(abs((mass[a - 1][(b + 1) % 3].pos - mass[a][b].pos).Length()) - DiagLen)*K2;

		if (a != FloorNum)//右边的
			mass[a][b].force += (mass[a + 1][(b + 2) % 3].pos - mass[a][b].pos).Unit()*(abs((mass[a + 1][(b + 2) % 3].pos - mass[a][b].pos).Length()) - DiagLen)*K2;
		if (a != 0)
			mass[a][b].force += (mass[a - 1][(b + 2) % 3].pos - mass[a][b].pos).Unit()*(abs((mass[a - 1][(b + 2) % 3].pos - mass[a][b].pos).Length()) - DiagLen)*K2;

		//弯曲弹簧
		if (a != FloorNum && a != FloorNum - 1)
			mass[a][b].force += (mass[a + 2][b].pos - mass[a][b].pos).Unit()*(abs((mass[a + 2][b].pos - mass[a][b].pos).Length()) - 2.0f*FloorHeight)*K4;
		if (a != 0 && a != 1)
			mass[a][b].force += (mass[a - 2][b].pos - mass[a][b].pos).Unit()*(abs((mass[a - 2][b].pos - mass[a][b].pos).Length()) - 2.0f*FloorHeight)*K4;

	}

	void MassUpd(int flag,int topforce,Stem BaseStem)//0，不受限，1受限，,是否进行顶端受力,+受限源
	{
		if (flag == 1) {
			int i = 1,j=0;
			mass[i][0].pos = BaseStem.mass[BaseStem.FloorNum][0].pos;
			//printf("%f\n", BaseStem.mass[BaseStem.FloorNum ][0].pos.y);
			mass[i][1].pos = mass[i][0].pos + (BaseStem.mass[BaseStem.FloorNum][1].pos- mass[i][0].pos)*SideLen/BaseStem.SideLen;
			mass[i][2].pos = mass[i][0].pos + (BaseStem.mass[BaseStem.FloorNum][2].pos - mass[i][0].pos)*SideLen / BaseStem.SideLen;

			mass[j][0].pos = mass[i][0].pos - (BaseStem.mass[BaseStem.FloorNum][0].pos - BaseStem.mass[BaseStem.FloorNum - 1][0].pos).Unit()*FloorHeight;
			mass[j][1].pos = mass[i][1].pos - (BaseStem.mass[BaseStem.FloorNum][1].pos - BaseStem.mass[BaseStem.FloorNum - 1][1].pos).Unit()*FloorHeight;
			mass[j][2].pos = mass[i][2].pos - (BaseStem.mass[BaseStem.FloorNum][2].pos - BaseStem.mass[BaseStem.FloorNum - 1][2].pos).Unit()*FloorHeight;
		}
		//for (int i = 0; i < FloorNum + 1; i++) {
		//	printf("%f ", mass[i][0].pos.y);
		//}
		//printf("\n\n");
		//int a = 0, b = 0;
		//printf("%f,%f,%f\n", BaseStem.mass[a][b].pos.x, BaseStem.mass[a][b].pos.y, BaseStem.mass[a][b].pos.z);
		//printf("%f,%f,%f\n--------\n", BranStem.mass[a][b].pos.x, BranStem.mass[a][b].pos.y, BranStem.mass[a][b].pos.z);
		//int a = 0, b = 0;
		//printf("%f,%f,%f\n", BaseStem.mass[a][b].pos.x, BaseStem.mass[a][b].pos.y, BaseStem.mass[a][b].pos.z);
		//printf("%f,%f,%f\n--------\n", mass[a][b].pos.x, mass[a][b].pos.y, mass[a][b].pos.z);
		//第一二层根固定，从第三层开始
		for (int i = 2; i < FloorNum + 1; i++) {
			for (int j = 0; j < 3; j++) {
				if (FlashTime == FPS / 100) {//速度衰减
					mass[i][j].vel *= VelDamp;
					FlashTime = 0;
				}
				FlashTime++;
				mass[i][j].pos += mass[i][j].vel * FrameTime;
				ForceCal(i, j);

				if (mass[i][j].force.Length() > TremorHold&&mass[i][j].force.Length() < OvershootHold) {//防振喘,超弹
					mass[i][j].vel += (mass[i][j].force / MassWeight) * FrameTime;
				}
				mass[i][j].force = Vector3D(0.0f, 0.0f, 0.f);
			}
		}
		//顶端受力
		if (topforce = 1) {
			mass[FloorNum][0].force += deviaforce;
			mass[FloorNum][1].force += deviaforce;
			mass[FloorNum][2].force += deviaforce;
		}

	}

	void ModelInit(int flag,Stem BaseStem,Vector3D BuildPos) {//1代表受限,0不受限，受限源，不受限的位置
		if (flag == 1) {
			int i = 1;
			mass[i][0].pos = BaseStem.mass[BaseStem.FloorNum][0].pos;
			//printf("%f\n", BaseStem.mass[BaseStem.FloorNum ][0].pos.y);
			mass[i][1].pos = mass[i][0].pos + (BaseStem.mass[BaseStem.FloorNum][1].pos - mass[i][0].pos)*SideLen / BaseStem.SideLen;
			mass[i][2].pos = mass[i][0].pos + (BaseStem.mass[BaseStem.FloorNum][2].pos - mass[i][0].pos)*SideLen / BaseStem.SideLen;

			for (int j = 0; j < FloorNum+1; j++) {
				mass[j][0].pos = mass[i][0].pos + (BaseStem.mass[BaseStem.FloorNum][0].pos - BaseStem.mass[BaseStem.FloorNum - 1][0].pos).Unit()*FloorHeight*(j-1);
				mass[j][1].pos = mass[i][1].pos + (BaseStem.mass[BaseStem.FloorNum][1].pos - BaseStem.mass[BaseStem.FloorNum - 1][1].pos).Unit()*FloorHeight*(j-1);
				mass[j][2].pos = mass[i][2].pos + (BaseStem.mass[BaseStem.FloorNum][2].pos - BaseStem.mass[BaseStem.FloorNum - 1][2].pos).Unit()*FloorHeight*(j-1);
			}

		}
		else if(flag==0) {
			for (int i = 0; i < FloorNum + 1; i++) {
				float devia = 0.0f;// FloorHeight * 2 * sin(PI / 2 * i / FloorNum);
				mass[i][0].pos = Vector3D(devia + 0.0f, i*FloorHeight, devia + 0.0f) + BuildPos;
				mass[i][1].pos = Vector3D(devia + SideLen, i*FloorHeight, devia + 0.0f) + BuildPos;
				mass[i][2].pos = Vector3D(devia + SideLen / 2, i*FloorHeight, devia + SideLen / 2 * sqrt(3)) + BuildPos;
				for (int j = 0; j < 3; j++) {
					mass[i][j].vel = Vector3D(0.0f, 0.0f, 0.0f);
					mass[i][j].force = Vector3D(0.0f, 0.0f, 0.0f);
				}
			}
		}
		//for (int i = 0; i < FloorNum + 1; i++) {
		//	printf("---%f ", mass[i][0].pos.y);
		//}
		//printf("\n");
		//float deviapos = 1;
		//mass[3][0].pos = Vector3D(devia + 0.0f, 3*FloorHeight, devia + 0.0f);
		//mass[3][1].pos = Vector3D(devia + SideLen, 3*FloorHeight, devia + 0.0f);
		//mass[3][2].pos = Vector3D(devia + SideLen / 2, 3*FloorHeight, devia + SideLen / 2 * sqrt(3));
	}

};




//视角参数
const float PI = acos(-1);
float R = 10, hu = 0.0, du = 90, OriX = -1, OriY = -1;   //du是视点和x轴的夹角,hu是视点和水平面的夹角,r是视点绕y轴的半径，
float C = PI / 180.0;    //弧度和角度转换参数
float ViewH = 0;//视点中心高度
float GroundL = 0.0f;//物体离地高度

int GRID_SIZE = 5;  //地板砖边长的一半

Stem BaseStem(20, 10, 0.5, 20);//,BranStem(3, 3, 1.2, 18),BBranStem(2, 3, 0.9, 18);//高度，层数，边长，重量


void DrawGrid()  //画地板
{
	glBegin(GL_LINES);
	glColor3f(0.5f, 0.5f, 0.5f);
	for (int i = -GRID_SIZE; i <= GRID_SIZE; i++)
	{
		glVertex3f((float)i, 0, (float)-GRID_SIZE);
		glVertex3f((float)i, 0, (float)GRID_SIZE);

		glVertex3f((float)-GRID_SIZE, 0, (float)i);
		glVertex3f((float)GRID_SIZE, 0, (float)i);
	}
	glEnd();
}

void StemPiece(Stem &top, Stem &bot) {
	for (int i = 0; i < 3; i++) {
		bot.mass[0][i].pos = top.mass[top.FloorNum - 1][i].pos;
		bot.mass[1][i].pos = top.mass[top.FloorNum][i].pos;
	}
}

void renderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除缓冲，GL_COLOR_BUFFER_BIT ：颜色缓冲标志位
	glLoadIdentity();                                       //重置当前矩阵为4*4的单位矩阵
	gluLookAt(R*cos(C*du), R*cos(C*hu), R*sin(C*du), 0.0f, ViewH, 0.0, 0.0, 1.0, 0.0);   //从视点看远点

	DrawGrid();
	//glTranslatef(0.0f, 0.5f, 0.0f);
	//glutWireTeapot(1.0f);
	BaseStem.MassUpd(0,1, BaseStem);
	//BranStem.MassUpd(1,1, BaseStem);
	//BBranStem.MassUpd(1,0, BranStem);

	//StemPiece(BaseStem, BranStem);
	BaseStem.DrawStem();
	//glTranslatef(0.0f, BaseStem.StemLen+0.5f, 0.0f);
	//BranStem.DrawStem();
	//BBranStem.DrawStem();
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
	R == 0 ? R++:R=R;
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
	case ' ':
		BaseStem.deviaforce = Vector3D(0.0f, 0.0f, 0.0f); 
		//BranStem.deviaforce = Vector3D(0.0f, 0.0f, 0.0f); break;
	case '1':
		BaseStem.deviaforce += Vector3D(10.0f, 0.0f, 0.0f); break;
	case '2':
		BaseStem.deviaforce += Vector3D(-10.0f, 0.0f, 0.0f); break;
	case '3':
		BaseStem.deviaforce += Vector3D(0.0f, 0.0f, 10.0f); break;
	case '4':
		BaseStem.deviaforce += Vector3D(0.0f, 0.0f, -10.0f); break;
	//case '5':
	//	BranStem.deviaforce += Vector3D(100.0f, 0.0f, 0.0f); break;
	//case '6':
	//	BranStem.deviaforce += Vector3D(-100.0f, 0.0f, 0.0f); break;
	//case '7':
	//	BranStem.deviaforce += Vector3D(0.0f, 0.0f, 100.0f); break;
	//case '8':
	//	BranStem.deviaforce += Vector3D(0.0f, 0.0f, -100.0f); break;
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

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);                                          //初始化glut库
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);      //设置初始显示模式
	glEnable(GL_DEPTH_TEST);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("Mass-Spring Model");
	BaseStem.ModelInit(0,BaseStem,Vector3D(0.0f, GroundL, 0.0f));
	//BranStem.ModelInit(1, BaseStem,BaseStem.mass[BaseStem.FloorNum-1][0].pos);
	//BBranStem.ModelInit(1, BranStem, BranStem.mass[BranStem.FloorNum - 1][0].pos);


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



