#include <cpptypr.hpp>

int main()
{
    {
        cpptypr::Logger log(cpptypr::LogLevel::Info, true);
        log.log(cpptypr::LogLevel::Info, "smoke test");
    }

    {
        cpptypr::Logger log1(cpptypr::LogLevel::Debug, false);
        cpptypr::Logger log2(std::move(log1));
        cpptypr::Logger log3(cpptypr::LogLevel::Error, true);
        log3 = std::move(log2);
    }

    return 0;
}
