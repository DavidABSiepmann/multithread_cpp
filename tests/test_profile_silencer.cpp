#include <gtest/gtest.h>
#include "profile_print.h"

class ProfileSilencer : public ::testing::Environment {
public:
    void SetUp() override { ProfilePrinter::get().mute(); }
    void TearDown() override { ProfilePrinter::get().unmute(); }
};

// Register a global environment to mute profile prints during tests
static ::testing::Environment* const profile_silencer_env = ::testing::AddGlobalTestEnvironment(new ProfileSilencer());
