#include <string>
#include <sstream>
#include <cctype>
#include "../lib/position.h"
#include "../lib/bitboard_utilities.h"
#include "../lib/attacks.h"

using namespace std;

// Names in the same order as the ray_directions enum
const char* direction_names[direction_count] = {
    "north_west", "north", "north_east", "east",
    "south_east", "south", "south_west", "west"
};

int main(){
    initialize_attack_tables();
    ray_directions direction = north_east;
    for(int i = 0; i < 64; ++i){
        Square current = static_cast<Square>(i);
        char file = 'a' + fileOf(current);
        char rank = '1' + rankOf(current);
        cout << "This is the board for the direction: " << direction_names[direction]
             << " and square: " << file << rank;
        printBitboard(full_ray_table[direction][i]);
    }
}