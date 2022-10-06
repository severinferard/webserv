#include "Client.hpp"

// Function passed to std::sort() to sort the nodes alphabetically, directories first
static bool            autoindexSortNodes(struct dirent &i, struct dirent &j)
{
    if (i.d_type == j.d_type)
        return std::string(i.d_name) < std::string(j.d_name);
    else
        return i.d_type < j.d_type; // DT_REG = 8 & DT_DIR = 4
}

// Function passed to std::remove_if() to remove the "." node from the directory listing
static bool            isCurrentDir(struct dirent &curr)
{
    return std::string(curr.d_name) == ".";
}

// Create one line representing one node with its name, last update date and size
static std::string             makeLine(std::string path, dirent &node)
{
    struct stat         stats;
    std::string         fullPath;
    char                dateBuff[200];
    std::stringstream   ss;

    // Calculate the full path to pass it to stat()
    fullPath = joinPath(path, node.d_name);
    fullPath = "." + fullPath;

    memset(&stats,0,sizeof(stats));
    stat(fullPath.c_str(), &stats);

    // Format the date in dateBuff[]
    strftime(dateBuff, sizeof(dateBuff), "%d-%b-%Y %H:%M", localtime(&stats.st_mtime));

    // Write the line to a string stream
    ss << "<a href=\"" << node.d_name << (node.d_type == DT_DIR ? "/" : "") << "\">" << node.d_name << (node.d_type == DT_DIR ? "/" : "") << "</a>" << std::setw(70 - strlen(node.d_name) - (node.d_type == DT_DIR ? 1 : 0)) << std::right << dateBuff <<  std::setw(30) << std::right << toString(stats.st_size) << "\n";
    return ss.str();
}

void            Client::_setupAutoIndex(std::string uri, std::string path)
{
    std::stringstream           ss;

    _dp = opendir(path.c_str());
    if (_dp == NULL)
        throw std::runtime_error("Error opening dir for listing");

    // Write the head
    ss << "<html>" << "\n";
    ss << "<head>" << "\n";
    ss << "<title>Index of " + uri + "</title>" << "\n";
    ss << "</head>" << "\n";
    ss << "<body>" << "\n";
    ss << "<h1>Index of " + uri + "</h1><hr>" << "\n";
    ss << "<pre>" << "\n";
    _response.appendToBody(ss.str());
    _core->registerFd(dirfd(_dp), POLLIN, this);
    _status = STATUS_WAIT_TO_READ_DIR;

}

void			Client::_onReadyToReadDir()
{
    struct dirent               *ep;
    std::stringstream           ss;
    
    // Store each node in a vector because we need to sort them.
    if ((ep = readdir(_dp)) != NULL)
    {
        _autoindexNodes.push_back(*ep);
        return;
    }
    _core->unregisterFd(dirfd(_dp));
    closedir(_dp);
    // Remove the "." symlink
    _autoindexNodes.erase(std::remove_if(_autoindexNodes.begin(), _autoindexNodes.end(), isCurrentDir)), _autoindexNodes.end();

    // Sort alphabetically and dir first then files
    std::sort(_autoindexNodes.begin(), _autoindexNodes.end(), autoindexSortNodes);

    

    // Loop over each node and write the corresponding line
    for (std::vector<struct dirent>::iterator it = _autoindexNodes.begin(); it != _autoindexNodes.end(); it++)
        _response.appendToBody(makeLine(_request.getUri(), *it));
    
    // Close the html tags
    _response.appendToBody("</pre><hr></body></html>");
    _response.setStatus(HTTP_STATUS_SUCCESS);
    _status = STATUS_WAIT_TO_SEND;
    _core->modifyFd(connection_fd, POLLOUT);
}
