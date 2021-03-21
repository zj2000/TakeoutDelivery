#include <graphics.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#pragma warning(disable:6385 6386)

using namespace std;

//定义订单结构 
typedef struct orders
{
	int iNumber;//订单序号 
	int iAppearTime;//订单出现时间
	int iSellerX, iSellerY;//餐馆坐标
	int iBuyerX, iBuyerY;//食客坐标
	int iStatus;//订单当前状态—1取货途中 / 2送货途中 / 3按时完成 / 4超时完成 
	int iBeforeTakenTime;//订单出现后被接单前经历的时间
	int iPastTime;//接单后经过的时间
	struct orders* nextPtr;//下一个结点
	struct orders* PrevPtr;//上一个节点

}ORDERS;
typedef ORDERS* ORDERSPTR;

typedef struct DistanceAndDestination
{
	int ifOvertime;//0超时（>60） 1按时完成   2罚时完成 
	int distance;
	int destlist[50][2];//x坐标 ，y坐标 
	int destNum;
	ORDERSPTR order[50];//目标点在哪个订单
	ORDERSPTR GAorder[25];//存骑手身上的订单，作为CurrentOrder的临时存储
}DAD;
typedef DAD* DADPTR;

//定义骑手结构 
typedef struct riders
{
	int iNumber;//骑手序号 
	int iRiderX, iRiderY;//骑手瓦片坐标 
	int orderNum;//该骑手接单数

	int stop;//是否停靠
	int identity[2];//停靠点的身份
	int houseLoc[2][2];//停靠房子的坐标
	int finishOrder[2][6];//每个房子完成的订单序号([0]:该点的订单数)

	int screenX, screenY;//骑手在屏幕上的坐标
	int directioni;//方向对应的图片位置0：上  1：下   2：左   3：右
	int direction[2];//每个时间单位走两步，记录每步的方向——0：不动  1：向右  2：向左  3：向上  4：向下
	int firstSX, firstSY;//骑手第一步所要到达格子的中心屏幕坐标
	int secondSX, secondSY;//骑手第二步所要到达格子的中心屏幕坐标
	int countTiles;//骑手走过的格子数 //计格器  一个单位时间应走2格
	int finishTime;//完成一个订单的时间（完成图标出现的时间）

	ORDERSPTR CurrentOrder[25];//骑手当前的订单 
	DAD disAndPath;
	struct riders* nextPtr;
	struct riders* prevPtr;
}RIDERS;
typedef RIDERS* RIDERSPTR;

#define MAXGEN 1000  // 最大进化代数 
#define MAXLENCHROM  30//染色体最大长度
#define SIZEPOP 100 // 种群数目
#define PCROSS 0.6 // 交叉概率
#define PMUTATION 0.1 // 变异概率
#define TIMELIMIT1 30//罚时时间
#define TIMELIMIT2 60//超时时间
#define timeNum 2//每个时间单位对应的秒值
#define timeDuration 2000 //每个时间单位对应的毫秒数
#define TextHEIGHT 22 //文本框高度
#define CANVASHEIGHT 558//画布高度
#define CANVASWIDTH 1088//画布宽度
#define FINISHWIDTH 42//完成图宽度
#define FINISHHEIGHT 27//完成图高度
#define RIDERWIDTH 24//骑手图片宽度
#define RIDERHEIGHT 43//骑手图片高度
#define riderDeltaImgX  11 //骑手图像贴图位置相对于x坐标的偏差
#define riderDeltaImgY  34//骑手图像贴图位置相对于y坐标的偏差
#define SCREENX0 576//骑手初始x位置screen
#define SCREENY0 302//骑手初始y位置screen
#define TILEX0 9//骑手初始瓦片坐标 x
#define TILEY0 8//骑手初始瓦片坐标 y
#define TILEWIDTH 64//瓦片宽度
#define TILEHEIGHT 32//瓦片高度
#define HOUSEX0 544//房子（0，0）的初始x坐标screen
#define HOUSEY0 14//房子（0，0）的初始y坐标screen
#define SPEED 1//骑手速度（像素）

int music = 1;//1:on  0:off
int clickTime = 0;//鼠标点击房子的次数 
int MONEY = 1000;//钱
int TIME = 0;//全局时间（显示的时间）
int takenNum = 0; // 接单数 
int achieveNum = 0;//完成数 
int overtimeNum = 0; //超时数
int riderNum = 3;//买的骑手数量
int destinationNum = 0;//所有骑手剩余的点数 
int isStop = 0;//是否暂停
int isOver = 0;//是否结束 
int curAchieveNum = 0;//每秒结单数 
int curOverNum = 0;//每秒罚单数 
int aindex[20];//每秒完成的订单号
int oindex[20];//每秒罚单的订单号 
int isdead = 0;//是否出现会让你破产的单子：0没有，1有 
int deadTime = 9999999;

clock_t startTime = 0;//开始时间
clock_t stopTime = 0;//暂停时间
clock_t endStopTime = 0;//结束暂停的时间（点击继续的时间）
clock_t stopInterval = 0;//暂停间隔
clock_t totStopInterval = 0;//总共暂停时间
clock_t currentTime = 0;//当前时间
IMAGE img_startUI;//开始界面
IMAGE img_map;//地图
IMAGE img_rider, img_ridercover;//人物图，人物掩码图
IMAGE img_finish, img_finishcover;//完成图标及其掩码图
IMAGE img_takeorder1, img_takeorder2;//接单事件中出现的提示图
IMAGE img_musicON, img_musicOFF;//音乐按钮图标
IMAGE img_newA, img_newB;//新下单的提示图
IMAGE img_orderIndexRed;//红色显示单号图
IMAGE img_orderIndexBlue;//蓝色显示单号图

ORDERSPTR headOrder = NULL; //订单头指针 
ORDERSPTR currentOrder = NULL;//当前订单
RIDERSPTR headRider = NULL;	//骑手头指针 


void InitStartUI();//初始化开始界面（尚未计时）
void EndPrint();//结束界面
void BreakPrint();//破产界面
void DrawFinishHint(RIDERSPTR pRider);
int Convert90to45_X(int screenX, int screenY);//直角坐标系转化为斜坐标系  X
int Convert90to45_Y(int screenX, int screenY);//直角坐标系转化为斜坐标系  Y
int Convert45to90_X(int tileX, int tileY);//斜坐标系转化为直角坐标系 X
int Convert45to90_Y(int tileX, int tileY);//斜坐标系转化为直角坐标系 Y
void MoveRider(RIDERSPTR pRider, int step);//移动骑手
void TurnLeft(RIDERSPTR rider);
void TurnRight(RIDERSPTR rider);
void TurnUp(RIDERSPTR rider);
void TurnDown(RIDERSPTR rider);
void Draw();//批量绘图（骑手）
void DrawHint(int i);//绘图（点单提示）
void StopEvent();//处理暂停事件
void ContinueEvent();//处理继续事件
void InitString(char s[], int len);//初始化字符串
void PrintText1();//打印文本框1
void PrintRiderText();//打印骑手信息文本框
void PrintMouseCoordinate();//打印鼠标的坐标
ORDERSPTR CreateOrderList(ORDERSPTR head, int x, int y);//创建订单链表
RIDERSPTR CreateRiderList(RIDERSPTR head);//建立骑手链表
void DeliverOrder(ORDERSPTR currentOrder);//处理分配新出现的订单
void UpdateDirection(RIDERSPTR p);//确定骑手的移动方向（还未改变骑手坐标）
void JudgeArrive(RIDERSPTR p);//判断是否到达目标点并处理
void LoadRiderImg(RIDERSPTR newPtr, int iNumber);//给骑手加载图片
void LoadFinishImg(int identity);//加载完成图片

DAD PathPlanning(RIDERSPTR rider, ORDERSPTR order);
DAD GAPathPlanning(int OrderNum, RIDERSPTR rider, ORDERSPTR order);
void InitChrom(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom); // 种群初始化函数
int Distance(int*, int*); // 计算两个城市之间的距离
int* Min(int*); // 计算距离数组的最小值
int PathLength(int* arr, int lenchrom, int destination[MAXLENCHROM][2], RIDERSPTR rider); // 计算某一个方案的路径长度，适应度函数为路线长度的倒数
void Choice(int chrom[SIZEPOP][MAXLENCHROM], int lenchrom, int destination[MAXLENCHROM][2], RIDERSPTR rider); // 选择操作
void Cross(int chrome[SIZEPOP][MAXLENCHROM], int lenChrom); // 交叉操作
void Mutation(int chrome[SIZEPOP][MAXLENCHROM], int lenChrom); // 变异操作
void Reverse(int chrom[SIZEPOP][MAXLENCHROM], int lenchrom, int destination[MAXLENCHROM][2], RIDERSPTR rider); // 逆转操作
RIDERSPTR ChooseRider(RIDERSPTR riders[], int riderNum, DAD riderDAD[]);//选择骑手

int main()
{
	int isStart = 0;//是否开始 0:未开始  1:开始
	int isEnd = 0;//是否点了end
	int isBreak = 0;//是否破产
	clock_t tempTime1 = -1;

	InitStartUI();

	while (isOver == 0)
	{
		if (isStart == 0)
		{
			if (MouseHit())
			{
				MOUSEMSG mouse = GetMouseMsg();
				if (mouse.mkLButton)
				{
					putimage(0, 0, &img_map);//贴背景图
					putimage(765, 30, &img_musicON);
					//加载声音
					mciSendString(_T("play ./Resource/BGM.wav"), 0, 0, 0);
					for (int i = 1; i <= riderNum; i++)//初始化骑手信息  开局购买3个骑手 
						headRider = CreateRiderList(headRider);

					isStart = 1;
					startTime = clock();
				}
			}
		}
		if (isStart == 1)
		{
			currentTime = clock();//获取当前时间
			if (tempTime1 == -1)
				tempTime1 = currentTime;

			if (MouseHit())//如果鼠标有反应
			{
				MOUSEMSG mouse = GetMouseMsg();
				//如果鼠标左键按下，则判断是点击哪个按钮并执行相关操作
				if (mouse.mkLButton)
				{
					mouse = GetMouseMsg();
					int mouseScreenX = mouse.x;//鼠标的屏幕坐标x
					int mouseScreenY = mouse.y;//鼠标的屏幕坐标y
					int mouseTileX = Convert90to45_X(mouseScreenX, mouseScreenY);//鼠标的瓦片坐标x
					int mouseTileY = Convert90to45_Y(mouseScreenX, mouseScreenY);//鼠标的瓦片坐标y

					//如果点击房子则是在点单
					if ((mouseTileX >= 0) && (mouseTileX <= 16) && (mouseTileY >= 0) && (mouseTileY <= 16) && (mouseTileX % 2 == 0) && (mouseTileY % 2 == 0))
					{
						clickTime++;
						if (clickTime % 2 == 1)//第一次点击的是餐馆
						{
							headOrder = CreateOrderList(headOrder, mouseTileX, mouseTileY);
							currentOrder = headOrder->PrevPtr;
						}
						if (clickTime % 2 == 0)//第二次点击的是食客
						{
							takenNum++;
							currentOrder->iBuyerX = mouseTileX;
							currentOrder->iBuyerY = mouseTileY;
							currentOrder->iAppearTime = TIME;

							if(currentOrder != NULL)
								DeliverOrder(currentOrder);

							RIDERSPTR p = headRider;
							while (p != NULL)
							{
								UpdateDirection(p);
								p = p->nextPtr;
							}

						}
					}

					//点击“Stop”暂停
					if ((mouseScreenX >= 920) && (mouseScreenX <= 1038) && (mouseScreenY >= 33) && (mouseScreenY <= 72))
					{
						StopEvent();
					}

					//点击“Continue”继续
					if ((mouseScreenX >= 920) && (mouseScreenX <= 1038) && (mouseScreenY >= 78) && (mouseScreenY <= 117))
					{
						ContinueEvent();
					}

					//点击“End”结束
					if ((mouseScreenX >= 920) && (mouseScreenX <= 1038) && (mouseScreenY >= 122) && (mouseScreenY <= 161))
					{
						isStop = 1;
						EndPrint();
					}

					//点击“Music”
					if ((mouseScreenX >= 765) && (mouseScreenX <= 906) && (mouseScreenY >= 30) && (mouseScreenY <= 71))
					{
						if (music == 1)//如果点击之前音乐是开的，则改成关的
						{
							putimage(765, 30, &img_musicOFF);
							music = 0;
							mciSendString(_T("stop ./Resource/BGM.wav"), 0, 0, 0);
						}
						else//如果之前是关的，则打开
						{
							putimage(765, 30, &img_musicON);
							music = 1;
							mciSendString(_T("play ./Resource/BGM.wav"), 0, 0, 0);
						}
					}
				}
			}

			//如果没有暂停,则需要更新界面
			if (isStop == 0)
			{
				int dt = (timeDuration / 2) / (TILEHEIGHT / 2);
				if (currentTime - tempTime1 >= dt)//如果满足时间间隔则刷新画面
				{
					tempTime1 = currentTime;
					RIDERSPTR p = headRider;
					while (p != NULL)//遍历一遍骑手
					{
						//如果骑手不动
						if (p->direction[p->countTiles] == 0)
						{
							JudgeArrive(p);
							UpdateDirection(p);
						}
						//如果骑手动
						if (p->direction[p->countTiles] != 0)
						{
							if (p->countTiles == 0)//如果还没走到第一格的中心
							{
								if (p->screenX == p->firstSX && p->screenY == p->firstSY)
									p->countTiles = 1;
								else
									MoveRider(p, 0);
							}
							if (p->countTiles == 1)//已走完了一格
							{
								if (!(p->screenX == p->secondSX && p->screenY == p->secondSY))
								{
									MoveRider(p, 1);
								}
								else//if (p->screenX == p->secondSX && p->screenY == p->secondSY)
								{
									if (p->stop != 0)
									{
										p->finishTime = -1;
										p->stop = 0;
										p->finishOrder[0][0] = 0;
										p->finishOrder[1][0] = 0;
									}
									p->countTiles = 0;
									p->iRiderX = Convert90to45_X(p->screenX, p->screenY);
									p->iRiderY = Convert90to45_Y(p->screenX, p->screenY);
									JudgeArrive(p);
									UpdateDirection(p);
								}
							}
						}
						
						p = p->nextPtr;
					}

					Draw();//刷新画面
				}
			}
			
			//如果暂停了
			if (isStop != 0)
			{
				stopInterval = currentTime - stopTime;
			}

			currentTime = clock();
			int newTIME = (currentTime - startTime - totStopInterval - stopInterval) / timeDuration;//获得新的TIME看是否增长
			if (newTIME - TIME == 1)//如果IIME+1，更新
			{
				TIME = newTIME;
				curAchieveNum = 0;//每秒结单数 
				curOverNum = 0;//每秒罚单数 
			}

			if ((MONEY < 0) || (isdead == 1 && TIME >= deadTime))
				BreakPrint();
		}
	}
	mciSendString(_T("close ./Resource/BGM.wav"), 0, 0, 0);
	closegraph();//关闭画布
}

//初始化开始界面（尚未计时）
void InitStartUI()
{
	initgraph(CANVASWIDTH, CANVASHEIGHT);

	//加载背景音乐
	mciSendString(_T("open ./Resource/BGM.wav"), 0, 0, 0);
	//加载图片
	loadimage(&img_startUI, _T("./Resource/StartUI.png"), CANVASWIDTH, CANVASHEIGHT);
	loadimage(&img_map, _T("./Resource/MAP1.png"), CANVASWIDTH, CANVASHEIGHT);
	loadimage(&img_finishcover, _T("./Resource/FINISHCOVER.png"), FINISHWIDTH, FINISHHEIGHT);
	loadimage(&img_takeorder1, _T("./Resource/TAKEORDER1.png"), 267, 37);
	loadimage(&img_takeorder2, _T("./Resource/TAKEORDER2.png"), 268, 37);
	loadimage(&img_musicON, _T("./Resource/MUSICON.png"), 141, 41);
	loadimage(&img_musicOFF, _T("./Resource/MUSICOFF.png"), 141, 41);
	loadimage(&img_newA, _T("./Resource/NEWA.png"), FINISHWIDTH, FINISHHEIGHT);
	loadimage(&img_newB, _T("./Resource/NEWB.png"), FINISHWIDTH, FINISHHEIGHT);
	loadimage(&img_orderIndexRed, _T("./Resource/ORDERINDEXRED.png"), FINISHWIDTH, FINISHHEIGHT);
	loadimage(&img_orderIndexBlue, _T("./Resource/ORDERINDEXBLUE.png"), FINISHWIDTH, FINISHHEIGHT);

	setbkmode(TRANSPARENT);

	putimage(0, 0, &img_startUI);
}

int Convert90to45_X(int screenX, int screenY)//直角坐标系转化为斜坐标系  
{
	int tileX;
	tileX = (int)(((double)screenX - (double)HOUSEX0 + 0.0) / (double)TILEWIDTH + ((double)screenY - (double)HOUSEY0 + 0.0) / (double)TILEHEIGHT);
	return tileX;
}
int Convert90to45_Y(int screenX, int screenY)//直角坐标系转化为斜坐标系  Y
{
	int tileY;
	tileY = (int)(-((double)screenX - (double)HOUSEX0 + 0.0) / (double)TILEWIDTH + ((double)screenY - (double)HOUSEY0 + 0.0) / (double)TILEHEIGHT);
	return tileY;
}
int Convert45to90_X(int tileX, int tileY)
{
	int sx;
	sx = (tileX - tileY) * TILEWIDTH / 2 + HOUSEX0;
	return sx;
}
int Convert45to90_Y(int tileX, int tileY)
{
	int sy;
	sy = (tileX + tileY) * TILEHEIGHT / 2 + HOUSEY0 + 16;
	return sy;
}

//创建订单链表
ORDERSPTR CreateOrderList(ORDERSPTR head, int x, int y)
{
	ORDERSPTR current = NULL;
	current = (ORDERSPTR)malloc(sizeof(ORDERS));
	if (current != NULL)
	{
		//初始化订单数据 
		current->iNumber = takenNum + 1;
		current->iSellerX = x;
		current->iSellerY = y;
		current->iStatus = 1;
		current->iPastTime = 0;

		if (head == NULL)
			head = current;
		else
			head->PrevPtr->nextPtr = current;

		current->nextPtr = NULL;
		head->PrevPtr = current;
	}
	return head;
}
//建立骑手链表 
RIDERSPTR CreateRiderList(RIDERSPTR head)
{
	RIDERSPTR newPtr = NULL;
	newPtr = (RIDERSPTR)malloc(sizeof(RIDERS));
	if (newPtr != NULL)
	{
		newPtr->iRiderX = TILEX0;
		newPtr->iRiderY = TILEY0;
		newPtr->screenX = SCREENX0;
		newPtr->screenY = SCREENY0;
		newPtr->orderNum = 0;
		newPtr->countTiles = 0;
		newPtr->stop = 0;
		newPtr->finishTime = -1;
		newPtr->finishOrder[0][0] = 0;
		newPtr->finishOrder[1][0] = 0;
		newPtr->disAndPath.destNum = 0;
		newPtr->disAndPath.distance = 0;
		newPtr->disAndPath.order[0] = NULL;
		for (int i = 0; i < 25; i++)
		{
			newPtr->CurrentOrder[i] = NULL;
		}
		newPtr->directioni = 1;

		if (head == NULL)
		{
			head = newPtr;
			newPtr->iNumber = 1;
		}
		else
		{
			newPtr->iNumber = head->prevPtr->iNumber + 1;
			head->prevPtr->nextPtr = newPtr;
		}

		LoadRiderImg(newPtr, newPtr->iNumber);
		putimage(SCREENX0 - riderDeltaImgX, SCREENY0 - riderDeltaImgY, RIDERWIDTH, RIDERHEIGHT, &img_ridercover, newPtr->directioni * RIDERWIDTH, 0, SRCPAINT);
		putimage(SCREENX0 - riderDeltaImgX, SCREENY0 - riderDeltaImgY, RIDERWIDTH, RIDERHEIGHT, &img_rider, newPtr->directioni * RIDERWIDTH, 0, SRCAND);


		head->prevPtr = newPtr;
		newPtr->nextPtr = NULL;

		MONEY -= 300;
	}
	return head;
}

//给骑手加载图片
void LoadRiderImg(RIDERSPTR newPtr, int iNumber)
{
	switch (iNumber % 5)
	{
	case 1:
		loadimage(&img_rider, _T("./Resource/RIDER1.png"), 4 * RIDERWIDTH, RIDERHEIGHT);
		loadimage(&img_ridercover, _T("./Resource/RIDERCOVER1.png"), 4 * RIDERWIDTH, RIDERHEIGHT);
		break;
	case 2:
		loadimage(&img_rider, _T("./Resource/RIDER2.png"), 4 * RIDERWIDTH, RIDERHEIGHT);
		loadimage(&img_ridercover, _T("./Resource/RIDERCOVER2.png"), 4 * RIDERWIDTH, RIDERHEIGHT);
		break;
	case 3:
		loadimage(&img_rider, _T("./Resource/RIDER3.png"), 4 * RIDERWIDTH, RIDERHEIGHT);
		loadimage(&img_ridercover, _T("./Resource/RIDERCOVER3.png"), 4 * RIDERWIDTH, RIDERHEIGHT);
		break;
	case 4:
		loadimage(&img_rider, _T("./Resource/RIDER4.png"), 4 * RIDERWIDTH, RIDERHEIGHT);
		loadimage(&img_ridercover, _T("./Resource/RIDERCOVER4.png"), 4 * RIDERWIDTH, RIDERHEIGHT);
		break;
	case 0:
		loadimage(&img_rider, _T("./Resource/RIDER5.png"), 4 * RIDERWIDTH, RIDERHEIGHT);
		loadimage(&img_ridercover, _T("./Resource/RIDERCOVER5.png"), 4 * RIDERWIDTH, RIDERHEIGHT);
		break;
	}
}
//加载完成图片
void LoadFinishImg(int identity)
{
	switch (identity)
	{
	case 1:
		loadimage(&img_finish, _T("./Resource/FINISH1.png"), FINISHWIDTH, FINISHHEIGHT);
		break;
	case 2:
		loadimage(&img_finish, _T("./Resource/FINISH2.png"), FINISHWIDTH, FINISHHEIGHT);
		break;
	case 3:
		loadimage(&img_finish, _T("./Resource/FINISH3.png"), FINISHWIDTH, FINISHHEIGHT);
		break;
	}
}


//处理暂停事件
void StopEvent()
{
	stopTime = currentTime;
	isStop = 1;
}
//处理继续事件
void ContinueEvent()
{
	endStopTime = clock();
	stopInterval = endStopTime - stopTime;
	totStopInterval += stopInterval;
	stopInterval = 0;
	isStop = 0;
}

//进行画面贴图
void Draw()
{
	BeginBatchDraw();
	//贴背景
	putimage(0, 0, &img_map);

	//贴音乐按钮
	if (music == 1)
		putimage(765, 30, &img_musicON);
	if (music == 0)
		putimage(765, 30, &img_musicOFF);

	//贴提示点单的图
	if (clickTime % 2 == 1)
		DrawHint(2);
	else
		DrawHint(1);
	//贴骑手的图
	RIDERSPTR pRider = headRider;
	while (pRider != NULL)
	{
		LoadRiderImg(pRider, pRider->iNumber);
		putimage(pRider->screenX - riderDeltaImgX, pRider->screenY - riderDeltaImgY, RIDERWIDTH, RIDERHEIGHT, &img_ridercover, pRider->directioni * RIDERWIDTH, 0, SRCPAINT);
		putimage(pRider->screenX - riderDeltaImgX, pRider->screenY - riderDeltaImgY, RIDERWIDTH, RIDERHEIGHT, &img_rider, pRider->directioni * RIDERWIDTH, 0, SRCAND);
		DrawFinishHint(pRider);
		pRider = pRider->nextPtr;
	}
	//打印文字部分
	COLORREF color = RGB(35, 43, 133);
	settextcolor(color);
	PrintText1();
	PrintRiderText();

	EndBatchDraw();
}
//打印点单时的提示
void DrawHint(int i)
{
	//设置字体格式
	settextstyle(12, 0, _T("华文新魏"));
	COLORREF color = RGB(136, 0, 21);//红棕色
	settextcolor(color);

	int x, y;
	if (currentOrder != NULL && currentOrder->iStatus < 2)
	{
		x = Convert45to90_X(currentOrder->iSellerX, currentOrder->iSellerY);
		y = Convert45to90_Y(currentOrder->iSellerX, currentOrder->iSellerY);//把瓦片坐标转换成直角坐标
		putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 30, &img_finishcover, SRCPAINT);
		putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 30, &img_orderIndexRed, SRCAND);//贴新餐馆的订单序号图
		putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 10, &img_finishcover, SRCPAINT);
		putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 10, &img_newA, SRCAND);//贴新餐馆的提示图
		
		//打印订单序号
		char s[15];
		int w = x - FINISHWIDTH / 2 + 27, h = y - FINISHHEIGHT - 24, len = 15;
		InitString(s, len);
		sprintf_s(s, len, "%d", currentOrder->iNumber);
		outtextxy(w, h, s);
	}
	if (i == 1)
	{
		if (currentOrder != NULL && currentOrder->iStatus < 2)
		{
			x = Convert45to90_X(currentOrder->iBuyerX, currentOrder->iBuyerY);
			y = Convert45to90_Y(currentOrder->iBuyerX, currentOrder->iBuyerY);
			putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 30, &img_finishcover, SRCPAINT);
			putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 30, &img_orderIndexRed, SRCAND);//贴新餐馆的订单序号图
			putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 10, &img_finishcover, SRCPAINT);
			putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 10, &img_newB, SRCAND);//贴新餐馆的提示图
			char s[15];
			int w = x - FINISHWIDTH / 2 + 27, h = y - FINISHHEIGHT - 24, len = 15;
			InitString(s, len);
			sprintf_s(s, len, "%d", currentOrder->iNumber);
			outtextxy(w, h, s);
		}
		putimage(800, 462, &img_takeorder1);
	}
	if (i == 2)
	{
		putimage(800, 462, &img_takeorder2);
	}
}
//打印到达目标点时的提示
void DrawFinishHint(RIDERSPTR pRider)
{
	settextstyle(12, 0, _T("华文新魏"));//设置字体
	COLORREF color = RGB(71, 94, 226);//蓝色
	settextcolor(color);

	if (pRider->stop != 0)
	{
		for (int i = 0; i < pRider->stop; i++)
		{
			int x = Convert45to90_X(pRider->houseLoc[i][0], pRider->houseLoc[i][1]);
			int y = Convert45to90_Y(pRider->houseLoc[i][0], pRider->houseLoc[i][1]);
			LoadFinishImg(pRider->identity[i]);
			if (pRider->identity[i] >= 2)
			{
				putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 30, &img_finishcover, SRCPAINT);
				putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 30, &img_orderIndexBlue, SRCAND);

				//打印订单序号
				char s[15];
				int w = x - FINISHWIDTH / 2 + 27, h = y - FINISHHEIGHT - 24, len = 15;
				for (int num = 1; num <= pRider->finishOrder[i][0]; num++)
				{
					InitString(s, len);
					sprintf_s(s, len, "%d", pRider->finishOrder[i][num]);
					outtextxy(w, h, s);
					w += 10;
				}
			}
			putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 10, &img_finishcover, SRCPAINT);
			putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 10, &img_finish, SRCAND);
		}
	}
}
//结束界面打印
void EndPrint()
{
	IMAGE img_end;
	loadimage(&img_end, _T("./Resource/EndUI.png"), CANVASWIDTH, CANVASHEIGHT);
	putimage(0, 0, &img_end);
	settextstyle(42, 0, _T("华文新魏"));
	COLORREF color = RGB(136, 0, 21);
	settextcolor(color);

	int h = 170, w = 543, len = 10, height = 50;
	char s[10];
	// print line1
	InitString(s, len);
	sprintf_s(s, len, "%d", TIME);
	outtextxy(w, h, s);

	// print line2
	h += height;
	InitString(s, len);
	sprintf_s(s, len, "%d", MONEY);
	outtextxy(w, h, s);

	// print line3
	h += height;
	InitString(s, len);
	sprintf_s(s, len, "%d", takenNum);
	outtextxy(w, h, s);

	//print line4
	h += height;
	InitString(s, len);
	sprintf_s(s, len, "%d", achieveNum);
	outtextxy(w, h, s);

	//print line5
	h += height;
	InitString(s, len);
	sprintf_s(s, len, "%d", overtimeNum);
	outtextxy(w, h, s);

	MOUSEMSG m = GetMouseMsg();
	while (!m.mkLButton)
		m = GetMouseMsg();
	isOver = 1;
}
//破产界面打印
void BreakPrint()
{
	IMAGE img_break;
	loadimage(&img_break, _T("./Resource/BreakUI.png"), CANVASWIDTH, CANVASHEIGHT);
	putimage(0, 0, &img_break);
	settextstyle(TextHEIGHT, 0, _T("华文新魏"));
	COLORREF color = RGB(136, 0, 21);
	settextcolor(color);

	int h = 263, w = 535, len = 10, height = 32;
	char s[10];
	// print line1
	InitString(s, len);
	sprintf_s(s, len, "%d", TIME);
	outtextxy(w, h, s);

	// print line2
	h += height;
	InitString(s, len);
	sprintf_s(s, len, "%d", MONEY);
	outtextxy(w, h, s);

	// print line3
	h += height;
	InitString(s, len);
	sprintf_s(s, len, "%d", takenNum);
	outtextxy(w, h, s);

	//print line4
	h += height;
	InitString(s, len);
	sprintf_s(s, len, "%d", achieveNum);
	outtextxy(w, h, s);

	//print line5
	h += height;
	InitString(s, len);
	sprintf_s(s, len, "%d", overtimeNum);
	outtextxy(w, h, s);

	MOUSEMSG m = GetMouseMsg();
	while (!m.mkLButton)
		m = GetMouseMsg();
	isOver = 1;
}

//移动骑手
void MoveRider(RIDERSPTR pRider, int i)
{
	switch (pRider->direction[i])
	{
	case 1:
		TurnRight(pRider);
		break;
	case 2:
		TurnLeft(pRider);
		break;
	case 3:
		TurnUp(pRider);
		break;
	case 4:
		TurnDown(pRider);
		break;
	default:break;
	}
}
void TurnLeft(RIDERSPTR rider)
{
	rider->screenX -= 2 * SPEED;
	rider->screenY -= SPEED;
	rider->directioni = 2;
}
void TurnRight(RIDERSPTR rider)
{
	rider->screenX += 2 * SPEED;
	rider->screenY += SPEED;
	rider->directioni = 3;
}
void TurnUp(RIDERSPTR rider)
{
	rider->screenX += 2 * SPEED;
	rider->screenY -= SPEED;
	rider->directioni = 0;
}
void TurnDown(RIDERSPTR rider)
{
	rider->screenX -= 2 * SPEED;
	rider->screenY += SPEED;
	rider->directioni = 1;
}

void InitString(char s[], int len)//初始化字符串
{
	for (int i = 0; i < len; i++)
		s[i] = 0;
}
void PrintText1()
{
	settextstyle(TextHEIGHT, 0, _T("华文新魏"));//设置字体

	int h = 17, w = 65, len = 15;
	int tabWidth = 25;
	char s[15];
	// print line1
	InitString(s, len);
	sprintf_s(s, len, "%d", TIME);
	outtextxy(w, h, s);

	// print line2
	h += TextHEIGHT + 1;
	w = 45;
	InitString(s, len);
	sprintf_s(s, len, "%d", MONEY);
	outtextxy(w, h, s);

	// print line3
	h += TextHEIGHT + 2;
	w = 80;
	InitString(s, len);
	sprintf_s(s, len, "%d", takenNum);
	outtextxy(w, h, s);

	//print line4
	h += TextHEIGHT + 2;
	w = 80;
	InitString(s, len);
	sprintf_s(s, len, "%d", achieveNum);
	outtextxy(w, h, s);
	if (curAchieveNum != 0)
	{
		w = 170;
		for (int i = 0; i < curAchieveNum; i++)
		{
			InitString(s, len);
			sprintf_s(s, len, "%d", aindex[i]);
			outtextxy(w, h, s);
			w += tabWidth;
		}
	}

	//print line5
	h += TextHEIGHT + 2;
	w = 80;
	InitString(s, len);
	sprintf_s(s, len, "%d", overtimeNum);
	outtextxy(w, h, s);
	if (curOverNum != 0)
	{
		w = 170;
		for (int i = 0; i < curOverNum; i++)
		{
			InitString(s, len);
			sprintf_s(s, len, "%d", oindex[i]);
			outtextxy(w, h, s);
			w += tabWidth;
		}
	}
}//打印文本框1
void PrintRiderText()//打印骑手文本框
{
	settextstyle(TextHEIGHT - 5, 0, _T("华文新魏"));

	char s[30];
	int len = 30;
	int h = 445, w = 10, tabWidth = 90;
	RIDERSPTR rider = headRider;
	while (rider != NULL)
	{
		InitString(s, len);
		sprintf_s(s, len, "骑手%d位置:(%d,%d) 停靠:", rider->iNumber , rider->iRiderX, rider->iRiderY);
		w = 10;
		outtextxy(w, h, s);
		if (rider->stop != 0)
		{
			w = 185;
			for (int i = 0; i < rider->stop; i++)
			{
				InitString(s, len);
				if (rider->identity[i] == 1)
					sprintf_s(s, len, "餐馆(%d,%d)", rider->houseLoc[i][0], rider->houseLoc[i][1]);
				if (rider->identity[i] == 2)
					sprintf_s(s, "食客(%d,%d)", rider->houseLoc[i][0], rider->houseLoc[i][1]);
				if (rider->identity[i] == 3)
					sprintf_s(s, len, "餐客(%d,%d)", rider->houseLoc[i][0], rider->houseLoc[i][1]);
				outtextxy(w, h, s);
				w += tabWidth;
			}
		}
		h += TextHEIGHT + 1;
		rider = rider->nextPtr;
	}
}

//确定骑手的移动方向（还未改变骑手坐标）
void UpdateDirection(RIDERSPTR p)
{
	ORDERSPTR order = p->disAndPath.order[0];
	int dnum = p->disAndPath.destNum;//骑手身上的目标点数 
	int rx, ry, dx, dy;//骑手和目标点坐标 
	int deltaX, deltaY;
	rx = p->iRiderX;
	ry = p->iRiderY;
	dx = p->disAndPath.destlist[0][0];
	dy = p->disAndPath.destlist[0][1];

	p->direction[0] = 0;
	p->direction[1] = 0;

	if (dnum != 0)//如果有目标点   x+:1  x-:2  y+:4  y-:3
	{
		//先解决上下方向 
		if (abs(ry - dy) > 2)//如果上下方向差值大于2 
		{
			//如果上下方向有房子 ,拐弯：要先左右走(x)再上下走(y)
			if (rx % 2 == 0)
			{
				if (dx - rx > 0)//目标点在骑手右边 
					p->direction[0] = 1;
				else if (dx - rx < 0)//目标点在骑手左边 
					p->direction[0] = 2;

				//目标点在骑手正下方，则根据后面的目标点来决定向左拐还是向右拐 
				else
				{
					if (rx == 0)
						p->direction[0] = 1;
					else if (rx == 16)
						p->direction[0] = 2;
					else
					{
						int i = 0;
						int tempx = dx;
						while (tempx == rx && i < dnum - 1)
						{
							i++;
							tempx = p->disAndPath.destlist[i][0];
						}
						if (tempx - rx > 0)
							p->direction[0] = 1;
						else
							p->direction[0] = 2;
					}
				}
				//若目标点在下方，向下拐 
				if (dy - ry > 0)
					p->direction[1] = 4;
				else
					p->direction[1] = 3;
			}
			//上下方向都是路，直行 	
			else
			{
				if (dy - ry > 0)
				{
					p->direction[0] = 4;
					p->direction[1] = 4;
				}
				else
				{
					p->direction[0] = 3;
					p->direction[1] = 3;
				}
			}
		}
		else if (abs(ry - dy) == 2)//上下差值等于2 ：需要拐弯（先上下走y）
		{
			if (abs(rx - dx) == 1)
			{
				int i = 0;
				int tempx = dx;
				while (abs(tempx - rx) == 1 && i < dnum - 1)
				{
					i++;
					tempx = p->disAndPath.destlist[i][0];
				}
				if ((abs(tempx - rx) == 1) || (abs(tempx - rx) >= 3 && (abs(tempx - dx) > abs(tempx - rx))))
				{
					if (dy - ry > 0)
					{
						p->direction[0] = 4;
						p->direction[1] = 4;
					}
					else
					{
						p->direction[0] = 3;
						p->direction[1] = 3;
					}
				}
				else
				{
					if (dy - ry > 0)
						p->direction[0] = 4;
					else
						p->direction[0] = 3;

					if (dx - rx > 0)
						p->direction[1] = 1;
					else
						p->direction[1] = 2;
				}
			}
			else
			{
				if (dy - ry > 0)
					p->direction[0] = 4;
				else
					p->direction[0] = 3;

				if (dx - rx > 0)
					p->direction[1] = 1;
				else
					p->direction[1] = 2;
			}
		} //end of （距离为二） 
		//再解决左右方向 
		else if (abs(ry - dy) == 1)//(差值=1) 都是先左右走x
		{
			deltaX = abs(dx - rx);
			deltaY = abs(dy - ry);
			if (!((deltaX == 1 && deltaY == 0) || (deltaY == 1 && deltaX == 0)))//还没到 
			{
				if (dx - rx > 0)
				{
					p->direction[0] = 1;
					p->direction[1] = 1;
				}
				else
				{
					p->direction[0] = 2;
					p->direction[1] = 2;
				}
			}
		}
		else//差为0，左右有房子   都是先上下走y
		{
			deltaX = abs(dx - rx);
			deltaY = abs(dy - ry);
			if (!((deltaX == 1 && deltaY == 0) || (deltaY == 1 && deltaX == 0)))//还没到
			{
				if (ry == 0)
					p->direction[0] = 4;
				else if (ry == 16)
					p->direction[0] = 3;
				else
				{
					int i = 0;
					int tempy = dy;
					while (tempy == ry && i < dnum - 1)
					{
						i++;
						tempy = p->disAndPath.destlist[i][1];
					}
					if (tempy - ry > 0)
						p->direction[0] = 4;
					else
						p->direction[0] = 3;
				}

				if (dx - rx > 0)//目标点在骑手右边 
					p->direction[1] = 1;
				else if (dx - rx < 0)//目标点在骑手左边 
					p->direction[1] = 2;
			}
		}
	}

	int firstTX = p->iRiderX, firstTY = p->iRiderY;
	switch (p->direction[0])
	{
	case 1://right
		firstTX++;
		break;
	case 2://left
		firstTX--;
		break;
	case 3://up
		firstTY--;
		break;
	case 4://down
		firstTY++;
		break;
	default:
		break;
	}
	p->firstSX = Convert45to90_X(firstTX, firstTY);
	p->firstSY = Convert45to90_Y(firstTX, firstTY);

	int secondTX = firstTX, secondTY = firstTY;
	switch (p->direction[0])
	{
	case 1://right
		p->secondSX++;
		break;
	case 2://left
		p->secondSX--;
		break;
	case 3://up
		p->secondSY--;
		break;
	case 4://down
		p->secondSY++;
		break;
	default:
		break;
	}
	p->secondSX = Convert45to90_X(secondTX, secondTY);
	p->secondSY = Convert45to90_Y(secondTX, secondTY);
}

//判断是否到达目标点并处理
void JudgeArrive(RIDERSPTR p)
{
	ORDERSPTR order = p->disAndPath.order[0];
	int dnum = p->disAndPath.destNum;//骑手身上的目标点数 
	int rx, ry, dx, dy;//骑手和目标点坐标 
	int deltaX, deltaY;
	rx = Convert90to45_X(p->screenX, p->screenY);
	ry = Convert90to45_Y(p->screenX, p->screenY);
	dx = p->disAndPath.destlist[0][0];
	dy = p->disAndPath.destlist[0][1];

	deltaX = abs(dx - rx);
	deltaY = abs(dy - ry);

	while (order != NULL && ((deltaX == 1 && deltaY == 0) || (deltaY == 1 && deltaX == 0)) && (dx < 17 && dy < 17))//到达目的地
	{
		p->finishTime = currentTime;
		if (p->stop == 0)//停靠的房子数为0
		{
			p->stop = 1;
			p->identity[p->stop - 1] = order->iStatus;//停靠点的身份 
			p->houseLoc[p->stop - 1][0] = dx;//停靠点的x坐标 
			p->houseLoc[p->stop - 1][1] = dy;//停靠点的y坐标 

			if (order->iStatus == 2)
			{
				p->finishOrder[0][0] = 1;
				p->finishOrder[0][1] = order->iNumber;
			}
		}
		if (p->stop == 1)//停靠的房子数为1
		{
			if (dx == p->houseLoc[0][0] && dy == p->houseLoc[0][1])
			{
				if (order->iStatus != p->identity[0])
					p->identity[0] = 3;//是餐客 
			}
			if (!(dx == p->houseLoc[0][0] && dy == p->houseLoc[0][1]))
			{
				p->stop = 2;
				p->identity[p->stop - 1] = order->iStatus;//停靠点的身份 
				p->houseLoc[p->stop - 1][0] = dx;//停靠点的x坐标 
				p->houseLoc[p->stop - 1][1] = dy;//停靠点的y坐标 
			}
			if (order->iStatus == 2)
			{
				if (order->iNumber != p->finishOrder[p->stop - 1][p->finishOrder[p->stop - 1][0]])
				{
					p->finishOrder[p->stop - 1][0]++;//该房子的订单数增加
					p->finishOrder[p->stop - 1][p->finishOrder[p->stop - 1][0]] = order->iNumber;//储存订单序号
				}
			}
		}
		if(p->stop == 2)//停靠的房子数为2
		{
			for (int t = 0; t < p->stop - 1; t++)
			{
				if (dx == p->houseLoc[t][0] && dy == p->houseLoc[t][1])//如果当前点坐标跟某个停靠房子相同
				{
					if(order->iStatus != p->identity[t])
						p->identity[t] = 3;

					if (order->iStatus == 2)
					{
						p->finishOrder[t][0]++;//该房子的订单数增加
						p->finishOrder[t][p->finishOrder[p->stop - 1][0]] = order->iNumber;//储存订单序号
					}
				}
			}
		}

		order->iStatus++;
		if (order->iStatus == 3)//订单完成 
		{
			if (TIME - order->iAppearTime > TIMELIMIT1)//罚时 
			{
				oindex[curOverNum] = order->iNumber;
				curOverNum++;
				achieveNum++;
				overtimeNum++;
				MONEY -= 50;
			}
			else
			{
				aindex[curAchieveNum] = order->iNumber;
				curAchieveNum++;
				achieveNum++;
				MONEY += 10;
			}

			p->orderNum--;
			if (p->orderNum == 0)
				p->CurrentOrder[0] = NULL;
			else
				for (int i = 0; i < p->orderNum; i++)//把后续订单往前挪 
				{
					p->CurrentOrder[i] = p->CurrentOrder[i + 1];
				}
			p->CurrentOrder[p->orderNum] = NULL;
		}//更新订单状态 和钱 


		destinationNum--;
		p->disAndPath.destNum--;
		if (p->disAndPath.destNum == 0)//没任务就让他不动
		{
			p->disAndPath.order[0] = NULL;
		}

		else//把后续坐标点往前挪 
		{
			for (int i = 0; i < p->disAndPath.destNum; i++)//把后续坐标点往前挪 
			{
				p->disAndPath.destlist[i][0] = p->disAndPath.destlist[i + 1][0];
				p->disAndPath.destlist[i][1] = p->disAndPath.destlist[i + 1][1];
				p->disAndPath.order[i] = p->disAndPath.order[i + 1];
			}
			p->disAndPath.destlist[p->disAndPath.destNum][0] = 99;
			p->disAndPath.destlist[p->disAndPath.destNum][1] = 99;
			p->disAndPath.order[p->disAndPath.destNum] = NULL;
		}

		dx = p->disAndPath.destlist[0][0];
		dy = p->disAndPath.destlist[0][1];
		order = p->disAndPath.order[0];
		deltaX = abs(dx - rx);
		deltaY = abs(dy - ry);
	}
}

//处理分配新出现的订单
void DeliverOrder(ORDERSPTR currentOrder)
{
	RIDERSPTR pRider = headRider;
	RIDERSPTR okRiders[10];//可按时完成订单的骑手
	RIDERSPTR lessOkRiders[10];//会罚时完成订单的骑手 
	DAD riderDAD[10]; //暂存对应序号的骑手路线，若被选中则把路线分配到骑手身上 
	int okRiderNum = 0, lessOkNum = 0;

	//遍历一遍骑手，存最短路径及距离
	while (pRider != NULL)
	{
		if (pRider->orderNum < 50)
			riderDAD[pRider->iNumber] = PathPlanning(pRider, currentOrder);//按方案一调度 
		else
		{
			riderDAD[pRider->iNumber] = GAPathPlanning(pRider->orderNum, pRider, currentOrder);
		}
			
		//把按时完成的骑手存起来 
		if (riderDAD[pRider->iNumber].ifOvertime == 1)
		{
			okRiderNum++;
			okRiders[okRiderNum] = pRider;
		}
		//把罚时完成的存起来 
		if (riderDAD[pRider->iNumber].ifOvertime == 2)
		{
			lessOkNum++;
			lessOkRiders[lessOkNum] = pRider;
		}
		pRider = pRider->nextPtr;
	}

	if (okRiderNum != 0)//如果有可以按时完成的骑手
	{
		RIDERSPTR chosenRider = ChooseRider(okRiders, okRiderNum, riderDAD);
		chosenRider->disAndPath = riderDAD[chosenRider->iNumber];
		chosenRider->CurrentOrder[chosenRider->orderNum] = currentOrder;
		chosenRider->orderNum++;
	}
	else//挑个会罚时的或买新的或破产	
	{
		if (MONEY >= 300)//钱够就买新的 
		{
			headRider = CreateRiderList(headRider);
			//给新买的骑手分配任务 
			RIDERSPTR newRider = headRider->prevPtr;
			newRider->disAndPath = PathPlanning(newRider, currentOrder);
			newRider->CurrentOrder[newRider->orderNum] = currentOrder;
			newRider->orderNum++;
		}
		else if (lessOkNum != 0) //退而求其次
		{
			RIDERSPTR chosenRider = ChooseRider(lessOkRiders, lessOkNum, riderDAD);
			chosenRider->disAndPath = riderDAD[chosenRider->iNumber];
			chosenRider->CurrentOrder[chosenRider->orderNum] = currentOrder;
			chosenRider->orderNum++;
		}
		else//还不行就gg
		{
			if (isdead == 0)//如果是第一次出现会让你破产的单子 
			{
				isdead = 1;
				deadTime = TIME + TIMELIMIT2;//死亡倒计时 
			}
		}
	}
}

//选择骑手进行派单 
RIDERSPTR ChooseRider(RIDERSPTR riders[], int riderNum, DAD riderDAD[])//按总共
{
	RIDERSPTR minRider = riders[1];
	int index = minRider->iNumber;

	for (int i = 2; i <= riderNum; i++)
	{
		int j = riders[i]->iNumber;
		if (riderDAD[j].distance < riderDAD[index].distance)
		{
			index = j;
			minRider = riders[i];
		}
	}

	return minRider;
}

//普通顺路方法规划路径 
DAD PathPlanning(RIDERSPTR rider, ORDERSPTR order)
{
	DAD path = rider->disAndPath;
	int num = path.destNum;//目标点数 
	int ox1, oy1, ox2, oy2, rx, ry;
	int r[2];//骑手坐标 
	int dis = 0;

	ox1 = order->iSellerX;
	oy1 = order->iSellerY;
	ox2 = order->iBuyerX;
	oy2 = order->iBuyerY;
	rx = rider->iRiderX;
	ry = rider->iRiderY;
	r[0] = rx;
	r[1] = ry;

	if (num == 0)
	{
		path.destNum = 2;
		path.destlist[0][0] = ox1;
		path.destlist[0][1] = oy1;
		path.destlist[1][0] = ox2;
		path.destlist[1][1] = oy2;
		path.order[0] = order;
		path.order[1] = order;

		dis += Distance(r, path.destlist[0]);
		dis += Distance(path.destlist[0], path.destlist[1]);
		path.distance = dis;
	}
	//暂时不考虑绕路，先只考虑顺路情况 
	else
	{
		//先处理卖家 
		int index;
		for (int i = 0; i < num; i++)
		{
			int dx2, dy2, dx1, dy1;
			int minx, miny, maxx, maxy;

			dx2 = path.destlist[i][0];
			dy2 = path.destlist[i][1];
			if (i == 0)
			{
				dx1 = rx;
				dy1 = ry;
			}
			else
			{
				dx1 = path.destlist[i - 1][0];
				dy1 = path.destlist[i - 1][1];
			}
			minx = dx1 < dx2 ? dx1 : dx2;
			miny = dy1 < dy2 ? dy1 : dy2;
			maxx = dx1 > dx2 ? dx1 : dx2;
			maxy = dy1 > dy2 ? dy1 : dy2;

			//如果在顺路区域内，则让该点成为第i个目标点，其余目标点后移 
			if ((ox1 <= maxx) && (ox1 >= minx) && (oy1 <= maxy) && (oy1 >= miny))
			{
				for (int j = num; j > i; j--)
				{
					path.destlist[j][0] = path.destlist[j - 1][0];
					path.destlist[j][1] = path.destlist[j - 1][1];
					path.order[j] = path.order[j - 1];
				}
				path.destlist[i][0] = ox1;
				path.destlist[i][1] = oy1;
				path.order[i] = order;
				path.destNum++;
				index = i;
				break;
			}
		}//end of for-loop

		if (path.destNum == num)//若没办法顺路，则插在目标点末尾 
		{
			path.destlist[num][0] = ox1;
			path.destlist[num][1] = oy1;
			path.destlist[num + 1][0] = ox2;
			path.destlist[num + 1][1] = oy2;
			path.order[num] = order;
			path.order[num + 1] = order;
			dis += Distance(path.destlist[num - 1], path.destlist[num]);
			dis += Distance(path.destlist[num], path.destlist[num + 1]);
			path.destNum += 2;
			path.distance += dis;
		}
		else//若顺路成功，则在其后继续为买家寻找顺路的点 
		{
			num++;
			for (int i = index + 1; i < num; i++)
			{
				int dx2, dy2, dx1, dy1;
				int minx, miny, maxx, maxy;

				dx2 = path.destlist[i][0];
				dy2 = path.destlist[i][1];
				dx1 = path.destlist[i - 1][0];
				dy1 = path.destlist[i - 1][1];

				minx = dx1 < dx2 ? dx1 : dx2;
				miny = dy1 < dy2 ? dy1 : dy2;
				maxx = dx1 > dx2 ? dx1 : dx2;
				maxy = dy1 > dy2 ? dy1 : dy2;

				if ((ox2 <= maxx) && (ox2 >= minx) && (oy2 <= maxy) && (oy2 >= miny))
				{
					for (int j = num; j > i; j--)
					{
						path.destlist[j][0] = path.destlist[j - 1][0];
						path.destlist[j][1] = path.destlist[j - 1][1];
						path.order[j] = path.order[j - 1];
					}
					path.destlist[i][0] = ox2;
					path.destlist[i][1] = oy2;
					path.order[i] = order;
					path.destNum++;
					break;
				}
			}//end of for-loop

			if (path.destNum == num)
			{
				path.destlist[num][0] = ox2;
				path.destlist[num][1] = oy2;
				path.order[num] = order;
				path.destNum++;
				path.distance += Distance(path.destlist[num - 1], path.destlist[num]);
			}
		}
	}

	if (path.distance <= TIMELIMIT1)
		path.ifOvertime = 1;
	else if (path.distance < TIMELIMIT2)
		path.ifOvertime = 2;
	else
		path.ifOvertime = 0;

	return path;
}

//遗传算法计算路径 
DAD GAPathPlanning(int OrderNum, RIDERSPTR rider, ORDERSPTR order)
{
	int a = 0, b = 0;//a类订单有两个目的地，b类订单只有一个目的地 
	int lenChrom;// 染色体长度(这里即为城市个数)
	int chrom[SIZEPOP][MAXLENCHROM]; // 种群

	int destination[MAXLENCHROM][2];
	int minDistance; // 最短路径长度		

	int bestFitIndex = 0; //最短路径出现代数
	int distanceChrom[SIZEPOP];//每个种群的距离总长度 
	int dis;
	int* bestPath; // 计算最短路径及序号
	int bestIndex; // 最短路径序号

	int* newPath;
	int newMinDis;
	int newIndex;

	//GAorder初始化
	for (int i = 0; i < 25; i++)
	{
		rider->disAndPath.GAorder[i] = NULL;
	}
	for (int i = 0; i < 25; i++)
	{
		if (rider->CurrentOrder[i] == NULL)
		{
			rider->disAndPath.GAorder[i] = order;
			break;
		}
		else
		{
			rider->disAndPath.GAorder[i] = rider->CurrentOrder[i];
		}

	}
	//计算a类、b类订单数量 
	for (int i = 0; i < OrderNum + 1; i++)
	{
		if (rider->disAndPath.GAorder[i] != NULL)
		{
			if (rider->disAndPath.GAorder[i]->iStatus == 1) a++;
			if (rider->disAndPath.GAorder[i]->iStatus == 2) b++;
		}
	}
	lenChrom = 2 * a + b;

	//染色体编码
	for (int i = 0, temp = -1; i < a; i++)
	{
		int x1, x2, y1, y2;
		for (int j = temp + 1; j < OrderNum + 1; j++)
		{
			if (rider->disAndPath.GAorder[j]->iStatus == 1)
			{
				x1 = rider->disAndPath.GAorder[j]->iSellerX;
				y1 = rider->disAndPath.GAorder[j]->iSellerY;
				x2 = rider->disAndPath.GAorder[j]->iBuyerX;
				y2 = rider->disAndPath.GAorder[j]->iBuyerY;
				temp = j;
				break;
			}
		}
		int add = 0;
		destination[i + add][0] = x1;
		destination[i + add][1] = y1;
		destination[i + a][0] = x2;
		destination[i + a][1] = y2;

	}
	for (int i = 0, temp = -1; i < b; i++)
	{
		int x = 0, y = 0;
		for (int j = temp + 1; j < OrderNum + 1; j++)
		{
			if (rider->disAndPath.GAorder[j]->iStatus == 2)
			{
				x = rider->disAndPath.GAorder[j]->iBuyerX;
				y = rider->disAndPath.GAorder[j]->iBuyerY;
			}
			temp = j;
			break;
		}
		destination[i + 2 * a][0] = x;
		destination[i + 2 * a][1] = y;
	}

	srand((unsigned)time(NULL)); // 初始化随机数种子

	InitChrom(chrom, lenChrom); // 初始化种群


	//计算每个种群的距离总长度 
	for (int j = 0; j < SIZEPOP; j++)
	{
		dis = PathLength(chrom[j], lenChrom, destination, rider);
		distanceChrom[j] = dis;
	}

	int bestResult[MAXLENCHROM]; // 最佳路线
	for (int i = 0; i < MAXLENCHROM; i++)//初始化数组  bestResult[MAXLENCHROM]
		bestResult[i] = 0;

	bestPath = Min(distanceChrom); // 计算最短路径及序号
	minDistance = 10000; // 最短路径
	bestIndex = (*bestPath); // 最短路径序号
	for (int j = 0; j < lenChrom; j++)
		bestResult[j] = chrom[bestIndex][j]; // 最短路径序列

	// 开始进化
	int success = 0;
	for (int i = 0; i < MAXGEN; i++)
	{
		Choice(chrom, lenChrom, destination, rider); // 选择  
		Cross(chrom, lenChrom); //交叉
		Mutation(chrom, lenChrom); //变异
		Reverse(chrom, lenChrom, destination, rider); // 逆转操作

		for (int j = 0; j < SIZEPOP; j++)
			distanceChrom[j] = PathLength(chrom[j], lenChrom, destination, rider); // 距离数组
		newPath = Min(distanceChrom);//新的最短路径序号 
		newMinDis = *(newPath + 1); //新的最短路径长度 
		//判断最短路径是否满足先取货后送货 

		if (success == 0 && i == MAXGEN - 1)
		{

			for (int j = 1; j <= a; j++)
			{
				int seller, buyer;
				for (int k = 0; k < lenChrom; k++)
				{
					if (chrom[(*newPath)][k] == j) seller = k;
					if (chrom[(*newPath)][k] == j + a) buyer = k;
				}
				if (seller > buyer)
				{
					int temp;
					temp = chrom[(*newPath)][seller];
					chrom[(*newPath)][seller] = chrom[(*newPath)][buyer];
					chrom[(*newPath)][buyer] = temp;
				}
			}
			for (int j = 0; j < lenChrom; j++)
				bestResult[j] = chrom[*newPath][j]; // 更新最短路径序列
			minDistance = PathLength(chrom[*newPath], lenChrom, destination, rider);; // 更新最短路径
			newMinDis = minDistance;

		}
		if (newMinDis < minDistance)
		{
			int isOK = 1;
			for (int j = 1; j <= a && isOK == 1; j++)
			{
				int seller, buyer;
				for (int k = 0; k < lenChrom; k++)
				{
					if (chrom[(*newPath)][k] == j) seller = k;
					if (chrom[(*newPath)][k] == j + a) buyer = k;
				}
				if (seller > buyer)
				{
					isOK = 0;
				}
			}
			if (isOK == 1)
			{

				success++;
				minDistance = newMinDis; // 更新最短路径
				newIndex = (*newPath);
				for (int j = 0; j < lenChrom; j++)
					bestResult[j] = chrom[newIndex][j]; // 更新最短路径序列
				bestFitIndex = i + 1; // 最短路径代数

			}
		}
	}

	//判断是否超时并返回距离和该骑手下个目的地坐标
	ORDERSPTR bestResultOrder[MAXLENCHROM];//储存该点属于的订单
	for (int i = 0; i < MAXLENCHROM; i++)
		bestResultOrder[i] = NULL;//初始化

	int riderx = rider->iRiderX;
	int ridery = rider->iRiderY;
	int riderPos[1][2];
	riderPos[0][0] = riderx;
	riderPos[0][1] = ridery;
	int isOK = 1;
	int a1 = 0, b1 = 0;
	for (int i = 0; i < OrderNum + 1; i++)
	{
		int sum = Distance(riderPos[0], destination[0]);
		//如果是a类订单 
		if (rider->disAndPath.GAorder[i]->iStatus == 1)
		{
			a1++;
			for (int j = 0; j < lenChrom; j++)
			{
				if (bestResult[j] == a1)
				{
					bestResultOrder[j] = rider->disAndPath.GAorder[i];
				}
				if (bestResult[j] == a1 + a)
				{
					bestResultOrder[j] = rider->disAndPath.GAorder[i];
					for (int k = 0; k < j - 1; k++)
					{
						sum += Distance(destination[k], destination[k + 1]);
					}
					if (sum + TIME - rider->disAndPath.GAorder[i]->iAppearTime >= TIMELIMIT2) isOK = 0;
					else if (sum + TIME - rider->disAndPath.GAorder[i]->iAppearTime > TIMELIMIT1 && sum + TIME - rider->disAndPath.GAorder[i]->iAppearTime < TIMELIMIT2)
					{
						isOK = 2;
					}
					break;
				}
			}//end of j
		}//end of if a

		//如果是b类订单 
		if (rider->disAndPath.GAorder[i]->iStatus == 2)
		{
			b1++;
			for (int j = 0; j < lenChrom; j++)
			{
				if (bestResult[j] == b1 + 2 * a)
				{
					bestResultOrder[j] = rider->disAndPath.GAorder[i];
					for (int k = 0; k < j - 1; k++)
					{
						sum += Distance(destination[k], destination[k + 1]);
					}
					if (sum + TIME - rider->disAndPath.GAorder[i]->iAppearTime >= TIMELIMIT2) isOK = 0;   //超时 
					else if (sum + TIME - rider->disAndPath.GAorder[i]->iAppearTime > TIMELIMIT1) isOK = 2;//罚时了 
					break;
				}
			}//end of j
		}//end of if b
	}//end of i
	//确定下一个目的地坐标

	//确定返回值
	DAD P;
	P.ifOvertime = isOK;

	if (isOK == 1 || isOK == 2)
	{
		P.distance = minDistance + Distance(riderPos[0], destination[bestResult[0] - 1]);
		P.destNum = lenChrom;
		for (int i = 0; i < P.destNum; i++)
		{
			P.order[i] = bestResultOrder[i];

			P.destlist[i][0] = destination[bestResult[i] - 1][0];
			P.destlist[i][1] = destination[bestResult[i] - 1][1];
		}
	}

	return P;
}

// 种群初始化
void InitChrom(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom)
{
	int num = 0;
	while (num < SIZEPOP)
	{
		for (int i = 0; i < SIZEPOP; i++)
			for (int j = 0; j < lenChrom; j++)
				chrom[i][j] = j + 1;
		num++;
		for (int i = 0; i < lenChrom - 1; i++)
		{
			for (int j = i + 1; j < lenChrom; j++)
			{
				int temp = chrom[num][i];
				chrom[num][i] = chrom[num][j];
				chrom[num][j] = temp; // 交换第num个个体的第i个元素和第j个元素
				num++;
				if (num >= SIZEPOP)
					break;
			}
			if (num >= SIZEPOP)
				break;
		}
		// 如果经过上面的循环还是无法产生足够的初始个体，则随机再补充一部分
		// 具体方式就是选择两个基因位置，然后交换
		while (num < SIZEPOP)
		{
			double r1 = ((double)rand()) / (RAND_MAX + 1.0);
			double r2 = ((double)rand()) / (RAND_MAX + 1.0);
			int p1 = (int)(lenChrom * r1); // 位置1
			int p2 = (int)(lenChrom * r2); // 位置2
			int temp = chrom[num][p1];
			chrom[num][p1] = chrom[num][p2];
			chrom[num][p2] = temp;    // 交换基因位置
			num++;
		}
	}
}

// 距离函数
int Distance(int* city1, int* city2)
{
	int x1 = *city1;
	int y1 = *(city1 + 1);
	int x2 = *(city2);
	int y2 = *(city2 + 1);
	int dis;
	if (x1 == x2 || y1 == y2)
		dis = (abs(x1 - x2) + abs(y1 - y2)) / 2;
	else
		dis = (abs(x1 - x2) + abs(y1 - y2) - 1) / 2;
	return dis;
}

// Min()函数
int* Min(int* arr)
{
	static int bestPath[2];
	int min_dis = *arr;
	int min_index = 0;
	for (int i = 1; i < SIZEPOP; i++)
	{
		int dis = *(arr + i);
		if (dis < min_dis)
		{
			min_dis = dis;
			min_index = i;
		}
	}
	bestPath[0] = min_index;
	bestPath[1] = min_dis;
	return bestPath;
}\

// 计算路径长度
int PathLength(int* arr, int lenChrom, int destination[MAXLENCHROM][2], RIDERSPTR rider)
{
	int path = 0; // 初始化路径长度
	int bestIndex = *arr; // 定位到第一个数字(城市序号)
	for (int i = 0; i < lenChrom - 1; i++)
	{
		int index1 = *(arr + i);
		int index2 = *(arr + i + 1);
		int dis = Distance(destination[index1 - 1], destination[index2 - 1]);
		path += dis;
	}
	//初始化骑手坐标
	int riderx = rider->iRiderX;
	int ridery = rider->iRiderY;
	int riderPos[2];
	riderPos[0] = riderx;
	riderPos[1] = ridery;
	int first_index = *arr; // 第一个城市序号
	int first_dis = Distance(riderPos, destination[first_index - 1]);
	path = path + first_dis;
	return path; // 返回总的路径长度
}

// 选择操作
void Choice(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom, int destination[MAXLENCHROM][2], RIDERSPTR rider)
{
	double pick;
	int choice_arr[SIZEPOP][MAXLENCHROM];
	double fit_pro[SIZEPOP];
	double sum = 0;
	double fit[SIZEPOP]; // 适应度函数数组(距离的倒数)
	for (int j = 0; j < SIZEPOP; j++)
	{
		double path = PathLength(chrom[j], lenChrom, destination, rider);
		double fitness = 1.0 / path;
		fit[j] = fitness;
		sum += fitness;
	}
	for (int j = 0; j < SIZEPOP; j++)
	{
		fit_pro[j] = fit[j] / sum; // 概率数组
	}
	// 开始轮盘赌
	for (int i = 0; i < SIZEPOP; i++)
	{
		pick = ((double)rand()) / RAND_MAX; // 0到1之间的随机数  
		for (int j = 0; j < SIZEPOP; j++)
		{
			pick = pick - fit_pro[j];
			if (pick <= 0)
			{
				for (int k = 0; k < lenChrom; k++)
					choice_arr[i][k] = chrom[j][k]; // 选中一个个体
				break;
			}
		}
	}
	for (int i = 0; i < SIZEPOP; i++)
	{
		for (int j = 0; j < lenChrom; j++)
			chrom[i][j] = choice_arr[i][j];
	}
}

//交叉操作
void Cross(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom)
{
	double pick;
	double pick1, pick2;
	int choice1, choice2;
	int pos1, pos2;
	int temp;
	int conflict1[MAXLENCHROM]; // 冲突位置
	int conflict2[MAXLENCHROM];
	int num1, num2;
	int index1, index2;
	int move = 0; // 当前移动的位置
	while (move < lenChrom - 1)
	{
		pick = ((double)rand()) / RAND_MAX; // 用于决定是否进行交叉操作
		if (pick > PCROSS)
		{
			move += 2;
			continue; // 本次不进行交叉
		}
		// 采用部分映射杂交
		choice1 = move; // 用于选取杂交的两个父代
		choice2 = move + 1; // 注意避免下标越界
		pick1 = ((double)rand()) / (RAND_MAX + 1.0);
		pick2 = ((double)rand()) / (RAND_MAX + 1.0);
		pos1 = (int)(pick1 * lenChrom); // 用于确定两个杂交点的位置
		pos2 = (int)(pick2 * lenChrom);
		while (pos1 > lenChrom - 2 || pos1 < 1)
		{
			pick1 = ((double)rand()) / (RAND_MAX + 1.0);
			pos1 = (int)(pick1 * lenChrom);
		}
		while (pos2 > lenChrom - 2 || pos2 < 1)
		{
			pick2 = ((double)rand()) / (RAND_MAX + 1.0);
			pos2 = (int)(pick2 * lenChrom);
		}
		if (pos1 > pos2)
		{
			temp = pos1;
			pos1 = pos2;
			pos2 = temp; // 交换pos1和pos2的位置
		}
		for (int j = pos1; j <= pos2; j++)
		{
			temp = chrom[choice1][j];
			chrom[choice1][j] = chrom[choice2][j];
			chrom[choice2][j] = temp; // 逐个交换顺序
		}
		num1 = 0;
		num2 = 0;
		if (pos1 > 0 && pos2 < lenChrom - 1)
		{
			for (int j = 0; j <= pos1 - 1; j++)
			{
				for (int k = pos1; k <= pos2; k++)
				{
					if (chrom[choice1][j] == chrom[choice1][k])
					{
						conflict1[num1] = j;
						num1++;
					}
					if (chrom[choice2][j] == chrom[choice2][k])
					{
						conflict2[num2] = j;
						num2++;
					}
				}
			}
			for (int j = pos2 + 1; j < lenChrom; j++)
			{
				for (int k = pos1; k <= pos2; k++)
				{
					if (chrom[choice1][j] == chrom[choice1][k])
					{
						conflict1[num1] = j;
						num1++;
					}
					if (chrom[choice2][j] == chrom[choice2][k])
					{
						conflict2[num2] = j;
						num2++;
					}
				}

			}
		}
		if ((num1 == num2) && num1 > 0)
		{
			for (int j = 0; j < num1; j++)
			{
				index1 = conflict1[j];
				index2 = conflict2[j];
				temp = chrom[choice1][index1]; // 交换冲突的位置
				chrom[choice1][index1] = chrom[choice2][index2];
				chrom[choice2][index2] = temp;
			}
		}
		move += 2;
	}
}

// 变异操作   变异策略采取随机选取两个点，将其对换位置
void Mutation(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom) // 变异操作
{
	double pick, pick1, pick2;
	int pos1, pos2, temp;
	for (int i = 0; i < SIZEPOP; i++)
	{
		pick = ((double)rand()) / RAND_MAX; // 用于判断是否进行变异操作
		if (pick > PMUTATION)
			continue;
		pick1 = ((double)rand()) / (RAND_MAX + 1.0);
		pick2 = ((double)rand()) / (RAND_MAX + 1.0);
		pos1 = (int)(pick1 * lenChrom); // 选取进行变异的位置
		pos2 = (int)(pick2 * lenChrom);
		while (pos1 > lenChrom - 1)
		{
			pick1 = ((double)rand()) / (RAND_MAX + 1.0);
			pos1 = (int)(pick1 * lenChrom);
		}
		while (pos2 > lenChrom - 1)
		{
			pick2 = ((double)rand()) / (RAND_MAX + 1.0);
			pos2 = (int)(pick2 * lenChrom);
		}
		temp = chrom[i][pos1];
		chrom[i][pos1] = chrom[i][pos2];
		chrom[i][pos2] = temp;
	}
}

// 进化逆转操作
void Reverse(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom, int destination[MAXLENCHROM][2], RIDERSPTR rider)
{
	double pick1, pick2;
	double dis, reverse_dis;
	int n;
	int flag, pos1, pos2, temp;
	int reverse_arr[MAXLENCHROM];

	for (int i = 0; i < SIZEPOP; i++)
	{
		flag = 0; // 用于控制本次逆转是否有效
		while (flag == 0)
		{
			pick1 = ((double)rand()) / (RAND_MAX + 1.0);
			pick2 = ((double)rand()) / (RAND_MAX + 1.0);
			pos1 = (int)(pick1 * lenChrom); // 选取进行逆转操作的位置
			pos2 = (int)(pick2 * lenChrom);
			while (pos1 > lenChrom - 1)
			{
				pick1 = ((double)rand()) / (RAND_MAX + 1.0);
				pos1 = (int)(pick1 * lenChrom);
			}
			while (pos2 > lenChrom - 1)
			{
				pick2 = ((double)rand()) / (RAND_MAX + 1.0);
				pos2 = (int)(pick2 * lenChrom);
			}
			if (pos1 > pos2)
			{
				temp = pos1;
				pos1 = pos2;
				pos2 = temp; // 交换使得pos1 <= pos2
			}
			if (pos1 < pos2)
			{
				for (int j = 0; j < lenChrom; j++)
					reverse_arr[j] = chrom[i][j]; // 复制数组
				n = 0; // 逆转数目
				for (int j = pos1; j <= pos2; j++)
				{
					reverse_arr[j] = chrom[i][pos2 - n]; // 逆转数组
					n++;
				}
				reverse_dis = PathLength(reverse_arr, lenChrom, destination, rider); // 逆转之后的距离
				dis = PathLength(chrom[i], lenChrom, destination, rider); // 原始距离
				if (reverse_dis < dis)
				{
					for (int j = 0; j < lenChrom; j++)
						chrom[i][j] = reverse_arr[j]; // 更新个体
				}
			}
			flag = 1;
		}
	}
}
