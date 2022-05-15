#include "HashMap.h"

#include <stdlib.h>

int HashMap::hash_int(int key) {
    key = ~key + (key << 15);
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057;
    key = key ^ (key >> 16);
    return key;
}

int HashMap::hash(int x, int y, int z) {
    x = hash_int(x);
    y = hash_int(y);
    z = hash_int(z);
    return x ^ y ^ z;
}

void HashMap::map_alloc(Map* map) {
    map->mask = 0xfff;
    map->size = 0;
    map->data = (Entry*)calloc(map->mask + 1, sizeof(Entry));
}

void HashMap::map_free(Map* map) {
    free(map->data);
}

void HashMap::map_set(Map* map, int x, int y, int z, int w) {
    unsigned int index = hash(x, y, z) & map->mask;
    Entry* entry = map->data + index;
    int overwrite = 0;
    while (!(!(entry)->x && !(entry)->y && !(entry)->z && !(entry)->w)) {
        if (entry->x == x && entry->y == y && entry->z == z) {
            overwrite = 1;
            break;
        }
        index = (index + 1) & map->mask;
        entry = map->data + index;
    }
    if (overwrite) {
        entry->w = w;
    }
    else{
        entry->x = x;
        entry->y = y;
        entry->z = z;
        entry->w = w;
        map->size++;
        if (map->size * 2 > map->mask) {
            map_grow(map);
        }
    }
}

int HashMap::map_get(Map* map, int x, int y, int z) {
    unsigned int index = hash(x, y, z) & map->mask;
    Entry* entry = map->data + index;
    while (!(!(entry)->x && !(entry)->y && !(entry)->z && !(entry)->w)) {
        if (entry->x == x && entry->y == y && entry->z == z) {
            return entry->w;
        }
        index = (index + 1) & map->mask;
        entry = map->data + index;
    }
    return 0;
}

void HashMap::map_grow(Map* map) {
    Map new_map;
    new_map.mask = (map->mask << 1) | 1;
    new_map.size = 0;
    new_map.data = (Entry*)calloc(new_map.mask + 1, sizeof(Entry));
    for (unsigned int index = 0; index <= map->mask; index++) {
        Entry* entry = map->data + index;
        if (!(!(entry)->x && !(entry)->y && !(entry)->z && !(entry)->w)) {
            map_set(&new_map, entry->x, entry->y, entry->z, entry->w);
        }
    }
    free(map->data);
    map->mask = new_map.mask;
    map->size = new_map.size;
    map->data = new_map.data;
}
