#ifndef DOUBLE_LINK_HXX
#define DOUBLE_LINK_HXX
#include<stdio.h>  
#include<stdlib.h>  
#include<malloc.h>  

#include "typedefs.h"
#include "dfs_node.h"
#include "sparse_parent_bitwise.h"

typedef struct DOUBLE_LIST  
{  
    varset data;  
    struct DOUBLE_LIST *prev;  
    struct DOUBLE_LIST *next;  
}double_list;

double_list *createlist(varset allvariables)       //创建有n个元素的双向链表 并输入元素
{
    double_list *head,*q;
    head = (double_list *)malloc(sizeof(double_list));
    q = (double_list *)malloc(sizeof(double_list));
    q->data = allvariables;
    head->prev = q;
    head->next = q;
    q->prev = head;
    q->next = head;
    return head;
}
//遍历并且输出这些元素
void printlist(double_list *head)
{
    double_list *p;
    p = head;
    p = p->next;
    while(p!=head)
    {
        varset variables=p->data;
        printf("%s \n", varsetToString(variables).c_str());
        p = p->next;
    }
    printf("\n");
}
//得到现在双向链表中的元素的个数
int lengthlist(double_list *head)
{
    double_list *p;
    p = head;
    p = p->next;
    int coun = 0;
    while(p!=head)
    {
        coun++;
        p = p->next;
    }
    return coun;
}
//判断现在双向链表中的元素的个数是否大于变量个数
bool enoughlist(double_list *head, int variableCount)
{
    double_list *p;
    p = head;
    p = p->next;
    int coun = 0, sign = 0;
    while(p!=head)
    {
        coun++;
        p = p->next;
        if(coun>=variableCount*2) {sign = 1; break;}
    }
    if(sign == 1) return true;
        else return false;
}
//在指定元素之后插入数据data
void insertlist_b(double_list* d,varset data)
{
    double_list *q;
    q = (double_list *)malloc(sizeof(double_list));
    q->data = data;
    q->next = d->next;
    d->next->prev = q;
    q->prev = d;
    d->next = q;
}
//在指定元素之前插入数据data
void insertlist_u(double_list* d,varset data)
{
    double_list *q;
    q = (double_list *)malloc(sizeof(double_list));
    q->data = data;
    q->prev = d->prev;
    d->prev->next = q;
    q->next = d;
    d->prev = q;
}
//在第i个元素之前插入数据data
void insertlist_f(double_list *head,int i, varset data)
{
    double_list *p = head, *q;
    p = p->next;
    i--;
    while(i--)
        p = p->next;
    q = (double_list *)malloc(sizeof(double_list));
    q->data = data;
    (p->prev)->next = q;
    q->prev = p->prev;
    q->next = p;
    p->prev = q;
}
//删除当前位置一个元素
void deletelist_f(double_list* d)
{
    double_list* p = d;
    (p->prev)->next = p->next;
    (p->next)->prev = p->prev;
    free(p);
}
//删除第i个位置的元素
void deletelist_i(double_list *head,int i)
{
    double_list *p = head;
    p = p->next;
    i--;
    while(i--)
        p = p->next;
    (p->prev)->next = p->next;
    (p->next)->prev = p->prev;
    free(p);
}
//删除值为x的元素
void deletelist_x(double_list *head,varset variables)
{
    double_list *p = head, *q;
    p = p->next;
    while(p!=head)
        if(p->data == variables)
        {
            q = p->next;
            (p->prev)->next = p->next;
            (p->next)->prev = p->prev;
            free(p);
            p = q;
        }
        else
            p = p->next;
}
/*
//对双向链表进行排序
void sortlist(double_list *head)  //升序
{
    double_list *p = head, *t;
    p = p->next;
    for(;p!=head;p=p->next)
        for(t = p->next;t!=head;t=t->next)
        {
            if((p->data)->getF() > (t->data)->getF())
            {
                DFSNode *a = p->data;
                p->data = t->data;
                t->data = a;
            }
        }
}
*/
#endif
