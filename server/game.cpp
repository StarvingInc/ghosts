#include "game.hpp"
#include <iostream>
#include <vector>
#include <cstring>

enum Command {BOARD, WIN, LOSE, MOVE, OK};

void get_board(char id, char *board, char **red, char **blue)
{
	memset(board, 0, 36);
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
}

inline char find(char a, char *tab) {
	char ret = -1;
	for(int i = 0; i < 4; ++i)
		if(tab[i] == a)
			ret = i;
	return ret;
}

bool validate_move(char id, char *move, char **red, char ** blue) {
	if(move[0] != 1 && move[0] != 2)
		return false;
	if(move[1] < 0 || move[2] < 0)
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

void game(sf::TcpSocket *socket)
{
	char buf[36];
	std::size_t size;
	bool running = true;
	char act_player = 0;
	Command command, status = OK;
	char taken_red[2] = {0}, taken_blue[2] = {0};

/*
 * Sending ID of player to him
 */
	buf[0] = 0; 
	socket[0].send(buf, 1);
	buf[0] = 1;
	socket[1].send(buf, 1);
/*
 * Geting initial position of ghosts from players
 */
	char *red[2], *blue[2];
	for(int i = 0; i < 2; ++i) {
		red[i] = new char[4];
		blue[i] = new char[4];
		socket[i].receive(buf, 8, size);
		memcpy(red[i], buf, 4);
		memcpy(blue[i], buf + 4, 4);
	}
/*
 * Sending board status and geting next move from player
 * board format: 36 chars, each contaion info about one field
 * ( 0-nothing, 1- red ghost, 2-blue ghost, 3-enemie ghost)
 * move respond format:
 * move[0] : 1 when moving ghost is red, 2 when moving ghost is blue
 * move[1] : old position of ghost
 * move[2] : new position of ghost
 */
	while(running) {
		for(int i = 0; i < 2; ++i) {
			command = BOARD;
			socket[i].send(&command, sizeof(command));
			socket[i].receive(buf, 1, size);
			get_board(i, buf, red, blue);
			socket[i].send(buf, 36);
			socket[i].receive(buf, 1, size);
		}
		if(status == WIN) {
			command = LOSE;
			socket[act_player].send(&command, sizeof(command));
			command = WIN;
			socket[1 - act_player].send(&command, sizeof(command));
			break;
		}
		else if(status == LOSE) {
			command = WIN;
			socket[act_player].send(&command, sizeof(command));
			command = LOSE;
			socket[1 - act_player].send(&command, sizeof(command));
			break;
		}
		else if(status == OK) {
			command = MOVE;
			socket[act_player].send(&command, sizeof(command));
			socket[act_player].receive(buf, 3, size);
			//validate if player cheating via modification of client (or bug in client)
			if(!validate_move(act_player, buf, red, blue)) {
				std::cerr << "wrong move" << std::endl;
				exit(1);
			}
			status = make_move(act_player, buf, red, blue, taken_red, taken_blue);
		}
		act_player = 1 - act_player;
	}

	std::cerr << "close connections and free memory  ";
	socket[0].disconnect();
	socket[1].disconnect();
	delete socket;
	std::cerr << "DONE" << std::endl;
}
