#pragma once
#include <iostream>
using namespace std;

#define SHIELDCARD 0
#define POISONCARD 1
#define WEAKCARD 2
#define MISSILECARD 3
#define LIGHTCARD 4

#define NOTEXIST           0
#define PILE               1
#define DISCARDPILE        2
#define DISCARDPILECOVERED 3
#define HAND               4
#define EFFECT             5
#define COVERED            6

//������
class CardEntity
{
public:
    //����,��������ǰ���������Ϣ��һ��String
    CardEntity(string cardEntry);
    int getID();
    //��ȡ���ƶ��м���Ϣ
    int getHasSpeciality();
    //��ȡ����/�������
    string getType();
    //��ȡ��������
    string getElement();
    //��ȡ���Ƽ���Ѫ��ӽ��ʥ�������
    string getProperty();
    //��ȡ�������ƣ�ע��������
    string getName();
    //��ȡ���ƶ��м��б�
    string* getSpecialityList();
    //������������תΪint
    int getMagicName(){return this->magicName;}
    void setMagicName(int name){this->magicName = name;}
    //�˲鿨���Ƿ��ǻ���������
    int checkBasicMagic(string cardName);
protected:
    int id;
    int hasSpeceiality;
    string type;
    string element;
    string property;
    string name;
    string specialityList[2];
    int magicName;
};

