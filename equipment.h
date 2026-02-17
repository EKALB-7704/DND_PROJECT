
#ifndef _EQUIPMENT_
#define _EQUIPMENT_


#include <iostream>
#include <string>
#include <vector>



using namespace std;



class equipment
{
    private:
    string name;
    string use;



    public:
    equipment(string n, string u);

    string getEquipmentName();
    string getEquipmentuse();


    void setEquipmentName(string n);
    void setEquipmentUse(string u);


    void displayEquipmentDetails();
};


#endif