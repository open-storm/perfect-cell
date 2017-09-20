#include "tests.h"
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "extern.h"
#include "modem.h"
#include "startup.h"

static const char TEST_HOST[] = "ec2-13-58-145-29.us-east-2.compute.amazonaws.com"
static const int TEST_PORT = 8086;
static const char USERNAME[] = "test_user";
static const char PASSWORD[] = "test_pass";
static const char DATABASE[] = "TEST_DB";
static const int SSL_ENABLED = 1;

static const int TEST_COUNT = 7;
static const char TEST_LABELS[][20] = {
    "test_modem",   "test_data",    "test_valve",      "test_ultrasonic",
    "test_decagon", "test_updater", "test_autosampler"};

typedef char* (*VOID_FN)(result_t*);
static const VOID_FN TEST_FNS[TEST_COUNT] = {
    &test_modem,   &test_data,    &test_valve_run,  &test_ultrasonic,
    &test_decagon, &test_updater, &test_autosampler};

static void save_state() {}
static void restore_state() {}

void test_run_all(result_t* r) {
    // save the program's current state
    save_state();

    // initialize our struct to hold all that data
    r->size = TEST_COUNT;
    r->labels = (char**) malloc(sizeof(char*) * TEST_COUNT);
    r->results = (char**) malloc(sizeof(char*) * TEST_COUNT);

    for (int i = 0; i < TEST_COUNT; ++i) {
        r->labels[i] = strdup(TEST_LABELS[i]);
        r->results[i] = TEST_FNS[i](r);
    }

    // restore the last saved state
    restore_state();
}

void test_free_results(result_t* r) {
    // free all strings first
    for (int i = 0; i < r->size; ++i) {
        free(r->labels[i]);
        r->labels[i] = NULL;

        free(r->results[i]);
        r->results[i] = NULL;
    }

    // free allocated arrays
    free(r->labels);
    r->labels = NULL;

    free(r->results);
    r->results = NULL;

    r->size = 0;
}

void test_post_results(result_t* r) { return; }

/**
 * @brief Tests modem connectivity.
 *
 * @return Results of the test. String must be freed after use.
 */
char* test_modem() { return NULL; }

char* test_data() { return NULL; }

char* test_valve_run() { return NULL; }

char* test_ultrasonic() { return NULL; }

char* test_decagon() { return NULL; }

char* test_updater() { return NULL; }

char* test_autosampler() { return NULL; }
