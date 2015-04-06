struct Coord{
    Coord(){};
    Coord(int a,int b){
        x = a;
        y = b;
    }
    Choord(int num){
        x = num%6;
        y = num/6;
    }
    int x,y;
}
class Board{
    char field[6][6]; ///0 mean that field is free, 1 - red ghost, 2 - blue ghost, 3 - enemie ghost
    int my_red = 4,my_blue = 4,op_red = 4,op_blue = 4;
    bool can_i(Coord where,Coord form){
        bool ret = false;
        if(where == )
    }
    bool move(char where,char from){

    }
}
