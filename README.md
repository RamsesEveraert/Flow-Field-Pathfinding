# Flow Field Pathfinding
## Introduction
**Pathfinding**, the process of navigating through an environment from point A to point B, is a fundamental aspect of many applications. This varies from real-time strategy games to simulations involving particle movement. **Traditional pathfinding methods** like A* or Dijkstra's algorithm focus on finding the shortest path for **individual units**, which can be computationally intensive and **less efficient when dealing with large groups**. With this project I want to introduce an exploration of **flow field pathfinding as a more efficient alternative for guiding large groups of units** - in this case, particles. This flow field pathfinding operates on a node grid, directing multiple units simultaneously toward a goal while dynamically avoiding obstacles. This method not only streamlines the movement of large groups but also ensures the shortest possible path is taken, significantly enhancing efficiency in situations where coordinated movement is very important. <br>
![Final Result](https://i.imgur.com/pcSFiar.gif) <br>
### What is a flow field?
**Flow field** or **Vector field** pathfinding is a method used in scenarios like real-time strategy games to direct **large groups** efficiently. It differs from traditional pathfinding methods by creating a **dynamic map that guides all units simultaneously towards a common destination**. This approach significantly **reduces computational load**, as it employs a single calculation for the entire group, rather than individual paths for each unit. 
### Three components of a flow field:
A flow field is made up of 3 big components
 
| Component |
| ------ |
| Heat Map / Cost Field |
| Vector Field | 
| Large group of units to move |
## Heat Map or Cost Field:
### definition:
A heatmap is a **graphical representation of data** where individual values are represented as colors. It's often used in data visualization to show **patterns, variances, or densities in complex data sets**. In this particular case, the heatmap stores the path distance from every tile on the map to the goal.
The path is a calculation of distance between two points only passing through traversable terrain using a **wavefront algorithm**.
> [The wavefront algorithm](https://www.cs.tufts.edu/comp/150IR/labs/wavefront.html), also known as the brushfire algorithm, operates by executing a breadth-first search across a grid. During this process, it records the number of steps required to reach each node.
### Calculation of the Heat Map:

To calculate the heatmap for pathfinding I did the followingsteps:<br>
1. **Initialization:** <br>
By setting the value of each node on the grid to an 'unvisited' state I differentiate between nodes that have been processed and those that haven't.
2. **Pathfinding Setup:** <br>
Choosing a pathfinding algorithm suitable for my needs. For example, **Breadth-First Search (BFS)** is often used for **uniform-cost grids**, while A* is preferred for grids with varying traversal costs. In my case, I wanted the shortest path possible with different terrain types like mud or water.
So I used the A* algorithm.
3. **Loop Over Nodes:** <br>
Iterate through every node in the grid, performing the following sub-steps:<br>
a. **Goal Node:** If the current node is the goal, its cost is set to 0 since it is the destination.<br>
b. **Non-traversable Nodes:** If the current node represents a terrain type that cannot be traversed (like water), assign it a distinct value that indicates it is non-traversable.<br>
c. **Path Calculation:** For all other nodes, use the chosen pathfinding algorithm to find a path to the goal node. The length of this path represents the cost to reach the goal from that node.<br>
d. **Heatmap Update:** Assign the calculated **path length** as the cost value in the heatmap for the current node.<br>
e. **Terrain Costs:** If the node represents difficult terrain (like mud), which is harder to traverse, adjust the cost accordingly by **adding a penalty cost**.<br>

> The shortest path is calculated using the [A* Algorithm](https://www.simplilearn.com/tutorials/artificial-intelligence-tutorial/a-star-algorithm#:~:text=The%20Basic%20Concept%20of%20A*%20Algorithm&text=All%20graphs%20have%20different%20nodes,the%20cost%20of%20that%20route) <br>

![HeatMap Demo](https://i.imgur.com/38GXQhu.gif) <br>
## Vector Field:
After calculating the distance from each node to the destination in the HeatMap, the next step is to identify the optimal path towards the goal. While this could be computed dynamically for each pathfinder in every frame, a more efficient approach is to produce a vector field in advance. This pre-calculated field serves as a reference for the agents or particles at runtime. <br><br>
I calculated the vector field as followed:<br>
1. **HeatMap Calculation:** <br>
Each node on the grid is assigned a value that represents its **distance to the goal**. This value accounts for the traversability of the terrain, with non-traversable nodes marked distinctly to indicate that they cannot be passed.
2. **Vector Field Generation:** <br>
A vector is calculated for each node. This vector points in the **direction of the adjacent node with the lowest heatmap value**, indicating the direction of the shortest path to the goal. If a neighbor node is non-traversable, the vector instead points towards the current node's position.
3. **Normalization:** <br>
Each vector is normalized to ensure **consistency in magnitude**, which is essential for the proper functioning of the pathfinders that will follow these vectors. <br><br>
**Result:** <br>
![Vector Field Demo](https://i.imgur.com/zSSAC0W.gif) <br>
## Moving a large group of units using a Flow Field:
Once the vector field is generated, determining the movement for the units becomes very easy. Assuming the Vector Field functionally retrieves the earlier calculated direction vector, and we have a scalar value: the desired velocity. The velocity for a pathfinder located at tile (x, y) can be computed using the following equation: <br>
```VelocityVector = vector_field(x, y) * desired_velocity;``` <br>
![Final Result](https://i.imgur.com/pcSFiar.gif) <br>
## Usage:
Flow field pathfinding is used when there's need to coordinate the movement of **many** agents or units towards a **common destination**, especially in situations where these agents are **densely packed or need to move as unit**, like in real-time strategy games. The advantage of flow field is its efficiency in **handling large numbers of units with minimal computational resources**, as it calculates a single, dynamic direction map for all units instead of individual paths for each one. This makes it ideal for situations where **quick, synchronized movements of groups** are crucial.
## Conclusion:
While this method offers a simple approach to pathfinder dynamics, it also lays the groundwork for more intricate movement mechanisms.
For instance, integrating principles from ["Understanding Steering Behaviors"](https://code.tutsplus.com/series/understanding-steering-behaviors--gamedev-12732) can enhance the movement dynamics for pathfinders. the computed VelocityVector serves as the 'desired velocity'. Steering behaviors then utilize this velocity as a basis to generate the actual path and speed of the pathfinder at each update cycle.
## sources:
www.gameaipro.com/GameAIPro/GameAIPro_Chapter23_Crowd_Pathfinding_and_Steering_Using_Flow_Field_Tiles.pdf <br>
https://leifnode.com/2013/12/flow-field-pathfinding/ <br>
https://howtorts.github.io/2014/01/04/basic-flow-fields.html <br>
https://williamtingley.tech.blog/2016/12/11/flow-field-pathfinding/ <br>
https://www.redblobgames.com/pathfinding/tower-defense/ <br>
https://code.tutsplus.com/understanding-goal-based-vector-field-pathfinding--gamedev-9007t <br>
