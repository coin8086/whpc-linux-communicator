#include "NamingClient.h"
#include "NodeManagerConfig.h"
#include "../utils/Logger.h"
#include "../utils/ReaderLock.h"
#include "HttpHelper.h"
#include <stdlib.h>

using namespace web::http;
using namespace web::http::client;
using namespace hpc::core;
using namespace hpc::utils;

std::shared_ptr<NamingClient> NamingClient::instance;

std::string NamingClient::GetServiceLocation(const std::string& serviceName)
{
    std::map<std::string, std::string>::iterator location;

    {
        ReaderLock readerLock(&this->lock);
        location = this->serviceLocations.find(serviceName);
    }

    if (location == this->serviceLocations.end())
    {
        WriterLock writerLock(&this->lock);
        location = this->serviceLocations.find(serviceName);
        if (location == this->serviceLocations.end())
        {
            std::string temp;
            this->RequestForServiceLocation(serviceName, temp);
            this->serviceLocations[serviceName] = temp;
            location = this->serviceLocations.find(serviceName);
        }
    }

    Logger::Debug("ResolveServiceLocation> Resolved serviceLocation {1} for {0}", location->second, serviceName);
    return location->second;
}

void NamingClient::RequestForServiceLocation(const std::string& serviceName, std::string& serviceLocation)
{
    int selected = rand() % this->namingServicesUri.size();
    std::string uri;
    int interval = this->intervalSeconds;

    while (true)
    {
        try
        {
            selected %= this->namingServicesUri.size();
            uri = this->namingServicesUri[selected++] + serviceName;
            Logger::Debug("ResolveServiceLocation> Fetching from {0}", uri);
            http_client client = HttpHelper::GetHttpClient(uri);

            http_request request = HttpHelper::GetHttpRequest(methods::GET);
            http_response response = client.request(request, this->cts.get_token()).get();
            if (response.status_code() == http::status_codes::OK)
            {
                serviceLocation = JsonHelper<std::string>::FromJson(response.extract_json().get());
                Logger::Debug("ResolveServiceLocation> Fetched from {0} response code {1}, location {2}", uri, response.status_code(), serviceLocation);
                return;
            }
            else
            {
                Logger::Debug("ResolveServiceLocation> Fetched from {0} response code {1}", uri, response.status_code());
            }
        }
        catch (const http_exception& httpEx)
        {
            Logger::Warn("ResolveServiceLocation> HttpException occurred when fetching from {0}, ex {1}", uri, httpEx.what());
        }
        catch (const std::exception& ex)
        {
            Logger::Error("ResolveServiceLocation> Exception occurred when fetching from {0}, ex {1}", uri, ex.what());
        }
        catch (...)
        {
            Logger::Error("ResolveServiceLocation> Unknown error occurred when fetching from {0}", uri);
        }

        sleep(interval);
        interval *= 2;
        if (interval > 300) interval = 300;
    }
}
