Milestone 1

Leon Kabue:

Aaron Cheng:

Kevin Zhang:
 - Biomes
 used a bedrock map and a rainfall-temperature map with fBm and Perlin Noise to decide the placement of 10 biomes 
 (ocean, beaches, tundra, plains, desert, taiga, forest, savanna, swamp, rainforest, river)
 added BlockTypes to display some of these biomes in better contrast
 - Terrain height
 use an "erosion" factor to blend a different set of height functions for each biome.
 height functions used "hybrid-multifractal" noise, fBm, and Perlin. 
 beaches create a shoreline by eroding ground terrain above bedrock next to oceans. shoreline also varies in size by fBm noise.
 snow, rock, and dirt terrain lines were also determined using noise functions for variance
 - Rivers
 used as a biome classification in our project. Generated using 2d Perlin worms, carved out during height map generation
 - Trees
 made assets for oak and spruce trees, which have limited randomness. they are spawned in appropriate biomes
 added metadata storage to terrain class to store block updates on uninitialized threads.
 - Multithreading
 terrain generates with multithreads on a chunk by chunk basis. used QThreadPool for threads, std mutexes. chunks are also deloaded from the VBO once 2 chunks outside of render distance
 multithreading exists for BlockTypeWorkers, VBOWorkers, and StructureWorkers, which generate structures.
 - Village
 attempt to generate one in an X by X area, which places a village center
 to generate the village, I first created the road network with a L-system and path constraints to prevent road spaghetti
 then, buildings are placed next to roads by iterating through road segments and probabilistically attempting to place a building
 two buildings so far modelled: library and small house. different buildings have different spawn chances depending on their distance from village center
 this is to simulate districts like public ultilities, commerical, residential, etc.
 
