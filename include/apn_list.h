#include <map>

#ifndef APN_LIST_H_
#define APN_LIST_H_

#define DEFAULT_APN_NAME "internet"

#define MCC_CZ 230 // Czech Republic

std::map<int, std::map<int, const char *>> apn_list = {
    {MCC_CZ, { // Czech Republic
        {1, "internet.t-mobile.cz"}, // T-Mobile
        {2, "internet"}, // O2
        {3, "internet"} // Vodafone
    }}
};

#endif // APN_LIST_H_
