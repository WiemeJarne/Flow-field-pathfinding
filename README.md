# Flow-field-pathfinding
# Calculating the flow field
Calculating a flow field involves 3 stages: <br>
### 1. Create cost field
The cost field is all of the obstacles on the grid like walls or difficult to travers terain like water, mud...
Each cell on the grid has a value between 1 and 255. By default all the cells have a value of 1. If a value is higher then 1 is should be avoided. If a cell has a value of 255 the it is not walkable and therefor has to be avoided. For example a wall should have a value of 255 because you can't walk through walls.
### 2. Generate an integration field

### 3. Generate the flow field
# sources:
https://www.redblobgames.com/pathfinding/tower-defense/ <br>
https://www.aaai.org/Papers/AIIDE/2008/AIIDE08-031.pdf <br>
https://leifnode.com/2013/12/flow-field-pathfinding/ <br>
https://howtorts.github.io/2014/01/04/basic-flow-fields.html <br>
