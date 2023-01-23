# freecell-solver
A solver for Freecell games in C, using informed and uninformed search algorithms.

***
# File structure
Currently, this repo contains the solver in C and a Solutions folder. A standard layout of 4 open cells, 4 open foundations, and 8 cascades is assumed.

The Solutions folder contains 13 folders, one for each value of N, where N is the number of cards per suit (e.g. N2 contains deals with 2 cards per suit, meaning 8 cards in total), max 13, just like a regular deal. Each folder contains:
 - 4 randomly generated deals (except N1 and N2 which contain 1 and 2 deals respectively, due to their symmetrical nature),
 - A step-by-step solution for each deal (if found within 60 seconds) using BFS, DFS, A* and (greedy) best-first search,
 - A .exe file compiled for that specific value of N (N is a defined constant in the solver). The syntax for running compiled solver is: `<executable> <method> <input-file> <output-file>`, where:
   + `<method> = breadth|depth|best|astar`, 
   + `<input-file>` is a file containing a freecell game, 
   + `<output-file>` is the file where the solution will be written.
 
The solver file contains the entirety of the code necessary for solving deals.

***
# Important heads-up
This is the final version of the project for an AI course assignment. It's been a while since I last checked the code, so it's definitely not optimized and absolutely needs to be cleaned up, but nevertheless works like it's intended as-is. Most comments are also in Greek. I will get to updating the code and translating the comments to English in the near future.

We were also asked for a short report on our findings (differences between the efficiency of the search algorithms, analyze our heuristic function, etc). That file is written entirely in Greek and is not in the repo at this time, but I'm planning to fully translate it to English and upload it here for posterity's sake.
