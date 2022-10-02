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
    stat(fullPath.c_str(), &stats);

    // Format the date in dateBuff[]
    strftime(dateBuff, sizeof(dateBuff), "%d-%b-%Y %H:%M", localtime(&stats.st_mtime));

    // Write the line to a string stream
    ss << "<a href=\"" << node.d_name << (node.d_type == DT_DIR ? "/" : "") << "\">" << node.d_name << (node.d_type == DT_DIR ? "/" : "") << "</a>" << std::setw(70 - strlen(node.d_name) - (node.d_type == DT_DIR ? 1 : 0)) << std::right << dateBuff <<  std::setw(30) << std::right << toString(stats.st_size) << "\n";
    return ss.str();
}

void			Client::_autoIndex(std::string uri, std::string path)
{
    DIR                         *dp;
    struct dirent               *ep;
    std::vector<struct dirent>  nodes;
    std::stringstream           ss;

    dp = opendir(path.c_str());
    if (dp == NULL)
        throw std::runtime_error("Error opening dir for listing");

    // Store each node in a vector because we need to sort them.
    while ((ep = readdir(dp)) != NULL)
        nodes.push_back(*ep);
    closedir(dp);

    // Remove the "." symlink
    nodes.erase(std::remove_if(nodes.begin(), nodes.end(), isCurrentDir)), nodes.end();

    // Sort alphabetically and dir first then files
    std::sort(nodes.begin(), nodes.end(), autoindexSortNodes);

    // Write the head
    ss << "<html>" << "\n";
    ss << "<head>" << "\n";
    ss << "<title>Index of " + uri + "</title>" << "\n";
    ss << "</head>" << "\n";
    ss << "<body>" << "\n";
    ss << "<h1>Index of " + uri + "</h1><hr>" << "\n";
    ss << "<pre>" << "\n";
    _response.appendToBody(ss.str());

    // Loop over each node and write the corresponding line
    for (std::vector<struct dirent>::iterator it = nodes.begin(); it != nodes.end(); it++)
        _response.appendToBody(makeLine(path, *it));
    
    // Close the html tags
    _response.appendToBody("</pre><hr></body></html>");
}
