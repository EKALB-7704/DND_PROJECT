#include "equipment.h"

equipment::equipment(string n, string u)
{
    name = n;
    use = u;
}

string equipment::getEquipmentName()
{
    return name;
}

string equipment::getEquipmentuse()
{
    return use;
}

void equipment::setEquipmentName(string n)
{
    name = n;
}

void equipment::setEquipmentUse(string u)
{
    use = u;
}

void equipment::displayEquipmentDetails()
{
    cout << "Equipment name: "<< getEquipmentName() << endl;
    cout << "Use: " << getEquipmentuse() << endl;
}
