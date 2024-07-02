#pragma once

namespace linc
{
    struct Target final
    {
        enum class Architecture: unsigned char
        {
            I386, AMD64
        };

        enum class Platform: unsigned char
        {
            Unix, Windows
        };

        Architecture architecture;
        Platform platform;
    };
}