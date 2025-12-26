#include "profile_print.h"
#include <chrono>

using namespace std::chrono;

ProfilePrinter& ProfilePrinter::get()
{
    static ProfilePrinter inst;
    return inst;
}

ProfilePrinter::ProfilePrinter() : muted_(false) {}

ProfilePrinter::~ProfilePrinter()
{
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

bool ProfilePrinter::open_file(const std::string &path)
{
    std::lock_guard<std::mutex> lk(mtx_);
    
    // Close existing file if open
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    // Open file for appending
    log_file_.open(path, std::ios::app);
    if (!log_file_.is_open()) {
        return false;
    }
    
    // Write header if file is empty
    log_file_.seekp(0, std::ios::end);
    if (log_file_.tellp() == 0) {
        log_file_ << "thread,time,status\n";
        log_file_.flush();
    }
    
    return true;
}

void ProfilePrinter::set_stream(std::ofstream &&stream)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (log_file_.is_open()) {
        log_file_.close();
    }
    log_file_ = std::move(stream);
}

void ProfilePrinter::write_line(const char *name, long long t, int status)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (muted_ || !log_file_.is_open()) return;
    
    log_file_ << name << "," << t << "," << status << "\n";
    log_file_.flush();
}

void ProfilePrinter::start(const char *name)
{
    long long t = duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
    write_line(name, t, 0);
    write_line(name, t, 1);
}

void ProfilePrinter::stop(const char *name)
{
    long long t = duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
    write_line(name, t, 1);
    write_line(name, t, 0);
}

void ProfilePrinter::mute()
{
    std::lock_guard<std::mutex> lk(mtx_);
    muted_ = true;
}

void ProfilePrinter::unmute()
{
    std::lock_guard<std::mutex> lk(mtx_);
    muted_ = false;
}
