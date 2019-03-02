#pragma once

#include <vector>
#include <thread>
#include <mutex>

#include "../remote_process/IProcess.h"
#include "../remote_process/IAddressableProcess.h"
#include "../injector/injector.hpp"

#include "SearchType.h"

namespace pkn
{

using SeekMemoryTestFunc = bool(uint8_t *, uint64_t);// this is just an example
using RegionFilterFunc = bool(const MemoryRegion &region);// this is just an example

enum class SeekMemoryRegionSource
{
    ReadOnly,
    ReadWrite,
    ReadExecute,
    ReadWriteExecute
};

template <bool find_all,
    size_t padding,
    int align,
    class TestFunc>
    void thread_seek_memory(
        TestFunc test_func,
        std::mutex &input_mutex,
        Inputs &inputs,
        std::mutex &output_mutex,
        Outputs &outputs,
        std::atomic<size_t> &number_to_seek
    )
{
    auto &process = SingletonInjector<IProcessReader>::get();
    Outputs my_ouputs;
    std::vector<uint8_t> buffer(0x10240);
    bool finished = false;
    while (!inputs.empty() && !finished)
    {
        Input input;
        {
            std::lock_guard<std::mutex> l(input_mutex);
            if (inputs.empty())
                break;
            input = inputs.back();
            inputs.pop_back();
        }
        try
        {
            size_t size_required = input.size + padding + 8;
            if (buffer.size() < size_required)
                buffer.resize(size_required);
            process.read_unsafe(input.base, input.size, &buffer[0]);
            try
            {
                // try to read padding
                process.read_unsafe(input.base + input.size, padding, (char *)(&buffer[0]) + input.size);
            }
            catch (const std::exception&)
            {
            }
            auto local_start = &buffer[0];
            for (uint8_t *local_address = local_start; local_address < (local_start + input.size) && !finished; local_address += align)
            {
                if constexpr (!find_all)
                    if (number_to_seek == 0) { finished = true; break; }

                try
                {
                    uint64_t remote_address = input.base + (local_address - local_start);
                    if (test_func(local_address, remote_address))
                    {
                        my_ouputs.push_back(remote_address);
                        if constexpr (!find_all)
                            if (--number_to_seek == 0) { finished = true; break; }
                    }
                }
                catch (const std::exception&)
                {
                }
            }
        }
        catch (const std::exception&)
        {
        }
    }
    std::lock_guard<std::mutex> l(output_mutex);
    outputs.insert(outputs.end(), my_ouputs.begin(), my_ouputs.end());
}

template <size_t reserve_size,
    int number_to_seek = -1,
    int offset = 0,
    int align = 8,
    size_t max_offset_to_seek = 0,
    class TestFunc>
    seek_results_t seek_regions(
        const MemoryRegions &regions,
        TestFunc test_func,
        int nthread = 0)
{
    //constexpr size_t padding = reserve_size + align + offset; // supply this to template argument is not supported ???
    std::mutex input_mutex;
    Inputs inputs;

    if (nthread == 0)
    {
        nthread = std::thread::hardware_concurrency() - 1;
        nthread = nthread == 0 ? 1 : nthread;
    }

    std::atomic<size_t> atomic_number_to_seek = number_to_seek;
    std::mutex output_mutex;
    Outputs results;

    // prepare input data for worker thread
    constexpr size_t aligned_limit = (max_offset_to_seek + align - 1) / align * align;
    for (const auto region : regions)
    {
        // split data
        if constexpr (max_offset_to_seek == 0)
        {
            size_t size_per_thread = ((region.size - offset) / nthread + align - 1) / align * align;
            for (size_t i = 0; i < nthread; i++)
            {
                Input input{ region.base + size_per_thread * i + offset, size_per_thread };
                inputs.push_back(input);
            }
        }
        else
        {
            size_t size_per_thread = region.size - offset < aligned_limit - offset ? region.size - offset : aligned_limit - offset;
            Input input{ region.base + offset, size_per_thread };
            inputs.push_back(input);
        }
    }

    // spawn worker thread
    std::vector<std::thread> ths;
    for (int i = 0; i < nthread; i++)
    {
        if (atomic_number_to_seek == 0)
            break;
        ths.emplace_back([&]()
                         {
                             thread_seek_memory<number_to_seek == -1, reserve_size + align + offset, align>(test_func, input_mutex, inputs, output_mutex, results, atomic_number_to_seek);
                         }
        );
    }
    for (auto &th : ths)
    {
        th.join();
    }
    results.erase(std::unique(results.begin(), results.end()), results.end());
    return results;
}

template <size_t reserve_size,
    int number_to_seek = -1,
    bool heap = true, // if heap is false, ignore regions seems located at heap
    SeekMemoryRegionSource source = SeekMemoryRegionSource::ReadWrite,
    int offset = 0,
    int align = 8,
    size_t minimun_region_size = 0x1000,
    size_t max_offset_to_seek = 0,
    class TestFunc,
    class RegionFilterFunc = DefaultRegionFilter>
    seek_results_t seek_memory(TestFunc test_func,
                              int nthread = 0,
                              RegionFilterFunc extra_region_filter = DefaultRegionFilter())
{
    auto &pr = SingletonInjector<IProcessRegions>::get();
    auto &address_type_judger = SingletonInjector<ProcessAddressTypeInfo>::get();

    const MemoryRegions *pregions = nullptr;
    if constexpr (source == SeekMemoryRegionSource::ReadOnly)
        pregions = &pr.readable_regions();
    else if constexpr (source == SeekMemoryRegionSource::ReadWrite)
        pregions = &pr.readwritable_regions();
    else if constexpr (source == SeekMemoryRegionSource::ReadExecute)
        pregions = &pr.readexecutable_regions();
    else if constexpr (source == SeekMemoryRegionSource::ReadWriteExecute)
        pregions = &pr.readwritexecutable_regions();
    else
        pregions = &pr.memory_regions();

    MemoryRegions regions_selected;
    for (const auto &region : *pregions)
    {
        if (minimun_region_size != 0 && region.size < minimun_region_size)
            continue;
        if (!extra_region_filter(region))
            continue;
        if constexpr (!heap)
            if(address_type_judger.seems_heap_address(region.base))
                continue;
        regions_selected.push_back(region);
    }
    return seek_regions<reserve_size, number_to_seek, offset, align, max_offset_to_seek>(regions_selected, test_func, nthread);
}



}
