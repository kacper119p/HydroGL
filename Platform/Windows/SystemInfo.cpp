#pragma comment(lib, "dxgi.lib")

#include "SystemInfo.h"

#include <Windows.h>

#include <array>
#include <dxgi1_6.h>
#include <intrin.h>
#include <iostream>
#include <psapi.h>
#include <sysinfoapi.h>


#include "Debug.h"

namespace Platform::Detail
{
    namespace
    {
        IDXGIFactory6* DxgiFactory = nullptr;
        IDXGIAdapter4* DxgiAdapter = nullptr;
    }
}

namespace Platform
{
    void SystemInfo::Initialize()
    {
#if NDEBUG
        (void) CreateDXGIFactory1(__uuidof(IDXGIFactory4), reinterpret_cast<void**>(&Detail::DxgiFactory));
        (void) Detail::DxgiFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                               __uuidof(IDXGIAdapter4),
                                                               reinterpret_cast<void**>(&Detail::DxgiAdapter));
#else
        CHECK(SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory4), reinterpret_cast<void**>(&Detail::DxgiFactory))));
        CHECK(SUCCEEDED(Detail::DxgiFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            __uuidof(IDXGIAdapter4), reinterpret_cast<void**>(&Detail::DxgiAdapter))));
#endif
    }

    void SystemInfo::Release()
    {
        Detail::DxgiAdapter->Release();
        Detail::DxgiFactory->Release();
    }

    std::string SystemInfo::GetCpuInfo()
    {
        std::array<int32_t, 4> integerBuffer = {};
        constexpr size_t sizeofIntegerBuffer = sizeof(int32_t) * integerBuffer.size();

        std::array<char, 64> charBuffer = {};

        constexpr std::array<uint32_t, 3> functionIds = {
            0x80000002, //manufacturer
            0x80000003, //model
            0x80000004 //clockspeed
        };

        std::string cpu;

        for (const uint32_t id : functionIds)
        {
            __cpuid(integerBuffer.data(), static_cast<int32_t>(id));
            std::memcpy(charBuffer.data(), integerBuffer.data(), sizeofIntegerBuffer);
            cpu += std::string(charBuffer.data());
        }

        return cpu;
    }

    MemoryInfo SystemInfo::GetRamInfo()
    {
        PROCESS_MEMORY_COUNTERS_EX processMemoryCounters;
        GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&processMemoryCounters),
                             sizeof(processMemoryCounters));

        uint64_t totalMemory;
        GetPhysicallyInstalledSystemMemory(&totalMemory);

        return MemoryInfo(totalMemory * 1024ull, processMemoryCounters.WorkingSetSize);
    }

    MemoryInfo SystemInfo::GetVRamInfo()
    {
        DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
        DXGI_ADAPTER_DESC3 desc;
        std::cout << "Getting VRAM info\n";
        if (SUCCEEDED(Detail::DxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo))
            && SUCCEEDED(Detail::DxgiAdapter->GetDesc3(&desc)))
        {
            return MemoryInfo(desc.DedicatedVideoMemory, memInfo.CurrentUsage);
        }

        return MemoryInfo{0ull, 0ull};
    }
}
