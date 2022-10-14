# include "Response.hpp"

//MimeTypes from https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
std::map<std::string, std::string> Response::initMimeTypes(void)
{
    std::map<std::string, std::string> ret;
    ret["aac"] = "audio/ac";
    ret["abw"] = "application/x-abiword";
    ret["arc"] = "application/x-freearc";
    ret["avif"] = "image/avif";
    ret["avi"] = "video/x-msvideo";
    ret["azw"] = "application/vnd.amazon.ebook";
    ret["bin"] = "application/octet-stream";
    ret["bmp"] = "image/bmp";
    ret["bz"] = "application/x-bzip";
    ret["bz2"] = "application/x-bzip2";
    ret["cda"] = "application/x-cdf";
    ret["csh"] = "application/x-csh";
    ret["css"] = "text/css";
    ret["csv"] = "text/csv";
    ret["doc"] = "application/msword";
    ret["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    ret["eot"] = "application/vnd.ms-fontobject";
    ret["epub"] = "application/epub+zip";
    ret["gz"] = "application/gzip";
    ret["gif"] = "image/gif";
    ret["htm"] = "text/html";
    ret["html"] = "text/html";
    ret["ico"] = "image/vnd.microsoft.icon";
    ret["ics"] = "text/calendar";
    ret["jar"] = "application/java-archive";
    ret["jpg"] = "image/jpeg";
    ret["jpeg"] = "image/jpeg";
    ret["js"] = "text/javascript"; 
    ret["json"] = "application/json";
    ret["jsonld"] = "application/ld+json";
    ret["midi"] = "audio/midi";
    ret["mid"] = "audio/midi";
    ret["mjs"] = "text/javascript";
    ret["mp3"] = "audio/mpeg";
    ret["mp4"] = "video/mp4";
    ret["mpeg"] = "video/mpeg";
    ret["mpkg"] = "application/vnd.apple.installer+xml";
    ret["odp"] = "application/vnd.oasis.opendocument.presentation";
    ret["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    ret["odt"] = "application/vnd.oasis.opendocument.text";
    ret["oga"] = "audio/ogg";
    ret["ogv"] = "video/ogg";
    ret["ogx"] = "application/ogg";
    ret["opus"] = "audio/opus";
    ret["otf"] = "font/otf";
    ret["png"] = "image/png";
    ret["pdf"] = "application/pdf";
    ret["php"] = "application/x-httpd-php";
    ret["ppt"] = "application/vnd.ms-powerpoint";
    ret["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    ret["rar"] = "application/vnd.rar";
    ret["rtf"] = "application/rtf";
    ret["sh"] = "application/x-sh";
    ret["svg"] = "image/svg+xml";
    ret["tar"] = "application/x-tar";
    ret["tiff"] = "image/tiff";
    ret["tif"] = "image/tiff";
    ret["ts"] = "video/mp2t";
    ret["ttf"] = "font/ttf";
    ret["txt"] = "text/plain";
    ret["vsd"] = "application/vnd.visio";
    ret["wav"] = "audio/wav";
    ret["weba"] = "audio/webm";
    ret["webm"] = "video/webm";
    ret["webp"] = "image/webp";
    ret["woff"] = "font/woff";
    ret["woff2"] = "font/woff2";
    ret["xhtml"] = "application/xhtml+xml";
    ret["xls"] = "application/vnd.ms-excel";
    ret["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    ret["xml"] = "application/xml";
    ret["xul"] = "application/vnd.mozilla.xul+xml";
    ret["zip"] = "application/zip";
    ret["3gp"] = "video/3gpp";
    ret["3g2"] = "video/3gpp2";
    ret["7z"] = "application/x-7z-compressedi";

    return ret;
}