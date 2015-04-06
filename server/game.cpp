#include <iostream>
#include <vector>
#include <cstring>
#include "game.hpp"

void get_board(char id, char *board, char **red, char **blue, char *taken_red, char *taken_blue)
{
	memset(board, 0, 38);
	for(int i = 0; i < 4; ++i) {
		if(red[id][i] != -1)
			board[red[id][i]] = 1;
		if(blue[id][i] != -1)
			board[blue[id][i]] = 2;
		if(red[1 - id][i] != -1)
			board[red[1 - id][i]] = 3;
		if(blue[1 - id][i] != -1)
			board[blue[1 - id][i]] = 3;
	}
	board[36] = taken_red[1 - id];
	board[37] = taken_blue[1 - id];
}

inline char find(char a, char *tab)
{
	char ret = -1;
	for(int i = 0; i < 4; ++i)
		if(tab[i] == a)
			ret = i;
	return ret;
}

bool validate_move(char id, char *move, char **red, char ** blue)
{
	if(move[0] != 1 && move[0] != 2)
		return false;
	if(move[1] < 0 || move[2] < 0)
		return false;
	if(move[1] >= 36 || move[2] >= 36)
		return false;
	if(move[0] == 1)
		if(find(move[1], red[id]) == -1)
			return false;
	if(move[0] == 2)
		if(find(move[1], blue[id]) == -1)
			return false;
	if(find(move[2], red[id]) != -1 || find(move[2], blue[id]) != -1)
		return false;
	if(move[1] % 6 != 0 && move[2] == (move[1] - 1))
		return true;
	if(move[1] % 6 != 5 && move[2] == (move[1] + 1))
		return true;
	if(move[1] / 6 != 0 && move[2] == (move[1] - 6))
		return true;
	if(move[1] / 6 != 5 && move[2] == (move[1] + 6))
		return true;
	return false;
}

Command make_move(char id, char *move, char **red, char **blue, char *taken_red, char *taken_blue)
{
	if(move[0] == 1)
		red[id][find(move[1], red[id])] = move[2];
	if(move[0] == 2)
		blue[id][find(move[1], blue[id])] = move[2];
	char position = find(move[2], red[1 - id]);
	if(position != -1){
		red[1 - id][position] = -1;
		taken_red[1 - id]++;
		if(taken_red[1 - id] == 4)
			return LOSE;
	}
	position = find(move[2], blue[1 - id]);
	if(position != -1) {
		blue[1 - id][position] = -1;
		taken_blue[1 - id]++;
		if(taken_blue[1 - id] == 4)
			return WIN;
	}
	if(id == 0 && move[0] == 2 && (move[2] == 30 || move[2] == 35))
		return WIN;
	if(id == 1 && move[0] == 2 && (move[2] == 0 || move[2] == 5))
		return WIN;
	return OK;
}

bool validate_initial_data(char **red, char **blue)
{
	for(int i = 1; i <= 4; ++i) {
		if(find(i, red[0]) == -1 && find(i, blue[0]) == -1)
			return false;
	}
	for(int i = 7; i <= 10; ++i) {
		if(find(i, red[0]) == -1 && find(i, blue[0]) == -1)
			return false;
	}
	for(int i = 34; i >= 31; --i) {
		if(find(i, red[1]) == -1 && find(i, blue[1]) == -1)
			return false;
	}
	for(int i = 28; i >= 25; --i) {
		if(find(i, red[1]) == -1 && find(i, blue[1]) == -1)
			return false;
	}
	return true;
}
