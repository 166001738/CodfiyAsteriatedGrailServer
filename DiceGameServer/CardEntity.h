#pragma once
#include <iostream>

#define EXCLUSIVE_NUM 4
enum Exclusive{
	EX_WU_XI_SHU_FU,
	EX_TIAO_XIN,
	EX_LING_HUN_LIAN_JIE,
	EX_TONG_SHENG_GONG_SI
};

struct BasicEffect
{
    int card;
    int srcUser;
};

enum CardType{
	TYPE_ATTACK = 1,
	TYPE_MAGIC = 2,
};

enum CardProperty{
	PROPERTY_BLOOD = 1,
	PROPERTY_MARTIAL = 2,
	PROPERTY_PHANTOM = 3,
	PROPERTY_CAST = 4,
	PROPERTY_HOLY = 5,
};

enum CardElement{
	ELEMENT_WIND = 1,
	ELEMENT_WATER = 2,
	ELEMENT_FIRE = 3,
	ELEMENT_EARTH = 4,
	ELEMENT_THUNDER = 5,
	ELEMENT_LIGHT = 6,
	ELEMENT_DARKNESS = 7,
};

enum CardName{
	NAME_POISON = 1,
	NAME_WEAKEN = 2,
	NAME_SHIELD = 3,
	NAME_MISSILE = 4,
	NAME_HOLYLIGHT = 5,
	NAME_WINDSLASH = 6,
	NAME_WATERSLASH = 7,
	NAME_FIRESLASH = 8,
	NAME_EARTHSLASH = 9,
	NAME_THUNDERSLASH = 10,
	NAME_DARKSLASH = 11,
};

//������
class CardEntity
{
public:
    //����,��������ǰ���������Ϣ��һ��String
    CardEntity(std::string cardEntry);
	int getID() { return id; }   
    //��ȡ����/�������
	int getType() { return type; }
    //��ȡ��������
	int getElement() { return element; }
    //��ȡ���Ƽ���Ѫ��ӽ��ʥ�������
	int getProperty() { return property; }
    //��ȡ��������
	int getName() { return name; }
	//��ȡ���ƶ��м���Ϣ
	bool checkSpeciality(int speciality);
private:
    int id;
    int specialityCount;
    int type;
    int element;
    int property;
    int name;
    int specialityList[2];
};

