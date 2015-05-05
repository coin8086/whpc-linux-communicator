#ifndef PROCESSTEST_H
#define PROCESSTEST_H

#ifdef DEBUG

namespace hpc
{
    namespace tests
    {
        class ProcessTest
        {
            public:
                ProcessTest();

                static bool SimpleEcho();
                static bool Affinity();
                static bool RemainingProcess();

            protected:
            private:
        };
    }
}

#endif // DEBUG

#endif // PROCESSTEST_H
