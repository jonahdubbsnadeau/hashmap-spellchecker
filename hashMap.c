/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Jonah Dubbs-Nadeau
 * Date: December 4, 2019
 */

#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

int hashFunction1(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += key[i];
    }
    return r;
}

int hashFunction2(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += (i + 1) * key[i];
    }
    return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink* hashLinkNew(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link Link to delete.
 */
static void hashLinkDelete(HashLink* link)
{
    free(link->key);
    free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map HashMap to initialize.
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap* map, int capacity)
{
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink*) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        map->table[i] = NULL;
    }
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map HashMap to clean up.
 */
void hashMapCleanUp(HashMap* map)
{
    assert(map != NULL);
        
    for (int i = 0; i < map->capacity; i++) {
        HashLink* current = map->table[i];
        while (current != NULL) {
            HashLink* next = current->next;
            hashLinkDelete(current);
            current = next;
        }
    }
    
    free(map->table);
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap* hashMapNew(int capacity)
{
    HashMap* map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map HashMap to delete.
 */
void hashMapDelete(HashMap* map)
{
    hashMapCleanUp(map);
    free(map);
}

/**
 * Returns a pointer to the value of the link with the given key  and skip traversing as well. Returns NULL
 * if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map HashMap to search.
 * @param key The key to search for.
 * @return Link value or NULL if no matching link.
 */
int* hashMapGet(HashMap* map, const char* key)
{
    assert(map != NULL);
    assert(key != NULL);
    
    int index = HASH_FUNCTION(key) % map->capacity;
    HashLink* current = map->table[index];
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return &(current->value);
        }
        current = current->next;
    }
    
    return NULL;
}

/**
 * Resizes the hash table to have a number of buckets equal to the given 
 * capacity (double of the old capacity). After allocating the new table, 
 * all of the links need to rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map HashMap to resize.
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap* map, int capacity)
{
    assert(map != NULL);
    
    HashMap* newMap = hashMapNew(capacity);
    assert(newMap != NULL);
        
    for (int i = 0; i < map->capacity; i++) {
        HashLink* current = map->table[i];
        
        while (current != NULL) {
            hashMapPut(newMap, current->key, current->value);
            current = current->next;
        }
    }
    
    hashMapCleanUp(map);
    map->table = newMap->table;
    map->size = newMap->size;
    map->capacity = newMap->capacity;
    
    free(newMap);
}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 * 
 * @param map HashMap to search.
 * @param key The key to add or update.
 * @param value The new value to be associated with the given key.
 */
void hashMapPut(HashMap* map, const char* key, int value)
{
    assert(map != NULL);
    assert(key != NULL);
    
    int index = HASH_FUNCTION(key) % map->capacity;
    HashLink* current = map->table[index];
    HashLink* tail = map->table[index];;
    
    int keyFound = 0;
    
    while (current != NULL && keyFound == 0) {
        if (strcmp(current->key, key) == 0) {
            // A link with the given key already exists
            current->value = value;
            keyFound = 1;
        }
        
        if (current->next == NULL) {
            // The current link is the last in the bucket
            tail = current;
        }
        
        current = current->next;
    }
    
    if (keyFound == 0) {
        // A link with the given key does not already exist and needs to be created
        if (tail == NULL) {
            // The bucket is empty
            map->table[index] = hashLinkNew(key, value, NULL);
        } else {
            // The new link will be added to the end of the bucket
            tail->next = hashLinkNew(key, value, NULL);
        }
        map->size += 1;
    }
    
    if (hashMapTableLoad(map) >= MAX_TABLE_LOAD) {
        resizeTable(map, map->capacity * 2);
    }
}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map HashMap to search.
 * @param key The key to search for.
 */
void hashMapRemove(HashMap* map, const char* key)
{
    assert(map != NULL);
    assert(key != NULL);
    
    int index = HASH_FUNCTION(key) % map->capacity;
    HashLink* current = map->table[index];
    
    int removed = 0;
    
    while (current != NULL && removed == 0) {
        HashLink* next = current->next;
        
        if (current == map->table[index] && strcmp(current->key, key) == 0) {
            // Removing the head of the list
            map->table[index] = next;
            
            hashLinkDelete(current);
            current = NULL;

            map->size -= 1;
            removed = 1;
        } else if (next != NULL && strcmp(next->key, key) == 0) {
            // Removing any other link
            current->next = next->next;
            
            hashLinkDelete(next);
            next = NULL;
            
            map->size -= 1;
            removed = 1;
        } else {
            current = current->next;
        }
    }
}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map HashMap to search.
 * @param key The key to search for.
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap* map, const char* key)
{
    assert(map != NULL);
    assert(key != NULL);
    
    int index = HASH_FUNCTION(key) % map->capacity;
    HashLink* current = map->table[index];
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return 1;
        }
        current = current->next;
    }
    
    return 0;
}

/**
 * Returns the number of links in the table.
 * @param map HashMap in question.
 * @return Number of links in the table.
 */
int hashMapSize(HashMap* map)
{
    assert(map != NULL);
    return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map HashMap in question.
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap* map)
{
    assert(map != NULL);
    return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map HashMap in question.
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap* map)
{
    assert(map != NULL);
    
    int count = 0;
    
    for (int i = 0; i < map->capacity; i++) {
        HashLink* current = map->table[i];
        
        if (current == NULL) {
            count++;
        }
    }
    
    return count;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map HashMap in question.
 * @return Table load.
 */
float hashMapTableLoad(HashMap* map)
{
    assert(map != NULL);
    return (float) map->size / (float) map->capacity;
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map HashMap to print.
 */
void hashMapPrint(HashMap* map)
{
    assert(map != NULL);
    
    for (int i = 0; i < map->capacity; i++) {
        HashLink* current = map->table[i];
        printf("\nBucket %d:", i + 1);
        
        while (current != NULL) {
            printf(" [%s: %d]", current->key, current->value);
            current = current->next;
        }
    }
}
