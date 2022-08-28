//
// Created by dop on 12/25/21.
//

#ifndef AIDA_COLOR_H
#define AIDA_COLOR_H

#include <cstdint>
#include <string>
#include <string_view>
#include <istream>

#include <boost/lexical_cast.hpp>
#include <fmt/format.h>
#include <cereal/cereal.hpp>

/// Credit : Mike Lundy (SO)
/// For use with boost::lexical_cast
template <typename ElemT>
struct HexTo {
    ElemT value;
    operator ElemT() const {return value;}
    friend std::istream& operator>>(std::istream& in, HexTo& out) {
        in >> std::hex >> out.value;
        return in;
    }
};

namespace m32 {
    struct Color {
    public:
        uint16_t red, green, blue;

        static Color from_hex_rgb(std::string_view hex_color) {
            if (hex_color.starts_with('#'))
                hex_color.remove_prefix(1);

            // strip alpha if present
            if (hex_color.size() == 8)
                hex_color.remove_prefix(2);

            return Color{
                    .red = boost::lexical_cast<HexTo<uint16_t>>(hex_color.substr(0, 2)),
                    .green = boost::lexical_cast<HexTo<uint16_t>>(hex_color.substr(2, 2)),
                    .blue = boost::lexical_cast<HexTo<uint16_t>>(hex_color.substr(4, 2))
            };
        }

        std::string to_hex_rgb() const {
            return fmt::format("#{0:02X}{1:02X}{2:02X}", red, green, blue);
        }

        inline bool operator== (const Color& other) const {
            return (
                    red == other.red &&
                    green == other.green &&
                    blue == other.blue
                    );
        }

        template <typename Archive>
        void serialize(Archive& ar) {
            ar (CEREAL_NVP(red), CEREAL_NVP(green), CEREAL_NVP(blue));
        }
    };
}
#endif //AIDA_COLOR_H
