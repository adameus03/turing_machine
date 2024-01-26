#include "turing.h"
#include <stdint.h>
#include <SDL2/SDL.h>

#define VISUALIZER_WINDOW_WIDTH 300 // 0 means full width
#define VISUALIZER_WINDOW_HEIGHT 300 // 0 means full height

#define FRAME_DURATION_MS 10
#define NUM_STEPS_PER_WINDOW 100
#define MIN_TAPE_POS 450//400
#define MAX_TAPE_POS 550//600

//void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius);
//void DrawHollowCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius);
//void draw_customer_locations(SDL_Renderer* m_window_renderer, cvrptw_problem_t problem, unsigned int window_width, unsigned int window_height, ready_time_t t);
//void draw_vehicle_locations(SDL_Renderer* m_windows_renderer, cvrptw_problem_t problem, cvrptw_solution_t sol, unsigned int window_width, unsigned int window_height, ready_time_t t);
void animate_tm(turing_machine_t* tm);
int display_tm_visualization_window();
void load_visualization_tm(char* tm_path);