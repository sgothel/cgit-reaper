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

#include <cstdio>
#include <cmath>
#include <cinttypes>

#include <jau/io/file_util.hpp>
#include <jau/debug.hpp>
#include <jau/enum_util.hpp>

#include <cgit.hpp>
#include "Version.hpp"
#include "jau/basic_types.hpp"
#include "jau/fraction_type.hpp"

using namespace jau::enums;
using namespace jau::int_literals;

int main(int argc, char *argv[])
{
    const cgit::config & cfg = cgit::config::get();

    std::string exe_name = "cgit-reaper";
    if (argc>0) {
        exe_name = jau::io::fs::basename(argv[0]);
    }
    bool verbose = false;
    bool dry_run = false;
    size_t max_files = cfg.cache_size;
    int ttl_min = cgit::get_cache_max_ttl(cfg);
    for (int i = 1; i < argc; ++i) {
        if (!strcmp("-v", argv[i]) || !strcmp("--verbose", argv[i])) {
            verbose=true;
        } else if (!strcmp("-n", argv[i]) || !strcmp("--dry-run", argv[i])) {
            dry_run=true;
        } else if(!strcmp("--ttl", argv[i]) && i+1<argc) {
            ttl_min = std::atoi(argv[i+1]);
            ++i;
        } else if(!strcmp("--files", argv[i]) && i+1<argc) {
            max_files = (size_t)std::strtoul(argv[i+1], nullptr, 10);
            ++i;
        } else if (!strcmp("-h", argv[i]) || !strcmp("--help", argv[i])) {
            jau_fprintf_ts(stdout, "%s [-h|--help] [-n|--dry-run] [--ttl <minutes>] [--files <number>]\n", exe_name);
            return 0;
        }
    }
    if (verbose) {
        jau_fprintf_ts(stdout, "%s: version %s\n", exe_name, cgitc_reaper::VERSION);
        std::cout << cfg << "\n";
    }

    size_t total_size = 0;
    size_t file_count = 0;
    size_t removed_files = 0;
    size_t removed_size = 0;
    size_t remaining_files = 0;
    size_t remaining_size = 0;
    const jau::fraction_timespec now = jau::getWallClockTime();
    const jau::fraction_timespec ttl((int64_t)ttl_min * 60_i64, 0);

    jau_fprintf_ts(stdout, "%s: ttl %.0f min, max-files %'zu (cfg %'zu) @ %s, dry-run %s\n",
        exe_name, ttl.to_double()/60.0, max_files, cfg.cache_size, cfg.cache_root, dry_run);

    if (!max_files) {
        return 0; // nothing to do
    }
    {
        // Pass-1: Delete all expired files (ttl)
        const jau::io::fs::path_visitor pv = [&](jau::io::fs::traverse_event tevt, const jau::io::fs::file_stats &stat, size_t, size_t idx, size_t) -> bool {
            if (!is_set(tevt, jau::io::fs::traverse_event::file)) {
                return true; // skip
            }
            ++file_count;
            total_size += stat.size();
            const jau::fraction_timespec &mt = stat.mtime();
            const jau::fraction_timespec age = now - mt;
            bool removed = false;
            if (stat.item().basename().ends_with(".lock")) {
                if (verbose) {
                    jau_fprintf(stdout, "- ignore %zu/%zu: path %s, mtime %s (age %.1f min), size %" PRIu64 "\n",
                                    removed_files+1, idx, stat.path(), mt.toISO8601String(true), age.to_double() / 60.0, stat.size());
                }
            } else if (age > ttl) {
                if (verbose) {
                    jau_fprintf(stdout, "- remove %zu/%zu: path %s, mtime %s (age %.1f min), size %" PRIu64 "\n",
                                    removed_files+1, idx, stat.path(), mt.toISO8601String(true), age.to_double() / 60.0, stat.size());
                }
                int res = dry_run ? 0 : ::unlink(stat.path().c_str());
                if (!res) {
                    removed=true;
                    ++removed_files;
                    removed_size += stat.size();
                } else {
                    jau_ERR_PRINT("remove failed: %s, age %.1f min, res %d",
                        stat.toString(), age.to_double() / 60.0, res); // NOLINT(bugprone-lambda-function-name)
                }
            }
            if (!removed) {
                ++remaining_files;
                remaining_size += stat.size();
            }
            return true;
        };
        jau::io::fs::visit(cfg.cache_root, jau::io::fs::traverse_options::none, pv);
        jau_fprintf_ts(stdout, "%s: pass-1: total files %'zu (size %.2f MB), expired files %'zu (size %.2f MB), remaining files %'zu (size %.2f MB)\n",
            exe_name,
            file_count, (double)total_size/1000000.0,
            removed_files, (double)removed_size/1000000.0,
            remaining_files, (double)remaining_size/1000000.0);
    }
    if (remaining_files > max_files) {
        // Pass-2: Delete the oldest files exceeding cache-size
        total_size = 0;
        file_count = 0;
        removed_files = 0;
        removed_size = 0;
        remaining_files = 0;
        remaining_size = 0;
        size_t to_be_deleted = remaining_files - max_files;
        const jau::io::fs::path_visitor pv = [&](jau::io::fs::traverse_event tevt, const jau::io::fs::file_stats &stat, size_t, size_t idx, size_t) -> bool {
            if (!is_set(tevt, jau::io::fs::traverse_event::file)) {
                return true; // skip
            }
            if (0 == to_be_deleted) {
                return false; // done
            }
            ++file_count;
            total_size += stat.size();
            const jau::fraction_timespec &mt = stat.mtime();
            const jau::fraction_timespec age = now - mt;
            bool removed = false;
            if (stat.item().basename().ends_with(".lock")) {
                if (verbose) {
                    jau_fprintf(stdout, "- ignore %zu/%zu: path %s, mtime %s (age %.1f min), size %" PRIu64 "\n",
                                    removed_files+1, idx, stat.path(), mt.toISO8601String(true), age.to_double() / 60.0, stat.size());
                }
            } else {
                if (verbose) {
                    jau_fprintf(stdout, "- remove %zu/%zu: path %s, mtime %s (age %.1f min), size %" PRIu64 "\n",
                                    removed_files+1, idx, stat.path(), mt.toISO8601String(true), age.to_double() / 60.0, stat.size());
                }
                int res = dry_run ? 0 : ::unlink(stat.path().c_str());
                if (!res) {
                    removed=true;
                    ++removed_files;
                    removed_size += stat.size();
                    --to_be_deleted;
                } else {
                    jau_ERR_PRINT("remove failed: %s, age %.1f min, res %d",
                        stat.toString(), age.to_double() / 60.0, res); // NOLINT(bugprone-lambda-function-name)
                }
            }
            if (!removed) {
                ++remaining_files;
                remaining_size += stat.size();
            }
            return true;
        };
        jau::io::fs::visit(cfg.cache_root, jau::io::fs::traverse_options::mtime_order, pv);
        jau_fprintf_ts(stdout, "%s: pass-2: total files %'zu (size %.2f MB), removed files %'zu (size %.2f MB), remaining files %'zu (size %.2f MB)\n",
            exe_name,
            file_count, (double)total_size/1000000.0,
            removed_files, (double)removed_size/1000000.0,
            remaining_files, (double)remaining_size/1000000.0);
    }
	return 0;
}
