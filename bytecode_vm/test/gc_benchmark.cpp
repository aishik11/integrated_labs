#include "../src/vm.hpp"
#include <iostream>
#include <chrono>
#include <vector>

void run_benchmark() {
    std::cout << "Starting GC Performance Benchmark..." << std::endl;
    VM vm;
    vm.setVerbose(false);

    // 1. Allocation Phase
    // Allocate 100,000 objects (Pairs)
    // Keep 10% reachable (push to stack), discard 90% (unreachable)
    // Stack size is 10,240, so we can store ~10,000 roots safely.
    size_t total_objects = 100000;
    size_t kept_objects = 0;
    
    auto start_alloc = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < total_objects; ++i) {
        Object* obj = vm.new_pair(nullptr, nullptr);
        if (i % 10 == 0) {
            vm.register_stack.push((long)obj, true);
            kept_objects++;
        }
    }
    
    auto end_alloc = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> alloc_ms = end_alloc - start_alloc;
    
    std::cout << "Allocation Complete." << std::endl;
    std::cout << "  Total Allocated: " << total_objects << std::endl;
    std::cout << "  Reachable (Roots): " << kept_objects << std::endl;
    std::cout << "  Garbage (to free): " << (total_objects - kept_objects) << std::endl;
    std::cout << "  Allocation Time: " << alloc_ms.count() << " ms" << std::endl;
    
    // 2. Garbage Collection Phase
    std::cout << "Triggering GC..." << std::endl;
    
    size_t objects_before = vm.num_objects;
    
    auto start_gc = std::chrono::high_resolution_clock::now();
    vm.gc();
    auto end_gc = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> gc_ms = end_gc - start_gc;
    
    size_t objects_after = vm.num_objects;
    size_t freed = objects_before - objects_after;

    std::cout << "GC Complete." << std::endl;
    std::cout << "  Time Taken: " << gc_ms.count() << " ms" << std::endl;
    std::cout << "  Objects Freed: " << freed << std::endl;
    std::cout << "  Objects Surviving: " << objects_after << std::endl;
    
    if (freed == (total_objects - kept_objects)) {
        std::cout << "[SUCCESS] Correct number of objects freed." << std::endl;
    } else {
        std::cout << "[FAILURE] Mismatch in freed objects." << std::endl;
    }
}

int main() {
    try {
        run_benchmark();
    } catch (const std::exception& e) {
        std::cerr << "Benchmark Failed: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
