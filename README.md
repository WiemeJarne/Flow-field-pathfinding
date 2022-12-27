# Flow-field-pathfinding
# what is a flow field?
A flow field, also known as a vector field, is a representation of the direction of flow or movement at each point in a given space. Flow fields are commonly used to visualize and analyze the movement of fluids, particles, or objects in a system, and can be used to model a variety of phenomena, including the movement of water in a river, the spread of a gas or vapor, and realistic movement of large crowds.
# Calculating the flow field
### 1. Create cost field
The cost field represents various obstacles on the grid, such as walls or difficult terrain like water or mud. Each cell on the grid is assigned a value between 1 and 255, with a default value of 1. Higher values indicate more difficult or impassible terrain, with a value of 255 indicating an impassible cell.
### 2. Generate an integration field
1. Set the value of all cells to a large number, such as 1000.
2. Set the value of the destination node to 0 and add it to the open list, which is a list containing the indices of cells that need to be evaluated.
3. While the open list is not empty, retrieve the first node in the list and set it as the current node.
4. Find the neighbors of the current node. If a neighbor is a wall, ignore it.
5. For each of the remaining neighbors, calculate their new cost. If the new cost is lower than the current cost, check if the neighbor is already on the open list. If it is not, add it to the open list and set its cost to the new calculated cost.
### 3. Generate the flow field
Loop over all the cells in the field and determining the cheapest neighbor by looking at the integration field. If a cell is unreachable, it is assigned a zero vector.
# sources:
https://www.redblobgames.com/pathfinding/tower-defense/ <br>
https://www.aaai.org/Papers/AIIDE/2008/AIIDE08-031.pdf <br>
https://leifnode.com/2013/12/flow-field-pathfinding/ <br>
https://howtorts.github.io/2014/01/04/basic-flow-fields.html <br>
