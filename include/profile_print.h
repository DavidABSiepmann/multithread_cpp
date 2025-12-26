
#ifndef PROFILE_PRINT
#define PROFILE_PRINT

#include <fstream>
#include <mutex>
#include <string>

// Small thread-safe singleton that manages the profile output file stream
class ProfilePrinter {
public:
    static ProfilePrinter& get();

    // Open (or create) a file for appending profile events; returns false on error
    bool open_file(const std::string &path);

    // Set an already-open file stream (for testing)
    void set_stream(std::ofstream &&stream);

    void write_line(const char *name, long long t, int status);

    void start(const char *name);
    void stop(const char *name);

    // Mute/unmute output (useful for unit tests)
    void mute();
    void unmute();

private:
    ProfilePrinter();
    ~ProfilePrinter();

    std::mutex mtx_;
    std::ofstream log_file_;
    bool muted_;
};

// Backwards-compatible inline helpers
inline void write_profile_line(const char *name, long long t, int status) { ProfilePrinter::get().write_line(name, t, status); }
inline void startProfile(const char *name) { ProfilePrinter::get().start(name); }
inline void stopProfile(const char *name) { ProfilePrinter::get().stop(name); }

#endif