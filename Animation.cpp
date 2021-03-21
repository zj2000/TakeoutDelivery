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

//���嶩���ṹ 
typedef struct orders
{
	int iNumber;//������� 
	int iAppearTime;//��������ʱ��
	int iSellerX, iSellerY;//�͹�����
	int iBuyerX, iBuyerY;//ʳ������
	int iStatus;//������ǰ״̬��1ȡ��;�� / 2�ͻ�;�� / 3��ʱ��� / 4��ʱ��� 
	int iBeforeTakenTime;//�������ֺ󱻽ӵ�ǰ������ʱ��
	int iPastTime;//�ӵ��󾭹���ʱ��
	struct orders* nextPtr;//��һ�����
	struct orders* PrevPtr;//��һ���ڵ�

}ORDERS;
typedef ORDERS* ORDERSPTR;

typedef struct DistanceAndDestination
{
	int ifOvertime;//0��ʱ��>60�� 1��ʱ���   2��ʱ��� 
	int distance;
	int destlist[50][2];//x���� ��y���� 
	int destNum;
	ORDERSPTR order[50];//Ŀ������ĸ�����
	ORDERSPTR GAorder[25];//���������ϵĶ�������ΪCurrentOrder����ʱ�洢
}DAD;
typedef DAD* DADPTR;

//�������ֽṹ 
typedef struct riders
{
	int iNumber;//������� 
	int iRiderX, iRiderY;//������Ƭ���� 
	int orderNum;//�����ֽӵ���

	int stop;//�Ƿ�ͣ��
	int identity[2];//ͣ��������
	int houseLoc[2][2];//ͣ�����ӵ�����
	int finishOrder[2][6];//ÿ��������ɵĶ������([0]:�õ�Ķ�����)

	int screenX, screenY;//��������Ļ�ϵ�����
	int directioni;//�����Ӧ��ͼƬλ��0����  1����   2����   3����
	int direction[2];//ÿ��ʱ�䵥λ����������¼ÿ���ķ��򡪡�0������  1������  2������  3������  4������
	int firstSX, firstSY;//���ֵ�һ����Ҫ������ӵ�������Ļ����
	int secondSX, secondSY;//���ֵڶ�����Ҫ������ӵ�������Ļ����
	int countTiles;//�����߹��ĸ����� //�Ƹ���  һ����λʱ��Ӧ��2��
	int finishTime;//���һ��������ʱ�䣨���ͼ����ֵ�ʱ�䣩

	ORDERSPTR CurrentOrder[25];//���ֵ�ǰ�Ķ��� 
	DAD disAndPath;
	struct riders* nextPtr;
	struct riders* prevPtr;
}RIDERS;
typedef RIDERS* RIDERSPTR;

#define MAXGEN 1000  // ���������� 
#define MAXLENCHROM  30//Ⱦɫ����󳤶�
#define SIZEPOP 100 // ��Ⱥ��Ŀ
#define PCROSS 0.6 // �������
#define PMUTATION 0.1 // �������
#define TIMELIMIT1 30//��ʱʱ��
#define TIMELIMIT2 60//��ʱʱ��
#define timeNum 2//ÿ��ʱ�䵥λ��Ӧ����ֵ
#define timeDuration 2000 //ÿ��ʱ�䵥λ��Ӧ�ĺ�����
#define TextHEIGHT 22 //�ı���߶�
#define CANVASHEIGHT 558//�����߶�
#define CANVASWIDTH 1088//�������
#define FINISHWIDTH 42//���ͼ���
#define FINISHHEIGHT 27//���ͼ�߶�
#define RIDERWIDTH 24//����ͼƬ���
#define RIDERHEIGHT 43//����ͼƬ�߶�
#define riderDeltaImgX  11 //����ͼ����ͼλ�������x�����ƫ��
#define riderDeltaImgY  34//����ͼ����ͼλ�������y�����ƫ��
#define SCREENX0 576//���ֳ�ʼxλ��screen
#define SCREENY0 302//���ֳ�ʼyλ��screen
#define TILEX0 9//���ֳ�ʼ��Ƭ���� x
#define TILEY0 8//���ֳ�ʼ��Ƭ���� y
#define TILEWIDTH 64//��Ƭ���
#define TILEHEIGHT 32//��Ƭ�߶�
#define HOUSEX0 544//���ӣ�0��0���ĳ�ʼx����screen
#define HOUSEY0 14//���ӣ�0��0���ĳ�ʼy����screen
#define SPEED 1//�����ٶȣ����أ�

int music = 1;//1:on  0:off
int clickTime = 0;//��������ӵĴ��� 
int MONEY = 1000;//Ǯ
int TIME = 0;//ȫ��ʱ�䣨��ʾ��ʱ�䣩
int takenNum = 0; // �ӵ��� 
int achieveNum = 0;//����� 
int overtimeNum = 0; //��ʱ��
int riderNum = 3;//�����������
int destinationNum = 0;//��������ʣ��ĵ��� 
int isStop = 0;//�Ƿ���ͣ
int isOver = 0;//�Ƿ���� 
int curAchieveNum = 0;//ÿ��ᵥ�� 
int curOverNum = 0;//ÿ�뷣���� 
int aindex[20];//ÿ����ɵĶ�����
int oindex[20];//ÿ�뷣���Ķ����� 
int isdead = 0;//�Ƿ���ֻ������Ʋ��ĵ��ӣ�0û�У�1�� 
int deadTime = 9999999;

clock_t startTime = 0;//��ʼʱ��
clock_t stopTime = 0;//��ͣʱ��
clock_t endStopTime = 0;//������ͣ��ʱ�䣨���������ʱ�䣩
clock_t stopInterval = 0;//��ͣ���
clock_t totStopInterval = 0;//�ܹ���ͣʱ��
clock_t currentTime = 0;//��ǰʱ��
IMAGE img_startUI;//��ʼ����
IMAGE img_map;//��ͼ
IMAGE img_rider, img_ridercover;//����ͼ����������ͼ
IMAGE img_finish, img_finishcover;//���ͼ�꼰������ͼ
IMAGE img_takeorder1, img_takeorder2;//�ӵ��¼��г��ֵ���ʾͼ
IMAGE img_musicON, img_musicOFF;//���ְ�ťͼ��
IMAGE img_newA, img_newB;//���µ�����ʾͼ
IMAGE img_orderIndexRed;//��ɫ��ʾ����ͼ
IMAGE img_orderIndexBlue;//��ɫ��ʾ����ͼ

ORDERSPTR headOrder = NULL; //����ͷָ�� 
ORDERSPTR currentOrder = NULL;//��ǰ����
RIDERSPTR headRider = NULL;	//����ͷָ�� 


void InitStartUI();//��ʼ����ʼ���棨��δ��ʱ��
void EndPrint();//��������
void BreakPrint();//�Ʋ�����
void DrawFinishHint(RIDERSPTR pRider);
int Convert90to45_X(int screenX, int screenY);//ֱ������ϵת��Ϊб����ϵ  X
int Convert90to45_Y(int screenX, int screenY);//ֱ������ϵת��Ϊб����ϵ  Y
int Convert45to90_X(int tileX, int tileY);//б����ϵת��Ϊֱ������ϵ X
int Convert45to90_Y(int tileX, int tileY);//б����ϵת��Ϊֱ������ϵ Y
void MoveRider(RIDERSPTR pRider, int step);//�ƶ�����
void TurnLeft(RIDERSPTR rider);
void TurnRight(RIDERSPTR rider);
void TurnUp(RIDERSPTR rider);
void TurnDown(RIDERSPTR rider);
void Draw();//������ͼ�����֣�
void DrawHint(int i);//��ͼ���㵥��ʾ��
void StopEvent();//������ͣ�¼�
void ContinueEvent();//��������¼�
void InitString(char s[], int len);//��ʼ���ַ���
void PrintText1();//��ӡ�ı���1
void PrintRiderText();//��ӡ������Ϣ�ı���
void PrintMouseCoordinate();//��ӡ��������
ORDERSPTR CreateOrderList(ORDERSPTR head, int x, int y);//������������
RIDERSPTR CreateRiderList(RIDERSPTR head);//������������
void DeliverOrder(ORDERSPTR currentOrder);//��������³��ֵĶ���
void UpdateDirection(RIDERSPTR p);//ȷ�����ֵ��ƶ����򣨻�δ�ı��������꣩
void JudgeArrive(RIDERSPTR p);//�ж��Ƿ񵽴�Ŀ��㲢����
void LoadRiderImg(RIDERSPTR newPtr, int iNumber);//�����ּ���ͼƬ
void LoadFinishImg(int identity);//�������ͼƬ

DAD PathPlanning(RIDERSPTR rider, ORDERSPTR order);
DAD GAPathPlanning(int OrderNum, RIDERSPTR rider, ORDERSPTR order);
void InitChrom(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom); // ��Ⱥ��ʼ������
int Distance(int*, int*); // ������������֮��ľ���
int* Min(int*); // ��������������Сֵ
int PathLength(int* arr, int lenchrom, int destination[MAXLENCHROM][2], RIDERSPTR rider); // ����ĳһ��������·�����ȣ���Ӧ�Ⱥ���Ϊ·�߳��ȵĵ���
void Choice(int chrom[SIZEPOP][MAXLENCHROM], int lenchrom, int destination[MAXLENCHROM][2], RIDERSPTR rider); // ѡ�����
void Cross(int chrome[SIZEPOP][MAXLENCHROM], int lenChrom); // �������
void Mutation(int chrome[SIZEPOP][MAXLENCHROM], int lenChrom); // �������
void Reverse(int chrom[SIZEPOP][MAXLENCHROM], int lenchrom, int destination[MAXLENCHROM][2], RIDERSPTR rider); // ��ת����
RIDERSPTR ChooseRider(RIDERSPTR riders[], int riderNum, DAD riderDAD[]);//ѡ������

int main()
{
	int isStart = 0;//�Ƿ�ʼ 0:δ��ʼ  1:��ʼ
	int isEnd = 0;//�Ƿ����end
	int isBreak = 0;//�Ƿ��Ʋ�
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
					putimage(0, 0, &img_map);//������ͼ
					putimage(765, 30, &img_musicON);
					//��������
					mciSendString(_T("play ./Resource/BGM.wav"), 0, 0, 0);
					for (int i = 1; i <= riderNum; i++)//��ʼ��������Ϣ  ���ֹ���3������ 
						headRider = CreateRiderList(headRider);

					isStart = 1;
					startTime = clock();
				}
			}
		}
		if (isStart == 1)
		{
			currentTime = clock();//��ȡ��ǰʱ��
			if (tempTime1 == -1)
				tempTime1 = currentTime;

			if (MouseHit())//�������з�Ӧ
			{
				MOUSEMSG mouse = GetMouseMsg();
				//������������£����ж��ǵ���ĸ���ť��ִ����ز���
				if (mouse.mkLButton)
				{
					mouse = GetMouseMsg();
					int mouseScreenX = mouse.x;//������Ļ����x
					int mouseScreenY = mouse.y;//������Ļ����y
					int mouseTileX = Convert90to45_X(mouseScreenX, mouseScreenY);//������Ƭ����x
					int mouseTileY = Convert90to45_Y(mouseScreenX, mouseScreenY);//������Ƭ����y

					//���������������ڵ㵥
					if ((mouseTileX >= 0) && (mouseTileX <= 16) && (mouseTileY >= 0) && (mouseTileY <= 16) && (mouseTileX % 2 == 0) && (mouseTileY % 2 == 0))
					{
						clickTime++;
						if (clickTime % 2 == 1)//��һ�ε�����ǲ͹�
						{
							headOrder = CreateOrderList(headOrder, mouseTileX, mouseTileY);
							currentOrder = headOrder->PrevPtr;
						}
						if (clickTime % 2 == 0)//�ڶ��ε������ʳ��
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

					//�����Stop����ͣ
					if ((mouseScreenX >= 920) && (mouseScreenX <= 1038) && (mouseScreenY >= 33) && (mouseScreenY <= 72))
					{
						StopEvent();
					}

					//�����Continue������
					if ((mouseScreenX >= 920) && (mouseScreenX <= 1038) && (mouseScreenY >= 78) && (mouseScreenY <= 117))
					{
						ContinueEvent();
					}

					//�����End������
					if ((mouseScreenX >= 920) && (mouseScreenX <= 1038) && (mouseScreenY >= 122) && (mouseScreenY <= 161))
					{
						isStop = 1;
						EndPrint();
					}

					//�����Music��
					if ((mouseScreenX >= 765) && (mouseScreenX <= 906) && (mouseScreenY >= 30) && (mouseScreenY <= 71))
					{
						if (music == 1)//������֮ǰ�����ǿ��ģ���ĳɹص�
						{
							putimage(765, 30, &img_musicOFF);
							music = 0;
							mciSendString(_T("stop ./Resource/BGM.wav"), 0, 0, 0);
						}
						else//���֮ǰ�ǹصģ����
						{
							putimage(765, 30, &img_musicON);
							music = 1;
							mciSendString(_T("play ./Resource/BGM.wav"), 0, 0, 0);
						}
					}
				}
			}

			//���û����ͣ,����Ҫ���½���
			if (isStop == 0)
			{
				int dt = (timeDuration / 2) / (TILEHEIGHT / 2);
				if (currentTime - tempTime1 >= dt)//�������ʱ������ˢ�»���
				{
					tempTime1 = currentTime;
					RIDERSPTR p = headRider;
					while (p != NULL)//����һ������
					{
						//������ֲ���
						if (p->direction[p->countTiles] == 0)
						{
							JudgeArrive(p);
							UpdateDirection(p);
						}
						//������ֶ�
						if (p->direction[p->countTiles] != 0)
						{
							if (p->countTiles == 0)//�����û�ߵ���һ�������
							{
								if (p->screenX == p->firstSX && p->screenY == p->firstSY)
									p->countTiles = 1;
								else
									MoveRider(p, 0);
							}
							if (p->countTiles == 1)//��������һ��
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

					Draw();//ˢ�»���
				}
			}
			
			//�����ͣ��
			if (isStop != 0)
			{
				stopInterval = currentTime - stopTime;
			}

			currentTime = clock();
			int newTIME = (currentTime - startTime - totStopInterval - stopInterval) / timeDuration;//����µ�TIME���Ƿ�����
			if (newTIME - TIME == 1)//���IIME+1������
			{
				TIME = newTIME;
				curAchieveNum = 0;//ÿ��ᵥ�� 
				curOverNum = 0;//ÿ�뷣���� 
			}

			if ((MONEY < 0) || (isdead == 1 && TIME >= deadTime))
				BreakPrint();
		}
	}
	mciSendString(_T("close ./Resource/BGM.wav"), 0, 0, 0);
	closegraph();//�رջ���
}

//��ʼ����ʼ���棨��δ��ʱ��
void InitStartUI()
{
	initgraph(CANVASWIDTH, CANVASHEIGHT);

	//���ر�������
	mciSendString(_T("open ./Resource/BGM.wav"), 0, 0, 0);
	//����ͼƬ
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

int Convert90to45_X(int screenX, int screenY)//ֱ������ϵת��Ϊб����ϵ  
{
	int tileX;
	tileX = (int)(((double)screenX - (double)HOUSEX0 + 0.0) / (double)TILEWIDTH + ((double)screenY - (double)HOUSEY0 + 0.0) / (double)TILEHEIGHT);
	return tileX;
}
int Convert90to45_Y(int screenX, int screenY)//ֱ������ϵת��Ϊб����ϵ  Y
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

//������������
ORDERSPTR CreateOrderList(ORDERSPTR head, int x, int y)
{
	ORDERSPTR current = NULL;
	current = (ORDERSPTR)malloc(sizeof(ORDERS));
	if (current != NULL)
	{
		//��ʼ���������� 
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
//������������ 
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

//�����ּ���ͼƬ
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
//�������ͼƬ
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


//������ͣ�¼�
void StopEvent()
{
	stopTime = currentTime;
	isStop = 1;
}
//��������¼�
void ContinueEvent()
{
	endStopTime = clock();
	stopInterval = endStopTime - stopTime;
	totStopInterval += stopInterval;
	stopInterval = 0;
	isStop = 0;
}

//���л�����ͼ
void Draw()
{
	BeginBatchDraw();
	//������
	putimage(0, 0, &img_map);

	//�����ְ�ť
	if (music == 1)
		putimage(765, 30, &img_musicON);
	if (music == 0)
		putimage(765, 30, &img_musicOFF);

	//����ʾ�㵥��ͼ
	if (clickTime % 2 == 1)
		DrawHint(2);
	else
		DrawHint(1);
	//�����ֵ�ͼ
	RIDERSPTR pRider = headRider;
	while (pRider != NULL)
	{
		LoadRiderImg(pRider, pRider->iNumber);
		putimage(pRider->screenX - riderDeltaImgX, pRider->screenY - riderDeltaImgY, RIDERWIDTH, RIDERHEIGHT, &img_ridercover, pRider->directioni * RIDERWIDTH, 0, SRCPAINT);
		putimage(pRider->screenX - riderDeltaImgX, pRider->screenY - riderDeltaImgY, RIDERWIDTH, RIDERHEIGHT, &img_rider, pRider->directioni * RIDERWIDTH, 0, SRCAND);
		DrawFinishHint(pRider);
		pRider = pRider->nextPtr;
	}
	//��ӡ���ֲ���
	COLORREF color = RGB(35, 43, 133);
	settextcolor(color);
	PrintText1();
	PrintRiderText();

	EndBatchDraw();
}
//��ӡ�㵥ʱ����ʾ
void DrawHint(int i)
{
	//���������ʽ
	settextstyle(12, 0, _T("������κ"));
	COLORREF color = RGB(136, 0, 21);//����ɫ
	settextcolor(color);

	int x, y;
	if (currentOrder != NULL && currentOrder->iStatus < 2)
	{
		x = Convert45to90_X(currentOrder->iSellerX, currentOrder->iSellerY);
		y = Convert45to90_Y(currentOrder->iSellerX, currentOrder->iSellerY);//����Ƭ����ת����ֱ������
		putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 30, &img_finishcover, SRCPAINT);
		putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 30, &img_orderIndexRed, SRCAND);//���²͹ݵĶ������ͼ
		putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 10, &img_finishcover, SRCPAINT);
		putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 10, &img_newA, SRCAND);//���²͹ݵ���ʾͼ
		
		//��ӡ�������
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
			putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 30, &img_orderIndexRed, SRCAND);//���²͹ݵĶ������ͼ
			putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 10, &img_finishcover, SRCPAINT);
			putimage(x - FINISHWIDTH / 2, y - FINISHHEIGHT - 10, &img_newB, SRCAND);//���²͹ݵ���ʾͼ
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
//��ӡ����Ŀ���ʱ����ʾ
void DrawFinishHint(RIDERSPTR pRider)
{
	settextstyle(12, 0, _T("������κ"));//��������
	COLORREF color = RGB(71, 94, 226);//��ɫ
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

				//��ӡ�������
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
//���������ӡ
void EndPrint()
{
	IMAGE img_end;
	loadimage(&img_end, _T("./Resource/EndUI.png"), CANVASWIDTH, CANVASHEIGHT);
	putimage(0, 0, &img_end);
	settextstyle(42, 0, _T("������κ"));
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
//�Ʋ������ӡ
void BreakPrint()
{
	IMAGE img_break;
	loadimage(&img_break, _T("./Resource/BreakUI.png"), CANVASWIDTH, CANVASHEIGHT);
	putimage(0, 0, &img_break);
	settextstyle(TextHEIGHT, 0, _T("������κ"));
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

//�ƶ�����
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

void InitString(char s[], int len)//��ʼ���ַ���
{
	for (int i = 0; i < len; i++)
		s[i] = 0;
}
void PrintText1()
{
	settextstyle(TextHEIGHT, 0, _T("������κ"));//��������

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
}//��ӡ�ı���1
void PrintRiderText()//��ӡ�����ı���
{
	settextstyle(TextHEIGHT - 5, 0, _T("������κ"));

	char s[30];
	int len = 30;
	int h = 445, w = 10, tabWidth = 90;
	RIDERSPTR rider = headRider;
	while (rider != NULL)
	{
		InitString(s, len);
		sprintf_s(s, len, "����%dλ��:(%d,%d) ͣ��:", rider->iNumber , rider->iRiderX, rider->iRiderY);
		w = 10;
		outtextxy(w, h, s);
		if (rider->stop != 0)
		{
			w = 185;
			for (int i = 0; i < rider->stop; i++)
			{
				InitString(s, len);
				if (rider->identity[i] == 1)
					sprintf_s(s, len, "�͹�(%d,%d)", rider->houseLoc[i][0], rider->houseLoc[i][1]);
				if (rider->identity[i] == 2)
					sprintf_s(s, "ʳ��(%d,%d)", rider->houseLoc[i][0], rider->houseLoc[i][1]);
				if (rider->identity[i] == 3)
					sprintf_s(s, len, "�Ϳ�(%d,%d)", rider->houseLoc[i][0], rider->houseLoc[i][1]);
				outtextxy(w, h, s);
				w += tabWidth;
			}
		}
		h += TextHEIGHT + 1;
		rider = rider->nextPtr;
	}
}

//ȷ�����ֵ��ƶ����򣨻�δ�ı��������꣩
void UpdateDirection(RIDERSPTR p)
{
	ORDERSPTR order = p->disAndPath.order[0];
	int dnum = p->disAndPath.destNum;//�������ϵ�Ŀ����� 
	int rx, ry, dx, dy;//���ֺ�Ŀ������� 
	int deltaX, deltaY;
	rx = p->iRiderX;
	ry = p->iRiderY;
	dx = p->disAndPath.destlist[0][0];
	dy = p->disAndPath.destlist[0][1];

	p->direction[0] = 0;
	p->direction[1] = 0;

	if (dnum != 0)//�����Ŀ���   x+:1  x-:2  y+:4  y-:3
	{
		//�Ƚ�����·��� 
		if (abs(ry - dy) > 2)//������·����ֵ����2 
		{
			//������·����з��� ,���䣺Ҫ��������(x)��������(y)
			if (rx % 2 == 0)
			{
				if (dx - rx > 0)//Ŀ����������ұ� 
					p->direction[0] = 1;
				else if (dx - rx < 0)//Ŀ������������ 
					p->direction[0] = 2;

				//Ŀ������������·�������ݺ����Ŀ�������������ջ������ҹ� 
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
				//��Ŀ������·������¹� 
				if (dy - ry > 0)
					p->direction[1] = 4;
				else
					p->direction[1] = 3;
			}
			//���·�����·��ֱ�� 	
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
		else if (abs(ry - dy) == 2)//���²�ֵ����2 ����Ҫ���䣨��������y��
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
		} //end of ������Ϊ���� 
		//�ٽ�����ҷ��� 
		else if (abs(ry - dy) == 1)//(��ֵ=1) ������������x
		{
			deltaX = abs(dx - rx);
			deltaY = abs(dy - ry);
			if (!((deltaX == 1 && deltaY == 0) || (deltaY == 1 && deltaX == 0)))//��û�� 
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
		else//��Ϊ0�������з���   ������������y
		{
			deltaX = abs(dx - rx);
			deltaY = abs(dy - ry);
			if (!((deltaX == 1 && deltaY == 0) || (deltaY == 1 && deltaX == 0)))//��û��
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

				if (dx - rx > 0)//Ŀ����������ұ� 
					p->direction[1] = 1;
				else if (dx - rx < 0)//Ŀ������������ 
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

//�ж��Ƿ񵽴�Ŀ��㲢����
void JudgeArrive(RIDERSPTR p)
{
	ORDERSPTR order = p->disAndPath.order[0];
	int dnum = p->disAndPath.destNum;//�������ϵ�Ŀ����� 
	int rx, ry, dx, dy;//���ֺ�Ŀ������� 
	int deltaX, deltaY;
	rx = Convert90to45_X(p->screenX, p->screenY);
	ry = Convert90to45_Y(p->screenX, p->screenY);
	dx = p->disAndPath.destlist[0][0];
	dy = p->disAndPath.destlist[0][1];

	deltaX = abs(dx - rx);
	deltaY = abs(dy - ry);

	while (order != NULL && ((deltaX == 1 && deltaY == 0) || (deltaY == 1 && deltaX == 0)) && (dx < 17 && dy < 17))//����Ŀ�ĵ�
	{
		p->finishTime = currentTime;
		if (p->stop == 0)//ͣ���ķ�����Ϊ0
		{
			p->stop = 1;
			p->identity[p->stop - 1] = order->iStatus;//ͣ�������� 
			p->houseLoc[p->stop - 1][0] = dx;//ͣ�����x���� 
			p->houseLoc[p->stop - 1][1] = dy;//ͣ�����y���� 

			if (order->iStatus == 2)
			{
				p->finishOrder[0][0] = 1;
				p->finishOrder[0][1] = order->iNumber;
			}
		}
		if (p->stop == 1)//ͣ���ķ�����Ϊ1
		{
			if (dx == p->houseLoc[0][0] && dy == p->houseLoc[0][1])
			{
				if (order->iStatus != p->identity[0])
					p->identity[0] = 3;//�ǲͿ� 
			}
			if (!(dx == p->houseLoc[0][0] && dy == p->houseLoc[0][1]))
			{
				p->stop = 2;
				p->identity[p->stop - 1] = order->iStatus;//ͣ�������� 
				p->houseLoc[p->stop - 1][0] = dx;//ͣ�����x���� 
				p->houseLoc[p->stop - 1][1] = dy;//ͣ�����y���� 
			}
			if (order->iStatus == 2)
			{
				if (order->iNumber != p->finishOrder[p->stop - 1][p->finishOrder[p->stop - 1][0]])
				{
					p->finishOrder[p->stop - 1][0]++;//�÷��ӵĶ���������
					p->finishOrder[p->stop - 1][p->finishOrder[p->stop - 1][0]] = order->iNumber;//���涩�����
				}
			}
		}
		if(p->stop == 2)//ͣ���ķ�����Ϊ2
		{
			for (int t = 0; t < p->stop - 1; t++)
			{
				if (dx == p->houseLoc[t][0] && dy == p->houseLoc[t][1])//�����ǰ�������ĳ��ͣ��������ͬ
				{
					if(order->iStatus != p->identity[t])
						p->identity[t] = 3;

					if (order->iStatus == 2)
					{
						p->finishOrder[t][0]++;//�÷��ӵĶ���������
						p->finishOrder[t][p->finishOrder[p->stop - 1][0]] = order->iNumber;//���涩�����
					}
				}
			}
		}

		order->iStatus++;
		if (order->iStatus == 3)//������� 
		{
			if (TIME - order->iAppearTime > TIMELIMIT1)//��ʱ 
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
				for (int i = 0; i < p->orderNum; i++)//�Ѻ���������ǰŲ 
				{
					p->CurrentOrder[i] = p->CurrentOrder[i + 1];
				}
			p->CurrentOrder[p->orderNum] = NULL;
		}//���¶���״̬ ��Ǯ 


		destinationNum--;
		p->disAndPath.destNum--;
		if (p->disAndPath.destNum == 0)//û�������������
		{
			p->disAndPath.order[0] = NULL;
		}

		else//�Ѻ����������ǰŲ 
		{
			for (int i = 0; i < p->disAndPath.destNum; i++)//�Ѻ����������ǰŲ 
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

//��������³��ֵĶ���
void DeliverOrder(ORDERSPTR currentOrder)
{
	RIDERSPTR pRider = headRider;
	RIDERSPTR okRiders[10];//�ɰ�ʱ��ɶ���������
	RIDERSPTR lessOkRiders[10];//�ᷣʱ��ɶ��������� 
	DAD riderDAD[10]; //�ݴ��Ӧ��ŵ�����·�ߣ�����ѡ�����·�߷��䵽�������� 
	int okRiderNum = 0, lessOkNum = 0;

	//����һ�����֣������·��������
	while (pRider != NULL)
	{
		if (pRider->orderNum < 50)
			riderDAD[pRider->iNumber] = PathPlanning(pRider, currentOrder);//������һ���� 
		else
		{
			riderDAD[pRider->iNumber] = GAPathPlanning(pRider->orderNum, pRider, currentOrder);
		}
			
		//�Ѱ�ʱ��ɵ����ִ����� 
		if (riderDAD[pRider->iNumber].ifOvertime == 1)
		{
			okRiderNum++;
			okRiders[okRiderNum] = pRider;
		}
		//�ѷ�ʱ��ɵĴ����� 
		if (riderDAD[pRider->iNumber].ifOvertime == 2)
		{
			lessOkNum++;
			lessOkRiders[lessOkNum] = pRider;
		}
		pRider = pRider->nextPtr;
	}

	if (okRiderNum != 0)//����п��԰�ʱ��ɵ�����
	{
		RIDERSPTR chosenRider = ChooseRider(okRiders, okRiderNum, riderDAD);
		chosenRider->disAndPath = riderDAD[chosenRider->iNumber];
		chosenRider->CurrentOrder[chosenRider->orderNum] = currentOrder;
		chosenRider->orderNum++;
	}
	else//�����ᷣʱ�Ļ����µĻ��Ʋ�	
	{
		if (MONEY >= 300)//Ǯ�������µ� 
		{
			headRider = CreateRiderList(headRider);
			//����������ַ������� 
			RIDERSPTR newRider = headRider->prevPtr;
			newRider->disAndPath = PathPlanning(newRider, currentOrder);
			newRider->CurrentOrder[newRider->orderNum] = currentOrder;
			newRider->orderNum++;
		}
		else if (lessOkNum != 0) //�˶������
		{
			RIDERSPTR chosenRider = ChooseRider(lessOkRiders, lessOkNum, riderDAD);
			chosenRider->disAndPath = riderDAD[chosenRider->iNumber];
			chosenRider->CurrentOrder[chosenRider->orderNum] = currentOrder;
			chosenRider->orderNum++;
		}
		else//�����о�gg
		{
			if (isdead == 0)//����ǵ�һ�γ��ֻ������Ʋ��ĵ��� 
			{
				isdead = 1;
				deadTime = TIME + TIMELIMIT2;//��������ʱ 
			}
		}
	}
}

//ѡ�����ֽ����ɵ� 
RIDERSPTR ChooseRider(RIDERSPTR riders[], int riderNum, DAD riderDAD[])//���ܹ�
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

//��ͨ˳·�����滮·�� 
DAD PathPlanning(RIDERSPTR rider, ORDERSPTR order)
{
	DAD path = rider->disAndPath;
	int num = path.destNum;//Ŀ����� 
	int ox1, oy1, ox2, oy2, rx, ry;
	int r[2];//�������� 
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
	//��ʱ��������·����ֻ����˳·��� 
	else
	{
		//�ȴ������� 
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

			//�����˳·�����ڣ����øõ��Ϊ��i��Ŀ��㣬����Ŀ������ 
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

		if (path.destNum == num)//��û�취˳·�������Ŀ���ĩβ 
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
		else//��˳·�ɹ�������������Ϊ���Ѱ��˳·�ĵ� 
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

//�Ŵ��㷨����·�� 
DAD GAPathPlanning(int OrderNum, RIDERSPTR rider, ORDERSPTR order)
{
	int a = 0, b = 0;//a�ඩ��������Ŀ�ĵأ�b�ඩ��ֻ��һ��Ŀ�ĵ� 
	int lenChrom;// Ⱦɫ�峤��(���ＴΪ���и���)
	int chrom[SIZEPOP][MAXLENCHROM]; // ��Ⱥ

	int destination[MAXLENCHROM][2];
	int minDistance; // ���·������		

	int bestFitIndex = 0; //���·�����ִ���
	int distanceChrom[SIZEPOP];//ÿ����Ⱥ�ľ����ܳ��� 
	int dis;
	int* bestPath; // �������·�������
	int bestIndex; // ���·�����

	int* newPath;
	int newMinDis;
	int newIndex;

	//GAorder��ʼ��
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
	//����a�ࡢb�ඩ������ 
	for (int i = 0; i < OrderNum + 1; i++)
	{
		if (rider->disAndPath.GAorder[i] != NULL)
		{
			if (rider->disAndPath.GAorder[i]->iStatus == 1) a++;
			if (rider->disAndPath.GAorder[i]->iStatus == 2) b++;
		}
	}
	lenChrom = 2 * a + b;

	//Ⱦɫ�����
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

	srand((unsigned)time(NULL)); // ��ʼ�����������

	InitChrom(chrom, lenChrom); // ��ʼ����Ⱥ


	//����ÿ����Ⱥ�ľ����ܳ��� 
	for (int j = 0; j < SIZEPOP; j++)
	{
		dis = PathLength(chrom[j], lenChrom, destination, rider);
		distanceChrom[j] = dis;
	}

	int bestResult[MAXLENCHROM]; // ���·��
	for (int i = 0; i < MAXLENCHROM; i++)//��ʼ������  bestResult[MAXLENCHROM]
		bestResult[i] = 0;

	bestPath = Min(distanceChrom); // �������·�������
	minDistance = 10000; // ���·��
	bestIndex = (*bestPath); // ���·�����
	for (int j = 0; j < lenChrom; j++)
		bestResult[j] = chrom[bestIndex][j]; // ���·������

	// ��ʼ����
	int success = 0;
	for (int i = 0; i < MAXGEN; i++)
	{
		Choice(chrom, lenChrom, destination, rider); // ѡ��  
		Cross(chrom, lenChrom); //����
		Mutation(chrom, lenChrom); //����
		Reverse(chrom, lenChrom, destination, rider); // ��ת����

		for (int j = 0; j < SIZEPOP; j++)
			distanceChrom[j] = PathLength(chrom[j], lenChrom, destination, rider); // ��������
		newPath = Min(distanceChrom);//�µ����·����� 
		newMinDis = *(newPath + 1); //�µ����·������ 
		//�ж����·���Ƿ�������ȡ�����ͻ� 

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
				bestResult[j] = chrom[*newPath][j]; // �������·������
			minDistance = PathLength(chrom[*newPath], lenChrom, destination, rider);; // �������·��
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
				minDistance = newMinDis; // �������·��
				newIndex = (*newPath);
				for (int j = 0; j < lenChrom; j++)
					bestResult[j] = chrom[newIndex][j]; // �������·������
				bestFitIndex = i + 1; // ���·������

			}
		}
	}

	//�ж��Ƿ�ʱ�����ؾ���͸������¸�Ŀ�ĵ�����
	ORDERSPTR bestResultOrder[MAXLENCHROM];//����õ����ڵĶ���
	for (int i = 0; i < MAXLENCHROM; i++)
		bestResultOrder[i] = NULL;//��ʼ��

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
		//�����a�ඩ�� 
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

		//�����b�ඩ�� 
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
					if (sum + TIME - rider->disAndPath.GAorder[i]->iAppearTime >= TIMELIMIT2) isOK = 0;   //��ʱ 
					else if (sum + TIME - rider->disAndPath.GAorder[i]->iAppearTime > TIMELIMIT1) isOK = 2;//��ʱ�� 
					break;
				}
			}//end of j
		}//end of if b
	}//end of i
	//ȷ����һ��Ŀ�ĵ�����

	//ȷ������ֵ
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

// ��Ⱥ��ʼ��
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
				chrom[num][j] = temp; // ������num������ĵ�i��Ԫ�غ͵�j��Ԫ��
				num++;
				if (num >= SIZEPOP)
					break;
			}
			if (num >= SIZEPOP)
				break;
		}
		// ������������ѭ�������޷������㹻�ĳ�ʼ���壬������ٲ���һ����
		// ���巽ʽ����ѡ����������λ�ã�Ȼ�󽻻�
		while (num < SIZEPOP)
		{
			double r1 = ((double)rand()) / (RAND_MAX + 1.0);
			double r2 = ((double)rand()) / (RAND_MAX + 1.0);
			int p1 = (int)(lenChrom * r1); // λ��1
			int p2 = (int)(lenChrom * r2); // λ��2
			int temp = chrom[num][p1];
			chrom[num][p1] = chrom[num][p2];
			chrom[num][p2] = temp;    // ��������λ��
			num++;
		}
	}
}

// ���뺯��
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

// Min()����
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

// ����·������
int PathLength(int* arr, int lenChrom, int destination[MAXLENCHROM][2], RIDERSPTR rider)
{
	int path = 0; // ��ʼ��·������
	int bestIndex = *arr; // ��λ����һ������(�������)
	for (int i = 0; i < lenChrom - 1; i++)
	{
		int index1 = *(arr + i);
		int index2 = *(arr + i + 1);
		int dis = Distance(destination[index1 - 1], destination[index2 - 1]);
		path += dis;
	}
	//��ʼ����������
	int riderx = rider->iRiderX;
	int ridery = rider->iRiderY;
	int riderPos[2];
	riderPos[0] = riderx;
	riderPos[1] = ridery;
	int first_index = *arr; // ��һ���������
	int first_dis = Distance(riderPos, destination[first_index - 1]);
	path = path + first_dis;
	return path; // �����ܵ�·������
}

// ѡ�����
void Choice(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom, int destination[MAXLENCHROM][2], RIDERSPTR rider)
{
	double pick;
	int choice_arr[SIZEPOP][MAXLENCHROM];
	double fit_pro[SIZEPOP];
	double sum = 0;
	double fit[SIZEPOP]; // ��Ӧ�Ⱥ�������(����ĵ���)
	for (int j = 0; j < SIZEPOP; j++)
	{
		double path = PathLength(chrom[j], lenChrom, destination, rider);
		double fitness = 1.0 / path;
		fit[j] = fitness;
		sum += fitness;
	}
	for (int j = 0; j < SIZEPOP; j++)
	{
		fit_pro[j] = fit[j] / sum; // ��������
	}
	// ��ʼ���̶�
	for (int i = 0; i < SIZEPOP; i++)
	{
		pick = ((double)rand()) / RAND_MAX; // 0��1֮��������  
		for (int j = 0; j < SIZEPOP; j++)
		{
			pick = pick - fit_pro[j];
			if (pick <= 0)
			{
				for (int k = 0; k < lenChrom; k++)
					choice_arr[i][k] = chrom[j][k]; // ѡ��һ������
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

//�������
void Cross(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom)
{
	double pick;
	double pick1, pick2;
	int choice1, choice2;
	int pos1, pos2;
	int temp;
	int conflict1[MAXLENCHROM]; // ��ͻλ��
	int conflict2[MAXLENCHROM];
	int num1, num2;
	int index1, index2;
	int move = 0; // ��ǰ�ƶ���λ��
	while (move < lenChrom - 1)
	{
		pick = ((double)rand()) / RAND_MAX; // ���ھ����Ƿ���н������
		if (pick > PCROSS)
		{
			move += 2;
			continue; // ���β����н���
		}
		// ���ò���ӳ���ӽ�
		choice1 = move; // ����ѡȡ�ӽ�����������
		choice2 = move + 1; // ע������±�Խ��
		pick1 = ((double)rand()) / (RAND_MAX + 1.0);
		pick2 = ((double)rand()) / (RAND_MAX + 1.0);
		pos1 = (int)(pick1 * lenChrom); // ����ȷ�������ӽ����λ��
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
			pos2 = temp; // ����pos1��pos2��λ��
		}
		for (int j = pos1; j <= pos2; j++)
		{
			temp = chrom[choice1][j];
			chrom[choice1][j] = chrom[choice2][j];
			chrom[choice2][j] = temp; // �������˳��
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
				temp = chrom[choice1][index1]; // ������ͻ��λ��
				chrom[choice1][index1] = chrom[choice2][index2];
				chrom[choice2][index2] = temp;
			}
		}
		move += 2;
	}
}

// �������   ������Բ�ȡ���ѡȡ�����㣬����Ի�λ��
void Mutation(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom) // �������
{
	double pick, pick1, pick2;
	int pos1, pos2, temp;
	for (int i = 0; i < SIZEPOP; i++)
	{
		pick = ((double)rand()) / RAND_MAX; // �����ж��Ƿ���б������
		if (pick > PMUTATION)
			continue;
		pick1 = ((double)rand()) / (RAND_MAX + 1.0);
		pick2 = ((double)rand()) / (RAND_MAX + 1.0);
		pos1 = (int)(pick1 * lenChrom); // ѡȡ���б����λ��
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

// ������ת����
void Reverse(int chrom[SIZEPOP][MAXLENCHROM], int lenChrom, int destination[MAXLENCHROM][2], RIDERSPTR rider)
{
	double pick1, pick2;
	double dis, reverse_dis;
	int n;
	int flag, pos1, pos2, temp;
	int reverse_arr[MAXLENCHROM];

	for (int i = 0; i < SIZEPOP; i++)
	{
		flag = 0; // ���ڿ��Ʊ�����ת�Ƿ���Ч
		while (flag == 0)
		{
			pick1 = ((double)rand()) / (RAND_MAX + 1.0);
			pick2 = ((double)rand()) / (RAND_MAX + 1.0);
			pos1 = (int)(pick1 * lenChrom); // ѡȡ������ת������λ��
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
				pos2 = temp; // ����ʹ��pos1 <= pos2
			}
			if (pos1 < pos2)
			{
				for (int j = 0; j < lenChrom; j++)
					reverse_arr[j] = chrom[i][j]; // ��������
				n = 0; // ��ת��Ŀ
				for (int j = pos1; j <= pos2; j++)
				{
					reverse_arr[j] = chrom[i][pos2 - n]; // ��ת����
					n++;
				}
				reverse_dis = PathLength(reverse_arr, lenChrom, destination, rider); // ��ת֮��ľ���
				dis = PathLength(chrom[i], lenChrom, destination, rider); // ԭʼ����
				if (reverse_dis < dis)
				{
					for (int j = 0; j < lenChrom; j++)
						chrom[i][j] = reverse_arr[j]; // ���¸���
				}
			}
			flag = 1;
		}
	}
}
