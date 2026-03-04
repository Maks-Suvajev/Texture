#include "TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace gfx {

TextureManager::TextureManager(GfxAssetRegistry* assetsManager, QOpenGLExtraFunctions* openGLFunctions)
    : m_openGLFunctions(openGLFunctions),
      m_assetsManager(assetsManager)
{
    registerAllTextures();
}

void TextureManager::registerAllTextures()
{
    for (const auto& texturePath : m_assetsManager->getTexturePaths())
    {
        registerTexture(texturePath);
    }
}

void TextureManager::loadAllTextures()
{
    for (const auto& [key, texture] : m_textures)
    {
        loadTexture(key);
    }
}

void TextureManager::updateTexturePath(std::string path)
{
    std::filesystem::path filePath(path);

    m_assetsManager->updateTextureFolderPath(filePath);
}

std::filesystem::path TextureManager::getCurrentWorkingDirectory()
{
    return m_assetsManager->getTextureFolderPath();
}


// Used for cleaning out textures from the map if file is removed from folder
void TextureManager::deleteTexture(std::string name)
{
    m_textures.erase(name);
}

std::vector<std::string> TextureManager::loadActiveTextureKeys()
{
    std::vector<std::string> keys;

    for (const auto& [key, texture] : m_textures)
    {
        keys.push_back(key);
    }

    return keys;
}

void TextureManager::refreshTextures()
{
    std::vector<std::string> oldKeys = loadActiveTextureKeys();
    std::vector<std::string> newKeys;

    for (const auto& texturePath : m_assetsManager->getTexturePaths())
    {
        std::string name = extractTextureName(texturePath);

        auto iter = std::find(oldKeys.begin(), oldKeys.end(), name);

        if (iter == oldKeys.end())
        {
            registerTexture(texturePath);

        }
        else
        {
            oldKeys.erase(iter);
        }
    }

    for (const auto& name : oldKeys)
    {
        // Keep old keys as long as the file still exists
        if (!std::filesystem::exists(m_textures[name].get()->systemSourcePath))
        {
            #ifdef ENABLE_DEBUG_MESSAGES
                std::cout << "ERROR::TextureManager::refreshTextures::Deleting key because it no longer exists: " << name << std::endl;
            #endif

            deleteTexture(name);
        };
    }
}

const std::unordered_map<std::string, std::unique_ptr<Texture>>& TextureManager::getMap()
{
    return m_textures;
}


std::string TextureManager::extractTextureName(std::filesystem::path texturePath)
{
    return texturePath.filename().string();
}

void TextureManager::printAllTextures()
{
    std::cout << "----------------------------------------------------------------------------" << std::endl;

    std::cout << "| ----- Printing currently available textures and their source paths ----- |" << std::endl;

    for (const auto& [key, item] : m_textures)
    {
        std::cout << "----------------------------------------------------------------------------" << std::endl;
        std::cout << "Key: " << key << std::endl;

        if (item)
        {
            std::cout << "Path: " << item->systemSourcePath.string() << std::endl;
        }
        else
        {
            std::cout << "Path: NULLPTR" << std::endl;
        }        
    }

    std::cout << "----------------------------------------------------------------------------" << std::endl << std::endl;
}

void TextureManager::registerTexture(const std::filesystem::path& texturePath)
{
    const auto key = std::filesystem::canonical(texturePath).generic_string();

    if (m_textures.contains(key))
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "ERROR::Texture already loaded with the key: " << texturePath.string() << std::endl;
        #endif

        return;
    }

    Texture textureData{};
    textureData.systemSourcePath = std::filesystem::canonical(texturePath);
    textureData.name             = extractTextureName(texturePath);
    m_textures[key] = std::make_unique<Texture>(textureData);     
}


void TextureManager::resetTexture(Texture* texture)
{
    m_openGLFunctions->glDeleteTextures(1, &texture->textureID);

    texture->textureID     = INVALID_TEXTURE_ID;
    texture->textureFormat = INVALID_TEXTURE_FORMAT;
    texture->width         = 0;
    texture->height        = 0;
    texture->nrChannels    = 0;
    texture->isLoaded      = false;
}


void TextureManager::unloadTexture(std::string key)
{
    if (!m_textures.contains(key))
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "DEBUG::TextureManager::unloadTexture::Texture not found with the key: " << key << std::endl;
        #endif

        return;
    }

    Texture* texture = m_textures[key].get();
    
    if (!texture->isLoaded)
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "DEBUG::TextureManager::unloadTexture::Texture already unloaded: " << key << std::endl;
        #endif

        return;
    }

    resetTexture(texture);
}


// Using name as hash, user can load the same texture under different names if they want
void TextureManager::loadTexture(std::string key)
{
    if (!m_textures.contains(key))
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "DEBUG::TextureManager::loadTexture::Texture not found with the key: " << key << std::endl;
        #endif

        return;
    }

    Texture* texture = m_textures[key].get();

    if (texture->isLoaded)
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "DEBUG::TextureManager::loadTexture::Textur already loaded: " << key << std::endl;
        #endif

        return; // Assume data is correct if it's loaded
    }

    #ifdef ENABLE_DEBUG_MESSAGES
        std::cout << "DEBUG::TextureManager::loadTexture::Loading texture path: " << texture->systemSourcePath.string() << std::endl;
    #endif

    if (!std::filesystem::exists(texture->systemSourcePath))
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "Error::TextureManager::loadTexture::File does not exist: " << texture->systemSourcePath.string() << std::endl;
        #endif

        texture->loadError = true;
        return;
    }

    m_openGLFunctions->glGenTextures(1, &texture->textureID);
    m_openGLFunctions->glBindTexture(texture->config.textureType, texture->textureID);

    // Set the texture wrapping parameters
    m_openGLFunctions->glTexParameteri(texture->config.textureType, GL_TEXTURE_WRAP_S, texture->config.wrapParam_S);
    m_openGLFunctions->glTexParameteri(texture->config.textureType, GL_TEXTURE_WRAP_T, texture->config.wrapParam_T);

    // Set the texture filtering parameters
    m_openGLFunctions->glTexParameteri(texture->config.textureType, GL_TEXTURE_MIN_FILTER, texture->config.minFilter);
    m_openGLFunctions->glTexParameteri(texture->config.textureType, GL_TEXTURE_MAG_FILTER, texture->config.minFilter);

    // Load image into texture 1 using STB library
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(texture->config.flipOnLoad);

    unsigned char* data = stbi_load(texture->systemSourcePath.string().c_str(), &width, &height, &nrChannels, 0);

    if (!data)
    {
        m_openGLFunctions->glDeleteTextures(1, &texture->textureID);
        stbi_image_free(data);

        texture->isLoaded  = false;
        texture->loadError = true;

        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "ERROR::TextureManager::loadTexture::stbi_load() returned invalid data!" << std::endl;
        #endif
    }

    GLenum internalFormat; // GPU side format 8-bit vs 16-bit pixel precision. TODO: Not sure what I might use this for right now

    switch(nrChannels)
    {
        case 1:
            texture->textureFormat = GL_RED;
            internalFormat = GL_R8;
            break;

        case 2:
            texture->textureFormat = GL_RG;
            internalFormat = GL_RG8;
            break;

        case 3:
            texture->textureFormat = GL_RGB;
            internalFormat = GL_RGB8;
            break;

        case 4:
            texture->textureFormat = GL_RGBA;
            internalFormat = GL_RGBA8;
            break;

        default:
            texture->textureFormat = INVALID_TEXTURE_FORMAT;
            internalFormat = INVALID_TEXTURE_FORMAT;
            break;
    }

    #ifdef ENABLE_DEBUG_MESSAGES
        std::cout << "DEBUG::TextureManager::loadTexture::Number of channels detected = " << nrChannels << std::endl;
    #endif

    m_openGLFunctions->glTexImage2D(texture->config.textureType, 0, internalFormat, width, height, 0, texture->textureFormat, GL_UNSIGNED_BYTE, data);
    m_openGLFunctions->glGenerateMipmap(texture->config.textureType);

    texture->width = width;
    texture->height = height;
    texture->nrChannels = nrChannels;
    texture->isLoaded  = true;
    texture->loadError = false;

    stbi_image_free(data);
}

GLuint TextureManager::getTextureID(std::string key)
{
    auto it = m_textures.find(key);

    // Check if key exists, also check if unique_ptr is valid that it points to
    if (it == m_textures.end() || !it->second) 
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "ERROR::Invalid key given: " << key << std::endl;
        #endif

        return INVALID_TEXTURE_ID;
    }

    #ifdef ENABLE_DEBUG_MESSAGES
        std::cout << "DEBUG::Key: " << key << " Texture ID Found: " << m_textures[key]->textureID << std::endl;
    #endif

    return m_textures[key]->textureID;
}

};