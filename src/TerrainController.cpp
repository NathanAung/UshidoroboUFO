#include "TerrainController.h"


Ogre::Entity* TerrainController::GenerateTerrainMesh(Ogre::SceneManager *sceneMgr, int width, int depth, float scale)
{
    // 1. Seed and Initialize Noise (from your SFML code)
    Noise noise;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    int seed = std::rand();
    noise.seed(seed);

    // Define terrain types
    TerrainType waterTerrain(0.2f, 0.4f, {30, 176, 251}, {40, 255, 255});
    TerrainType sandTerrain(0.4f, 0.5f, {215, 192, 158}, {255, 246, 193}, 0.3f);
    TerrainType grassTerrain(0.5f, 0.7f, {2, 166, 155}, {118, 239, 124});
    TerrainType treesTerrain(0.7f, 0.75f, {22, 181, 141}, {10, 145, 113}, -0.5f);
    TerrainType rockTerrain(0.75f, 1.0f, {205, 133, 63}, {120, 72, 36}, 0.1f);

    std::vector<TerrainType> terrainTypes = {
        waterTerrain, sandTerrain, grassTerrain, treesTerrain, rockTerrain
    };

    // 2. Generate noise map matching the width and depth
    auto noiseMap = TerrainController::generateNoiseMap(width, depth, noise, 125.0f, 4, 0.5f, 2.0f, 0.0f, 0.0f);

    // 3. Create a custom material that tracks vertex colors
    std::string matName = "TerrainMaterial";
    Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
        matName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    material->setLightingEnabled(true);
    material->getTechnique(0)->getPass(0)->setVertexColourTracking(Ogre::TVC_DIFFUSE);

    // 4. Create manual object for the 3D terrain mesh
    Ogre::ManualObject *terrain = sceneMgr->createManualObject("TerrainObject");
    terrain->begin(matName, Ogre::RenderOperation::OT_TRIANGLE_LIST);

    // Adjust this multiplier to make mountains taller or flatter
    float heightMultiplier = 50.0f; 

    // 5. Generate vertices with heights and colors derived from noise
    for (int z = 0; z < depth; ++z)
    {
        for (int x = 0; x < width; ++x)
        {
            float noiseValue = noiseMap[x][z];

            // Use noise value for vertex elevation (Y-axis)
            float vertexHeight = noiseValue * heightMultiplier;

            // Determine terrain color using your SFML terrain type logic
            std::array<int, 3> rgb = {255, 255, 255};
            for (const auto &terrainType : terrainTypes)
            {
                if (noiseValue < terrainType.maxHeight)
                {
                    rgb = TerrainController::getTerrainColor(noiseValue, terrainType);
                    break;
                }
            }

            // Set vertex position
            terrain->position(x * scale, vertexHeight, z * scale);

            // Set vertex color (convert 0-255 int to 0.0f-1.0f float for OGRE)
            terrain->colour(
                static_cast<float>(rgb[0]) / 255.0f,
                static_cast<float>(rgb[1]) / 255.0f,
                static_cast<float>(rgb[2]) / 255.0f
            );

            // Default normal pointing up
            terrain->normal(0.0f, 1.0f, 0.0f);

            // Texture coordinates
            terrain->textureCoord(static_cast<float>(x) / width, static_cast<float>(z) / depth);
        }
    }

    // 6. Create indices for grid triangles
    for (int z = 0; z < depth - 1; ++z)
    {
        for (int x = 0; x < width - 1; ++x)
        {
            int topLeft = z * width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            // First triangle
            terrain->index(topLeft);
            terrain->index(bottomLeft);
            terrain->index(topRight);

            // Second triangle
            terrain->index(topRight);
            terrain->index(bottomLeft);
            terrain->index(bottomRight);
        }
    }

    terrain->end();

    // 7. Convert ManualObject to Mesh, create Entity, and attach to SceneNode
    Ogre::MeshPtr mesh = terrain->convertToMesh("TerrainMesh");
    Ogre::Entity *entity = sceneMgr->createEntity("TerrainEntity", mesh);
    return entity;
}

std::vector<std::vector<float>> TerrainController::generateNoiseMap(
    int mapWidth, int mapHeight,
    Noise &noise, float scale,
    int octaves, float persistence,
    float lacunarity, float offY, float offX)
{
    std::vector<std::vector<float>> noiseMap(mapWidth, std::vector<float>(mapHeight, 0.0f));
    std::vector<std::pair<float, float>> octaveOffsets;

    for (int i = 0; i < octaves; ++i)
    {
        float offsetX = offX;
        float offsetY = offY;
        octaveOffsets.emplace_back(offsetX, offsetY);
    }

    if (scale <= 0.0f)
        scale = 0.0001f;

    float maxNoiseHeight = -INFINITY;
    float minNoiseHeight = INFINITY;

    float halfWidth = mapWidth / 2.0f;
    float halfHeight = mapHeight / 2.0f;

    for (int y = 0; y < mapHeight; ++y)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            float amplitude = 1.0f;
            float frequency = 1.0f;
            float noiseHeight = 0.0f;

            for (int i = 0; i < octaves; ++i)
            {
                float sampleX = (x - halfWidth) / scale * frequency + octaveOffsets[i].first * frequency;
                float sampleY = (y - halfHeight) / scale * frequency - octaveOffsets[i].second * frequency;

                float perlinValue = noise.perlin2(sampleX, sampleY) * 2.0f - 1.0f;
                noiseHeight += perlinValue * amplitude;

                amplitude *= persistence;
                frequency *= lacunarity;
            }

            maxNoiseHeight = std::max(maxNoiseHeight, noiseHeight);
            minNoiseHeight = std::min(minNoiseHeight, noiseHeight);

            noiseMap[x][y] = noiseHeight;
        }
    }

    // Normalize
    for (int y = 0; y < mapHeight; ++y)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            noiseMap[x][y] = Noise::clamp((noiseMap[x][y] - minNoiseHeight) / (maxNoiseHeight - minNoiseHeight), 0.0f, 1.0f);
        }
    }

    return noiseMap;
}

std::array<int, 3> TerrainController::lerpColor(const std::array<int, 3> &colorA,
                                                const std::array<int, 3> &colorB,
                                                float t)
{
    return {
        static_cast<int>(colorA[0] + (colorB[0] - colorA[0]) * t),
        static_cast<int>(colorA[1] + (colorB[1] - colorA[1]) * t),
        static_cast<int>(colorA[2] + (colorB[2] - colorA[2]) * t)};
}

std::array<int, 3> TerrainController::getTerrainColor(float noiseValue, const TerrainType &terrain)
{
    float normalized = Noise::normalize(noiseValue, terrain.maxHeight, terrain.minHeight);
    float t = Noise::clamp(normalized + terrain.lerpAdjustment, 0.0f, 1.0f);
    return lerpColor(terrain.minColor, terrain.maxColor, t);
}   
