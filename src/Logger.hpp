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
            va_list args;
            va_start(args, format);

            std::cout << getTimestamp() << " ";
            if (level >= _verbosity) {
                switch (level) {
                case DebugP: std::cout <<" \x1b[1;36m[DEBUG]\x1b[0m  "; break;
                case InfoP: std::cout  << " \x1b[1;34m[INFO]\x1b[0m   "; break;
                case WarnP: std::cout  << " \x1b[1;33m[WARN]\x1b[0m    "; break;
                case ErrorP: std::cout << " \x1b[1;31m[ERROR]\x1b[0m  "; break;
                case FatalP: std::cout << " \x1b[1;31m[FATAL]\x1b[0m  "; break;
                }
            }
            vprintf(format, args);
            std::cout << "\n";
            va_end(args);
        }

        static void LogClient(LogLevel level, int fd, const char* format, va_list args) {
            std::cout << getTimestamp() << " ";
            std::cout << std::setw(4) << std::right << fd << " ";
            if (level >= _verbosity) {
                switch (level) {
                case DebugP: std::cout <<" \x1b[1;36m[DEBUG]\x1b[0m  "; break;
                case InfoP: std::cout  << " \x1b[1;34m[INFO]\x1b[0m   "; break;
                case WarnP: std::cout  << " \x1b[1;33m[WARN]\x1b[0m    "; break;
                case ErrorP: std::cout << " \x1b[1;31m[ERROR]\x1b[0m  "; break;
                case FatalP: std::cout << " \x1b[1;31m[FATAL]\x1b[0m  "; break;
                }
            }
            vprintf(format, args);
            std::cout << "\n";
        }
};

#endif /* LOGGER_HPP */
