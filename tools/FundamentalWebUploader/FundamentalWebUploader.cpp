#include <curl/curl.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
const char* kCsvPath =
    "C:\\Program Files\\FundamentalUpdater_rev5\\Data\\Fundamental\\fundamental_v4.csv";

const char* kUploadUrl =
    "https://fundamentalweb-backend.onrender.com/api/upload-csv";

std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open CSV: " + path);
    }

    std::ostringstream content;
    content << file.rdbuf();

    const std::string csv = content.str();
    if (csv.empty()) {
        throw std::runtime_error("CSV is empty: " + path);
    }

    return csv;
}

struct CurlGuard {
    CurlGuard() {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
            throw std::runtime_error("curl_global_init failed");
        }
    }

    ~CurlGuard() {
        curl_global_cleanup();
    }
};

size_t writeResponse(char* ptr, size_t size, size_t nmemb, void* userdata) {
    const size_t bytes = size * nmemb;
    static_cast<std::string*>(userdata)->append(ptr, bytes);
    return bytes;
}
}

int main() {
    try {
        const char* token = std::getenv("FUNDAMENTALWEB_UPLOAD_TOKEN");
        if (!token || std::string(token).empty()) {
            throw std::runtime_error(
                "FUNDAMENTALWEB_UPLOAD_TOKEN is not set");
        }

        const std::string csv = readFile(kCsvPath);

        CurlGuard curlGuard;

        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("curl_easy_init failed");
        }

        std::string response;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers,
            "Content-Type: text/csv; charset=utf-8");

        const std::string tokenHeader =
            std::string("X-Upload-Token: ") + token;
        headers = curl_slist_append(headers, tokenHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, kUploadUrl);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, csv.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE,
                         static_cast<curl_off_t>(csv.size()));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        const CURLcode result = curl_easy_perform(curl);

        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (result != CURLE_OK) {
            throw std::runtime_error(
                std::string("Upload failed: ") +
                curl_easy_strerror(result));
        }

        std::cout << "HTTP: " << httpCode << "\n";
        std::cout << "Response: " << response << "\n";

        if (httpCode < 200 || httpCode >= 300) {
            throw std::runtime_error(
                "Backend rejected CSV upload");
        }

        std::cout << "FundamentalWeb upload: OK\n";
        return EXIT_SUCCESS;

    } catch (const std::exception& error) {
        std::cerr << "FundamentalWeb upload: FAILED\n";
        std::cerr << error.what() << "\n";
        return EXIT_FAILURE;
    }
}
