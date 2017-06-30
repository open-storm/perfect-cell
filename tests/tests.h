#ifndef TESTS_H
#define TESTS_H

typedef struct result_t {
    int size;
    char **labels;
    char **results;
} result_t;

void test_run_all(result_t *r);
void test_post_results(result_t *r);
void test_free_results(result_t *r);
void test_configuration(result_t *r);

char* test_modem();
char* test_data();
char* test_valve_run();
char* test_ultrasonic();
char* test_decagon();
char* test_updater();
char* test_autosampler();

#endif
