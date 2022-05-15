#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct {
    int x;
    int y;
    int z;
    int w;
} Entry;

typedef struct {
    unsigned int mask;
    unsigned int size;
    Entry* data;
} Map;

class HashMap
{
public:
    void map_alloc(Map* map);
    void map_free(Map* map);
    void map_set(Map* map, int x, int y, int z, int w);
    int map_get(Map* map, int x, int y, int z);
    void map_grow(Map* map);
    int hash_int(int key);
    int hash(int x, int y, int z);
};

#endif // HASHMAP_H
