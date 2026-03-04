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
#include "GfxAssetRegistry.h"

namespace gfx 
{

class TextureManager
{
    public:
        TextureManager(GfxAssetRegistry* assetsManager, QOpenGLExtraFunctions* openGLFunctions );
        void registerTexture(const std::filesystem::path& texturePath);
        void registerAllTextures();
        void loadTexture(std::string key);
        void unloadTexture(std::string key);
        void resetTexture(Texture* texture);
        void deleteTexture(std::string key);
        void loadAllTextures();
        void refreshTextures();
        void updateTexturePath(std::string path);
        std::vector<std::string> loadActiveTextureKeys();

        std::string extractTextureName(std::filesystem::path texturePath);
        std::filesystem::path getCurrentWorkingDirectory();
        GLuint getTextureID(std::string key);
        void printAllTextures();
        const std::unordered_map<std::string, std::unique_ptr<Texture>>& getMap();

    private:
        std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
        GfxAssetRegistry*                                         m_assetsManager;
        QOpenGLExtraFunctions*                                    m_openGLFunctions;


};


}

#endif