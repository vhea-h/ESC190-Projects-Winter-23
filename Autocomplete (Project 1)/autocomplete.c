//IMPLEMENTATION DONE BY: Simona Tenche and Vhea He

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "autocomplete.h"

//HELPER FUNCTION
//Converts string with appropriate format into a term
void stot(char *str, char *name, double *weight){
    char weight_s[200];
    //while not tab, add to weight, otherwise add to name
    int i = 0;
    while (str[i] != '\t'){
        weight_s[i] = str[i];
        i++;
    }
    i++;
    weight_s[i] = '\0';

    int j = i;
    while (str[i] != '\n'){ //Stop reading at newline 
        name[i-j] = str[i];
        i++;
    }
    name[i-j] = '\0';
    
    //convert weight into number
    *weight = atof(weight_s);
}

//HELPER FUNCTION
//Comapres two terms and determines how they should relate lexicographically 
int compare_terms(const void *term1, const void *term2){
    const term *t1 = (const term *)term1;
    const term *t2 = (const term *)term2;
    return strcmp(t1->term, t2->term);
}

//Sorts array of terms lexicographically 
void sort_terms(term *terms, int num_terms){
    qsort(terms, num_terms, sizeof(term), compare_terms);
}

//FUNCTION 1
void read_in_terms(struct term **terms, int *pnterms, char *filename){
    FILE* contents = fopen(filename, "r");

    // Checking if file can be opened
    if (NULL == contents) {
        printf("ERROR. File can't be opened. \n");
    }

    char num_of_lines_str[200];

    //read first line
    if (fgets(num_of_lines_str, 200, contents) != NULL){
        *pnterms = atoi(num_of_lines_str);
    }

    *terms = malloc(*pnterms * sizeof(term));

    char str[200];
    int i = 0;
    while (fgets(str, 200, contents) != NULL){
        stot(str, (*terms)[i].term, &(*terms)[i].weight);
        i++;
    }

    //Sort everything alphabetically using term attribute
    sort_terms(*terms, *pnterms);

    // Closing the file
    fclose(contents);
}

//HELPER FUNCTION
//Determines if the first letters of a string match the letters of the given substring
int starts_with(char *str, char *substr){
    int substr_len = strlen(substr);
    return strncmp(str, substr, substr_len) == 0;
    //strncmp compares the first n bytes of each string
}

//FUNCTION 2
//Returns index of the lowest match
int lowest_match(struct term *terms, int nterms, char *substr){
    
    int low = 0;
    int high = nterms - 1; // nterms is the number of terms in terms
    int low_match_index = -1;

    //while low index does not surpass high (checks everything)
    while (low <= high){
        int middle = (low + high) / 2; // find the middle
        if (starts_with(terms[middle].term, substr)){
            low_match_index = middle;
            high = middle - 1; //high = mid - 1;
        }
        else if (strcmp(terms[middle].term, substr) < 0){ //current is before target
            low = middle + 1;
        }
        else { 
            high = middle - 1;
        }
    }
    return low_match_index;
}

//FUNCTION 3
//Returns index of the highest match
int highest_match(struct term *terms, int nterms, char *substr){
    int low = 0;
    int high = nterms - 1;
    int high_match_index = -1;

    //while low index does not surpass high (checks everything)
    while (low <= high){
        int mid = (low + high) / 2;
        if (starts_with(terms[mid].term, substr)){
            high_match_index = mid;
            low = mid + 1; //high = mid - 1;
        }
        else if (strcmp(terms[mid].term, substr) < 0){ //current is before target
            low = mid + 1;
        }
        else { 
            high = mid - 1;
        }
    }

    return high_match_index;
}

//HELPER FUNCTION 
//Compares two terms by weight
int compare_terms_weight(const void *term1, const void *term2){
    const term *t1 = (const term *)term1;
    const term *t2 = (const term *)term2;
    return ((t2->weight) - (t1->weight));
}

//HELPER FUNCTION 
//Sorts list of terms by weight
void sort_terms_weight(term *terms, int num_terms){
    qsort(terms, num_terms, sizeof(term), compare_terms_weight);
}

//FUNCTION 4
void autocomplete(struct term **answer, int *n_answer, struct term *terms, int nterms, char *substr){
    int first_match_index = lowest_match(terms, nterms, substr);
    int last_match_index = highest_match(terms, nterms, substr);
    *n_answer = last_match_index - first_match_index + 1; //Inclusive difference

    *answer = malloc(*n_answer * sizeof(term));

    for (int i = 0; i < *n_answer; i++){
        (*answer)[i] = terms[first_match_index + i];
    }
    sort_terms_weight(*answer, *n_answer);
}