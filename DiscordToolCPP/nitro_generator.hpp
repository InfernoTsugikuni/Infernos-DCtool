#ifndef NITRO_GENERATOR_HPP
#define NITRO_GENERATOR_HPP

#include <string>

namespace NitroGenerator {

    void initialize();
    void cleanup();
    std::string generateNitroCode();
    void checkNitroCode(const std::string& save_file);
    void runChecks(int thread_count, const std::string& save_file);

}

#endif // NITRO_GENERATOR_HPP
