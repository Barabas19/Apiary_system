#include <map>
#include <cstring>

#ifndef APN_LIST_H_
#define APN_LIST_H_

#define DEFAULT_APN_NAME "internet"

struct cmp_str
{
   bool operator()(char const *a, char const *b) const
   {
      return strcmp(a, b) < 0;
   }
};

std::map<const char *, const char *, cmp_str> apn_list = {
    {"T-Mobile CZ", "internet.t-mobile.cz"},
    {"O2 CZ", "internet"},
    {"Vodafone CZ", "internet"}
};

#endif // APN_LIST_H_
