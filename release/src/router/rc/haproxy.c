/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 *
 * Copyright 2017 Eric Sauvageau.
 *
 */
#include <rc.h>
#include <shared.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>

#define HAPROXY_CONF "/etc/haproxy.cfg"

void start_haproxy(void) {
	if (config_haproxy() == 0)
		eval("/usr/sbin/haproxy", "-f", "/etc/haproxy.cfg");
}


void stop_haproxy(void) {
	if (pids("haproxy"))
		killall_tk("haproxy");
}


int config_haproxy(void) {
	FILE *fp;
	char *lan_ipaddr;

	if(f_exists(HAPROXY_CONF))
		unlink(HAPROXY_CONF);

	if ((fp = fopen(HAPROXY_CONF, "w")) == NULL)
		return -1;

	lan_ipaddr = nvram_safe_get("lan_ipaddr");

	fprintf(fp,"global\n"
			"tune.ssl.default-dh-param 2048\n"
			"maxconn 128\n"
			"uid 65534\ngid 65534\n"
			"daemon\n"

			"defaults\n"
			" timeout connect 5s\n"
			" timeout server 30s\n"

			"frontend https_frontend\n"
			" bind %s:%d ssl crt /etc/server.pem\n"
			" mode http\n"
			" option forwardfor\n"
			" option http-server-close\n"
			" option httpclose\n"

			" default_backend http_backend\n"

			"backend http_backend\n"
			" mode http\n"
			" server web %s:%d\n",

			lan_ipaddr, nvram_get_int("https_lanport"),
			lan_ipaddr, 80);

	append_custom_config("haproxy.cfg", fp);
	fclose(fp);

	use_custom_config("haproxy.cfg", HAPROXY_CONF);
	run_postconf("haproxy", HAPROXY_CONF);

	return 0;
}

