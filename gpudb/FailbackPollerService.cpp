#include "gpudb/GPUdb.hpp"
#include "gpudb/FailbackPollerService.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>

namespace gpudb {

const int FailbackPollerService::DEFAULT_START_DELAY = 0;
const int FailbackPollerService::DEFAULT_POLLING_INTERVAL = 5;


    /// @brief The main method that is run in a background thread
    void FailbackPollerService::polling_task() {
        std::this_thread::sleep_for(std::chrono::seconds(0)); // DEFAULT_START_DELAY
        while (is_running.load()) {
            try {
                if (poll()) {
                    reset_cluster_pointers();
                    stop(); // Stop if polling succeeds
                    break; // break out of the infinite loop
                }
                std::this_thread::sleep_for(std::chrono::seconds(pollingInterval));
            } catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }
    }

    /// @brief The actual `poll` method which is called in
    /// a loop till a successful connection to the primary cluster
    /// can be made
    /// @return true or false
    bool FailbackPollerService::poll() {
        // 1) Liveness ping: is the primary answering at all?
        if (!db.isKineticaRunning(primaryUrl)) {
            return false;
        }

        // Use a DIRECT connection to the primary (failover AND auto-discovery
        // disabled) so we can observe the primary even while it is draining.
        gpudb::GPUdb::Options options;
        options.setDisableAutoDiscovery(true);
        options.setDisableFailover(true);
        options.setUsername(db.getUsername());
        options.setPassword(db.getPassword());

        try {
            gpudb::GPUdb conn(db.getPrimaryURL(), options);

            // 2) Drain check: read ha_status.drained directly from the PRIMARY
            // (via conn), NOT from the main client db (which is on the backup).
            // The ha_status map value is a JSON object like {"drained":"..."}.
            std::map<std::string, std::string> empty_params;
            std::map<std::string, std::string> statusMap =
                conn.showSystemStatus(empty_params).statusMap;

            std::map<std::string, std::string>::const_iterator it =
                statusMap.find("ha_status");
            if (it != statusMap.end()) {
                std::string drained;
                try {
                    boost::property_tree::ptree pt;
                    std::istringstream ss(it->second);
                    boost::property_tree::json_parser::read_json(ss, pt);
                    drained = pt.get<std::string>("drained", "");
                } catch (const std::exception&) {
                    // Malformed ha_status: treat as not-ready and keep polling.
                    return false;
                }
                if (drained == "draining") {
                    return false;   // still catching up
                }
            }

            // 3) Readiness probe: prove the primary can actually serve a query.
            std::vector<std::vector<uint8_t> > noData;
            std::map<std::string, std::string> noOptions;
            conn.executeSql("SELECT 1", 0, 1, "", noData, noOptions);

            std::stringstream ss;
            ss << "Failback to primary cluster at [" << primaryUrl << "] succeeded";
            std::cout << ss.str() << std::endl;
            return true;
        } catch (const gpudb::GPUdbException& e) {
            return false;
        }
    }

    /// @brief Sets the `m_currentUrl` member to 0 once the connection to
    /// the primary cluster is successful
    void FailbackPollerService::reset_cluster_pointers() {
        std::lock_guard<std::mutex> lock(service_mutex);
        this->db.m_currentUrl = 0;
    }


    bool FailbackPollerService::isRunning() {
        return is_running.load();
    }

    void FailbackPollerService::start() {
        std::lock_guard<std::mutex> lock(service_mutex);
        if (is_running.load()) {
            std::cout << "Poller is already running." << std::endl;
            return;
        }

        std::cout << "Starting the poller..." << std::endl;
        is_running.store(true);
        scheduler = std::make_unique<std::thread>(&FailbackPollerService::polling_task, this);
    }

    void FailbackPollerService::stop() {
        std::lock_guard<std::mutex> lock(service_mutex);
        if (!is_running.load()) {
            std::cout << "Poller is already stopped." << std::endl;
            return;
        }

        std::cout << "Stopping the poller..." << std::endl;
        is_running.store(false);
    }

    void FailbackPollerService::restart() {
        std::cout << "Restarting the poller..." << std::endl;
        stop();
        start();
    }

    FailbackPollerService::~FailbackPollerService() {
        stop();
        if (scheduler && scheduler->joinable()) {
            scheduler->join();
        }
    }

}
