#include "tests.h"
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "extern.h"
#include "modem.h"
#include "startup.h"

struct result_t {
    int size;
    char **labels;
    char **results;
};

static const char TEST_HOST[] = "ec2-52-87-156-130.compute-1.amazonaws.com";
static const int TEST_PORT = 8086;
static const char USERNAME[] = "home_user";
static const char PASSWORD[] = "home_password";
static const char DATABASE[] = "_TEST";
static const int SSL_ENABLED = 1;

static const int TEST_COUNT = 7;
static const char TEST_LABELS[][20] = {
    "test_modem",   "test_data",    "test_valve",      "test_ultrasonic",
    "test_decagon", "test_updater", "test_autosampler"};

typedef char* (*VOID_FN)(void);
static const VOID_FN TEST_FNS[TEST_COUNT] = {
    &test_modem,   &test_data,    &test_valve,      &test_ultrasonic,
    &test_decagon, &test_updater, &test_autosampler};

void test_run_all(result_t* r) {
    // initialize our struct to hold all that data
    r->size = TEST_COUNT;
    r->labels = (char**) malloc(sizeof(char*) * TEST_COUNT);
    r->results = (char**) malloc(sizeof(char*) * TEST_COUNT);

    for (int i = 0; i < TEST_COUNT; ++i) {
        r->labels[i] = strdup(TEST_LABELS[i]);
        r->results[i] = TEST_FNS[i](r);
    }
}

void test_configuration(result_t *r) {
    const char[][15] var_names = {"cell_fer", "cell_strength", "conn_attempts",
                                  "v_bat"};
    const float[] readings = {17f, 0f, 10f, 5.5f};

    char socket_dial_str[100] = {'\0'};
    char body[MAX_PACKET_LENGTH] = {'\0'};
    char send_str[MAX_PACKET_LENGTH] = {'\0'};
    char response_str[MAX_PACKET_LENGTH] = {'\0'};

    // tmp variable to store current config
    char user_[20] = user;
    char pass_[50] = pass;
    char database_[20] = database;

    int main_port_ = main_port;
    char main_host_[100] = main_host;

    // reuse current config
    user = USERNAME;
    pass = PASSWORD;
    database = DATABASE;

    main_port = TEST_PORT;
    main_host = TEST_HOST;

    // send phony data
    send_readings(body, send_str, response_str, socket_dial_str, var_names,
                  readings, sizeof(var_names));
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
char* test_modem() {
    // Update metadata (node_id, user, pass, database
    if (modem_startup(&connection_attempt_counter)) {
        // Initialize SSL if enabled
        initialize_ssl(&ssl_enabled, &ssl_initialized);
        // Update metadata if enabled
        status = run_meta_subroutine(meid, send_str, response_str, 1u);
        modem_shutdown();
    }

    // Blink the LED to indicate the board is awake
    blink_LED(4u);
}

char* test_data() { return NULL: }

char* test_valve_run() { return NULL; }

char* test_ultrasonic() { return NULL; }

char* test_decagon() { return NULL; }

char* test_updater() { return NULL; }

char* test_autosampler() { return NULL; }
