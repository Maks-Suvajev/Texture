#include "TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace gfx {

TextureManager::TextureManager(AssetRegistry* assetRegistry, QOpenGLExtraFunctions* openGLFunctions)
    : ResourceManager<Texture>(assetRegistry, supportedTextureFileTypes),
      m_openGLFunctions(openGLFunctions)
{
    loadDefaultTextures();
    refreshElements();
}

void TextureManager::loadDefaultTextures()
{
    std::filesystem::path defaultTexturePath = std::filesystem::current_path() / defaultTextureDir;

    std::filesystem::path diffuseFilePath = defaultTexturePath / defaultDiffuseFilename;
    std::filesystem::path specularFilePath = defaultTexturePath / defaultSpecularFilename;

    registerElement(diffuseFilePath);
    registerElement(specularFilePath);

    loadTexture(std::filesystem::canonical(diffuseFilePath).generic_string());
    loadTexture(std::filesystem::canonical(specularFilePath).generic_string());
}

GLuint TextureManager::getDefaultTexture(const aiTextureType& type)
{
    std::filesystem::path defaultTexturePath = std::filesystem::current_path() / defaultTextureDir;

    std::filesystem::path filePath;

    switch(type)
    {
        case aiTextureType_DIFFUSE:
            filePath = defaultTexturePath / defaultDiffuseFilename;
            return getTextureID(std::filesystem::canonical(filePath).generic_string());

        case aiTextureType_SPECULAR:
            filePath = defaultTexturePath / defaultSpecularFilename;
            return getTextureID(std::filesystem::canonical(filePath).generic_string());
        default:
            return 0;
    }
}

void TextureManager::registerElement(const std::filesystem::path& texturePath)
{
    const auto key = std::filesystem::canonical(texturePath).generic_string();

    if (m_elements.contains(key))
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "WARNING::Texture already loaded with the key: " << texturePath.string() << std::endl;
        #endif

        return;
    }

    Texture textureData{};
    textureData.systemSourcePath = std::filesystem::canonical(texturePath);
    textureData.name             = texturePath.filename().string();
    m_elements[key]              = std::make_unique<Texture>(textureData);     
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


void TextureManager::unloadTexture(const std::string& key)
{
    if (!m_elements.contains(key))
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "DEBUG::TextureManager::unloadTexture::Texture not found with the key: " << key << std::endl;
        #endif

        return;
    }

    Texture* texture = m_elements[key].get();
    
    if (!texture->isLoaded)
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "DEBUG::TextureManager::unloadTexture::Texture already unloaded: " << key << std::endl;
        #endif

        return;
    }

    resetTexture(texture);
}

void TextureManager::loadTexture(const std::string& key)
{
    if (!m_elements.contains(key))
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "DEBUG::TextureManager::loadTexture::Texture not found with the key: " << key << std::endl;
        #endif

        return;
    }

    Texture* texture = m_elements[key].get();

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
    m_openGLFunctions->glBindTexture(texture->config.textureTarget, texture->textureID);

    // Set the texture wrapping parameters
    m_openGLFunctions->glTexParameteri(texture->config.textureTarget, GL_TEXTURE_WRAP_S, texture->config.wrapParam_S);
    m_openGLFunctions->glTexParameteri(texture->config.textureTarget, GL_TEXTURE_WRAP_T, texture->config.wrapParam_T);

    // Set the texture filtering parameters
    m_openGLFunctions->glTexParameteri(texture->config.textureTarget, GL_TEXTURE_MIN_FILTER, texture->config.minFilter);
    m_openGLFunctions->glTexParameteri(texture->config.textureTarget, GL_TEXTURE_MAG_FILTER, texture->config.magFilter);
 
    // Load image into texture 1 using STB library
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(texture->config.flipOnLoad);

    unsigned char* data = stbi_load(texture->systemSourcePath.string().c_str(), &width, &height, &nrChannels, 0);

    if (!data)
    {
        m_openGLFunctions->glDeleteTextures(1, &texture->textureID);

        texture->isLoaded  = false;
        texture->loadError = true;

        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "ERROR::TextureManager::loadTexture::stbi_load() returned invalid data!" << std::endl;
        #endif

        return;
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

    m_openGLFunctions->glTexImage2D(texture->config.textureTarget, 0, internalFormat, width, height, 0, texture->textureFormat, GL_UNSIGNED_BYTE, data);
    m_openGLFunctions->glGenerateMipmap(texture->config.textureTarget);

    texture->width = width;
    texture->height = height;
    texture->nrChannels = nrChannels;
    texture->isLoaded  = true;
    texture->loadError = false;

    stbi_image_free(data);
}

GLuint TextureManager::getTextureID(const std::string& key)
{
    auto it = m_elements.find(key);

    // Check if key exists, also check if unique_ptr is valid that it points to
    if (it == m_elements.end() || !it->second) 
    {
        #ifdef ENABLE_DEBUG_MESSAGES
            std::cout << "ERROR::Invalid key given: " << key << std::endl;
        #endif

        return INVALID_TEXTURE_ID;
    }

    #ifdef ENABLE_DEBUG_MESSAGES
        std::cout << "DEBUG::Key: " << key << " Texture ID Found: " << m_elements[key]->textureID << std::endl;
    #endif

    return it->second->textureID;
}



};