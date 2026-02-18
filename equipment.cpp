#include "equipment.h"

// Constructors definitions
equipment::equipment()
{
    name ="";
    use = "";
}

equipment::equipment(string n, string u)
{
    name = n;
    use = u;
}

// Getter definitions
string equipment::getEquipmentName()
{
    return name;
}

string equipment::getEquipmentuse()
{
    return use;
}

// Setter definitions
void equipment::setEquipmentName(string n)
{
    name = n;
}

void equipment::setEquipmentUse(string u)
{
    use = u;
}

// Display definitions
void equipment::displayEquipmentDetails()
{
    cout << "Equipment name: "<< getEquipmentName() << endl;
    cout << "Use: " << getEquipmentuse() << endl;
}
