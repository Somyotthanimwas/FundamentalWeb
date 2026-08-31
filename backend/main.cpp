#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <mutex>
#include <cctype>
#include <thread>
#include <chrono>
#include <ctime>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = net::ip::tcp;

struct Stock {
    std::string symbol;
    std::string last;
    std::string percentChange;
    std::string volume;
    std::string value;
    std::string marketCap;
    std::string pe;
    std::string pbv;
    std::string deRatio;
    std::string dps;
    std::string eps;
    std::string roa;
    std::string roe;
    std::string netProfitMargin;
    std::string dividendYield;
    std::string bookValuePerShare;
    std::string listedShare;
};

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (char c : line) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }

    fields.push_back(field);

    for (auto& value : fields) {
        if (!value.empty() && value.back() == '\r') {
            value.pop_back();
        }
    }

    return fields;
}

std::string escapeJson(const std::string& value) {
    std::string result;

    for (char c : value) {
        switch (c) {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
        }
    }

    return result;
}

std::vector<Stock> loadStocks(const std::string& filename) {
    std::ifstream file(filename);

    if (!file) {
        throw std::runtime_error(
            "Cannot open CSV: " + filename
        );
    }

    std::vector<Stock> stocks;
    std::string line;

    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        const auto fields = splitCsvLine(line);

        if (fields.size() != 17) {
            std::cerr
                << "Skipping invalid row: "
                << fields.size()
                << " fields\n";

            continue;
        }

        Stock stock{
            fields[0],
            fields[1],
            fields[2],
            fields[3],
            fields[4],
            fields[5],
            fields[6],
            fields[7],
            fields[8],
            fields[9],
            fields[10],
            fields[11],
            fields[12],
            fields[13],
            fields[14],
            fields[15],
            fields[16]
        };

        stocks.push_back(std::move(stock));
    }

    return stocks;
}

std::string stockToJson(const Stock& s) {
    std::ostringstream json;

    json
        << "{"
        << "\"symbol\":\"" << escapeJson(s.symbol) << "\","
        << "\"last\":\"" << escapeJson(s.last) << "\","
        << "\"percentChange\":\"" << escapeJson(s.percentChange) << "\","
        << "\"volume\":\"" << escapeJson(s.volume) << "\","
        << "\"value\":\"" << escapeJson(s.value) << "\","
        << "\"marketCap\":\"" << escapeJson(s.marketCap) << "\","
        << "\"pe\":\"" << escapeJson(s.pe) << "\","
        << "\"pbv\":\"" << escapeJson(s.pbv) << "\","
        << "\"deRatio\":\"" << escapeJson(s.deRatio) << "\","
        << "\"dps\":\"" << escapeJson(s.dps) << "\","
        << "\"eps\":\"" << escapeJson(s.eps) << "\","
        << "\"roa\":\"" << escapeJson(s.roa) << "\","
        << "\"roe\":\"" << escapeJson(s.roe) << "\","
        << "\"netProfitMargin\":\"" << escapeJson(s.netProfitMargin) << "\","
        << "\"dividendYield\":\"" << escapeJson(s.dividendYield) << "\","
        << "\"bookValuePerShare\":\"" << escapeJson(s.bookValuePerShare) << "\","
        << "\"listedShare\":\"" << escapeJson(s.listedShare) << "\""
        << "}";

    return json.str();
}

std::string stocksToJson(const std::vector<Stock>& stocks) {
    std::ostringstream json;

    json << "[";

    for (std::size_t i = 0; i < stocks.size(); ++i) {
        if (i > 0) {
            json << ",";
        }

        json << stockToJson(stocks[i]);
    }

    json << "]";

    return json.str();
}

http::response<http::string_body> makeResponse(
    const http::request<http::string_body>& request,
    http::status status,
    const std::string& body
) {
    http::response<http::string_body> response{
        status,
        request.version()
    };

    response.set(
        http::field::server,
        "FundamentalWeb-Cpp"
    );

    response.set(
        http::field::content_type,
        "application/json; charset=utf-8"
    );

    response.set(
        http::field::access_control_allow_origin,
        "*"
    );

    response.set(
        http::field::access_control_allow_methods,
        "GET, POST, OPTIONS"
    );

    response.set(
        http::field::access_control_allow_headers,
        "Content-Type"
    );

    response.keep_alive(request.keep_alive());

    response.body() = body;
    response.prepare_payload();

    return response;
}

void reloadIfChanged(
    const std::string& csv,
    std::vector<Stock>& stocks,
    std::filesystem::file_time_type& lastWriteTime,
    std::mutex& dataMutex
) {
    std::error_code error;

    const auto currentWriteTime =
        std::filesystem::last_write_time(csv, error);

    if (error) {
        return;
    }

    if (currentWriteTime == lastWriteTime) {
        return;
    }

    try {
        auto newStocks = loadStocks(csv);

        if (newStocks.empty()) {
            std::cerr
                << "AUTO RELOAD: CSV is empty, keeping old data\n";
            return;
        }

        if (newStocks.size() != 869) {
            std::cerr
                << "AUTO RELOAD WARNING: Expected 869 stocks, got "
                << newStocks.size()
                << "\n";
        }

        {
            std::lock_guard<std::mutex> lock(dataMutex);
            stocks = std::move(newStocks);
            lastWriteTime = currentWriteTime;
        }

        std::cout
            << "AUTO RELOAD: Fundamental V4 CSV updated. Stocks loaded: "
            << stocks.size()
            << "\n";

    } catch (const std::exception& error) {
        std::cerr
            << "AUTO RELOAD FAILED: "
            << error.what()
            << " — keeping old data\n";
    }
}


bool syncCsvFromGitHub(
    const std::string& csv
) {
    const std::string tmp = csv + ".github_tmp";

    const std::string url =
        "https://api.github.com/repos/"
        "Somyotthanimwas/FundamentalWeb/"
        "contents/data/fundamental_v4.csv?ref=main";

    const std::string command =
        "curl -fsSL --max-time 20 "
        "-A 'FundamentalWeb-Cpp' "
        "-H 'Accept: application/vnd.github.raw+json' "
        "'" + url + "' "
        "-o '" + tmp + "'";

    const int result = std::system(command.c_str());

    if (result != 0) {
        std::cerr
            << "GITHUB SYNC: download failed\n";
        std::remove(tmp.c_str());
        return false;
    }

    try {
        auto newStocks = loadStocks(tmp);

        if (newStocks.empty()) {
            std::cerr
                << "GITHUB SYNC: downloaded CSV is empty\n";
            std::remove(tmp.c_str());
            return false;
        }

        if (newStocks.size() != 869) {
            std::cerr
                << "GITHUB SYNC: invalid stock count: "
                << newStocks.size()
                << "\n";
            std::remove(tmp.c_str());
            return false;
        }

        std::error_code error;

        std::filesystem::copy_file(
            tmp,
            csv,
            std::filesystem::copy_options::overwrite_existing,
            error
        );

        std::remove(tmp.c_str());

        if (error) {
            std::cerr
                << "GITHUB SYNC: cannot update local CSV: "
                << error.message()
                << "\n";
            return false;
        }

        std::cout
            << "GITHUB SYNC: CSV updated from GitHub. "
            << "Stocks: "
            << newStocks.size()
            << "\n";

        return true;

    } catch (const std::exception& error) {
        std::cerr
            << "GITHUB SYNC: CSV validation failed: "
            << error.what()
            << "\n";

        std::remove(tmp.c_str());
        return false;
    }
}

void githubSyncLoop(
    const std::string& csv
) {
    for (;;) {
        std::this_thread::sleep_for(
            std::chrono::seconds(30)
        );

        syncCsvFromGitHub(csv);
    }
}

http::response<http::string_body> handleRequest(
    const http::request<http::string_body>& request,
    std::vector<Stock>& stocks,
    const std::string& csv,
    std::filesystem::file_time_type& lastWriteTime,
    std::mutex& dataMutex
) {
    if (request.method() == http::verb::options) {
        return makeResponse(
            request,
            http::status::no_content,
            ""
        );
    }

    if (request.method() == http::verb::post &&
        request.target() == "/api/reload") {

        std::error_code error;

        const auto currentWriteTime =
            std::filesystem::last_write_time(csv, error);

        if (error) {
            return makeResponse(
                request,
                http::status::internal_server_error,
                R"({"status":"error","message":"Cannot read CSV timestamp"})"
            );
        }

        try {
            auto newStocks = loadStocks(csv);

            if (newStocks.empty()) {
                return makeResponse(
                    request,
                    http::status::internal_server_error,
                    R"({"status":"error","message":"CSV is empty"})"
                );
            }

            {
                std::lock_guard<std::mutex> lock(dataMutex);
                stocks = std::move(newStocks);
                lastWriteTime = currentWriteTime;
            }

            std::ostringstream body;

            body
                << "{"
                << "\"status\":\"ok\","
                << "\"stocks\":" << stocks.size()
                << "}";

            return makeResponse(
                request,
                http::status::ok,
                body.str()
            );

        } catch (const std::exception& error) {
            return makeResponse(
                request,
                http::status::internal_server_error,
                std::string(R"({"status":"error","message":")") +
                escapeJson(error.what()) +
                "\"}"
            );
        }
    }

    if (request.method() != http::verb::get) {
        return makeResponse(
            request,
            http::status::method_not_allowed,
            R"({"error":"Method not allowed"})"
        );
    }

    reloadIfChanged(
        csv,
        stocks,
        lastWriteTime,
        dataMutex
    );

    if (request.target() == "/api/health") {
        std::lock_guard<std::mutex> lock(dataMutex);

        std::ostringstream body;

        body
            << "{"
            << "\"status\":\"ok\","
            << "\"service\":\"FundamentalWeb Backend\","
            << "\"stocks\":" << stocks.size()
            << "}";

        return makeResponse(
            request,
            http::status::ok,
            body.str()
        );
    }

    if (request.target() == "/api/stocks") {
        std::lock_guard<std::mutex> lock(dataMutex);

        return makeResponse(
            request,
            http::status::ok,
            stocksToJson(stocks)
        );
    }

    const std::string prefix = "/api/stocks/";

    if (request.target().starts_with(prefix)) {
        std::string symbol =
            std::string(request.target()).substr(prefix.size());

        std::transform(
            symbol.begin(),
            symbol.end(),
            symbol.begin(),
            [](unsigned char c) {
                return static_cast<char>(
                    std::toupper(c)
                );
            }
        );

        std::lock_guard<std::mutex> lock(dataMutex);

        const auto it = std::find_if(
            stocks.begin(),
            stocks.end(),
            [&symbol](const Stock& stock) {
                return stock.symbol == symbol;
            }
        );

        if (it == stocks.end()) {
            return makeResponse(
                request,
                http::status::not_found,
                R"({"error":"Stock not found"})"
            );
        }

        return makeResponse(
            request,
            http::status::ok,
            stockToJson(*it)
        );
    }

    return makeResponse(
        request,
        http::status::not_found,
        R"({"error":"Not found"})"
    );
}

int main() {
    const std::string csv =
        "data/fundamental_v4.csv";

    try {
        std::cout
            << "Loading Fundamental V4 CSV...\n";

        auto stocks = loadStocks(csv);

        std::cout
            << "Stocks loaded: "
            << stocks.size()
            << "\n";

        if (stocks.size() != 869) {
            std::cerr
                << "WARNING: Expected 869 stocks, got "
                << stocks.size()
                << "\n";
        }

        std::error_code fileError;

        auto lastWriteTime =
            std::filesystem::last_write_time(csv, fileError);

        if (fileError) {
            throw std::runtime_error(
                "Cannot read CSV timestamp: " +
                fileError.message()
            );
        }

        std::mutex dataMutex;

        const auto address =
            net::ip::make_address("0.0.0.0");

        const char* envPort = std::getenv("PORT");
        const unsigned short port =
            envPort ? static_cast<unsigned short>(std::stoi(envPort)) : 8080;

        net::io_context io_context{1};

        tcp::acceptor acceptor{
            io_context,
            {address, port}
        };

        std::cout
            << "FundamentalWeb Backend\n"
            << "HTTP server listening on "
            << "0.0.0.0:"
            << port
            << "\n"
            << "Auto Reload: ENABLED\n"
            << "GitHub CSV Sync: ENABLED (30s)\n";

        std::thread githubThread(
            githubSyncLoop,
            csv
        );

        githubThread.detach();

        for (;;) {
            tcp::socket socket{io_context};

            acceptor.accept(socket);

            beast::flat_buffer buffer;

            http::request<http::string_body> request;

            beast::error_code error;

            http::read(
                socket,
                buffer,
                request,
                error
            );

            if (error == http::error::end_of_stream) {
                socket.shutdown(
                    tcp::socket::shutdown_send,
                    error
                );

                continue;
            }

            if (error) {
                std::cerr
                    << "HTTP read error: "
                    << error.message()
                    << "\n";

                continue;
            }

            auto response =
                handleRequest(
                    request,
                    stocks,
                    csv,
                    lastWriteTime,
                    dataMutex
                );

            http::write(
                socket,
                response,
                error
            );

            if (error) {
                std::cerr
                    << "HTTP write error: "
                    << error.message()
                    << "\n";
            }

            socket.shutdown(
                tcp::socket::shutdown_send,
                error
            );
        }

    } catch (const std::exception& error) {
        std::cerr
            << "SERVER ERROR: "
            << error.what()
            << "\n";

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
