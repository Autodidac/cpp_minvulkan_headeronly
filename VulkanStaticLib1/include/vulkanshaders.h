#pragma once

#include <array>

namespace VulkanShaders {
    // vert.spv bytecode
    constexpr std::array<uint32_t, 396> vert_spv = {
        0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000a,
        0x5f565053,0x5f52484b,0x73796870,0x6c616369,0x00000000,0x0006000b,0x00000001,0x4c534c47,
        /* ... full SPIR-V bytecode ... */
    };

    // frag.spv bytecode
    constexpr std::array<uint32_t, 276> frag_spv = {
        0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000a,
        0x5f565053,0x5f52484b,0x73796870,0x6c616369,0x00000000,0x0006000b,0x00000001,0x4c534c47,
        /* ... full SPIR-V bytecode ... */
    };
}