#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

// STL
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <iostream>

//  OpenGL
#include <QOpenGLExtraFunctions>

#include "TextureTypes.h"

// Asset manager
#include "AssetRegistry.h"

#include "ResourceManager.h"

namespace gfx 
{

class TextureManager : public ResourceManager<Texture>
{
    public:
        TextureManager(AssetRegistry* assetsManager, QOpenGLExtraFunctions* openGLFunctions );

        void refreshElements() override;
        void registerTexture(const std::filesystem::path& texturePath);
        void registerAllTextures();

        void unloadTexture(std::string key);
        void loadTexture(std::string key);
        void resetTexture(Texture* texture);

        GLuint getTextureID(std::string key);
        void printAllTextures();

    private:
        QOpenGLExtraFunctions* m_openGLFunctions;
};


}

#endif