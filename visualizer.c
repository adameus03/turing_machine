#include "visualizer.h"
#include <SDL2/SDL_ttf.h>
#include <pthread.h>
#include <math.h>

SDL_Window   *m_window          = NULL;
SDL_Renderer *m_window_renderer = NULL;
SDL_Event     m_window_event;
unsigned int window_width = VISUALIZER_WINDOW_WIDTH;
unsigned int window_height = VISUALIZER_WINDOW_HEIGHT;

void* eventListenerThreadHandler(void* arg_p) {
    while(1)
    {
        SDL_WaitEvent(&m_window_event);
        //while(SDL_PollEvent(&m_window_event) > 0)
        switch(m_window_event.type) {
            case SDL_QUIT:
                SDL_DestroyRenderer(m_window_renderer);
                SDL_DestroyWindow(m_window);
                exit(0);
        }
        //update(1.0/60.0, &x, &y);
        //draw(m_window_renderer, x, y);
    }
}

/**
 * @todo Get rid of magic numbers
*/
void render_counter(unsigned int counter) {
    // Clear counter area
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 90;
    rect.h = 30;
    SDL_SetRenderDrawColor(m_window_renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(m_window_renderer, &rect);
    SDL_RenderCopy(m_window_renderer, NULL, &rect, &rect);

    TTF_Font* Sans = TTF_OpenFont("../assets/fonts/OpenSans-SemiboldItalic.ttf", 24);
    if (Sans == NULL) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        return;
    }

    SDL_Color White = {255, 255, 255};
    char text[20];
    sprintf(text, "Step: %u", counter);
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, text, White);
    SDL_Texture* Message = SDL_CreateTextureFromSurface(m_window_renderer, surfaceMessage);
    SDL_Rect Message_rect;
    Message_rect.x = 0;   
    Message_rect.y = 0; 
    Message_rect.w = 90;
    Message_rect.h = 30; 
    SDL_RenderCopy(m_window_renderer, Message, NULL, &Message_rect);
    //SDL_RenderPresent(m_window_renderer);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
}

void clear_window() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = window_width;
    rect.h = window_height;
    
    //Fill the full window with black
    SDL_SetRenderDrawColor(m_window_renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(m_window_renderer, &rect);
    SDL_RenderCopy(m_window_renderer, NULL, &rect, &rect);
    //SDL_RenderPresent(m_window_renderer);
}

/**
 * Render a row of red or green rectangles depending on the tape content.
 * If the tape content is 0, the rectangle is red, otherwise it is green.
 * The x-coordinate of the tape is 0
 * The y-coordinate of the rendered tape is determined by tm_stat->num_steps, NUM_STEPS_PER_WINDOW and the window height.
 * The width of the tape is determined by the window width.
 * The height of the tape is determined by NUM_STEPS_PER_WINDOW and the window height.
*/
void render_current_tm_tape(turing_machine_t* tm, turing_machine_stat_t* tm_stat) {
    tm_tape_numeric_t render_tape_num_cells = MAX_TAPE_POS - MIN_TAPE_POS + 1;
    for (tm_tape_numeric_t i = 0; i < render_tape_num_cells; i++) {
        SDL_Rect rect;
        rect.x = i * window_width / render_tape_num_cells;
        rect.y = tm_stat->num_steps * window_height / NUM_STEPS_PER_WINDOW;
        rect.y %= window_height; // wrap around
        rect.w = window_width / render_tape_num_cells;
        rect.h = window_height / NUM_STEPS_PER_WINDOW;
        if (tm->tape[MIN_TAPE_POS + i] == 0) {
            //SDL_SetRenderDrawColor(m_window_renderer, 255, 0, 0, 255);
            SDL_SetRenderDrawColor(m_window_renderer, 0, 0, 0, 255);
        }
        else {
            //SDL_SetRenderDrawColor(m_window_renderer, 0, 255, 0, 255);
            SDL_SetRenderDrawColor(m_window_renderer, 255, 255, 255, 255);
        }
        SDL_RenderFillRect(m_window_renderer, &rect);
        SDL_RenderCopy(m_window_renderer, NULL, &rect, &rect);
    }
}

/**
 * @todo Add support for solution
*/
void animate_tm(turing_machine_t* tm) {
    turing_machine_stat_t tm_stat;
    tm_stat_init(&tm_stat, tm->num_symbols, tm->num_states);


    while(tm_get_status(tm) == TM_STATUS_RUNNING) {
        tm_make_transition(tm, &tm_stat);
        render_counter(tm_stat.num_steps);

        render_current_tm_tape(tm, &tm_stat);

        if ((tm_stat.num_steps % NUM_STEPS_PER_WINDOW) == 0) {
            clear_window();
            SDL_RenderClear(m_window_renderer);
        }
        SDL_RenderPresent(m_window_renderer);
        SDL_Delay(FRAME_DURATION_MS);
        
    }
    while (1) {
        SDL_Delay(FRAME_DURATION_MS);
    }
}

int init_ttf() {
    if (TTF_Init() < 0) {
        printf("TTF_Init: %s\n", TTF_GetError());
        return 1;
    }
    return 0;
}

int display_tm_visualization_window() {
    SDL_Init(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow("TM Visualizer",
                                0/*SDL_WINDOWPOS_CENTERED*/,
                                0/*SDL_WINDOWPOS_CENTERED*/,
                                VISUALIZER_WINDOW_WIDTH, VISUALIZER_WINDOW_HEIGHT,
                                0);

    if(m_window == NULL)
    {
        printf("Failed to create window\n");
        printf("SDL2 Error: %s\n", SDL_GetError());
        return 1;
    }

    //unsigned int window_width = VISUALIZER_WINDOW_WIDTH;
    //unsigned int window_height = VISUALIZER_WINDOW_HEIGHT;
    if (VISUALIZER_WINDOW_WIDTH == 0 || VISUALIZER_WINDOW_HEIGHT == 0) {
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(0, &dm))
        {
            printf("Error getting desktop display mode\n");
            return -1;
        }
        window_width = VISUALIZER_WINDOW_WIDTH == 0 ? dm.w : VISUALIZER_WINDOW_WIDTH;
        window_height = VISUALIZER_WINDOW_HEIGHT == 0 ? dm.h : VISUALIZER_WINDOW_HEIGHT;;
    }
    
    SDL_SetWindowSize(m_window, window_width, window_height);

    
    m_window_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if(m_window_renderer == NULL)
    {
        printf("Failed to create renderer\n");
        printf("SDL2 Error: %s\n", SDL_GetError());
        return 1;
    }

    init_ttf();
    
    pthread_t eventListenerTID;
    pthread_create(&eventListenerTID, NULL, eventListenerThreadHandler, NULL);

    return 0;
}

void load_visualization_tm(char* tm_path) {
    if (tm_path == NULL) {
        fprintf(stderr, "Error: tm_path is NULL!\n");
        exit(EXIT_FAILURE);
    }
    
    turing_machine_t tm;
    tm_from_file(&tm, tm_path);
    animate_tm(&tm);

    /*cvrptw_problem_t problem = cvrptw_data_get(problem_path);
    

    cvrptw_solution_t solution = cvrptw_solution_get(sol_path);
    if (solution.num_vehicles == 0) {
        perror("Error: solution.num_vehicles == 0");
        exit(EXIT_FAILURE);
    }
    else if (solution.total_num_customers != problem.num_customers) {
        perror("Error: Invalid solution file for given problem file!");
        exit(EXIT_FAILURE);
    }
    animate_problem_with_solution(problem, solution);
    */
    
}