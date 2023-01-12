# Flow-field-pathfinding
![ezgif com-gif-maker](https://user-images.githubusercontent.com/94701784/212093987-fd758d3a-2b85-4e6e-9a6a-f6be5c2a7892.gif)<br>
(the pink circles are agents)
## What is a flow field?
A flow field, also known as a vector field, is a representation of the direction of flow or movement at each point in a given space. 
## When to use a flow field
Flow fields are a way to visualize and analyze the movement of substances or objects in a system. They can be used to model a range of phenomena, such as the flow of water in a river or the spread of a gas. Flow fields can be helpful in pathfinding when the environment is constantly changing and the movement of agents needs to be flexible to these changes. They can also be useful when trying to optimize the movement of agents for a specific goal, such as minimizing energy usage or maximizing speed.
## Calculating the flow field
### 1. Create cost field
The cost field represents various obstacles on the grid, such as walls or difficult terrain like water or mud. Each cell on the grid is assigned a value between 1 and 255, with a default value of 1. Higher values indicate more difficult or impassible terrain, with a value of 255 indicating an impassible cell.<br>

![Screenshot 2023-01-12 153629](https://user-images.githubusercontent.com/94701784/212095761-25c389e1-6f04-4de0-b625-735549dc8931.png)
### 2. Generate an integration field
1. Set the value of all cells to a large number, such as 1000.
2. Set the value of the destination node to 0 and add it to the open list, which is a list containing the indices of cells that need to be evaluated.
3. While the open list is not empty, retrieve the first node in the list and set it as the current node.
4. Find the neighbors of the current node. If a neighbor is a wall, ignore it.
5. For each of the remaining neighbors, calculate their new cost. If the new cost is lower than the current cost, check if the neighbor is already on the open list. If it is not, add it to the open list and set its cost to the new calculated cost.<br>

![Screenshot 2023-01-12 153629](https://user-images.githubusercontent.com/94701784/212096376-78e60718-e7cf-40b9-98ec-c18b4cd94826.png)
### 3. Generate the flow field
Loop over all the cells in the field and determining the cheapest neighbor by looking at the integration field. If a cell is unreachable, it is assigned a zero vector.<br>

![Screenshot 2023-01-12 153629](https://user-images.githubusercontent.com/94701784/212096826-cdecc9bc-8430-471d-8107-bfb3d5b0b447.png)
## sources:
https://www.redblobgames.com/pathfinding/tower-defense/ <br>
https://leifnode.com/2013/12/flow-field-pathfinding/ <br>
https://howtorts.github.io/2014/01/04/basic-flow-fields.html <br>
