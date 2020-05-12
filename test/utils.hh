#pragma once

#include <string>

/* Try a few common locations where test trace files can be found, returning the first one
 * that matches or raising an exception */
std::string try_tracefile_names(const std::string& name);

/* Try a few common locations where test trace files can be found, returning the first one
 * that matches or raising an exception */
std::string try_configfile_names(const std::string& name);
