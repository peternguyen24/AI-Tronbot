#include <ai/Game.h>
#include <ai/AI.h>
#include <time.h>
#include <fstream>
#include <queue>
#include <iostream>
// ==================== HOW TO RUN THIS =====================
// Call:
// "AI_Template.exe -h [host] -p [port] -k [key]"
//
// If no argument given, it'll be 127.0.0.1:3011
// key is a secret string that authenticate the bot identity
// it is not required when testing
// ===========================================================

//////////////////////////////////////////////////////////////////////////////////////
//                                                                                  //
//                                    GAME RULES                                    //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////
// - Game board is an array of MAP_SIZExMAP_SIZE blocks                             //
// - 2 players starts at 2 corners of the game board                                //
// - Each player will take turn to move                                             //
// - Player can only move left/right/up/down and stay inside the game board         //
// - The game is over when one of 2 players cannot make a valid move                //
// - In a competitive match:                                                        //
//   + A player will lose if they cannot connect to server within 10 seconds        //
//   + A player will lose if they don't make a valid move within 3 seconds          //
//////////////////////////////////////////////////////////////////////////////////////

// This function is called automatically each turn.
// If it's your turn, remember to call AI_Move() with a valid move before the time is run out.
// See <ai/Game.h> and <ai/AI.h> for supported APIs.


//left up right down
int tx[4] = { -1, 0, 1, 0 };
int ty[4] = { 0, -1, 0, 1 };
int maxv = 100001;
int max_depth = 7;
bool timeout;
clock_t start_time;
int MAX_DIS = 100;

//queue for bfs
int queue_x[1000];
int queue_y[1000];

//count number of call recursive
int count_function = 0;

//tool function
bool inside_table(int x, int y) {
	return (x >= 0 && x < MAP_SIZE && y >= 0 && y < MAP_SIZE);
}


//dfs finding component 
//1 - can reach from mine; 2 - can reach from enemy, 3 can reach from both;
void dfs(int my_board[][MAP_SIZE], int visited[][MAP_SIZE], int x, int y,int visit_value){
	int u, v;
	visited[x][y] += visit_value;
	for (int i = 0; i < 4; i++) {
		u = x + tx[i];
		v = y + ty[i];
		// 3 mean overlapped
		if (inside_table(u,v) && my_board[u][v] == 0 && visited[u][v] != visit_value && visited[u][v] != 3) {
			dfs(my_board,visited,u, v,visit_value);
		}
	}
}



// if 2 player roi vao trang thai isolated, return heuristic value;
// if 2 player ko isolated, return -maxv
int visited[MAP_SIZE][MAP_SIZE];
int end_state(int my_board[][MAP_SIZE], Position myPos, Position enemyPos){
	memset(visited, 0, sizeof(visited));
	//dfs for find connected component of mine and enemy
	//1 - can reach from mine; 2 - can reach from enemy, 3 can reach from both;
	dfs(my_board,visited, myPos.x, myPos.y, 1);
	dfs(my_board,visited, enemyPos.x, enemyPos.y, 2);
	
	int my_odd, my_even, enemy_odd, enemy_even;
	my_odd = 0; my_even = 0; enemy_odd = 0; enemy_even = 0;
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++){
			if (visited[i][j] == 3) {
				return -maxv;
			}
			//do a new algorithm on counting longest path
			if (visited[i][j] == 1 && my_board[i][j] == 0) {
				if ((i + j) % 2 == 0) {
					my_even++;
				}
				else {
					my_odd++;
				}
			} 
			if (visited[i][j] == 2 && my_board[i][j] == 0) {
				if ((i + j) % 2 == 0) {
					enemy_even++;
				}
				else {
					enemy_odd++;
				}
			}
		}
	}
	return (min(my_even, my_odd) * 2 - min(enemy_even, enemy_odd) * 2)*100;
}

//bfs 
// SURE FINE

void bfs(int my_board[][MAP_SIZE], int distance[][MAP_SIZE], Position pos) {
	Position current, newpos;
	int first = 0;
	int last = 0;
	int x, y,u,v;
	queue_x[0]= pos.x;
	queue_y[0] = pos.y;
	distance[pos.x][pos.y] = 0;
	while (first<=last) {
		x = queue_x[first];
		y = queue_y[first];
		first++;
		for (int i = 0; i < 4; i++) {
			u = x + tx[i];
			v = y + ty[i];
			if (inside_table(u,v) && my_board[u][v] == 0 && distance[u][v] == MAX_DIS) {
				last++;
				queue_x[last] = u;
				queue_y[last] = v;
				distance[u][v] = distance[x][y] + 1;
			}
		}
	}
}


//voironoi heuristic
int voironoi(int my_board[][MAP_SIZE], Position myPos, Position enemyPos, int my_distance[][MAP_SIZE], int enemy_distance[][MAP_SIZE]){
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++) {
			my_distance[i][j] = MAX_DIS;
			enemy_distance[i][j] = MAX_DIS;
		}
	}
	my_distance[myPos.x][myPos.y] = 0;
	enemy_distance[enemyPos.x][enemyPos.y] = 0;
	bfs(my_board, my_distance, myPos);
	bfs(my_board, enemy_distance, enemyPos);
	int res = 0;
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++) {
			if (my_board[i][j] == 0) {
				if (my_distance[i][j]<enemy_distance[i][j]) {
					res++;
				}
				if (my_distance[i][j]>enemy_distance[i][j]) {
					res--;
				}
			}
		}
	}
	return res;
}


class Chamber {
public:
	Chamber() {
		odd_num = 0;
		even_num = 0;
		is_leaf = true;
		active = true;
		battle_front = false;
	}
	Chamber(Position entrance_) {
		entrance = entrance_;
		odd_num = 0;
		even_num = 0;
		is_leaf = true;
		active = true;
		battle_front = false;
	}
	int odd_num, even_num;
	Position entrance;
	bool is_leaf;
	bool active;
	bool battle_front;
};


//bottle neck type 2
bool bottle_neck_type_2(int my_board[][MAP_SIZE], Position a_cell) {
	if (inside_table(a_cell.x - 1, a_cell.y - 1) && inside_table(a_cell.x + 1, a_cell.y + 1)) {
		// cheo tren trai -> duoi phai
		if (my_board[a_cell.x - 1][a_cell.y - 1] == 1 && my_board[a_cell.x + 1][a_cell.y + 1] == 1){
			if (my_board[a_cell.x][a_cell.y - 1] == 0 && my_board[a_cell.x + 1][a_cell.y] == 0 && my_board[a_cell.x + 1][a_cell.y - 1] == 0) {
				return true;
			}
			if (my_board[a_cell.x- 1 ][a_cell.y] == 0 && my_board[a_cell.x][a_cell.y + 1] == 0 && my_board[a_cell.x - 1][a_cell.y + 1] == 0) {
				return true;
			}
		}
		// cheo tren phai -> duoi trai
		if (my_board[a_cell.x - 1][a_cell.y + 1] == 1 && my_board[a_cell.x + 1][a_cell.y - 1] == 1){
			if (my_board[a_cell.x][a_cell.y - 1] == 0 && my_board[a_cell.x - 1][a_cell.y] == 0 && my_board[a_cell.x - 1][a_cell.y - 1] == 0) {
				return true;
			}
			if (my_board[a_cell.x + 1][a_cell.y] == 0 && my_board[a_cell.x][a_cell.y + 1] == 0 && my_board[a_cell.x + 1][a_cell.y + 1] == 0) {
				return true;
			}
		}
	}
	return false;
}

//bottle neck sure true;
bool bottle_neck(int my_board[][MAP_SIZE], Position current_cell, Position next_cell){
	if (current_cell.y == next_cell.y) {
		if (inside_table(current_cell.x, current_cell.y - 1) && inside_table(next_cell.x, next_cell.y - 1)) {
			if (my_board[current_cell.x][current_cell.y - 1] == 0 && my_board[next_cell.x][next_cell.y - 1] == 0) {
				return false;
			}
		}
		if (inside_table(current_cell.x, current_cell.y + 1) && inside_table(next_cell.x, next_cell.y + 1)) {
			if (my_board[current_cell.x][current_cell.y + 1] == 0 && my_board[next_cell.x][next_cell.y + 1] == 0) {
				return false;
			}
		}
	}

	if (current_cell.x == next_cell.x) {
		if (inside_table(current_cell.x - 1, current_cell.y) && inside_table(next_cell.x - 1, next_cell.y)) {
			if (my_board[current_cell.x - 1][current_cell.y] == 0 && my_board[next_cell.x - 1][next_cell.y] == 0) {
				return false;
			}
		}
		if (inside_table(current_cell.x + 1, current_cell.y) && inside_table(next_cell.x + 1, next_cell.y)) {
			if (my_board[current_cell.x + 1][current_cell.y] == 0 && my_board[next_cell.x + 1][next_cell.y] == 0) {
				return false;
			}
		}
	}

	return true;
}

bool even_pos(Position pos) {
	return ((pos.x + pos.y) % 2 == 0);
}

//give rough estimation about how long can we go
int local_longest_estimation(bool is_on_leaf, int even_num, int odd_num, Position entrance_pos, Position exit_pos){
	int estimation = 0;
	bool entrance_is_even = even_pos(entrance_pos);
	bool exit_is_even = even_pos(exit_pos);
	if (is_on_leaf) {
		estimation = min(even_num, odd_num)*2;
		if (!entrance_is_even && even_num > odd_num) {
			estimation++;
		} 
		if (entrance_is_even && odd_num > even_num) {
			estimation++;
		}
		return estimation;
	}
	else {
		estimation = min(even_num, odd_num) * 2;
		if (entrance_is_even != exit_is_even) {
			estimation = min(even_num, odd_num) * 2+1;
			if (!entrance_is_even && even_num <= odd_num) {
				estimation -= 2;
			}
			if (entrance_is_even && odd_num <= even_num) {
				estimation -= 2;
			}
		}
		return estimation;
	}
	return min(even_num, odd_num) * 2;
}

Position pos_queue[1000];
Chamber chambers[100];
int chamber_of_id[100];
int cell_id[MAP_SIZE][MAP_SIZE];
int cham1_array[100];
int cham2_array[100];
//find chamber value for 1 player
//just count for every cell (x,y) that distance[x][y]>0
int isolated_chamber_value(int my_board[][MAP_SIZE], Position pos,int distance[][MAP_SIZE],int base,int min_way[][MAP_SIZE]){
	Position entr1, entr2;
	int cham_num = 0;
	//so luong chamber: 0-> cham_num (khong phai tru 1)
	//id init;
	for (int i = 0; i < 100; i++) {
		chamber_of_id[i] = i;
	}
	//cell varialbe
	Position current_cell, next_cell;
	// origin chamber init
	Chamber origin = Chamber(pos);
	chambers[0] = origin;
	//table init~ all id  =-1 
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++) {
			cell_id[i][j] = -1;
		}
	}
	cell_id[pos.x][pos.y] = 0;

	//init queue
	int first = 0;
	int last = 0;
	pos_queue[0] = pos;

	Chamber current_chamber;
	Chamber next_chamber;
	int current_chamber_id;
	int next_chamber_id;
	//for merging
	int chamid1, chamid2, depth1, depth2;
	Chamber cham1, cham2;
	Position ent_pos;
	int common_id;
	Chamber common_cham;

	while (first <= last) {
		//pop queue
		current_cell = pos_queue[first];
		first++;
		current_chamber = chambers[chamber_of_id[cell_id[current_cell.x][current_cell.y]]];
		for (int i = 0; i < 4; i++) {
			next_cell = current_cell;
			next_cell.x += tx[i];
			next_cell.y += ty[i];
			//neu khong phai la o trong
			if (!inside_table(next_cell.x, next_cell.y) || my_board[next_cell.x][next_cell.y] != 0) {
				continue;
			}

			//neu khong phai la o gan pos hon pos's enemy ~~ a little ambigous here
			if (distance[next_cell.x][next_cell.y]*base<=0){
				continue;
			}

			// entrance of current chamber
			if (current_chamber.entrance.x == next_cell.x && current_chamber.entrance.y == next_cell.y) {
				continue;
			}


			//next_cell not belong to any chambers + next_cell is bottle_neck
			if (cell_id[next_cell.x][next_cell.y] == -1 && (bottle_neck(my_board, current_cell, next_cell) || (bottle_neck_type_2(my_board, current_cell) ) || first == 1)){
				cham_num++;
				chambers[cham_num] = Chamber(current_cell);
				if (even_pos(next_cell)) {
					chambers[cham_num].even_num = 1;
				}
				else {
					chambers[cham_num].odd_num = 1;
				}
				cell_id[next_cell.x][next_cell.y] = cham_num;
				last++; pos_queue[last] = next_cell;
				continue;
			}

			//next_cell not belong to any chambers and is not bottle_neck
			if (cell_id[next_cell.x][next_cell.y] == -1) {
				if (even_pos(next_cell)) {
					chambers[chamber_of_id[cell_id[current_cell.x][current_cell.y]]].even_num++;
				}
				else {
					chambers[chamber_of_id[cell_id[current_cell.x][current_cell.y]]].odd_num++;
				}
				cell_id[next_cell.x][next_cell.y] = cell_id[current_cell.x][current_cell.y];
				// add to stack
				last++; pos_queue[last] = next_cell;
				continue;
			}
			// same chamber
			if (chamber_of_id[cell_id[next_cell.x][next_cell.y]] == chamber_of_id[cell_id[current_cell.x][current_cell.y]]) {
				continue;
			}

			//next_cell belong to another chamber ~ hard part
			if (chamber_of_id[cell_id[next_cell.x][next_cell.y]] != chamber_of_id[cell_id[current_cell.x][current_cell.y]]) {
				next_chamber_id = chamber_of_id[cell_id[next_cell.x][next_cell.y]];
				current_chamber_id = chamber_of_id[cell_id[current_cell.x][current_cell.y]];
				depth1 = 1;
				depth2 = 1;
				chamid1 = current_chamber_id;
				chamid2 = next_chamber_id;
				//find way to root from current chamber
				cham1_array[0] = chamid1;
				while (chamid1 != 0) {
					ent_pos = chambers[chamid1].entrance;
					chamid1 = chamber_of_id[cell_id[ent_pos.x][ent_pos.y]];
					depth1++;
					cham1_array[depth1 - 1] = chamid1;
				}
				//find way to root from next chamber
				cham2_array[0] = chamid2;
				while (chamid2 != 0) {
					ent_pos = chambers[chamid2].entrance;
					chamid2 = chamber_of_id[cell_id[ent_pos.x][ent_pos.y]];
					depth2++;
					cham2_array[depth2 - 1] = chamid2;
				}

				//find common ancestor
				int x = 0;
				while (x < min(depth1, depth2)) {
					x++;
					if (cham1_array[depth1 - x] != cham2_array[depth2 - x]) {
						break;
					}
				}
				if (cham1_array[depth1 - x] == cham2_array[depth2 - x]){
					x++;
				}
				// if they start from a same point (bottleneck_type2)
				if (depth1 - x >= 0 && depth2 - x >= 0) {
					if (chambers[cham1_array[depth1 - x]].entrance.x == chambers[cham2_array[depth2 - x]].entrance.x
						&& chambers[cham1_array[depth1 - x]].entrance.y == chambers[cham2_array[depth2 - x]].entrance.y
						&& bottle_neck_type_2(my_board, chambers[cham1_array[depth1 - x]].entrance)) {

						common_id = cham1_array[depth1 - x];
						for (int i = 0; i <= depth1 - x - 1; i++) {
							chamid1 = cham1_array[i];
							chamber_of_id[chamid1] = common_id;
							chambers[chamid1].active = false;
							chambers[common_id].even_num += chambers[chamid1].even_num;
							chambers[common_id].odd_num += chambers[chamid1].odd_num;
						}
						for (int i = 0; i <= depth2 - x; i++) {
							chamid2 = cham2_array[i];
							chamber_of_id[chamid2] = common_id;
							chambers[chamid2].active = false;
							chambers[common_id].even_num += chambers[chamid2].even_num;
							chambers[common_id].odd_num += chambers[chamid2].odd_num;
						}
						for (int i = 0; i <= cham_num; i++) {
								while (chamber_of_id[chamber_of_id[i]] != chamber_of_id[i]) {
									chamber_of_id[i] = chamber_of_id[chamber_of_id[i]];
								}
						}

						continue;
					}
				}

				common_id = cham1_array[depth1 - x + 1];
				common_cham = chambers[common_id];
				//least common ancestor
				// reuse chamid1, chamid2 for indication
				// cong don vao common ancestor
				// also, redirect chamber_of_id to common ancestor
				for (int i = 0; i <= depth1 - x; i++) {
					chamid1 = cham1_array[i];
					chamber_of_id[chamid1] = common_id;
					chambers[chamid1].active = false;
					chambers[common_id].even_num += chambers[chamid1].even_num;
					chambers[common_id].odd_num += chambers[chamid1].odd_num;
				}
				for (int i = 0; i <= depth2 - x; i++) {
					chamid2 = cham2_array[i];
					chamber_of_id[chamid2] = common_id;
					chambers[chamid2].active = false;
					chambers[common_id].even_num += chambers[chamid2].even_num;
					chambers[common_id].odd_num += chambers[chamid2].odd_num;
				}
				for (int i = 0; i <= cham_num; i++) {
						while (chamber_of_id[chamber_of_id[i]] != chamber_of_id[i]) {
							//cout << chamber_of_id[chamber_of_id[i]];
							chamber_of_id[i] = chamber_of_id[chamber_of_id[i]];
						}
				}
				continue;
			}
		}
	}
	//check wheather a chamber is battle chamber or not.
	int uu, vv;
	for (int u = 0; u < MAP_SIZE; u++) {
		for (int v = 0; v < MAP_SIZE; v++) {
			if (my_board[u][v] == 0 && distance[u][v] * base <= 0){
				for (int i = 0; i < 4; i++) {
					uu = u + tx[i]; vv = v + ty[i];
					if (inside_table(uu, vv) && cell_id[uu][vv] != -1) {
						chambers[chamber_of_id[cell_id[uu][vv]]].battle_front = true;
					}
				}
			}
		}
	}
	//finding longest 
	int longest_length, current_length;
	int cham_id;
	//find leaf chamber
	for (int i = 1; i <= cham_num; i++) {
		if (chambers[i].active) {
			chambers[chamber_of_id[cell_id[chambers[i].entrance.x][chambers[i].entrance.y]]].is_leaf = false;
		}
	}
	if (chambers[0].battle_front) {
		chambers[0].is_leaf = true;
	}

	//
	bool is_on_leaf = true;
	Position prev_entrance = Position(0,0);
	//longest path length
	longest_length = 0;
	for (int i = 0; i <= cham_num; i++) {
		if (chambers[i].active && chambers[i].is_leaf) {
			current_length = 0;
			cham_id = i;
			is_on_leaf = true;
			// if battle front
			if (chambers[i].battle_front) {
				current_length = min(chambers[cham_id].even_num, chambers[cham_id].odd_num) * 2;
				if (even_pos(chambers[cham_id].entrance) && chambers[cham_id].odd_num > chambers[cham_id].even_num) {
					current_length++;
				}
				if (!even_pos(chambers[cham_id].entrance) && chambers[cham_id].odd_num < chambers[cham_id].even_num) {
					current_length++;
				}
				current_length += min_way[chambers[cham_id].entrance.x][chambers[cham_id].entrance.y];
				longest_length = max(longest_length, current_length);
				continue;
			}
			is_on_leaf = true;
			//if not battle front
			
			while (true) {
				
				current_length += local_longest_estimation(is_on_leaf, chambers[cham_id].even_num, chambers[cham_id].odd_num, chambers[cham_id].entrance, prev_entrance);
				//go father
				prev_entrance = chambers[cham_id].entrance;
				is_on_leaf = false;
				if (cham_id == 0) break;
				cham_id = chamber_of_id[cell_id[chambers[cham_id].entrance.x][chambers[cham_id].entrance.y]];
			}
			longest_length = max(longest_length, current_length);
		}
	}
	return longest_length;

}

//this case is when it's too close and 2 must compete for the line between
bool special_case_too_close(int my_board[][MAP_SIZE], Position myPos, Position enemyPos) {
	if (abs(myPos.x - enemyPos.x) == 1 && abs(myPos.y - enemyPos.y) == 1) {
		if (my_board[myPos.x][enemyPos.y] == 1 || my_board[enemyPos.x][myPos.y] == 1) {
			return true;
		}
	}
	return false;
}

int distance_2side[MAP_SIZE][MAP_SIZE];
//heuristic for Chamber tree alogrotim 
int chamber_tree_heuristic(int my_board[][MAP_SIZE], Position myPos, Position enemyPos, int my_distance[][MAP_SIZE], int enemy_distance[][MAP_SIZE],bool max_turn){
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++) {
			my_distance[i][j] = MAX_DIS;
			enemy_distance[i][j] = MAX_DIS;
		}
	}
	my_distance[myPos.x][myPos.y] = 0;
	enemy_distance[enemyPos.x][enemyPos.y] = 0;
	bfs(my_board, my_distance, myPos);
	bfs(my_board, enemy_distance, enemyPos);

	bool special1 = special_case_too_close(my_board, myPos, enemyPos);

	//real calculation
	int res = 0;
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++) {
			distance_2side[i][j] = enemy_distance[i][j] - my_distance[i][j];
			if (distance_2side[i][j] == 0 && special1) {
				if (max_turn) {
					distance_2side[i][j]+=1;
				}
				else {
					distance_2side[i][j]-=1;
				}
			}
		}
	}
	// if my_distance[i][j]>0 ==> (i,j) is closer to myPos than enemyPos
	int my_longest_path = isolated_chamber_value(my_board, myPos, distance_2side, 1,my_distance);
	int enemy_longest_path = isolated_chamber_value(my_board, enemyPos, distance_2side, -1,enemy_distance);
	return my_longest_path - enemy_longest_path;
}

//child count
int child_count(int my_board[][MAP_SIZE], Position pos) {
	int res = 0;
	int x, y;
	for (int i = 0; i < 4; i++) {
		x = pos.x + tx[i];
		y = pos.y + ty[i];
		if (inside_table(x, y) && my_board[x][y] == 0) {
			res++;
		}
	}
	return res;
}

//recursive calculation 
//this one calculate longest path in
int fake_distance[MAP_SIZE][MAP_SIZE] = { { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};
//global value
int longest_res = 0;
//child count
void longest_dfs(int my_board[][MAP_SIZE], Position pos, int depth, int current_length){
	int temp_res;
	int potential_res;
	Position newPos = Position(0, 0);

	if (child_count(my_board, pos) == 0) {
		longest_res = max(longest_res, current_length);
		return;
	}
	if (depth == 0) {
		temp_res = current_length + isolated_chamber_value(my_board, pos, fake_distance, 1, fake_distance);
		longest_res = max(longest_res, temp_res);
		return;
	}
	potential_res = current_length + isolated_chamber_value(my_board, pos, fake_distance, 1, fake_distance);
	if (potential_res<longest_res) {
		return;
	}

	for (int i = 0; i<4; i++) {
		newPos.x = pos.x + tx[i];
		newPos.y = pos.y + ty[i];
		if (inside_table(newPos.x, newPos.y) && my_board[newPos.x][newPos.y] == 0) {
			my_board[newPos.x][newPos.y] = 1;
			longest_dfs(my_board, newPos, depth - 1, current_length + 1);
			my_board[newPos.x][newPos.y] = 0;
		}
	}

}
//this one calculate longest path in isolated mode
int isolated_recurrence_value(int my_board[][MAP_SIZE], Position pos, int remain_depth) {
	// init longest res
	longest_res = isolated_chamber_value(my_board, pos, fake_distance, 1, fake_distance) / 10 * 8;
	longest_dfs(my_board, pos, remain_depth, 0);
	return longest_res;
}
//end recursive calculation


//heuristic using recursive
int recursive_heuristic(int my_board[][MAP_SIZE], Position myPos, Position enemyPos,int depth_remain) {
	int myPath, enemyPath;
	int remain = depth_remain*2;
	myPath = isolated_recurrence_value(my_board,myPos,remain);
	enemyPath = isolated_recurrence_value(my_board, enemyPos, remain);
	return myPath - enemyPath;
}


//heuristic type 2
int heuristic_type2(int my_board[][MAP_SIZE], Position myPos, Position enemyPos,int depth_remain){
	int value;
	value = recursive_heuristic(my_board,myPos,enemyPos,depth_remain) * 100;
	return value;
}


// heuristic
int heuristic(int my_board[][MAP_SIZE], Position myPos, Position enemyPos, int my_distance[][MAP_SIZE], int enemy_distance[][MAP_SIZE],bool max_turn){
	int value;
	value = chamber_tree_heuristic(my_board, myPos, enemyPos,my_distance,enemy_distance,max_turn) * 100;
	return value;
}



int alpha_beta_prunning(int my_board[][MAP_SIZE], Position myPos, Position enemyPos, int depth, int alpha, int beta, int my_distance[][MAP_SIZE], int enemy_distance[][MAP_SIZE],int game_state) {
	//check whether it is max or min turn 
	bool max_turn = false;
	if ((max_depth - depth) % 2 == 1) {
		max_turn = true;
	}
	//if overtime, break;
	clock_t current_time = clock();
	if ((float(current_time) - float(start_time)) / CLOCKS_PER_SEC > 2.0) {
		timeout = true;
		return 0;
	}
	//
	int value,temp_value;
	//for debugging
	count_function++;

	
	//check if 2 player are isolated or not 
	// if not isolated, return -maxv. if isolated, return the heuristic value.
	/*if (game_state != 3) {
		
	}*/

	
	//this value is reflection of that if the space is even, who go first will be loser.
	int turn_priority = 0;
	if (max_turn) {
		turn_priority = -50; 
	}
	else {
		turn_priority = 50;
	}

	


	//calculate heuristic and save it into heuristic_value --> for using once per function
	int heuristic_value;

	// if it is isolated, calculate using heuristic value and return
	int isolated_value = end_state(my_board, myPos, enemyPos);
	if (isolated_value != -maxv) {
		heuristic_value = heuristic(my_board, myPos, enemyPos, my_distance, enemy_distance, max_turn);
		if (abs(heuristic_value)<=400 ) {
			heuristic_value = heuristic_type2(my_board, myPos, enemyPos, depth);
		}
		return heuristic_value + turn_priority;
	}

	//if it's maximum depth
	if (depth == 0 ) {
		heuristic_value = heuristic(my_board, myPos, enemyPos, my_distance, enemy_distance, max_turn);
		return heuristic_value + turn_priority;
	}


	// max player
	if ((max_depth - depth) % 2 == 1) {
		//the purpose of this is prevent the function from returning -maxv value
		if (child_count(my_board, myPos) == 0) {
			//-1 becuase max player will lost
			return heuristic_value - 50;
		}
		//go deep
		value = -maxv;
		for (int i = 0; i < 4; i++) {
			myPos.x = myPos.x + tx[i];
			myPos.y = myPos.y + ty[i];
			if (inside_table(myPos.x, myPos.y) && my_board[myPos.x][myPos.y] == 0) {
				my_board[myPos.x][myPos.y] = 1;
				temp_value = alpha_beta_prunning(my_board, myPos, enemyPos, depth - 1, alpha, beta, my_distance, enemy_distance, game_state);
				value = max(value,temp_value  );
				my_board[myPos.x][myPos.y] = 0;
				alpha = max(alpha, value);
				if (beta <= alpha) {
					break;
				}
			}
			myPos.x = myPos.x - tx[i];
			myPos.y = myPos.y - ty[i];
		}
		return value;
	}
	// min player
	else {
		// avoid the case that it's pick the lower positive over higher positive
		if (child_count(my_board, enemyPos) == 0) {
			//+ 1 because min player will lose
			return heuristic_value + 50;
		}
		//go deep
		value = maxv;
		for (int i = 0; i < 4; i++) {
			enemyPos.x = enemyPos.x + tx[i];
			enemyPos.y = enemyPos.y + ty[i];
			if (inside_table(enemyPos.x, enemyPos.y) && my_board[enemyPos.x][enemyPos.y] == 0) {
				my_board[enemyPos.x][enemyPos.y] = 1;
				temp_value = alpha_beta_prunning(my_board, myPos, enemyPos, depth - 1, alpha, beta, my_distance, enemy_distance, game_state);
				value = min(value, temp_value);
				my_board[enemyPos.x][enemyPos.y] = 0;
				beta = min(beta, value);
				if (beta <= alpha) {
					break;
				}
			}
			enemyPos.x = enemyPos.x - tx[i];
			enemyPos.y = enemyPos.y - ty[i];
		}
		return value;
	}
}


int possibility_check(int my_board[][MAP_SIZE], Position myPos, Position enemyPos, int priority[], int my_distance[][MAP_SIZE], int enemy_distance[][MAP_SIZE],int game_state) {
	//value -10000 -> 10000
	// -10000 opponent sure win
	// 10000 me sure win
	//this one choose the highest
	
	//debugging
	count_function = 0;
	//count
	int value;
	int opt_value = -maxv-1;  //fixed bug
	int opt_i = -1;
	for (int i = 0; i < 4; i++) {
		myPos.x = myPos.x + tx[i];
		myPos.y = myPos.y + ty[i];
		if (inside_table(myPos.x, myPos.y) && my_board[myPos.x][myPos.y] == 0) {
			my_board[myPos.x][myPos.y] = 1;
			value = alpha_beta_prunning(my_board, myPos, enemyPos, max_depth, -maxv, maxv, my_distance,enemy_distance,game_state) + priority[i];
			//cout << "possible[" << i+1 << "]" <<value<< endl;
			if (value>opt_value) {
				opt_value = value;
				opt_i = i;
			}
			my_board[myPos.x][myPos.y] = 0;
		}
		myPos.x = myPos.x - tx[i];
		myPos.y = myPos.y - ty[i];
	}
	//cout << "max_deph"<< max_depth<< endl;
	//cout << "dem" << count_function << endl;
	//cout << "move taken" << opt_i + 1 << endl;
	return opt_i + 1;
}


//check the state of game 
int game_state_check(int my_board[][MAP_SIZE], Position myPos, Position enemyPos,int priority[]){
	int pri_con = 100;
	AI *p_ai = AI::GetInstance();
	int * board = p_ai->GetBoard();
	int v; 
	int number_of_ocupied = 0;

	int end_state_value = end_state(my_board, myPos, enemyPos);
	if (end_state_value != -maxv) {
		priority[0] = 0; priority[1] = 0;
		priority[2] = 0; priority[3] = 0;
		return 3;
	}

	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++){
			v = board[CONVERT_COORD(i, j)];
			if (board[CONVERT_COORD(myPos.x, myPos.y)] == 1 && (v == 1 || v == 2)) {
				number_of_ocupied++;
			} 
			if (board[CONVERT_COORD(myPos.x, myPos.y)] == 3 && (v == 3 || v == 4)) {
				number_of_ocupied++;
			}
		}
	}
	if (number_of_ocupied <= 7){
		priority[0] = 0; priority[1] = 0;
		priority[2] = 0; priority[3] = 0;
		//set priority when it's far away
		/*if (board[CONVERT_COORD(myPos.x, myPos.y)] == 1){
			if (myPos.x < 4) {
				priority[2] = pri_con;
			}
			if (myPos.y < 4) {
				priority[3] = pri_con;
			}
		}
		else {
			if (myPos.x >= 7) {
				priority[0] = pri_con;
			}
			if (myPos.y >= 7) {
				priority[1] = pri_con;
			}
		}*/
		
		return 1;
	}
	else {
		priority[0] = 0; priority[1] = 0;
		priority[2] = 0; priority[3] = 0;
		return 2;
	}
	
}


int state_3_solution(int my_board[][MAP_SIZE], Position pos){
	int res = -1;
	int opt_length = -1;
	int temp_length = -1;
	Position newPos = pos;
	for (int i = 0; i < 4; i++) {
		newPos.x = pos.x + tx[i];
		newPos.y = pos.y + ty[i];
		if (inside_table(newPos.x, newPos.y) && (my_board[newPos.x][newPos.y] == 0)) {
			my_board[newPos.x][newPos.y] = 1;
			temp_length = isolated_recurrence_value(my_board,newPos,100);
			if (temp_length>opt_length) {
				opt_length = temp_length;
				res = i + 1;
			}
			my_board[newPos.x][newPos.y] = 0;
		}
	}
	cout << "longest path isolated" << opt_length << endl;
	return res;
}


void case_classify(int my_board[][MAP_SIZE], Position myPos, Position enemyPos) {
	// for heuristic use, just pass around until heuristic function use. 
	int my_distance[MAP_SIZE][MAP_SIZE];
	int enemy_distance[MAP_SIZE][MAP_SIZE];

	//analyze and give the case of game status
	// 1 -> warm up ; 2-> real battle; 3 -> space filler
	int priority[4] = { 0, 0, 0, 0 };
	int res = -1;
	int game_state = game_state_check(my_board,myPos,enemyPos,priority);
	cout << "Turn started-----------------------------------------" << endl;
	cout << "state" << game_state << endl;
	cout <<"priority" <<priority[0] << " " << priority[1] << " " << priority[2] << " " << priority[3] << endl;
	//isolated case
	if (game_state == 3) {
		res = state_3_solution(my_board, myPos);
		Game::GetInstance()->AI_Move(res);
		return;
	}
	//if game state is 1, we should give priority to right and down
	max_depth = 10; //global variable for control
	timeout = false;
	start_time = clock();
	int temp_res = -1;
	//mark for starting a turn
	while (true) {
		max_depth++;
		temp_res = possibility_check(my_board, myPos, enemyPos, priority, my_distance, enemy_distance, game_state);
		if (!timeout && max_depth<=100) {
			res = temp_res;
		}
		else {
			break;
		}
	}
	/*else if (game_state == 3){
		res = filler_move(my_board, myPos, enemyPos);
	}*/
	cout << max_depth - 1<<" "<<res<<endl;
	Game::GetInstance()->AI_Move(res);
	//cout << "finish count";
}

void pre_process() {
	// take info
	AI *p_ai = AI::GetInstance();
	int * board = p_ai->GetBoard();	// Access block at (x, y) by using board[CONVERT_COORD(x,y)]

	//creating my_board as status of game. 
	int my_board[MAP_SIZE][MAP_SIZE];
	memset(my_board, 0, sizeof(my_board));
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++) {
			if (p_ai->GetBlock(Position(i, j)) != 0) {
				my_board[i][j] = 1;
			}
		}
	}

	//just write for debugging
	/*ofstream myfile;
	myfile.open("F:/AIcontest/Pack1.5b/Pack/debug_write/bot_debug.txt");
	myfile.close();*/
	//end writting

	// take position
	Position myPos = p_ai->GetMyPosition();
	Position enemyPos = p_ai->GetEnemyPosition();

	//case classify
	case_classify(my_board, myPos, enemyPos);

}
void AI_Update()
{
	AI *p_ai = AI::GetInstance();
	if (p_ai->IsMyTurn())
	{
		//analyzing
		pre_process();
	}
	else
	{
		// Do something while waiting for your opponent
	}
}

////////////////////////////////////////////////////////////
//                DON'T TOUCH THIS PART                   //
////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	srand(clock());
	
#ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        printf("WSAStartup Failed.\n");
        return 1;
    }
#endif

	Game::CreateInstance();
	Game * p_Game = Game::GetInstance();
	
	// Create connection
	if (p_Game->Connect(argc, argv) == -1)
	{
		LOG("Failed to connect to server!\n");
		return -1;
	}

	// Set up function pointer
	AI::GetInstance()->Update = &AI_Update;
	
	p_Game->PollingFromServer();

	Game::DestroyInstance();

#ifdef _WIN32
    WSACleanup();
#endif
	return 0;
}