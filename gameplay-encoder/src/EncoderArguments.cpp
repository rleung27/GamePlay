#include <algorithm>
#include "EncoderArguments.h"
#include "StringUtil.h"

#ifdef WIN32
    #define PATH_MAX    _MAX_PATH
    #define realpath(A,B)    _fullpath(B,A,PATH_MAX)
#endif

EncoderArguments::EncoderArguments(size_t argc, const char** argv) :
    _fontSize(0),
    _parseError(false),
    _fontPreview(false),
    _textOutput(false),
    _daeOutput(false)
{
    if (argc > 1)
    {
        size_t filePathIndex = argc - 1;
        if (argv[filePathIndex])
        {
            _filePath.assign(getRealPath(argv[filePathIndex]));
        }
        
        // read the options
        std::vector<std::string> options;
        for (size_t i = 1; i < filePathIndex; ++i)
        {
            options.push_back(argv[i]);
        }
        
        for (size_t i = 0; i < options.size(); ++i)
        {
            if (options[i][0] == '-')
            {
                readOption(options, &i);
            }
        }
    }
    else
    {
        _parseError = true;
    }
}

EncoderArguments::~EncoderArguments(void)
{
}

const std::string& EncoderArguments::getFilePath() const
{
    return _filePath;
}

const char* EncoderArguments::getFilePathPointer() const
{
    return _filePath.c_str();
}

const std::string& EncoderArguments::getDAEOutputPath() const
{
    return _daeOutputPath;
}

const std::vector<std::string>& EncoderArguments::getGroupAnimationNodeId() const
{
    return _groupAnimationNodeId;
}

const std::vector<std::string>& EncoderArguments::getGroupAnimationAnimationId() const
{
    return _groupAnimationAnimationId;
}

bool EncoderArguments::containsGroupNodeId(const std::string& nodeId) const
{
    return find(_groupAnimationNodeId.begin(), _groupAnimationNodeId.end(), nodeId) != _groupAnimationNodeId.end();
}

const std::string EncoderArguments::getAnimationId(const std::string& nodeId) const
{
    std::vector<std::string>::const_iterator it = find(_groupAnimationNodeId.begin(), _groupAnimationNodeId.end(), nodeId);
    for (size_t i = 0, size = _groupAnimationNodeId.size(); i < size; ++i)
    {
        if (_groupAnimationNodeId[i].compare(nodeId) == 0)
        {
            return _groupAnimationAnimationId[i];
        }
    }
    return "";
}

bool EncoderArguments::parseErrorOccured() const
{
    return _parseError;
}

bool EncoderArguments::fileExists() const
{
    if (_filePath.length() > 0)
    {
        struct stat buf;
        if (stat(_filePath.c_str(), &buf) != -1)
        {
            return true;
        }
    }
    return false;
}

void EncoderArguments::printUsage() const
{
    fprintf(stderr,"Usage: gameplay-encoder [options] <filepath>\n");
    fprintf(stderr,".dae file options:\n");
    fprintf(stderr," -i <id>\tFilter by node ID\n");
    fprintf(stderr," -t\tWrite text/xml\n");
    fprintf(stderr," -groupAnimations <nodeID> <animationID>\tGroup all animation channels targetting the nodes into a new animation\n");
    fprintf(stderr," -dae <filepath>\tOutput optimized DAE\n");
    fprintf(stderr,".ttf file options:\n");
    fprintf(stderr," -s <size of font> -p \n");
    exit(8);
}

bool EncoderArguments::fontPreviewEnabled() const
{
    return _fontPreview;
}

bool EncoderArguments::textOutputEnabled() const
{
    return _textOutput;
}

bool EncoderArguments::DAEOutputEnabled() const
{
    return _daeOutput;
}

const char* EncoderArguments::getNodeId() const
{
    if (_nodeId.length() == 0)
    {
        return NULL;
    }
    return _nodeId.c_str();
}

unsigned int EncoderArguments::getFontSize() const
{
    return _fontSize;
}

EncoderArguments::FileFormat EncoderArguments::getFileFormat() const
{
    if (_filePath.length() < 5)
    {
        return FILEFORMAT_UNKNOWN;
    }
    // Extract the extension
    std::string ext = "";
    size_t pos = _filePath.find_last_of(".");
    if (pos != std::string::npos)
    {
        ext = _filePath.substr(pos + 1);
    }
    
    // Match every supported extension with its format constant
    if (ext.compare("dae") == 0 || ext.compare("DAE") == 0)
    {
        return FILEFORMAT_DAE;
    }
    if (ext.compare("fbx") == 0 || ext.compare("FBX") == 0)
    {
        return FILEFORMAT_FBX;
    }
    if (ext.compare("ttf") == 0 || ext.compare("TTF") == 0)
    {
        return FILEFORMAT_TTF;
    }
    if (ext.compare("gpb") == 0 || ext.compare("GPB") == 0)
    {
        return FILEFORMAT_GPB;
    }

    return FILEFORMAT_UNKNOWN;
}

void EncoderArguments::readOption(const std::vector<std::string>& options, size_t *index)
{
    const std::string& str = options[*index];
    if (str.length() == 0 && str[0] != '-')
    {
        return;
    }
    switch (str[1])
    {
    case 'd':
        if (str.compare("-dae") == 0)
        {
            // read one string, make sure not to go out of bounds
            if ((*index + 1) >= options.size())
            {
                fprintf(stderr, "Error: -dae requires 1 argument.\n");
                _parseError = true;
                return;
            }
            (*index)++;
            _daeOutputPath = options[*index];
            _daeOutput = true;
        }
        break;
    case 'g':
        if (str.compare("-groupAnimations") == 0)
        {
            // read two strings, make sure not to go out of bounds
            if ((*index + 2) >= options.size())
            {
                fprintf(stderr, "Error: -groupAnimations requires 2 arguments.\n");
                _parseError = true;
                return;
            }
            (*index)++;
            _groupAnimationNodeId.push_back(options[*index]);
            (*index)++;
            _groupAnimationAnimationId.push_back(options[*index]);
        }
        break;
    case 'i':
    case 'o':
        // Node ID
        (*index)++;
        if (*index < options.size())
        {
            _nodeId.assign(options[*index]);
        }
        else
        {
            fprintf(stderr, "Error: missing arguemnt for -%c.\n", str[1]);
            _parseError = true;
            return;
        }
    case 'p':
        _fontPreview = true;
        break;
    case 's':
        // Font Size

        // old format was -s##
        if (str.length() > 2)
        {
            char n = str[2];
            if (n > '0' && n <= '9')
            {
                const char* number = str.c_str() + 2;
                _fontSize = atoi(number);
                break;
            }
        }

        (*index)++;
        if (*index < options.size())
        {
            _fontSize = atoi(options[*index].c_str());
        }
        else
        {
            fprintf(stderr, "Error: missing arguemnt for -%c.\n", str[1]);
            _parseError = true;
            return;
        }
        break;
    case 't':
        _textOutput = true;
        break;
    default:
        break;
    }
}

std::string EncoderArguments::getRealPath(const std::string& filepath)
{
    char path[PATH_MAX + 1]; /* not sure about the "+ 1" */
    realpath(filepath.c_str(), path);
    replace_char(path, '\\', '/');
    return std::string(path);
}

void EncoderArguments::replace_char(char* str, char oldChar, char newChar)
{
    for (; *str != '\0'; ++str)
    {
        if (*str == oldChar)
        {
            *str = newChar;
        }
    }
}
