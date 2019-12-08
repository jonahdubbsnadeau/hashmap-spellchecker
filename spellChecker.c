/* Sources Used:
   https://en.wikipedia.org/wiki/Levenshtein_distance
   https://www.geeksforgeeks.org/taking-string-input-space-c-3-different-methods/
 */

#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NUM_MATCHES 5

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file The file containing the words.
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    char* word = nextWord(file);
    
    while (word != NULL) {
        hashMapPut(map, word, -1);
        
        free(word);
        word = nextWord(file);
    }
}

/**
* Helper function that calculates the minimum of three values, specifically
* the values of deletion, insertion, and substitution that are calculated
* during the calculation of the Levenshtein distance between two words.
* Source referenced: https://en.wikipedia.org/wiki/Levenshtein_distance
* @param deletion The cost of deletion.
* @param insertion The cost of insertion.
* @param substitution The cost of substitution.
* @return The minimum of the three values.
*/

int minimum(int deletion, int insertion, int substitution) {
    int min = deletion;

    if (insertion < min) {
        min = insertion;
    }

    if (substitution < min) {
        min = substitution;
    }

    return min;
}

/**
* Calculates the Levenshtein distance between two words, using the
* Wagner–Fischer algorithm (iterative with full matrix).
* Source referenced: https://en.wikipedia.org/wiki/Levenshtein_distance
* @param word1 The first word.
* @param word2 The second word.
* @return The Levenshtein distance between the two words.
*/

int levenshtein(char * word1, char * word2) {
    int m = strlen(word1);
    int n = strlen(word2);
    int matrix[m + 1][n + 1];
    int substitutionCost;
    
    for (int i = 0; i <= m; i++) {
        for (int j = 0; j <= n; j++) {
            matrix[i][j] = 0;
        }
    }

    for (int i = 1; i <= m; i++) {
        matrix[i][0] = i;
    }

    for (int j = 1; j <= n; j++) {
        matrix[0][j] = j;
    }

    for (int j = 1; j <= n; j++) {
        for (int i = 1; i <= m; i++) {
            if (word1[i-1] == word2[j-1]) {
                substitutionCost = 0;
            } else {
                substitutionCost = 1;
            }

            matrix[i][j] = minimum(matrix[i-1][j] + 1, matrix[i][j-1] + 1, matrix[i-1][j-1] + substitutionCost);
        }
    }

    return matrix[m][n];
}

/**
* Checks a given input to ensure it does not contain any non-alphabetic
* characters.
* @param input The input to validate.
* @return 1 if the input is valid, 0 otherwise.
*/

int validateInput(char* input) {
    for (int i = 0; i < strlen(input); i++) {
        if (input[i] < 65) {
            return 0;
        } else if (input[i] > 90 && input[i] < 97) {
            return 0;
        } else if (input[i] > 123) {
            return 0;
        }
    }
    
    return 1;
}

/**
* Converts all characters in a given input to lowercase.
* @param input The input to sanitize.
*/

void sanitizeInput(char* input) {
    for (int i = 0; i < strlen(input); i++) {
        if (input[i] >= 65 && input[i] <= 90) {
            input[i] = tolower(input[i]);
        }
    }
}

/* Source used:
 https://www.geeksforgeeks.org/taking-string-input-space-c-3-different-methods/
 */

/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    HashMap* map = hashMapNew(1000);

    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];
    int quit = 0;
    while (!quit)
    {
        printf("Enter a word or \"quit\" to quit: ");
        scanf("%[^\n]%*c", inputBuffer);
        
        while(!validateInput(inputBuffer)) {
            printf("Invalid input\n");
            printf("Enter a word or \"quit\" to quit: ");
            scanf("%[^\n]%*c", inputBuffer);
        }
        
        sanitizeInput(inputBuffer);

        if (strcmp(inputBuffer, "quit") == 0) {
            quit = 1;
        }
        
        else {
            if (hashMapContainsKey(map, inputBuffer)) {
                printf("The inputted word \"%s\" is spelled correctly.\n", inputBuffer);
            }
            
            else {
                printf("The inputted word \"%s\" is spelled incorrectly.\n", inputBuffer);
                
                char* closestMatches[NUM_MATCHES];
                int closestDistances[NUM_MATCHES];
                
                // Initialize closest distances to -1
                for (int i = 0; i < NUM_MATCHES; i++) {
                    closestDistances[i] = -1;
                }
                
                for (int i = 0; i < map->capacity; i++) {
                    HashLink* current = map->table[i];
                    
                    while (current != NULL) {
                        int distance = levenshtein(inputBuffer, current->key);
                        current->value = distance;
                        
                        int j = 0;
                        int added = 0;
                        
                        // Check to see if the distance is lower than a current closest distance
                        while (j < NUM_MATCHES && !added) {
                            if (distance < closestDistances[j] || closestDistances[j] == -1) {
                                added = 1;
                                
                                // If it is, shift the current closest matches to the right
                                for (int k = NUM_MATCHES - 1; k > j; k--) {
                                    closestDistances[k] = closestDistances[k - 1];
                                    closestMatches[k] = closestMatches[k - 1];
                                }
                                
                                // Fill in the hole with the new closest match
                                closestDistances[j] = distance;
                                closestMatches[j] = current->key;
                            }
                            
                            j++;
                        }
                        
                        current = current->next;
                    }
                }
                
                printf("Did you mean...\n");
                for (int i = 0; i < NUM_MATCHES; i++) {
                    printf("%s?\n", closestMatches[i]);
                }
            }
        }
    }

    hashMapDelete(map);
    return 0;
}
