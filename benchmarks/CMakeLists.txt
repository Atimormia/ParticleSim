if (ENABLE_BENCHMARKS)
    add_executable(particlesim_bench 
        bench_layout.cpp
        bench_partitioning.cpp)
    target_link_libraries(particlesim_bench PRIVATE particlesim benchmark::benchmark)

    #if (ENABLE_TRACY)
    #    # Optional Tracy integration when ENABLE_TRACY=ON
    #    target_sources(particlesim_bench PRIVATE
    #        ${tracy_SOURCE_DIR}/public/TracyClient.cpp
    #    )
    #    target_include_directories(particlesim_bench PRIVATE
    #        ${tracy_SOURCE_DIR}/public
    #    )
    #    target_compile_definitions(particlesim_bench PRIVATE TRACY_ENABLE)
    #endif()

endif()