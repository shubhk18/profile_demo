#include "httplib.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <queue>
#include <vector>
#include <mutex>
#include <atomic>
#include <cstdlib>

using namespace httplib;

// -------- Thread Metrics --------
struct ThreadMetric {
    int thread_id;
    double latency_us;
    int throughput;
};

// -------- Simulation --------
std::vector<ThreadMetric> run_simulation() {
    const int NUM_THREADS = 4;
    const int TOTAL_JOBS = 15000;

    std::queue<int> q;
    std::mutex m;

    for (int i = 0; i < TOTAL_JOBS; i++) {
        q.push(i);
    }

    std::vector<ThreadMetric> results(NUM_THREADS);

    auto worker = [&](int id) {
        int local_processed = 0;

        auto start = std::chrono::high_resolution_clock::now();

        while (true) {
            int job;
            {
                std::lock_guard<std::mutex> lock(m);
                if (q.empty()) break;
                job = q.front();
                q.pop();
            }

            for (volatile int i = 0; i < 100; i++);
            local_processed++;
        }

        auto end = std::chrono::high_resolution_clock::now();

        double total_time =
            std::chrono::duration<double>(end - start).count();

        ThreadMetric tm;
        tm.thread_id = id;
        tm.throughput = local_processed / total_time;
        tm.latency_us = (total_time * 1e6) / std::max(1, local_processed);

        results[id] = tm;
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(worker, i);
    }

    for (auto &t : threads) t.join();

    return results;
}

// -------- MAIN --------
int main() {
    Server svr;

    svr.Get("/", [](const Request&, Response& res) {

        res.set_chunked_content_provider(
            "text/html",
            [](size_t, DataSink &sink) {

                // Slightly slower base delay
                auto send = [&](const std::string& data, int delay = 260) {
                    sink.write(data.c_str(), data.size());
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                };

                send(R"(<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>shubham@system</title>

<style>
body {
    margin: 0;
    background: #0d1117;
    color: #33ff99;
    font-family: "Courier New", monospace;
    padding: 20px;
    font-size: 14px;
}

.terminal {
    max-width: 900px;
    margin: auto;
    white-space: pre-wrap;
}

.prompt { color: #58a6ff; }
.metric { color: #f2cc60; }
.name { color: #ffffff; font-weight: bold; }

.note { color: #8b949e; }

.cursor {
    display: inline-block;
    width: 8px;
    height: 14px;
    background: #33ff99;
    margin-left: 4px;
    animation: blink 1s infinite;
}

@keyframes blink {
    50% { opacity: 0; }
}
</style>
</head>

<body>
<div class="terminal">
)");

                // -------- Boot --------
                send("<span class='prompt'>shubham@system:~$</span> boot\n", 350);
                send("Initializing system...\n", 260);
                send("Spawning worker threads...\n", 260);
                send("Allocating job queue...\n\n", 300);

                auto metrics = run_simulation();

                send("<span class='prompt'>shubham@system:~$</span> run workload\n\n", 350);

                // -------- Processing --------
                send("Processing requests", 350);
                for (int i = 0; i < 3; i++) {
                    send(".", 500);
                }
                send("\n\n", 300);

                // -------- Per-thread Metrics --------
                for (auto &m : metrics) {

                    double jitter = (rand() % 40 - 20) / 100.0;

                    if (rand() % 10 == 0) {
                        jitter += (rand() % 50) / 100.0;
                    }

                    double latency = m.latency_us + jitter;
                    int throughput = m.throughput - (rand() % 50000);

                    std::stringstream ss;

                    ss << "[thread-" << m.thread_id << "] "
                       << "<span class='metric'>Latency:</span> "
                       << latency << " µs   |   "
                       << "<span class='metric'>Throughput:</span> "
                       << throughput << " ops/sec\n";

                    send(ss.str(), 300 + rand() % 150);
                }

                send("\n");

                // -------- Identity --------
                send("<span class='prompt'>shubham@system:~$</span> whoami\n", 300);
                send("<span class='name'>Shubham Kushwaha</span>\n", 350);
                send("C++ Lead Software Engineer\n\n", 300);

                // -------- Skills --------
                send("<span class='prompt'>shubham@system:~$</span> skills\n", 250);
                send("- High-performance systems\n", 220);
                send("- Concurrency & multithreading\n", 220);
                send("- Low-latency architecture\n", 220);
                send("- Linux internals\n\n", 250);

                // -------- Status --------
                send("<span class='prompt'>shubham@system:~$</span> status\n", 250);
                send("System running at peak efficiency.\n", 250);

                // -------- Footer Note --------
                send("\n", 200);
                send("--------------------------------------------------\n", 180);
                send("<span class='note'>Note: This entire interface is built using C++</span>\n", 260);
                send("<span class='note'>including rendering, simulation and backend logic</span>\n", 260);
                send("--------------------------------------------------\n", 220);

                send("<span class='cursor'></span>\n");

                send(R"(
</div>

<script>
setTimeout(() => location.reload(), 10000);
</script>

</body>
</html>
)");

                sink.done();
                return true;
            }
        );
    });

    // -------- Railway Port --------
    int port = 8080;
    if (const char* p = std::getenv("PORT")) {
        port = std::stoi(p);
    }

    std::cout << "🚀 Running on port " << port << std::endl;

    svr.listen("0.0.0.0", port);
}
