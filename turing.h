typedef unsigned char tm_state_t;
typedef unsigned char tm_symbol_t;
typedef unsigned int tm_tape_numeric_t;

typedef unsigned int tm_stat_step_numeric_t;
typedef unsigned int tm_stat_read_numeric_t;
typedef unsigned int tm_stat_write_numeric_t;

#define TM_HALT_STATE 0xFFU
#define TM_INIT_STATE 0U
#define TM_BLANK_SYMBOL 0U
#define TM_INIT_HEAD 500U

#define TM_TAPE_LENGTH 1000U
#define TM_MAX_STATES 100U
#define TM_MAX_SYMBOLS 10

#define TM_GUARDS
#define TM_STDOUT_OUTPUT
#define TM_STDERR_OUTPUT
#define TM_STREAM_OUTPUT
#define TM_STAT_INTERFACE
#define TM_FILE_INTERFACE
//#define TM_DEBUG

typedef enum {
    TM_HEAD_LEFT,
    TM_HEAD_RIGHT    
} tm_head_dir_t;

typedef struct {
    tm_symbol_t read_symbol;
    tm_symbol_t write_symbol;
    tm_head_dir_t head_direction;
    tm_state_t state;
} tm_state_transition_t;

typedef struct {
    tm_state_transition_t transitions[TM_MAX_SYMBOLS];
    tm_symbol_t bundle_size;
} tm_transition_bundle_t;

#ifdef TM_GUARDS
typedef enum {
    TM_GUARD_OK,
    TM_GUARD_ERROR
} tm_initialization_guard_t;
#endif

typedef struct {
    tm_symbol_t tape[TM_TAPE_LENGTH];
    tm_tape_numeric_t head;
    tm_state_t state; // target state
    tm_transition_bundle_t transition_bundles[TM_MAX_STATES];
    #ifdef TM_GUARDS
    tm_initialization_guard_t transition_bundles_initialized;
    #endif
    tm_state_t num_states;
    tm_symbol_t num_symbols;
} turing_machine_t;

typedef enum {
    TM_VALIDATION_SUCCESS,
    TM_VALIDATION_FAILURE
} turing_machine_validation_result_t;

typedef enum {
    TM_STATUS_RUNNING,
    TM_STATUS_HALTED
} turing_machine_status_t;

void tm_init_tape(turing_machine_t* tm);

void tm_init(turing_machine_t* tm, tm_state_t num_states, tm_symbol_t num_symbols);

void tm_print_tape(turing_machine_t* tm);

turing_machine_validation_result_t tm_validate_machine(turing_machine_t* tm);

tm_transition_bundle_t* tm_get_transition_bundle(turing_machine_t* tm);

tm_state_transition_t* tm_get_transition(turing_machine_t* tm);

turing_machine_status_t tm_get_status(turing_machine_t* tm);


#ifdef TM_STAT_INTERFACE
typedef struct {
    tm_tape_numeric_t min_head;
    tm_tape_numeric_t max_head;
    tm_stat_step_numeric_t num_steps;
    tm_stat_read_numeric_t reads[TM_MAX_SYMBOLS];
    tm_stat_write_numeric_t writes[TM_MAX_SYMBOLS];
    tm_stat_step_numeric_t state_visits[TM_MAX_STATES];
    tm_symbol_t tm_num_symbols;
} turing_machine_stat_t;

void tm_make_transition(turing_machine_t* tm, turing_machine_stat_t* tm_stat);

void tm_stat_init(turing_machine_stat_t* tm_stat, tm_symbol_t tm_num_symbols, tm_state_t tm_num_states);

void tm_error(char* message);

#else
void tm_make_transition(turing_machine_t* tm);
#endif

#ifdef TM_FILE_INTERFACE
void tm_from_file(turing_machine_t* tm, char* path);
#endif
