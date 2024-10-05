#include "autocomplete.c"

int main(void)
{
    struct term *terms;
    int nterms;
    read_in_terms(&terms, &nterms, "cities.txt");
    // for (int i = 0; i < nterms; i++) {
    //     printf("%f: %s\n", terms[i].weight, terms[i].term);
    // }
    // sort_terms_weight(terms, nterms);
    // for (int i = 0; i < nterms; i++) {
    //     printf("%f: %s\n", terms[i].weight, terms[i].term);
    // }

    printf("read_in_terms ran successfully\n");
    
    lowest_match(terms, nterms, "Tor");
    printf("lowest_match ran successfully\n");

    highest_match(terms, nterms, "Tor");
    printf("highest_match ran successfully\n");
    
    struct term *answer;
    int n_answer;
    autocomplete(&answer, &n_answer, terms, nterms, "Sha");
    printf("Best Match: %s\n", answer[0].term);
    printf("autocomplete ran successfully\n");

    //free allocated blocks here -- not required for the project, but good practice
    free(terms);
    return 0;
}

