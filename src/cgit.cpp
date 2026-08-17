/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#include "cgit.hpp"

#include <bit>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>
#include <string_view>

#include <jau/string_util.hpp>

static void config_cb(cgit::config &cfg, const std::string &name, const std::string &value)
{
	if (name == "log-level")
		cfg.log_level = std::atoi(value.c_str());
	else if (name == "cache-size")
		cfg.cache_size = (size_t)std::strtoul(value.c_str(), nullptr, 10);
	else if (name == "cache-root")
		cfg.cache_root = value;
	else if (name == "cache-root-ttl")
		cfg.cache_root_ttl = std::atoi(value.c_str());
	else if (name == "cache-repo-ttl")
		cfg.cache_repo_ttl = std::atoi(value.c_str());
	else if (name == "cache-scanrc-ttl")
		cfg.cache_scanrc_ttl = std::atoi(value.c_str());
	else if (name == "cache-static-ttl")
		cfg.cache_static_ttl = std::atoi(value.c_str());
	else if (name == "cache-lock-fail")
		cfg.cache_lock_fail = std::atoi(value.c_str());
	else if (name == "cache-lock-retry")
		cfg.cache_lock_retry = std::atoi(value.c_str());
	else if (name == "cache-lock-timeout")
		cfg.cache_lock_timeout = std::atoi(value.c_str()); // ms
	else if (name == "client-io-idle-timeout")
		cfg.client_io_idle_timeout = std::atoi(value.c_str()) * 1000; // s -> ms
	else if (name == "client-io-min-rate")
		cfg.client_io_min_rate = std::atoi(value.c_str());
	else if (name == "cache-dynamic-ttl")
		cfg.cache_dynamic_ttl = std::atoi(value.c_str());
	else if (name == "cache-about-ttl")
		cfg.cache_about_ttl = std::atoi(value.c_str());
	else if (name == "cache-snapshot-ttl")
		cfg.cache_snapshot_ttl = std::atoi(value.c_str());
	else if (name == "local-time")
		cfg.local_time = std::atoi(value.c_str());
    else if (name == "pid-parent-dir")
        cfg.pid_parent_dir = value;
    else if (name == "cache-max-files")
        cfg.cache_max_files = (size_t)std::strtoul(value.c_str(), nullptr, 10);
    else if (name == "cache-min-ttl")
        cfg.cache_min_ttl = std::atoi(value.c_str());
    else if (name == "cache-max-ttl")
        cfg.cache_max_ttl = std::atoi(value.c_str());
}

struct linebuffer_t {
    char *data;
    size_t size;
};
static bool read_config_line(FILE *stream, linebuffer_t &linebuf, std::string &name, std::string &value)
{
    name.clear();
    value.clear();

    ssize_t nread;
    if((nread=::getline(&linebuf.data, &linebuf.size, stream)) <= 0) {
        return false; // EOF
    }

    // Skip comment lines, find name-start
    const char *p = linebuf.data;
    const char * const end = p + nread;
    while(p<end && ::isspace(*p)) { ++p; }
    if( p == end || *p == '#' || *p == ';') {
        return true; // comment or empty line w/ spaces
    }

    std::string_view line(p, end-p);
    auto p_assignment = line.find('=');
    if (   p_assignment == std::string_view::npos // no assignment-delim
        || 0 == p_assignment                      // empty name
        || line.length()-p_assignment==1          // empty value
     )
    {
        return true;
    }
    name.append(line.substr(0, p_assignment));
    jau::trimInPlace(name);
    value.append(line.substr(p_assignment+1, line.length()-p_assignment-1));
    jau::trimInPlace(value);
    return true;
}
static bool parse_configfile(cgit::config &cfg, const std::string &filename)
{
    static int nesting;
    std::string name;
    std::string value;

    /* cancel deeply nested include-commands */
    if (nesting > 8) {
        return false;
    }
    FILE *stream;
    if (!(stream = ::fopen(filename.c_str(), "r"))) {
        return false;
    }
    nesting++;

    linebuffer_t linebuf = { .data=nullptr, .size=0 };
    while (read_config_line(stream, linebuf, name, value)) {
        if (!name.empty() && !value.empty()) {
            config_cb(cfg, name, value);
        }
    }
    if (linebuf.data) {
        free(linebuf.data);
    }
    nesting--;
    ::fclose(stream);
    return true;
}

cgit::config::config(const char *default_filename) noexcept {
    {
        {
            const char *fn = getenv("CGIT_CONFIG");
            if (fn) {
                cgit_config = fn;
            } else {
                cgit_config = default_filename;
            }
        }
        cache_size = 0;
        cache_max_create_time = 5;
        cache_root = "/var/cache/cgit";
        cache_about_ttl = 15;
        cache_snapshot_ttl = 5;
        cache_repo_ttl = 5;
        cache_root_ttl = 5;
        cache_scanrc_ttl = 15;
        cache_dynamic_ttl = 5;
        cache_static_ttl = -1;
        cache_lock_fail = 200;
        cache_lock_retry = 30;
        cache_lock_timeout = 1000;
        client_io_idle_timeout = 20000;
        client_io_min_rate = 500;
        local_time = 0;
        pid_parent_dir = "/var/run";
        cache_max_files = 1048576; // ~1M files
        cache_min_ttl = 1,
        cache_max_ttl = 525600; // ~1 year in minutes;
    }
    parse_configfile(*this, cgit_config);
}

static int cgit_get_cache_max_ttl(const cgit::config &cfg) noexcept {
    return std::max({cfg.cache_root_ttl, cfg.cache_static_ttl, cfg.cache_dynamic_ttl, cfg.cache_scanrc_ttl,
                     cfg.cache_about_ttl, cfg.cache_snapshot_ttl, cfg.cache_repo_ttl, cfg.cache_min_ttl});
}
int cgit::get_cache_ttl(const config &cfg) noexcept { return std::min(cfg.cache_max_ttl, cgit_get_cache_max_ttl(cfg)); }

std::ostream& cgit::operator<<(std::ostream& os, const cgit::config& cfg)
{
    os << "cgit_config=" << cfg.cgit_config << "\n"
       << "log-level=" << cfg.log_level << "\n"
       << "cache-size=" << cfg.cache_size << " (" << std::bit_width(cfg.cache_size) << " hash-bits)\n"
       << "cache-root=" << cfg.cache_root << "\n"
       << "cache-max_create_time=" << cfg.cache_max_create_time << "\n"
       << "cache-ttl (minutes): " << get_cache_ttl(cfg)
       << " = min(" << cfg.cache_max_ttl << ", max(" << "min " << cfg.cache_min_ttl << ", root " << cfg.cache_root_ttl
       << ", static " << cfg.cache_static_ttl << ", dyn " << cfg.cache_dynamic_ttl << ", scan " << cfg.cache_scanrc_ttl
       << ", about " << cfg.cache_about_ttl << ", snap " << cfg.cache_snapshot_ttl << ", repo " << cfg.cache_repo_ttl << "))\n"
       << "cache-lock-fail=" << cfg.cache_lock_fail << "\n"
       << "cache-lock-retry=" << cfg.cache_lock_retry << "\n"
       << "cache-lock-timeout=" << cfg.cache_lock_timeout << "\n"
       << "client-io-idle-timeout=" << cfg.client_io_idle_timeout << "\n"
       << "client-io-min-rate=" << cfg.client_io_min_rate << "\n"
       << "pid-parent-dir=" << cfg.pid_parent_dir << "\n"
       << "cache-max-files=" << cfg.cache_max_files << "\n"
       << "cache-min-ttl=" << cfg.cache_min_ttl << "\n"
       << "cache-max-ttl=" << cfg.cache_max_ttl << "\n";
    return os;
}
