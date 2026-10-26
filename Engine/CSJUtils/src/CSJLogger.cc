#include "CSJLogger.h"

#include <mutex>
#include <cstdarg>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

using namespace std;

namespace csjutils {

class CSJLogger {
public:
    static CSJLogger* getLoggerInst() {
        static CSJLogger logger;
        return &logger;
    }

    ~CSJLogger() {
        
    }

    void init(const char* logPath) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_pLogger) {
            return ;
        }

        try {
            auto consoleSink = make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto fileSink = make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logPath,
                1024 * 1025 * 5, // the max size of single log file.
                5,               // the max number of log files.
                true
            );

            vector<spdlog::sink_ptr> sinks = {consoleSink, fileSink};

            m_pLogger = make_shared<spdlog::logger>("CSJLogger", begin(sinks), end(sinks));
            m_pLogger->set_level(spdlog::level::debug);
            m_pLogger->flush_on(spdlog::level::debug);
            spdlog::set_default_logger(m_pLogger);

            m_pLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
        } catch (...) {}
    }

    void uninit() {
        lock_guard<std::mutex> lock(m_mutex);
        spdlog::shutdown();
        m_pLogger.reset();
    }

    void debug(const char* file, int line, const char* fmt, va_list args) {
        log(spdlog::level::level_enum::debug, file, line, fmt, args);
    }

    void info(const char* file, int line, const char* fmt, va_list args) {
        log(spdlog::level::level_enum::info, file, line, fmt, args);
    }

    void warn(const char* file, int line, const char* fmt, va_list args) {
        log(spdlog::level::level_enum::warn, file, line, fmt, args);
    }

    void error(const char* file, int line, const char* fmt, va_list args) {
        log(spdlog::level::level_enum::err, file, line, fmt, args);
    }

protected:
    CSJLogger() = default;

    void log(spdlog::level::level_enum level, const char* file, int line, const char* fmt, va_list args) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!m_pLogger) {
            return ;
        }

        char buf[2048] = {0};
        vsnprintf(buf, sizeof(buf) - 1, fmt, args);

        spdlog::source_loc loc {file, line, __FUNCTION__};
        m_pLogger->log(loc, level, buf);
    }

private:
    std::mutex m_mutex;
    std::shared_ptr<spdlog::logger> m_pLogger;
};
} // namespace csjutils;

void CSJLog_Init(const char* logFile) {
    csjutils::CSJLogger::getLoggerInst()->init(logFile);
}

void CSJLog_Uninit() {
    csjutils::CSJLogger::getLoggerInst()->uninit();
}

void CSJLog_Debug(const char* file, int line, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    csjutils::CSJLogger::getLoggerInst()->debug(file, line, format, ap);
    va_end(ap);
}

void CSJLog_Info(const char* file, int line, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    csjutils::CSJLogger::getLoggerInst()->info(file, line, format, ap);
    va_end(ap);
}

void CSJLog_Warn(const char* file, int line, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    csjutils::CSJLogger::getLoggerInst()->warn(file, line, format, ap);
    va_end(ap);
}

void CSJLog_Error(const char* file, int line, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    csjutils::CSJLogger::getLoggerInst()->error(file, line, format, ap);
    va_end(ap);
}
