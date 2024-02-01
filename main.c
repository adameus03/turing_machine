/** 
 * Turing machine visualization
 * 
 * Compile & run:
 *  cmake --build ./build --config Release --target all --
 *  cd ./build
 *  ./turing
 *
 */

#include "visualizer.h"

int main()
{
    display_tm_visualization_window();
    //load_visualization_tm("../tms/bb2.tm");
    //load_visualization_tm("../tms/bb3.tm");
    //load_visualization_tm("../tms/bb4.tm");
    //load_visualization_tm("../tms/bb5c.tm");
    load_visualization_tm("../tms/bb6c.tm");
    return 0;
}
