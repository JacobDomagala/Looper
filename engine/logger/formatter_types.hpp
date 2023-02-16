#pragma once

#include <fmt/format.h>
#include <glm/glm.hpp>
#include <type_traits>

template < typename T >
struct fmt::formatter< glm::tmat4x4< T > >
{
   constexpr auto
   parse(format_parse_context& ctx)
   {
      return ctx.begin();
   }

   template < typename FormatContext >
   auto
   format(const glm::tmat4x4< T >& mat, FormatContext& ctx) const
   {
      return fmt::format_to(
         ctx.out(),
         std::is_same< T, float >::value
            ? "\n[{:.1f}, {:.1f}, {:.1f}, {:.1f}]\n[{:.1f}, {:.1f}, {:.1f}, {:.1f}]\n[{:.1f}, "
              "{:.1f}, {:.1f}, {:.1f}]\n[{:.1f}, {:.1f}, {:.1f}, {:.1f}]"
            : "\n[{0:d}, {0:d}, {0:d}, {0:d}]\n[{0:d}, {0:d}, {0:d}, {0:d}]\n[{0:d}, {0:d}, {0:d}, "
              "{0:d}]\n[{0:d}, {0:d}, {0:d}, {0:d}]",
         mat[0][0], mat[0][1], mat[0][2], mat[0][3], mat[1][0], mat[1][1], mat[1][2], mat[1][3],
         mat[2][0], mat[2][1], mat[2][2], mat[2][3], mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
   }
};

template < typename T >
struct fmt::formatter< glm::tvec4< T > >
{
   constexpr auto
   parse(format_parse_context& ctx)
   {
      return ctx.begin();
   }

   template < typename FormatContext >
   auto
   format(const glm::tvec4< T >& vec, FormatContext& ctx) const
   {
      return fmt::format_to(ctx.out(),
                            std::is_same< T, float >::value ? "({:.1f}, {:.1f}, {:.1f}, {:.1f})"
                                                            : "({0:d}, {0:d}, {0:d}, {0:d})",
                            vec.x, vec.y, vec.z, vec.y);
   }
};

template < typename T >
struct fmt::formatter< glm::tvec3< T > >
{
   constexpr auto
   parse(format_parse_context& ctx)
   {
      return ctx.begin();
   }

   template < typename FormatContext >
   auto
   format(const glm::tvec3< T >& vec, FormatContext& ctx) const
   {
      return fmt::format_to(ctx.out(),
                            std::is_same< T, float >::value ? "({:.1f}, {:.1f}, {:.1f})"
                                                            : "({0:d}, {0:d}, {0:d})",
                            vec.x, vec.y, vec.z);
   }
};

template < typename T >
struct fmt::formatter< glm::tvec2< T > >
{
   constexpr auto
   parse(format_parse_context& ctx)
   {
      return ctx.begin();
   }

   template < typename FormatContext >
   auto
   format(const glm::tvec2< T >& vec, FormatContext& ctx) const
   {
      return fmt::format_to(ctx.out(),
                            std::is_same< T, float >::value ? "({:.1f}, {:.1f})" : "({0:d}, {0:d})",
                            vec.x, vec.y);
   }
};