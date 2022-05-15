#ifndef LOADINGMAP_H
#define LOADINGMAP_H

#include <map>
#include "HashMap.h"
using namespace std;

//extern multimap< pair<int,int>, Pos > loadingmap;

extern map< pair<int,int>, Map > loadingmap;

extern float pos_x, pos_y, pos_z, pos_rx, pos_ry;

extern bool is_loadingmap;

#endif // LOADINGMAP_H
