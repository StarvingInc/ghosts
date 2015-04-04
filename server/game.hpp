#include <SFML/Network.hpp>

enum Command {BOARD, WIN, LOSE, MOVE, OK};

void get_board(char id, char *board, char **red, char **blue);

inline char find(char a, char *tab);

bool validate_move(char id, char *move, char **red, char ** blue);

Command make_move(char id, char *move, char **red, char **blue, char *taken_red, char *taken_blue);

void game(sf::TcpSocket* socket);

