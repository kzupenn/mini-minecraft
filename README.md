# Milestone 1

# Leon Kabue

# Aaron Cheng

# Kevin Zhang
## Biomes
 Used a bedrock map and a rainfall-temperature map with fBm and Perlin Noise to decide the placement of 10 biomes:
 (ocean, beaches, tundra, plains, desert, taiga, forest, savanna, swamp, rainforest, river).
 Also added BlockTypes to display some of these biomes in better contrast.
## Terrain height
 Used an "erosion" factor to blend a different set of height functions for each biome.
 Height functions used a variety of noise including "hybrid-multifractal" noise, fBm, Worley and Perlin. 
 Beaches create a shoreline by eroding ground terrain above bedrock next to oceans. Shoreline also varies in size by fBm noise.
 Snow, rock, and dirt height lines were also determined using noise functions for variance
## Rivers
 Used as a biome classification in our project. Generated using 2d Perlin worms and carved out during height map generation. River depth is still WIP, as alpha-rendering is still not implemented.
## Trees
 Made assets for oak and spruce trees, which have limited randomness. They are spawned in appropriate biomes.
 Added metadata storage to terrain class to store block updates on uninitialized threads to handle overflow into different chunks.
## Multithreading
 Terrain generation utilizes multithreading on a chunk by chunk basis. Used QThreadPool for threads, std mutexes for mutexes. Chunks are also deloaded from the VBO once 2 chunks outside of render distance.
 multithreading exists for BlockTypeWorkers, VBOWorkers, and StructureWorkers, which generate structures.
## Villages
 Attempt to generate one in an X by X area, which places a village center.
 To generate the village, I first created the road network with a L-system and path constraints to prevent road spaghetti.
 Then, buildings are placed next to roads by iterating through road segments and probabilistically attempting to place a building.
 Two building types modelled so far: library and small house. Buildings have different spawn chances depending on their distance from village center to simulate districts like public ultilities, commerical, residential, etc. Buildings will also carve out terrain and build ground platforms if generating in awkward areas.
 