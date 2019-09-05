
#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"






/*======================================================================*/

/* global variable of game Tictactoe */
int tmpQP[3][3]; //表示棋盘数据的临时数组，其中的元素0表示该格为空，
//1表示计算机放下的子，-1表示人放下的子。

#define MAX_NUM 1000//扩展生成状态节点的最大数目
const int NO_BLANK=-1001; //表示没有空格
const int TREE_DEPTH=3; //搜索树的最大深度，如果增加此值可以提高计算机的“智力”，
//但同时也需要增加MAX_NUM的值。
const int NIL=1001;    //表示空
static int s_count;     //用来表示当前分析的节点的下标

struct State//该结构表示棋盘的某个状态，也可看做搜索树中的一个节点
{
    int QP[3][3]; //棋盘格局
    int e_fun; //当前状态的评估函数值
    int child[9]; //儿女节点的下标
    int parent; //双亲节点的下标
    int bestChild; //最优节点（评估函数值最大）的儿女节点下标
}States[MAX_NUM]; //用来保存搜索树中状态节点的数组




void Init()   //初始化函数，当前的棋盘格局总是保存在States[0]中
{
    int i,j;
    s_count=0;
    for(i=0;i<3;i++)
        for(j=0;j<3;j++)
            States[0].QP[i][j]=0; //将棋盘清空
    States[0].parent=NIL;   //初始节点没有双亲节点
}

void PrintQP() //打印当棋盘格局的函数
{
    int i,j;
    for(i=0;i<3;i++)
    {
        for(j=0;j<3;j++)
            if (States[0].QP[i][j] == -1)
            {
                printf("%c       ",1);
            }
            else if (States[0].QP[i][j] == 1)
            {
                printf("%c       ",2);
            }
            else
            {
                printf("%d       ",0);
            }

            printf("\n");
    }
}

int IsWin(struct State s) //有人赢了吗？返回0表示没有人赢，返回-1表示人赢了，返回1表示计算机赢了
{
    int i,j;
    for(i=0;i<3;i++)
    {
        if(s.QP[i][0]==1&&s.QP[i][1]==1&&s.QP[i][2]==1)return 1;
        if(s.QP[i][0]==-1&&s.QP[i][1]==-1&&s.QP[i][2]==-1)return -1;
    }
    for(i=0;i<3;i++)
    {
        if(s.QP[0][i]==1&&s.QP[1][i]==1&&s.QP[2][i]==1)return 1;
        if(s.QP[0][i]==-1&&s.QP[1][i]==-1&&s.QP[2][i]==-1)return -1;
    }
    if((s.QP[0][0]==1&&s.QP[1][1]==1&&s.QP[2][2]==1)||(s.QP[2][0]==1&&s.QP[1][1]==1&&s.QP[0][2]==1))return 1;
    if((s.QP[0][0]==-1&&s.QP[1][1]==-1&&s.QP[2][2]==-1)||(s.QP[2][0]==-1&&s.QP[1][1]==-1&&s.QP[0][2]==-1))return -1;
    return 0;
}

int e_fun(struct State s)//评估函数
{
    int flag=1;
    int i,j;
    for(i=0;i<3;i++)
        for(j=0;j<3;j++)
            if(s.QP[i][j]==0)flag= FALSE;
    if(flag)return NO_BLANK;

    if(IsWin(s)==-1)return -MAX_NUM;//如果计算机输了，返回最小值
    if(IsWin(s)==1)return MAX_NUM;//如果计算机赢了，返回最大值
    int count=0;//该变量用来表示评估函数的值

    //将棋盘中的空格填满自己的棋子，既将棋盘数组中的0变为1
    for(i=0;i<3;i++)
        for(j=0;j<3;j++)
            if(s.QP[i][j]==0)tmpQP[i][j]=1;
            else tmpQP[i][j]=s.QP[i][j];

            //电脑一方
            //计算每一行中有多少行的棋子连成3个的
            for(i=0;i<3;i++)
                count+=(tmpQP[i][0]+tmpQP[i][1]+tmpQP[i][2])/3;
            //计算每一列中有多少列的棋子连成3个的
            for(i=0;i<3;i++)
                count+=(tmpQP[0][i]+tmpQP[1][i]+tmpQP[2][i])/3;
            //斜行有没有连成3个的？
            count+=(tmpQP[0][0]+tmpQP[1][1]+tmpQP[2][2])/3;
            count+=(tmpQP[2][0]+tmpQP[1][1]+tmpQP[0][2])/3;

            //将棋盘中的空格填满对方的棋子，既将棋盘数组中的0变为-1
            for(i=0;i<3;i++)
                for(j=0;j<3;j++)
                    if(s.QP[i][j]==0)tmpQP[i][j]=-1;
                    else tmpQP[i][j]=s.QP[i][j];

                    //对方
                    //计算每一行中有多少行的棋子连成3个的
                    for(i=0;i<3;i++)
                        count+=(tmpQP[i][0]+tmpQP[i][1]+tmpQP[i][2])/3;
                    //计算每一列中有多少列的棋子连成3个的
                    for(i=0;i<3;i++)
                        count+=(tmpQP[0][i]+tmpQP[1][i]+tmpQP[2][i])/3;
                    //斜行有没有连成3个的？
                    count+=(tmpQP[0][0]+tmpQP[1][1]+tmpQP[2][2])/3;
                    count+=(tmpQP[2][0]+tmpQP[1][1]+tmpQP[0][2])/3;

                    return count;
}

//计算机通过该函数决定走哪一步，并对当前的棋局做出判断。
int AutoDone()
{

    int
        MAX_F=NO_BLANK, //保存对自己最有利的棋局（最大）的评估函数值
        parent=-1, //以当前棋局为根生成搜索树，所以当前棋局节点无双亲节点
        count,   //用来计算当前生成的最后一个扩展节点的下标

        tag;   //标示每一层搜索树中最后一个节点的下标
    int
        max_min=TREE_DEPTH%2, //标识取下一层评估函数的最大值还是最小值？
        //max_min=1取下一层中的最大值，max_min=0取最小值
        IsOK=FALSE;    //有没有找到下一步落子的位置？
    s_count=0;   //扩展生成的节点数初始值为0

    if(IsWin(States[0])==-1)//如果人赢了
    {
        printf("Conguatulations! You Win! GAME OVER.\n");
        return TRUE;
    }

    int i,j,t,k,i1,j1;
    for(t=0;t<TREE_DEPTH;t++)//依次生成各层节点
    {
        count=s_count;//保存上一层节点生成的最大下标
        for(k=parent+1;k<=count;k++)//生成一层节点
        {
            int n_child=0;//该层节点的孩子节点数初始化为0
            for(i=0;i<3;i++)
                for(j=0;j<3;j++)
                    if(States[k].QP[i][j]==0)//如果在位置(i,j)可以放置一个棋子
                    {       //则
                        s_count++;    //生成一个节点，节点数（最大下标）数加1
                        for(i1=0;i1<3;i1++) //该3×3循环将当前棋局复制到新节点对应的棋局结构中
                            for(j1=0;j1<3;j1++)
                                States[s_count].QP[i1][j1]=States[k].QP[i1][j1];
                        States[s_count].QP[i][j]=t%2==0?1:-1;//根据是人下还是计算机下，在空位上落子
                        States[s_count].parent=k;   //将父母节点的下标k赋给新生成的节点
                        States[k].child[n_child++]=s_count; //下标为k的父母节点有多了个子女

                        //如果下一步有一步期能让电脑赢，则停止扩展节点，转向结局打印语句
                        if(t==0&&e_fun(States[s_count])==MAX_NUM)
                        {
                            States[k].e_fun=MAX_NUM;
                            States[k].bestChild=s_count;//最好的下一步棋所在的节点的下标为s_count
                            goto L2;
                        }
                    }
        }
        parent=count;   //将双亲节点设置为当前双亲节点的下一层节点
//      printf("%d\n",s_count); //打印生成节点的最大下标
    }

    tag=States[s_count].parent;//设置最底层标志，以便从下到上计算最大最小值以寻找最佳解路径。
    int pos_x,pos_y;//保存计算机落子的位置
    for(i=0;i<=s_count;i++)
    {
        if(i>tag) //保留叶节点的评估函数值
        {
            States[i].e_fun=e_fun(States[i]);
        }
        else //抹去非叶节点的评估函数值
            States[i].e_fun=NIL;
    }
    while(!IsOK)//寻找最佳落子的循环
    {
        for(i=s_count;i>tag;i--)
        {
            if(max_min)//取子女节点的最大值
            {
                if(States[States[i].parent].e_fun<States[i].e_fun||States[States[i].parent].e_fun==NIL)
                {
                    States[States[i].parent].e_fun=States[i].e_fun; //设置父母节点的最大最小值
                    States[States[i].parent].bestChild=i;   //设置父母节点的最佳子女的下标
                }
            }
            else//取子女节点的最小值
            {
                if(States[States[i].parent].e_fun>States[i].e_fun||States[States[i].parent].e_fun==NIL)
                {
                    States[States[i].parent].e_fun=States[i].e_fun; //设置父母节点的最大最小值
                    States[States[i].parent].bestChild=i;   //设置父母节点的最佳子女的下标
                }
            }
        }
        s_count=tag; //将遍历的节点上移一层
        max_min=!max_min; //如果该层都是MAX节点，则它的上一层都是MIN节点，反之亦然。
        if(States[s_count].parent!=NIL)//如果当前遍历的层中不包含根节点，则tag标志设为上一层的最后一个节点的下标
            tag=States[s_count].parent;
        else
            IsOK=TRUE; //否则结束搜索
    }
    int x,y;
L2: //取落子的位置，将x,y坐标保存在变量pos_x和pos_y中，并将根（当前）节点中的棋局设为最佳儿子节点的棋局

    for(x=0;x<3;x++)
    {
        for(y=0;y<3;y++)
        {
            if(States[States[0].bestChild].QP[x][y]!=States[0].QP[x][y])
            {
                pos_x=x;
                pos_y=y;
            }
            States[0].QP[x][y]=States[States[0].bestChild].QP[x][y];
        }
    }


    MAX_F=States[0].e_fun;
    //cout<<MAX_F<<endl;

    printf("The computer put a Chessman at: %d,%d\nThe QP now is:\n",pos_x+1,pos_y+1);
    PrintQP();
    if(MAX_F==MAX_NUM) //如果当前节点的评估函数为最大值，则计算机赢了
    {
        printf("The computer WIN! You LOSE! GAME OVER.\n");
        return TRUE;
    }
    if(MAX_F==NO_BLANK) //否则，如果棋盘时候没空可放了，则平局。
    {
        printf("DRAW GAME!\n");
        return TRUE;
    }
    return FALSE;
}

//用户通过此函数来输入落子的位置，
//比如，用户输入31，则表示用户在第3行第1列落子。
void UserInput(int fd_stdin,int fd_stdout)
{

    int n;
    int pos = -1,x,y;
    char szCmd[80]={0};
L1: printf("Please Input The Line Position where you put your Chessman (x): ");
    n = read(fd_stdin,szCmd,80);
    szCmd[1] = 0;
    atoi(szCmd,&x);
    printf("Please Input The Column Position where you put your Chessman (y): ");
    n = read(fd_stdin,szCmd,80);
    szCmd[1] = 0;
    atoi(szCmd,&y);
    if(x>0&&x<4&&y>0&&y<4&&States[0].QP[x-1][y-1]==0)
        States[0].QP[x-1][y-1]=-1;
    else
    {
        printf("Input Error!");
        goto L1;
    }

}





void TTT(int fd_stdin,int fd_stdout)
{
    char buf[80]={0};
    char IsFirst = 0;
    int IsFinish = FALSE;
    while(!IsFinish)
    {

        Init();
        printf("The QiPan (QP) is: \n");

        PrintQP();

        printf("Do you want do first?(y/n):");
        read(fd_stdin,buf,2);
        IsFirst = buf[0];
        do{

            if(IsFirst=='y')
            {
                UserInput(fd_stdin, fd_stdout);
                IsFinish=AutoDone();
            }else{
                IsFinish=AutoDone();
                if(!IsFinish)UserInput(fd_stdin, fd_stdout);
            }

        }while(!IsFinish);
        if (IsFinish)
        {
            printf("Play Again?(y/n)");
            char cResult;
            read(fd_stdin,buf,2);
            cResult = buf[0];
            printf("%c",cResult);
            if (cResult == 'y')
            {
                clear();
                IsFinish = FALSE;

            }
            else
            {
                clear();
            }

        }
    }

}