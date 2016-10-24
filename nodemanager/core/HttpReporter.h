#ifndef HTTPREPORTER_H
#define HTTPREPORTER_H

#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <functional>

#include "Reporter.h"

namespace hpc
{
    namespace core
    {
        using namespace web;

        class HttpReporter : public Reporter<json::value>
        {
            public:
                HttpReporter(
                    const std::string& reporterName,
                    std::function<std::string()> getUri,
                    int hold,
                    int interval,
                    std::function<json::value()> fetcher)
                : Reporter<json::value>(reporterName, getUri, hold, interval, fetcher)
                {
                }

                virtual ~HttpReporter()
                {
                    this->cts.cancel();
                    this->Stop();
                }

                virtual int Report();

            protected:
            private:
                pplx::cancellation_token_source cts;
        };
    }
}

#endif // HTTPREPORTER_H
