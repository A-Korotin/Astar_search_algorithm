#include <SDL.h>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <stack>
#include <ctime>

using namespace std;

#define WIDTH 1920
#define HEIGHT 1080


#define SEP 0
#define CELL_H 15
#define CELL_W 15


const unsigned short nROWS = (HEIGHT - SEP) / (CELL_H + SEP);
const unsigned short nCOLS = (WIDTH - SEP) / (CELL_W + SEP);


#define START_X 1
#define START_Y 1

#define END_X nCOLS - 2
#define END_Y nROWS - 2


class Grid;
class Cell;
class Node;

void click_manager(Grid& gr);
bool event_manager(SDL_Window* window, SDL_Event& evnt);


class Cell
{
public:

	Cell(unsigned short int r, unsigned short int c)
	{
		this->r = r;
		this->c = c;
	}

	void Set_coord(unsigned short int x, unsigned short int y)
	{
		this->x = x;
		this->y = y;
	}

	SDL_Rect Get_rect()
	{
		SDL_Rect tmp;

		tmp.x = this->x;
		tmp.y = this->y;
		tmp.w = CELL_W;
		tmp.h = CELL_H;

		return tmp;
	}

	bool IsImmutable()
	{
		return this->immutable;
	}

	void select()
	{
		if (!this->immutable)
		{
			this->color[0] = 200;
			this->color[1] = 191;
			this->color[2] = 191;
			this->avalible = false;
		}
	}

	void deselect()
	{
		if (!this->immutable)
		{		
			this->color[0] = 22;
			this->color[1] = 22;
			this->color[2] = 22;
			this->avalible = true;
		}
	}

	void StartStop()
	{
		this->color[0] = 43;
		this->color[1] = 121;
		this->color[2] = 217;
		this->immutable = true;
	}

	void Path()
	{
		this->color[0] = 196;
		this->color[1] = 65;
		this->color[2] = 78;
	}

	void InFrontier()
	{
		this->color[0] = 100;
		this->color[1] = 100;
		this->color[2] = 100;
	}

	void visit()
	{
		visited = true;
	}

	bool is_visited()
	{
		return visited;
	}

	bool Get_state()
	{
		return this->avalible;
	}

	Uint8* Get_color()
	{
		return this->color;
	}

	bool is_avalible()
	{
		return avalible;
	}


private:
	unsigned short r = 0;
	unsigned short c = 0;
	unsigned short x = 0;
	unsigned short y = 0;
	Uint8* color = new Uint8[3]{ 22, 22, 22 };

	bool immutable = false;

	bool avalible = true;

	bool visited = false;

};

class Grid
{
public:

	void init()
	{
		for (int r = 0; r < nROWS; r++)
		{
			vector <Cell> row;
			
			for (int c = 0; c < nCOLS; c++)
				row.push_back(Cell(r, c));
			container.push_back(row);
		}
		container[START_Y][START_X].StartStop();
		container[END_Y][END_X].StartStop();
	}

	void draw(SDL_Renderer* renderer)
	{
		int* pos_x = new int;
		int* pos_y = new int;
		*pos_x = 0;
		*pos_y = 0;

		SDL_SetRenderDrawColor(renderer, 44, 44, 44, 255);

		SDL_RenderClear(renderer);

		for (int i = 0; i < nROWS; i++)
		{
			for (int j = 0; j < nCOLS; j++)
			{
				container[i][j].Set_coord(*pos_x, *pos_y);

				Uint8* color = container[i][j].Get_color();
				SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 255);

				SDL_Rect r = container[i][j].Get_rect();

				SDL_RenderFillRect(renderer, &r);
				*pos_x += CELL_W + SEP;
			}
			*pos_x = 0;
			*pos_y += CELL_H + SEP;
		}
		delete pos_x;
		delete pos_y;
	}

	void set_obst(SDL_Renderer* renderer, SDL_Event& evnt, SDL_Window* win)
	{
		while (!event_manager(win, evnt))
		{
			this->draw(renderer);
			SDL_RenderPresent(renderer);
			click_manager(*this);
		}
	}

	void draw_path(vector<int>& loc, SDL_Renderer* renderer)
	{
		if (!container[loc[0]][loc[1]].IsImmutable())
		{
			container[loc[0]][loc[1]].Path();
			draw(renderer);
			SDL_RenderPresent(renderer);
		}
	}

	void draw_frontier(vector<int>& loc, SDL_Renderer* renderer)
	{
		if (!container[loc[0]][loc[1]].IsImmutable())
		{
			container[loc[0]][loc[1]].InFrontier();
			draw(renderer);
			SDL_RenderPresent(renderer);
		}
	}

	void set_walls()
	{
		for (int i = 0; i < nROWS; i++)
		{
			for (int j = 0; j < nCOLS; j++)
			{
				if (!(i % 2 != 0 && j % 2 != 0))
					container[i][j].select();
			}
		}
	}
	
	vector< vector<int> > get_unvisited()
	{
		vector< vector<int> > locs;
		for (int i = 0; i < nROWS; i++)
		{
			for (int j = 0; j < nCOLS; j++)
			{
				if ((i % 2 != 0 && j % 2 != 0) && !container[i][j].is_visited())
					locs.push_back(vector<int>{i, j});
			}
		}

		return locs;
	}

	void generate_maze(SDL_Renderer* rend, bool verbose)
	{
		srand(time(NULL));
		vector<int> current{ START_Y, START_X };
		container[current[0]][current[1]].visit();
		stack<vector<int>> steck;
		vector<vector<int>> neibours;

		vector<vector<int>> unvisited = this->get_unvisited();

		do
		{
			neibours = unvisited_neibours(*this, current);

			if (neibours.size())
			{
				steck.push(current);
				int rand_index = rand() % neibours.size();
				container[(current[0] + neibours[rand_index][0]) / 2][(current[1] + neibours[rand_index][1]) / 2].deselect();
				unvisited.erase(find(unvisited.begin(), unvisited.end(), neibours[rand_index]));

				if (verbose)
				{
					this->draw(rend);
					SDL_RenderPresent(rend);
				}

				container[neibours[rand_index][0]][neibours[rand_index][1]].visit();

				current = neibours[rand_index];

			}
			else if (!steck.empty())
			{
				current = steck.top();
				steck.pop();
			}
			else
			{
				current = unvisited[rand() % unvisited.size()];
				container[current[0]][current[1]].visit();
				unvisited.erase(find(unvisited.begin(), unvisited.end(), current));
			}


		} while (unvisited.size());

		for (int i = 0; i < nROWS; i++)
		{
			for (int j = 0; j < nCOLS; j++)
			{
				if (!container[i][j].is_avalible() && !(rand() % 40))
				{
					container[i][j].deselect();
					if (verbose)
					{
						this->draw(rend);
						SDL_RenderPresent(rend);
					}
				}
			}
		}
	}

private:

	vector< vector <Cell> > container;
	

	friend void click_manager(Grid& gr);
	friend int astar(Grid& gr, SDL_Renderer* renderer, bool verbose);
	friend Node;
	friend vector<vector<int>> unvisited_neibours(Grid& gr, vector<int>& loc);
	
};

vector<vector<int>> unvisited_neibours(Grid& gr, vector<int>& loc)
{
	vector<vector<int>> locs;

	if (loc[1] + 2 <= nCOLS - 1 && !gr.container[loc[0]][loc[1] + 2].is_visited())
		locs.push_back(vector<int>{ loc[0], loc[1] + 2 });
	if (loc[1] - 2 >= 0 && !gr.container[loc[0]][loc[1] - 2].is_visited())
		locs.push_back(vector<int>{ loc[0], loc[1] - 2 });
	if (loc[0] + 2 <= nROWS - 1 && !gr.container[loc[0] + 2][loc[1]].is_visited())
		locs.push_back(vector<int>{loc[0] + 2, loc[1]});
	if (loc[0] - 2 >= 0 && !gr.container[loc[0] - 2][loc[1]].is_visited())
		locs.push_back(vector<int>{loc[0] - 2, loc[1]});

	return locs;
}

class Node
{
public:
	Node(vector<int>& state, unsigned short cost = 0.0, unsigned short heruistic = 0.0)
	{
		this->state[0] = state[0];
		this->state[1] = state[1];
		this->cost = cost;
		this->heruistic = heruistic;
	}
	
	Node()
	{

	}

	unsigned short Get_cost() const
	{
		return cost;
	}

	vector<int> Get_state() const
	{
		return this->state;
	}

	bool goal_test() const
	{
		return state[0] == END_Y && state[1] == END_X;
	}

	vector<vector<int>> successors(Grid& gr) const
	{
		vector < vector<int> > locs;

		if ((state[0] < (nROWS - 1)) && gr.container[state[0] + 1][state[1]].Get_state())
			locs.push_back(vector<int>{ state[0] + 1, state[1] });
		if ((state[0] - 1 >= 0) && gr.container[state[0]- 1][state[1]].Get_state())
			locs.push_back(vector<int>{ state[0]- 1, state[1] });
		if ((state[1] + 1) < nCOLS && gr.container[state[0]][state[1]+ 1].Get_state())
			locs.push_back(vector<int>{state[0], state[1] + 1});
		if ((state[1] - 1 >= 0) && gr.container[state[0]][state[1] - 1].Get_state())
			locs.push_back(vector<int>{state[0], state[1] - 1});

		return locs;
	}

private:
	vector<int> state{0,0};
	unsigned short cost = 0;
	unsigned short heruistic = 0;

	friend bool operator <(const Node& a, const Node& b);
	friend bool operator >(const Node& a, const Node& b);
	friend bool operator >=(const Node& a, const Node& b);
	friend bool operator <=(const Node& a, const Node& b);
};

unsigned short calcuate_heru(vector<int>& loc)
{
	return abs(loc[0] - END_Y) + abs(loc[1] - END_X);
}

unsigned short calculate_cost(vector<int>& loc)
{
	return abs(loc[0] - START_Y) + abs(loc[1] - START_X);
}

bool goal_test(vector<int>& state)
{
	return state[0] == END_Y && state[1] == END_X;
}

bool operator >(const Node& a, const Node& b)
{
	return (a.cost +a.heruistic) > (b.cost + b.heruistic);
}


void set_path(Grid& gr, map<vector<int>, vector<int>>& ancestors, SDL_Renderer* renderer, vector<int>& start_loc)
{
	vector<vector<int>> path;
	while (ancestors.count(start_loc))
	{
		start_loc = ancestors[start_loc];
		path.push_back(start_loc);
	}

	reverse(path.begin(), path.end());

	for (vector<int> loc : path)
	{
		cout << loc[0] << '\t' << loc[1] << endl;
		gr.draw_path(loc, renderer);
	}
	cout << "Path length: " << path.size() << endl;
}

int astar(Grid& gr, SDL_Renderer* renderer, bool verbose)
{
	priority_queue<Node, vector<Node>, greater<Node>> frontier;
	vector<int> start = {START_Y, START_X};
	frontier.push(Node(start, 0, calcuate_heru(start)));
	vector<Node> deletedNodes;

	map<vector<int>, vector<int>> ancestors;

	map<vector<int>, double> explored;

	explored[vector<int>{START_Y, START_X}] = 0.0;

	Node curr_node;

	while ( !frontier.empty() )
	{
		curr_node = frontier.top();
		vector<int> curr_state = curr_node.Get_state();
		frontier.pop();

		if (curr_node.goal_test())
		{
			set_path(gr, ancestors, renderer, curr_state);
			return 1;
		}

		for (vector<int> successor : curr_node.successors(gr))
		{
			unsigned short new_cost = curr_node.Get_cost() + 1;

			if (!(explored.count(successor)) || (explored[successor] > new_cost))
			{
				explored[successor] = new_cost;

				if (!(ancestors.count(successor)) || (calculate_cost(ancestors[successor]) >= calculate_cost(curr_state)))
					ancestors[successor] = curr_state;

				frontier.push(Node(successor, new_cost, calcuate_heru(successor)));
				if (verbose)
					gr.draw_frontier(successor, renderer);
			}
		}
	}
	return 0;
}

void click_manager(Grid& gr)
{
	int* x_coord = new int;
	int* y_coord = new int;

	SDL_PumpEvents();
	if (SDL_GetMouseState(x_coord, y_coord) & SDL_BUTTON(SDL_BUTTON_LEFT))
		gr.container[*y_coord / (CELL_H + SEP)][*x_coord / (CELL_W + SEP)].select();
	else if (SDL_GetMouseState(x_coord, y_coord) & SDL_BUTTON(SDL_BUTTON_RIGHT))
		gr.container[*y_coord / (CELL_H + SEP)][*x_coord / (CELL_W + SEP)].deselect();

	delete x_coord;
	delete y_coord;
}

bool event_manager(SDL_Window* window, SDL_Event& evnt)
{	
	bool exit = false;

	while (SDL_PollEvent(&evnt))
	{
		switch (evnt.type)
		{
		case SDL_QUIT:
			SDL_DestroyWindow(window);
			SDL_Quit();
			break;
		case SDL_KEYDOWN:
			exit = true;
			return exit;
		}
	}
	return exit;
}

int main(int argc, char* argv[])
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		cout << "Init Failed: " << SDL_GetError() << endl;

	bool verbose;
	bool generate_maze;

	cout << "Verbose (1 or 0): ";
	cin >> verbose;
	cout << endl;

	cout << "Generate maze? (1 or 0): ";
	cin >> generate_maze;
	cout << endl;

	SDL_Window* win = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	SDL_SetRenderDrawColor(rend, 22, 22, 22, 255);

	SDL_RenderClear(rend);

	SDL_SetRenderDrawColor(rend, 200, 191, 191, 255);

	Grid gr;
	gr.init();
	SDL_Event evnt;	
	gr.draw(rend);
	SDL_RenderPresent(rend);

	if (generate_maze)
	{
		gr.set_walls();
		gr.generate_maze(rend, verbose);
		gr.draw(rend);
		SDL_RenderPresent(rend);
	}
	else
	{
		gr.set_obst(rend, evnt, win);
	}

	if (astar(gr, rend, verbose))
	{
		cout << "Path found" << endl;
		SDL_Delay(5000);
	}
	else cout << "No path found" << endl;

	SDL_Quit();

	return 0;
}
