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

#include <assimp/material.h>

namespace gfx 
{

class TextureManager : public ResourceManager<Texture>
{
    public:
        TextureManager(AssetRegistry* assetsManager, QOpenGLExtraFunctions* openGLFunctions );

        void registerElement(const std::filesystem::path& texturePath) override;

        void unloadTexture(const std::string& key);
        void loadTexture(const std::string& key);
        void resetTexture(Texture* texture);

        GLuint getTextureID(const std::string& key);

        void loadDefaultTextures();
        GLuint getDefaultTexture(const aiTextureType& type);

    private:
        QOpenGLExtraFunctions* m_openGLFunctions;

        static constexpr const char* defaultTextureDir          = "defaults";
        static constexpr const char* defaultDiffuseFilename     = "defaultDiffuse.png";
        static constexpr const char* defaultSpecularFilename    = "defaultSpecular.png";
};


}

#endif