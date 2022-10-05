#ifndef LOGGER_HPP
#define LOGGER_HPP

# include <iostream> 
# include <time.h>
# include <sys/time.h>
# include <string>
# include <stdarg.h>

enum LogLevel {
    DebugP,
    InfoP,
    WarnP,
    ErrorP,
    FatalP
};

# define LOG(...) Logger::Log(__log_fd, __VA_ARGS__)
# define DEBUG(...) Logger::Log(__log_fd, DebugP, __VA_ARGS__)
# define INFO(...) Logger::Log(__log_fd, InfoP, __VA_ARGS__)
# define WARNING(...) Logger::Log(__log_fd, WarnP, __VA_ARGS__)
# define ERROR(...) Logger::Log(__log_fd, ErrorP, __VA_ARGS__)
# define FATAL(...) Logger::Log(__log_fd, FatalP, __VA_ARGS__)

class Logger
{
    private:
        static LogLevel _verbosity;
        static std::string getTimestamp(void) {
            struct timeval tmnow;
            struct tm *tm;
            char buf[100], usec_buf[50];
            gettimeofday(&tmnow, NULL);
            tm = localtime(&tmnow.tv_sec);
            strftime(buf,100,"%H:%M:%S", tm);
            sprintf(usec_buf,".%dZ",(int)tmnow.tv_usec);
            usec_buf[5] = 0;
            strcat(buf,usec_buf);
            return std::string(buf);
        }
    public:
        static void Log(LogLevel level, const char* format, ...) {
            if (level >= _verbosity) {
                va_list args;
                va_start(args, format);

                std::cout << getTimestamp() << " ";
                switch (level) {
                case DebugP: std::cout << "       [DEBUG] "; break;
                case InfoP: std::cout  << "       [INFO]  "; break;
                case WarnP: std::cout  << "       [WARN]  "; break;
                case ErrorP: std::cout << "       [ERROR] "; break;
                case FatalP: std::cout << "       [FATAL] "; break;
                }
                vprintf(format, args);
                std::cout << "\n";
                va_end(args);
            }
        }

        static void Log(int fd, LogLevel level, const char* format, ...) {
            if (level >= _verbosity) {
                va_list args;
                va_start(args, format);

                std::cout << getTimestamp() << " ";
                std::cout << "CONN" <<std::setw(2) << std::right << fd << " ";
                switch (level) {
                case DebugP: std::cout << "[DEBUG] "; break;
                case InfoP: std::cout  << "[INFO]  "; break;
                case WarnP: std::cout  << "[WARN]  "; break;
                case ErrorP: std::cout << "[ERROR] "; break;
                case FatalP: std::cout << "[FATAL] "; break;
                }
                vprintf(format, args);
                std::cout << "\n";
                va_end(args);
            }
        }
};

#endif /* LOGGER_HPP */
