// Build with: g++ server.cpp -o server -lhttplib -pthread -std=c++17
// Install httplib: brew install cpp-httplib (on macOS)

#include "httplib.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <queue>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>

using namespace httplib;

// -------- Thread Pool Simulation --------
struct Metrics {
    double latency_us;
    int throughput;
};

Metrics run_simulation() {
    const int NUM_THREADS = 4;
    const int TOTAL_JOBS = 50000;

    std::queue<int> q;
    std::mutex m;
    std::condition_variable cv;

    std::atomic<int> processed{0};
    std::atomic<bool> done{false};

    // Fill queue
    for (int i = 0; i < TOTAL_JOBS; i++) {
        q.push(i);
    }

    auto start = std::chrono::high_resolution_clock::now();

    auto worker = [&]() {
        while (true) {
            int job;

            {
                std::unique_lock<std::mutex> lock(m);

                if (q.empty()) break;

                job = q.front();
                q.pop();
            }

            // Simulated work
            for (volatile int i = 0; i < 100; i++);

            processed++;
        }
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(worker);
    }

    for (auto &t : threads) t.join();

    auto end = std::chrono::high_resolution_clock::now();

    double total_time_sec =
        std::chrono::duration<double>(end - start).count();

    Metrics mtx;
    mtx.throughput = processed / total_time_sec;
    mtx.latency_us = (total_time_sec * 1e6) / processed;

    return mtx;
}

// -------- MAIN --------
int main() {
    Server svr;

    svr.Get("/", [](const Request&, Response& res) {

        res.set_chunked_content_provider(
            "text/html",
            [](size_t, DataSink &sink) {

                auto send = [&](const std::string& data, int delay = 400) {
                    sink.write(data.c_str(), data.size());
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                };

                send(R"(<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Shubham Kushwaha</title>

<style>
body {
    margin: 0;
    background: #0b0b0c;
    color: #d1d5db;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", monospace;
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh;
}
.container {
    max-width: 800px;
    width: 90%;
}
.terminal {
    background: #111113;
    border-radius: 16px;
    padding: 30px;
    box-shadow: 0 10px 40px rgba(0,0,0,0.6);
}
.line {
    opacity: 0;
    transform: translateY(10px);
    animation: fadeIn 0.8s ease forwards;
}
.highlight { color: #22c55e; }
.metric { color: #60a5fa; }

@keyframes fadeIn {
    to { opacity: 1; transform: translateY(0); }
}
</style>
</head>
<body>
<div class="container">
<div class="terminal">
)");

                // -------- Boot --------
                send("<div class='line'>$ Initializing system...</div>", 600);
                send("<div class='line'>Spawning worker threads</div>", 500);
                send("<div class='line'>Allocating job queue</div>", 500);

                // -------- Run Simulation --------
                send("<div class='line'><br>Running workload...</div>", 600);

                for (int i = 0; i < 3; i++) {
                    Metrics m = run_simulation();

                    std::stringstream ss;
                    ss << "<div class='line metric'>Latency: "
                       << m.latency_us << " µs &nbsp;&nbsp;|&nbsp;&nbsp; Throughput: "
                       << m.throughput << " ops/sec</div>";

                    send(ss.str(), 700);
                }

                // -------- Identity --------
                send("<div class='line'><br><br></div>", 200);
                send("<div class='line highlight'>Shubham Kushwaha</div>", 600);
                send("<div class='line'>C++ Lead Software Engineer</div>", 600);

                send("<div class='line'><br></div>", 200);

                send("<div class='line'>Specializing in:</div>", 400);
                send("<div class='line'>• High-performance systems</div>", 400);
                send("<div class='line'>• Concurrency & threading</div>", 400);
                send("<div class='line'>• Low-latency architecture</div>", 400);

                send("<div class='line'><br></div>", 200);
                send("<div class='line'>System operating at peak efficiency.</div>", 500);

                send(R"(</div></div>

<script>
setTimeout(() => location.reload(), 25000);
</script>

</body>
</html>)");

                sink.done();
                return true;
            }
        );
    });

    std::cout << "🚀 Running at http://0.0.0.0:8080\n";
    svr.listen("0.0.0.0", 8080);
}
