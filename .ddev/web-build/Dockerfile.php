# The PHP in DDEV's web image lags behind upstream patch releases, so upgrade
# the installed php8.2 packages to the latest from packages.sury.org (8.2.35+).
# Keep DDEV's modified config files (php-fpm.conf etc.) instead of prompting.
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --only-upgrade -o Dpkg::Options::="--force-confold" \
      $(dpkg-query -W -f='${db:Status-Abbrev} ${binary:Package}\n' 'php8.2*' | awk '/^ii/ {print $2}') && \
    rm -rf /var/lib/apt/lists/*
