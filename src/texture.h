#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <vector>

unsigned int loadTexture(const char* path, bool gamma = false);
unsigned int loadCubemap(const std::vector<std::string>& faces);

#endif