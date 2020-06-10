#pragma once

#include <string>

namespace TraceConverter {

enum class ConvertStatus { Success, Error, AlreadyExists };

/* Convert a text trace file to a binary trace file */
ConvertStatus convert(const std::string& in_fname, const std::string& out_fname,
                      bool force, bool display_progress = true) noexcept;

/* Replace the extension in the given file name with '.bin' */
std::string make_default_outname(const std::string& in_fname);
}  // namespace TraceConverter
