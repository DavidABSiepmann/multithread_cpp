#include <gtest/gtest.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>

#ifndef TEST_BIN_PATH
#error TEST_BIN_PATH not defined
#endif

std::string get_bin() { return std::string(TEST_BIN_PATH); }

static std::string mktemp_file(const char *tmpl) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s", tmpl);
    int fd = mkstemp(buf);
    if(fd >= 0) close(fd);
    return std::string(buf);
}

TEST(CLI, WritesFiles) {
    std::string out = mktemp_file("/tmp/results-XXXXXX.csv");
    std::string prof = mktemp_file("/tmp/profile-XXXXXX.csv");

    std::string cmd = get_bin() + " --duration 1 --work-us 0 --warmup 0 --repeats 1 --out " + out + " --profile " + prof;
    int ret = system(cmd.c_str());
    EXPECT_EQ(ret, 0);

    // Check files exist and have headers
    FILE *f = fopen(out.c_str(), "r");
    ASSERT_NE(f, nullptr);
    char line[256];
    ASSERT_TRUE(fgets(line, sizeof(line), f) != nullptr);
    std::string header(line);
    EXPECT_NE(header.find("threads"), std::string::npos);
    fclose(f);

    f = fopen(prof.c_str(), "r");
    ASSERT_NE(f, nullptr);
    ASSERT_TRUE(fgets(line, sizeof(line), f) != nullptr);
    header = std::string(line);
    EXPECT_NE(header.find("thread"), std::string::npos);
    fclose(f);

    // cleanup
    remove(out.c_str());
    remove(prof.c_str());
}
