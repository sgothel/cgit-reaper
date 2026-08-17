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

#ifndef CGIT_H
#define CGIT_H

#include <cstddef>
#include <string>

namespace cgit {

    enum log_level_t {
    	LOG_LVL_ERR=0, LOG_LVL_WARN=50, LOG_LVL_DBG=75, LOG_LVL_VERBOSE=100
    };

    class config {
      private:
        config(const char *default_filename) noexcept;
      public:
        /// Return singleton config instance, only the first call's default_filename is considered
        static const config& get(const char *default_filename = "/etc/cgitrc") noexcept {
            static config c(default_filename); // magic static
            return c;
        }
        std::string cgit_config;
    	int log_level; ///< defaults to zero
    	std::string cache_root;
    	size_t cache_size;
    	int cache_dynamic_ttl;
    	int cache_max_create_time;
    	int cache_repo_ttl;
    	int cache_root_ttl;
    	int cache_scanrc_ttl;
    	int cache_static_ttl;
    	int cache_about_ttl;
    	int cache_snapshot_ttl;
    	/* cache lock fail action as http-response code. 200 returns un-cached processed content, otherwise an error page of same code is served. Defaults to 200 (OK). */
    	int cache_lock_fail;
    	/* cache lock fail http-header `Retry-After` value in seconds, used if cache-lock-fail is not 200 (OK) and serves an error page. Default to 30s. */
    	int cache_lock_retry;
    	/* cache lock timeout in milliseconds to acquire the cache lock-file against concurrent processes. Defaults to 1000ms. */
    	int cache_lock_timeout;
    	/* idle timeout in milliseconds between sending/receiving chunks of the cached body to/from the client. Defaults to 20000ms. */
    	int client_io_idle_timeout;
    	/* minimum transfer rate in Bps for sending/receiving a full cached body to/from the client. Defaults to 500 Bps. */
    	int client_io_min_rate;
        int local_time;
        // extension: defaults to `/var/run`, for e.g. `/var/run/<exe>/<exe>.pid` lock file (singleton service)
        std::string pid_parent_dir;
    };

    int get_cache_max_ttl(const config &);
    std::ostream& operator<<(std::ostream& os, const config& p);
}

#endif /* CGIT_H */
