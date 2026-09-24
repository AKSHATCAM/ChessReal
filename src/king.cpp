#include <string>
#include <sstream>
#include <cctype>
#include "../lib/position.h"
#include "../lib/bitboard_utilities.h"
#include "../lib/attacks.h"


int main(){
    initialize_attack_tables();

    for(int i = 0; i < 64; ++i){
        printBitboard(king_table[i]);
    }
}




