#include "turing.h"

#if defined(TM_STDERR_OUTPUT) || defined(TM_STDOUT_OUTPUT)
#include <stdio.h>
#endif

#include <stdlib.h>
#include <stdarg.h>

void tm_init_tape(turing_machine_t* tm) {
    for (tm_tape_numeric_t i = 0; i < TM_TAPE_LENGTH; i++) {
        tm->tape[i] = TM_BLANK_SYMBOL;
    }
}

/**
 * @note Doesn't initialize transition bundles, it needs to be done separately
*/
void tm_init(turing_machine_t* tm, tm_state_t num_states, tm_symbol_t num_symbols) {
    tm_init_tape(tm);
    tm->head = TM_INIT_HEAD;
    tm->state = TM_INIT_STATE;
    tm->num_states = num_states;
    tm->num_symbols = num_symbols;
    #ifdef TM_GUARDS
    tm->transition_bundles_initialized = TM_GUARD_ERROR;
    #endif
}

void tm_error(char* message) {
    #ifdef TM_STDERR_OUTPUT
    fprintf(stderr, message);
    #endif
    exit(1);
}

void tm_debug(char* message) {
    #if defined(TM_DEBUG) && defined(TM_STDOUT_OUTPUT)
    printf(message);
    #endif
}

void tm_debugf(char* format, ...) {
    #if defined(TM_DEBUG) && defined(TM_STDOUT_OUTPUT)
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    #endif
}

void tm_errorf(char* format, ...) {
    #ifdef TM_STDERR_OUTPUT
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    #endif
    exit(1);
}

void tm_print(char* message) {
    #ifdef TM_STDOUT_OUTPUT
    printf(message);
    #endif
}

void tm_printf(char* format, ...) {
    #ifdef TM_STDOUT_OUTPUT
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    #endif
}

#ifdef TM_STREAM_OUTPUT
void tm_fprintf(FILE* stream, char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);
}
void tm_fdump_config(FILE* stream, turing_machine_t* tm) {
    tm_fprintf(stream, "Turing machine configuration:\n");
    tm_fprintf(stream, "Number of states: %hhu\n", tm->num_states);
    tm_fprintf(stream, "Number of symbols: %hhu\n", tm->num_symbols);
    tm_fprintf(stream, "Head position: %u\n", tm->head);
    tm_fprintf(stream, "State: %hhu\n", tm->state);
    tm_fprintf(stream, "Transition bundles:\n");
    for (tm_state_t i = 0; i < tm->num_states; i++) {
        tm_fprintf(stream, "State %hhu:\n", i);
        tm_transition_bundle_t* tb = &tm->transition_bundles[i];
        for (tm_symbol_t j = 0; j < tb->bundle_size; j++) {
            tm_state_transition_t* t = &tb->transitions[j];
            tm_fprintf(stream, "  Read symbol %hhu: write symbol %hhu, head direction %hhu, next state %hhu\n", t->read_symbol, t->write_symbol, t->head_direction, t->state);
        }
    }
    tm_printf("\n");
}
#endif

turing_machine_validation_result_t tm_validate_machine(turing_machine_t* tm) {
    unsigned char halt_symbol_found = 0;
    for (tm_state_t i = 0; i < tm->num_states; i++) {
        tm_transition_bundle_t* tb = &tm->transition_bundles[i];
        if (tb == 0) { 
            tm_errorf("Transition bundle for state %hhu is null", i);
            return TM_VALIDATION_FAILURE; 
        }
        if (tb->bundle_size <= 0) {
            tm_errorf("Transition bundle for state %hhu has bundle size %hhu, which is less than or equal to 0", i, tb->bundle_size);
            return TM_VALIDATION_FAILURE;
        }
        if (tb->bundle_size != tm->num_symbols) { 
            tm_errorf("Transition bundle for state %hhu has bundle size %hhu, which is different than the number of symbols %hhu. Required to be the same", i, tb->bundle_size, tm->num_symbols);
            return TM_VALIDATION_FAILURE;
        }
        for (tm_symbol_t j = 0; j < tb->bundle_size; j++) {
            tm_state_transition_t* t = &tb->transitions[j];
            if (t->read_symbol != j) {
                tm_errorf("Transition bundle for state %hhu has transition with read symbol %hhu, which is not equal to the expected read symbol %hhu", i, t->read_symbol, j);
                return TM_VALIDATION_FAILURE; 
            }
            if (t->write_symbol < 0 || t->write_symbol >= tm->num_symbols) { 
                tm_errorf("Transition bundle for state %hhu has transition with read symbol %hhu and write symbol %hhu, which is not in the range [0, %hhu)", i, t->read_symbol, t->write_symbol, tm->num_symbols);
                return TM_VALIDATION_FAILURE;
            }
            if (t->head_direction != TM_HEAD_LEFT && t->head_direction != TM_HEAD_RIGHT) {
                tm_errorf("Transition bundle for state %hhu has transition with read symbol %hhu and head direction %hhu, which is not TM_HEAD_LEFT or TM_HEAD_RIGHT", i, t->read_symbol, t->head_direction);
                return TM_VALIDATION_FAILURE;
            }
            if ((t->state < 0 || t->state >= tm->num_states) && t->state != TM_HALT_STATE) { 
                tm_errorf("Transition bundle for state %hhu has transition with read symbol %hhu and next state %hhu, which is not in the range [0, %hhu) or equal to TM_HALT_STATE", i, t->read_symbol, t->state, tm->num_states);
                return TM_VALIDATION_FAILURE;
            }
            if (t->state == TM_HALT_STATE) {
                halt_symbol_found = 1; 
            }
        }
    }

    if (!halt_symbol_found) {
        tm_error("No halt symbol found");
        return TM_VALIDATION_FAILURE; 
    }
    return TM_VALIDATION_SUCCESS;
}

tm_transition_bundle_t* tm_get_transition_bundle(turing_machine_t* tm) {
    return &tm->transition_bundles[tm->state];
}

tm_state_transition_t* tm_get_transition(turing_machine_t* tm) {
    tm_transition_bundle_t* tb = tm_get_transition_bundle(tm);
    tm_symbol_t read_symbol = tm->tape[tm->head];
    return tb->transitions + read_symbol;
}

turing_machine_status_t tm_get_status(turing_machine_t* tm) {
    if (tm->state == TM_HALT_STATE) {
        return TM_STATUS_HALTED;
    }
    return TM_STATUS_RUNNING;
}

#ifdef TM_STAT_INTERFACE
void tm_make_transition(turing_machine_t* tm, turing_machine_stat_t* tm_stat) {
#else
void tm_make_transition(turing_machine_t* tm) {
#endif
    if (tm->state == TM_HALT_STATE) {
        tm_error("Turing machine is already in halt state\n");
    }
    tm_state_transition_t* t = tm_get_transition(tm);
    tm_debugf("Read symbol %hhu, write symbol %hhu, head direction %hhu, next state %hhu, tape pos %u, state: %hhu\n", t->read_symbol, t->write_symbol, t->head_direction, t->state, tm->head, tm->state);

    tm->tape[tm->head] = t->write_symbol;

    #ifdef TM_STAT_INTERFACE
    tm_stat->reads[tm->tape[tm->head]]++;
    tm_stat->writes[t->write_symbol]++;
    tm_stat->state_visits[tm->state]++;
    #endif

    if (t->head_direction == TM_HEAD_LEFT) {
        if (tm->head == 0x0) {
            tm_error("Turing machine head out of range");
        }
        tm->head--;
    }
    else {
        if (tm->head == TM_TAPE_LENGTH - 1) {
            tm_error("Turing machine head out of range");
        }
        tm->head++;
    }
    
    #ifdef TM_STAT_INTERFACE
    if (tm->head < tm_stat->min_head) {
        tm_stat->min_head = tm->head;
    }
    else if (tm->head > tm_stat->max_head) {
        tm_stat->max_head = tm->head;
    }
    tm_stat->num_steps++;
    #endif

    tm->state = t->state;
}


#ifdef TM_STAT_INTERFACE
void tm_stat_init(turing_machine_stat_t* tm_stat, tm_symbol_t tm_num_symbols, tm_state_t tm_num_states) {
    tm_stat->min_head = TM_INIT_HEAD;
    tm_stat->max_head = TM_INIT_HEAD;
    tm_stat->num_steps = 0;
    tm_stat->tm_num_symbols = tm_num_symbols;
    for (tm_symbol_t i = 0; i < tm_num_symbols; i++) {
        tm_stat->reads[i] = 0;
        tm_stat->writes[i] = 0;
    }
    for (tm_state_t i = 0; i < tm_num_states; i++) {
        tm_stat->state_visits[i] = 0;
    }
}
#endif

#ifdef TM_FILE_INTERFACE

tm_state_t tm_resolve_state_name(char state_name, char* state_names, char halt_state_name, tm_state_t num_states) {
    for (tm_state_t i = 0; i < num_states; i++) {
        if (state_names[i] == state_name) {
            return i;
        }
    }
    if (state_name == halt_state_name) {
        return TM_HALT_STATE;
    }
    tm_errorf("Could not resolve state name %c", state_name);
    return 0;
}

tm_symbol_t tm_resolve_symbol_name(char symbol_name, char* symbol_names, tm_symbol_t num_symbols) {
    for (tm_symbol_t i = 0; i < num_symbols; i++) {
        if (symbol_names[i] == symbol_name) {
            return i;
        }
    }
    tm_errorf("Could not resolve symbol name %c", symbol_name);
    return 0;
}

tm_head_dir_t tm_resolve_head_direction_name(char head_direction_name, char* head_direction_names) {
    if (!head_direction_name) {
        tm_error("Null reference values for head direction names");
    }

    if (head_direction_name == head_direction_names[0]) {
        return TM_HEAD_LEFT;
    }
    else if (head_direction_name == head_direction_names[1]) {
        return TM_HEAD_RIGHT;
    }
    tm_errorf("Could not resolve head direction name %c", head_direction_name);
    return 0;
}

/**
 * *.tm file format specification:
 * 1st line: number of states (integer) and number of symbols (integer) space-separated
 * 2nd line: state name (single characters) space-separated
 * 3rd line: halt state name (single character)
 * 4th line: symbol names (single characters) space-separated
 * 5th line: head left name (single character) and head right name (single character) space-separated
 * 6th line and onwards: transition table
 * 
 * Transition table format:
 * Each line represents a collection of transitions
 * Transitions in the first line correspond to read symbol 0, transitions in the second line correspond to read symbol 1, etc.
 * Each line should contain a space-separated list of transition tuples
 * First transition tuple corresponds to current state 0, second transition tuple corresponds to current state 1, etc.
 * Each transition tuple is a string of 3 characters: write symbol, head direction, next state
 * 
 * @note This function initializes the turing machine - it calls tm_init() and initializes transition bundles
*/
void tm_from_file(turing_machine_t* tm, char* path) {
    char state_names[TM_MAX_STATES];
    char halt_state_name;
    char symbol_names[TM_MAX_SYMBOLS];
    char head_left_name;
    char head_right_name;

    tm_state_t num_states;
    tm_symbol_t num_symbols;
    
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        tm_error("Could not open file");
    }

    // Read number of states and symbols, example "3 2"
    int num_states_int;
    int num_symbols_int;
    if (fscanf(file, "%d %d", &num_states_int, &num_symbols_int) != 2) {
        tm_error("Could not read number of states and symbols from file");
    }
    num_states = (tm_state_t)num_states_int;
    num_symbols = (tm_symbol_t)num_symbols_int;

    // Validate number of states and symbols
    if (num_states > TM_MAX_STATES || num_symbols > TM_MAX_SYMBOLS) {
        tm_error("Number of states or symbols exceeds maximum");
    }
    if (num_states < 1 || num_symbols < 1) {
        tm_errorf("Number of states and symbols must be at least 1, got %hhu states and %hhu symbols", num_states, num_symbols);
    }
    
    // Move state names, example "A B C"
    for (tm_state_t i = 0; i < num_states; i++) {
        if (fscanf(file, " %c", &state_names[i]) != 1) {
            tm_error("Could not read state names from file");
        }
    }

    // Read halt state name, example "H"
    if (fscanf(file, " %c", &halt_state_name) != 1) {
        tm_error("Could not read halt state name from file");
    }

    // Read symbol names, example "0 1"
    for (tm_symbol_t i = 0; i < num_symbols; i++) {
        if (fscanf(file, " %c", &symbol_names[i]) != 1) {
            tm_error("Could not read symbol names from file");
        }
    }

    // Read head left and right names, example "L R"
    if (fscanf(file, " %c %c", &head_left_name, &head_right_name) != 2) {
        tm_error("Could not read head left and right names from file");
    }

    // Initialize transition bundles
    for (tm_state_t i = 0; i < num_states; i++) {
        tm_transition_bundle_t* tb = &tm->transition_bundles[i];
        tb->bundle_size = num_symbols;
    }
    // Read transition table
    for (tm_symbol_t i = 0; i < num_symbols; i++) {
        for (tm_state_t j = 0; j < num_states; j++) {
            tm_state_transition_t* t = &tm->transition_bundles[j].transitions[i];
            
            char write_symbol_name;
            char head_direction_name;
            char state_name;
            if (fscanf(file, " %c%c%c", &write_symbol_name, &head_direction_name, &state_name) != 3) {
                tm_error("Could not read transition table from file");
            }
            t->read_symbol = i;
            t->write_symbol = tm_resolve_symbol_name(write_symbol_name, symbol_names, num_symbols);
            t->head_direction = tm_resolve_head_direction_name(head_direction_name, (char[]){head_left_name, head_right_name});
            t->state = tm_resolve_state_name(state_name, state_names, halt_state_name, num_states);
        }
    }

    #ifdef TM_GUARDS
    tm->transition_bundles_initialized = TM_GUARD_OK;
    #endif

    fclose(file);
    
    // Fill the turing machine struct (transition bundles are already filled)
    tm_init(tm, num_states, num_symbols);

    tm_print("File TM data loaded\n");
    // Validate the turing machine
    if (tm_validate_machine(tm) != TM_VALIDATION_SUCCESS) {
        #ifdef TM_STREAM_OUTPUT
        tm_fdump_config(stderr, tm);
        #endif
        tm_error("Turing machine validation failed\n");
    }
}
#endif
